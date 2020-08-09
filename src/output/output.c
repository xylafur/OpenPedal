/*
 *  This module coresponds to the output signal that we generate and send to
 *  the amplifier.
 *
 *  The output module is designed to run in its own thread of execution.  The
 *  ouptut thread is triggered via a timer.
 *
 *  The output module has a buffer of output values.  Whenever the timer trips
 *  the output thread will check the buffer.  If the buffer has a value
 *  available, the output module will take the oldest value from the buffer and
 *  set the physical output voltage to that level.
 *
 *  The values in the beffer are expected to be converted into a usable format.
 *  This means that the output thread can simply take the value from the buffer
 *  and load it into the hardware module that generates the output voltage.
 *
 *  For instance, for PWM, the values in thebuffer should be duty cycle values.
 *  They should always be between the min and max duty cycle values.
 */
#include "stm32f0xx_conf.h"

#include "pwm.h"
#include "output.h"

#include "timer.h"
#include "core.h"
#include "logger.h"
#include "kcb.h"

#include <stdio.h>

#define OUTPUT_DEBUG_LEVEL 1

/******************************************************************************
 * Globals
 *****************************************************************************/
uint32_t output_inited = 0;
output_t output_type = OUTPUT_INVALID;
kcb_t output_buf;

/* We may not want to print if this happens in the future, instead just set a
 * bit.  This macro gives us that option */
#if OUTPUT_DEBUG_LEVEL == 2
uint32_t output_buffer_full = 0;
uint32_t output_buffer_empty = 0;
#endif

/******************************************************************************
 * Private Functions
 *****************************************************************************/
void TIM6_DAC_IRQHandler(void) {
    uint32_t output_value;
    status_t st;

    if (SUCCESS(kcb_read(&output_buf, &output_value))) {
        pwm_ch_duty(output_value);
    }

    TIM_ClearFlag(TIM6, 0x1);
}

static status_t init_interrupt_timer(uint32_t clock_frequency) {
    status_t st = STATUS_SUCCESS;
    uint32_t period = 0, prescale = 0;

    st = timer_calc_vals(clock_frequency, &prescale, &period, SystemCoreClock);
    if (FAIL(st)) {
        return st;
    }

    /* Enable the bus */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure = {
        .TIM_Prescaler = prescale,
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = period,
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,
    };

    /* Setup the timer settings (really just the period) */
    TIM_TimeBaseInit(TIM6, &timerInitStructure);

    /* Enable Interrupts from the timer's perspective */
    TIM_ITConfig(TIM6, 0x1, ENABLE);

    /* Enable the timer */
    TIM_Cmd(TIM6, ENABLE);

    /* Enable the timer6 interrupt in the NVIC */
    NVIC_EnableIRQ(TIM6_DAC_IRQn);

    return st;
}

/*  Intialize the timer so that the output buffer is polled with the provided
 *  frequency
 */
static status_t output_init_thread(uint32_t update_frequency) {
    switch (output_type) {
        case OUTPUT_PWM:
            /* TODO: Is this a good frequency?? */
            printf("Initting output pwm timer\n");
            init_interrupt_timer(update_frequency);

            break;
        default:
            return STATUS_ERROR;
    }
    return STATUS_SUCCESS;
}

/*
 *  Set the output for whichever output type.
 *
 *  val (IN) The value to update the hardware module that generates the output
 *           voltage with
 *
 *  returns status_t, success if the output module was successfully updated
 *  with the given value, error otherwise
 *
 *  For PWM, the duty cycle will be updated to the value 'val'.  This value is
 *  expected to be within the range of acceptable PWM duty cycle values.
 */
static status_t output_set_output(uint32_t val) {
    if (output_type == OUTPUT_PWM) {
        return pwm_ch_duty(val);
    }

    log_warn("Invalid output type (%d) for setting output\n", output_type);
    return STATUS_ERROR;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/
/*
 *  Take in a given value, convert it into a format the correct module can use
 *
 *  val (IN) The value we are converting and adding to the output buffer
 *  max (IN) The max value that val can take
 *
 *  returns status_t, success if we were able to write the value into the
 *  buffer, resource unavaliable if the buffer was full and error otherwise
 *
 */
status_t output_buffer_write(uint32_t val, uint32_t max) {
    uint32_t converted;
    status_t st;

    if (!output_inited) {
        log_warn("Output buffer not initialized\n");
        return STATUS_ERROR;
    }

    if (output_type == OUTPUT_PWM) {
        converted = pwm_convert_value(val, max);
    } else {
        log_warn("Output type (%d) not valid!\n", output_type);
        return STATUS_ERROR;
    }

    st = kcb_write(&output_buf, converted);
    if (FAIL(st)) {
#if OUTPUT_DEBUG_LEVEL == 1
        log_warn("Output buffer full!\n");
#elif OUTPUT_DEBUG_LEVEL == 2
        output_buffer_full = 1;
#endif
        return STATUS_RESOURCE_UNAVALIABLE;
    }

    return STATUS_SUCCESS;
}

status_t output_thread() {
    status_t st;
    uint32_t output_value;

    st = kcb_read(&output_buf, &output_value);

    if (FAIL(st)) {
#if OUTPUT_DEBUG_LEVEL == 1
        log_warn("Output buffer empty!\n");
#elif OUTPUT_DEBUG_LEVEL == 2
        output_buffer_empty = 1;
#endif
        return STATUS_RESOURCE_UNAVALIABLE;
    }

    st = output_set_output(output_value);
    if (FAIL(st)) {
        log_warn("Unable to set output in output thread\n");
    }

    return st;
}

/*
 *  Initializes the output for the specifed output type.
 *
 *  type             (IN) The output type (what method are we using to generate
 *                                         the correct output voltage?)
 *  update_frequency (IN) How often should we poll the ouput buffer and update
 *                        the output values?
 *  returns status_t, did we initialize everything properly
 */
status_t output_init(output_t type, uint32_t update_frequency) {
    status_t st;

    if (type == OUTPUT_PWM) {
        st = pwm_init(PWM_FREQ);
    } else {
        log_err("Invalid output type %d was passed into %s\n", type, __func__);

        return STATUS_ERROR;
    }

    if (FAIL(st)) {
        log_err("Failed initializing output module\n");
        return st;
    }

    output_type = type;

    st = output_init_thread(update_frequency);
    if (FAIL(st)) {
        log_err("Fail intializing the output thread\n");
        return st;
    }

    st = kcb_init(&output_buf, BUFFER_SIZE);
    if (FAIL(st)) {
        log_err("Fail intializing the output buffer\n");
        return st;
    }

    output_inited = 1;

    return STATUS_SUCCESS;
}

/*
 *  Prints information about the output module
 */
void output_print_info(uint32_t verbose) {
    printf("Output type: %d\n", output_type);
    printf("Output Initialized %lu\n", output_inited);

#if OUTPUT_DEBUG_LEVEL == 2
    printf("Buffer has gotten full %lu\n", output_buffer_full);
    printf("Buffer has been read while empty %lu\n", output_buffer_empty);
#endif

    kcb_print(&output_buf, verbose);
}
