#include "stm32f0xx_conf.h"

#include "core.h"
#include "generate.h"
#include "transform.h"

static uint32_t do_change = 0;

#define NUM_SAMPLES 128
//#define NUM_SAMPLES 2

uint32_t SIN_TABLE [NUM_SAMPLES] = {
    128, 134, 140, 146, 152, 159, 165, 171, 176, 182, 188, 193, 199, 204, 209,
    213, 218, 222, 226, 230, 234, 237, 240, 243, 246, 248, 250, 252, 253, 254,
    255, 255, 256, 255, 255, 254, 253, 252, 250, 248, 246, 243, 240, 237, 234,
    230, 226, 222, 218, 213, 209, 204, 199, 193, 188, 182, 176, 171, 165, 159,
    152, 146, 140, 134, 128, 121, 115, 109, 103, 96, 90, 84, 79, 73, 67, 62,
    56, 51, 46, 42, 37, 33, 29, 25, 21, 18, 15, 12, 9, 7, 5, 3, 2, 1, 0, 0, 0,
    0, 0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 21, 25, 29, 33, 37, 42, 46, 51, 56, 62,
    67, 73, 79, 84, 90, 96, 103, 109, 115, 121
};

uint32_t SIN_MAX = 512;

void SysTick_Handler(void) {
    if (!do_change) {
        do_change = 1;
    }

    do_change = 1;
}

#define GENERATE_FREQUENCY (SystemCoreClock/100)

int main(void)
{
    status_t st;
    uint32_t newval;
    uint32_t max = 0;
    uint32_t time = 0;

    uint32_t systick_clicks = SystemCoreClock / (NUM_SAMPLES * 440);

    //init_usart();

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;  // enable the clock to GPIOC
    GPIOC->MODER = (1 << 16);

    SysTick_Config(systick_clicks);

    st = init_transform(GENERATE_FREQUENCY, TRANSFORM_CONSTANT);
    if (FAIL(st)){
        DEBUG();
    }

    max = get_max_value();

    st = init_pwm(SystemCoreClock/10);
    if (FAIL(st)){
        DEBUG();
    }

    uint32_t ii = 0;

    while(1) {
        // Constant polling of do_change causes execution to get stuck.. wait a
        // few cycles before checking for update
        if (!((ii++)%3)) {
            continue;
        }

        if (do_change) {
            pwm_ch_duty(SIN_TABLE[(time++)%NUM_SAMPLES]);
            do_change = 0;
        }
    }
}
