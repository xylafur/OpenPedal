#include "stm32f0xx_conf.h"
#include "core.h"
#include "generate.h"


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
