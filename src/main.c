#include "stm32f0xx_conf.h"

#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"
#include <stdio.h>

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


#define ARR_MAX 0xFFFF
#define PRESCALE_MAX 0xFFFF
#define CCR_MAX 0xFFFF

typedef uint32_t status_t;

#define STATUS_SUCCESS 0
#define STATUS_ERROR 1

#define FAIL(st) (st != 0)

#define KHZ(val) (val * 1000)
#define MHZ(val) (val * 1000000)

status_t pwm_ch_duty(uint32_t duty);

void SysTick_Handler(void) {
    static uint32_t ii = 0;
    static uint16_t tick = 0;
    static status_t st = STATUS_SUCCESS;

    /* If anything ever went wrong, just stop processing */
    if (FAIL(st)) {
        return;
    }

    switch (tick++) {
        case 100:
            if (ii == 0) {
                st = pwm_ch_duty(250);
                ii = 1;

            } else {
                st = pwm_ch_duty(127);
                ii = 0;
            }
            tick = 0;
            break;
    }

    /* Turn on the LED if anything goes wrong */
    if (FAIL(st)){
        GPIOC->ODR = (1 << 8);
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

status_t calc_vals(uint32_t pwm_freq, uint32_t* prescale, uint32_t* period) {
    const uint32_t fclk = SystemCoreClock;
    *prescale = 0xdeadbeef;

    /* Find prescale value such that the pwm_freq can be acieved */
    /* Using: fpwm = (fclk / ( (ARR + 1) ( PSC + 1)) */
    while (*period > ARR_MAX) {
        *prescale = *prescale == 0xdeadbeef ? 0 : *prescale + 1;
        *period = fclk / (pwm_freq / (*prescale + 1)) - 1;
    }

    if (*prescale >= PRESCALE_MAX) {
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

/*  Inits Timer 3 to have the particular frequency and period
 */
status_t init_timer(uint32_t pwm_freq, uint32_t* period) {
    status_t st;
    uint32_t prescale;

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

uint32_t global_period;

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

void init_usart() {
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);

    /* Output a message on Hyperterminal using printf function */
    printf("\n\rUSART Printf Example: retarget the C library printf function to the USART\n\r");

    /* Loop until the end of transmission */
    /* The software must wait until TC=1. The TC flag remains cleared during all data
       transfers and it is set by hardware at the last frames end of transmission*/
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {}
}


int main(void)
{
    status_t st;

    //init_usart();

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;  // enable the clock to GPIOC
    GPIOC->MODER = (1 << 16);
    SysTick_Config(SystemCoreClock/100);

    st = init_pwm(1000);
    if (FAIL(st)){
        GPIOC->ODR = (1 << 8);
    }

    while(1);
}
