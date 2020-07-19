#include <stdio.h>

#include "stm32f0xx_conf.h"

#include "core.h"
#include "pwm.h"
#include "transform.h"
#include "cli.h"

static uint32_t do_change = 0;

#define NUM_SAMPLES 128
#define SYSTICK_FREQ (NUM_SAMPLES * 440)
#define GENERATE_FREQUENCY (SystemCoreClock/100)

// TODO: Going to need to enter critical section for ADC and systick
void SysTick_Handler(void) {
    do_change = 1;
}

int main(void)
{
    status_t st;
    uint32_t val = 0, ii = 0;
    uint32_t systick_clicks = SystemCoreClock / SYSTICK_FREQ;

    init_usart();

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;  // enable the clock to GPIOC
    GPIOC->MODER = (1 << 16);

    SysTick_Config(systick_clicks);

    st = init_transform(SYSTICK_FREQ, TRANSFORM_CONSTANT);
    if (FAIL(st)){
        DEBUG();
    }

    st = pwm_init(SystemCoreClock/10);
    if (FAIL(st)){
        DEBUG();
    }

    while(1) {
        if (!((ii++)%2)) {
            continue;
        }

        if (do_change) {
            if (SUCCESS(get_value(&val))) {
                pwm_ch_duty(val);
            }
            do_change = 0;
        }
    }
}
