#include "stm32f0xx_conf.h"

#include "stm32f0xx_tim.h"

#define ARR_MAX 0xFFFF
#define PRESCALE_MAX 0xFFFF

typedef uint32_t status_t;

#define STATUS_SUCCESS 0
#define STATUS_ERROR 1

#define FAIL(st) (st != 0)

#define KHZ(val) (val * 1000)
#define MHZ(val) (val * 1000000)


void SysTick_Handler(void) {
    static uint32_t ii = 0;
    static uint16_t tick = 0;

    switch (tick++) {
        case 400:
            if (ii == 0) {
                pwm_ch_freq(MHZ(10), .9);
                ii = 1;

            } else {
                pwm_ch_freq(MHZ(10), .4);
                ii = 0;
            }
            tick = 0;
            break;
    }
}

void init_pins()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpioStructure;

    gpioStructure.GPIO_Mode = GPIO_Mode_AF;
    gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;

    gpioStructure.GPIO_Pin = GPIO_Pin_4;

    GPIO_Init(GPIOB, &gpioStructure);
}

/*  Initializes Timer 3 to have a period of pwm_freq and returns the period
 */
status_t init_timer(uint32_t pwm_freq, uint32_t* clk_freq, uint32_t* period)
{
    const uint32_t fclk = SystemCoreClock;

    uint32_t prescale = 0xdeadbeef;

    *period = ARR_MAX + 1;

    /* Find prescale value such that the pwm_freq can be acieved */
    /* Using: fpwm = (fclk / ( (ARR + 1) ( PSC + 1)) */
    while (*period > ARR_MAX) {
        prescale = prescale == 0xdeadbeef ? 0 : prescale + 1;
        *period = fclk / (pwm_freq / (prescale + 1));
    }

    if (prescale > ARR_MAX) {
        return STATUS_ERROR;
    }

    *clk_freq = fclk / (prescale + 1);


    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;

    timerInitStructure.TIM_Prescaler = prescale;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = *period;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM3, &timerInitStructure);
    TIM_Cmd(TIM3, ENABLE);

    return STATUS_SUCCESS;
}

/*  Initializes PWM for the given frequency / duty cycle on timer 3 channel 1
 *
 *  Timer 3 Channel 1 coresponds to PB4
 */
status_t init_pwm(uint32_t pwm_freq, float duty)
{
    status_t st;
    uint32_t clk_freq, period;

    init_pins();

    st = init_timer(pwm_freq, &clk_freq, &period);
    if (FAIL(st)) {
        return st;
    }

    TIM_OCInitTypeDef outputChannelInit = {0,};

    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;

    outputChannelInit.TIM_Pulse = period * duty;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM3, &outputChannelInit);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_1);

    return STATUS_SUCCESS;
}

void pwm_ch_freq(uint32_t pwm_freq, float duty) {
    uint32_t clk_freq, period;
    status_t st;

    st = init_timer(pwm_freq, &clk_freq, &period);
    if (FAIL(st)) {
        return;
    }

    TIM_OCInitTypeDef outputChannelInit = {0,};

    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;

    outputChannelInit.TIM_Pulse = period * duty;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM3, &outputChannelInit);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
}



int main(void)
{
    status_t st;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;  // enable the clock to GPIOC
    GPIOC->MODER = (1 << 16);

    st = init_pwm(733, .5);
    if (FAIL(st)){
        GPIOC->ODR = (1 << 8);
    }

    //SysTick_Config(SystemCoreClock/100);
    while(1);
}
