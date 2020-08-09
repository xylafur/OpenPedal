/*
 * TODO Is this really modular enough to include in kesl?
 *      I think this will only work for STM32F0 (maybe other STM32 processors)
 *
 */

#include "timer.h"

#define ARR_MAX 0xFFFF
#define PRESCALE_MAX 0xFFFF

/*
 *  Determine the prescale and period values to achieve the given timer frequency
 *
 *  timer_freq  (IN)  The desired frequency
 *  prescale    (OUT) The value to use for the PWM prescale
 *  period      (OUT) The value to use for the PWM period
 *
 *  returns status_t, was this pwm frequency achievable?
 */
status_t timer_calc_vals(uint32_t timer_freq, uint32_t* prescale,
                         uint32_t* period, uint32_t baseclock) {
    *prescale = 0xdeadbeef;

    /* Find prescale value such that the timer_freq can be acieved */
    /* Using: fpwm = (baseclock / ( (ARR + 1) ( PSC + 1)) */
    do {
        *prescale = *prescale == 0xdeadbeef ? 0 : *prescale + 1;
        *period = baseclock / (timer_freq / (*prescale + 1)) - 1;
    } while (*period > ARR_MAX);

    if (*prescale >= PRESCALE_MAX) {
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}
