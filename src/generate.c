#include "stm32f0xx_tim.h"
#include "generate.h"

/******************************************************************************
 * Globals
 *****************************************************************************/

uint32_t global_period;

#define ARR_MAX 0xFFFF
#define PRESCALE_MAX 0xFFFF
#define CCR_MAX 0xFFFF

/******************************************************************************
 * Private Prototypes
 *****************************************************************************/
void init_pins(void);
status_t init_timer(uint32_t pwm_freq, uint32_t* period);

/******************************************************************************
 * Public
 *****************************************************************************/

/*  Initializes PWM for the given frequency / duty cycle on timer 3 channel 1
 *
 *  Timer 3 Channel 1 coresponds to PB4
 */
status_t init_pwm(uint32_t pwm_freq)
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

#define MAX_DUTY 256
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
static status_t calc_vals(uint32_t pwm_freq, uint32_t* prescale, uint32_t* period) {
    const uint32_t fclk = SystemCoreClock;
    *prescale = 0xdeadbeef;

    /* Find prescale value such that the pwm_freq can be acieved */
    /* Using: fpwm = (fclk / ( (ARR + 1) ( PSC + 1)) */
    do {
        *prescale = *prescale == 0xdeadbeef ? 0 : *prescale + 1;
        *period = fclk / (pwm_freq / (*prescale + 1)) - 1;
    } while (*period > ARR_MAX);

    if (*prescale >= PRESCALE_MAX) {
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

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

    st = calc_vals(pwm_freq, &prescale, period);
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
