#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"

#include "pwm.h"

#include "timer.h"

/******************************************************************************
 * Globals
 *****************************************************************************/

uint32_t global_period;

#define CCR_MAX 0xFFFF
#define MAX_DUTY 256

/******************************************************************************
 * Private Prototypes
 *****************************************************************************/
void init_pins(void);
status_t init_timer(uint32_t pwm_freq, uint32_t* period);

/******************************************************************************
 * Public
 *****************************************************************************/
/*
 *  Convert the given value into a range that the PWM module will accept.
 *
 *  val (IN) The value we are converting
 *  max (IN) The max value that val can take
 *
 *  We assume that the input val can range from [0, max], so we convert this
 *  into the range [0, PWM_MAX]
 */
uint32_t pwm_convert_value(uint32_t val, uint32_t max) {
    return val * MAX_DUTY / max;
}

/*  Initializes PWM for the given frequency / duty cycle on timer 3 channel 1
 *
 *  Timer 3 Channel 1 coresponds to PB4
 */
status_t pwm_init(uint32_t pwm_freq)
{
    status_t st;
    uint32_t period;

    init_pins();

    st = init_timer(pwm_freq, &period);
    if (FAIL(st)) {
        return st;
    }

    TIM_OCInitTypeDef channel = {
        .TIM_OCMode = TIM_OCMode_PWM1,
        /* Default to a duty of 50% */
        .TIM_Pulse = period * .5,
        .TIM_OutputState = TIM_OutputState_Enable,
        .TIM_OCPolarity = TIM_OCPolarity_High,
    };

    TIM_OC1Init(TIM3, &channel);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_1);

    global_period = period;

    return STATUS_SUCCESS;
}

status_t pwm_ch_duty(uint32_t duty) {
    uint32_t period = global_period;

    uint32_t new_duty = period * (((float)duty)/MAX_DUTY);
    if (new_duty > CCR_MAX) {
        return STATUS_ERROR;
    }

    TIM_SetCompare1(TIM3, new_duty);

    return STATUS_SUCCESS;
}

/******************************************************************************
 * Private
 *****************************************************************************/



/*
 *  Initialize the GPIO pins for the output.  Set them up for their alternate
 *  function (PWM)
 */
void init_pins(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpioStructure;

    gpioStructure.GPIO_Mode = GPIO_Mode_AF;
    gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;

    gpioStructure.GPIO_Pin = GPIO_Pin_4;

    GPIO_Init(GPIOB, &gpioStructure);
}

/*  Inits Timer 3 to have the particular frequency and period
 */
status_t init_timer(uint32_t pwm_freq, uint32_t* period) {
    status_t st;
    uint32_t prescale = 0;

    st = timer_calc_vals(pwm_freq, &prescale, period, SystemCoreClock);
    if (FAIL(st)) {
        return st;
    }

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure = {
        .TIM_Prescaler = prescale,
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = *period,
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,
    };

    TIM_TimeBaseInit(TIM3, &timerInitStructure);
    TIM_Cmd(TIM3, ENABLE);

    return STATUS_SUCCESS;
}
