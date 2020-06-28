#include "stm32f0xx_conf.h"

#include "core.h"
#include "generate.h"
#include "transform.h"

uint32_t do_change = 0;

void SysTick_Handler(void) {
    static uint32_t duty = 0;

    pwm_ch_duty(duty);

    if (duty != 255) {
        duty = 255;
    } else {
        duty = 0;
    }

    do_change = 1;
}

#define GENERATE_FREQUENCY (SystemCoreClock/100)


int main(void)
{
    status_t st;
    uint32_t newval;
    uint32_t max = 0;

    //init_usart();

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;  // enable the clock to GPIOC
    GPIOC->MODER = (1 << 16);
    SysTick_Config(54545);

    st = init_transform(GENERATE_FREQUENCY, TRANSFORM_CONSTANT);
    if (FAIL(st)){
        DEBUG();
    }

    max = get_max_value();

    st = init_pwm(SystemCoreClock/10);
    if (FAIL(st)){
        DEBUG();
    }


    while(1) {
    //    if (do_change) {
    //        do_change = 0;

    //        /* If there is nothing new in the buffer.. just return from the handler */
    //        if (FAIL(get_value(&newval))) {
    //            continue;
    //        }

    //        /* Convert new value into a form that the generate module can use */
    //        newval = amplitude_to_duty(newval, max);

    //        /* Turn on the LED if anything goes wrong */
    //        if (FAIL(pwm_ch_duty(newval))){
    //          GPIOC->ODR = (1 << 8);
    //        }
    //    }
    }
}
