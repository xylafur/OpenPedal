#ifndef __GENERATE_HEADER__
#define __GENERATE_HEADER__

#include "core.h"

status_t pwm_ch_duty(uint32_t duty);
status_t init_pwm(uint32_t pwm_freq);
uint32_t amplitude_to_duty(uint32_t val, uint32_t max);

#endif
