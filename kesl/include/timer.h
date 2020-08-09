#ifndef __TIMER_HEADER__
#define __TIMER_HEADER__

#include "core.h"
#include <stdint.h>

status_t timer_calc_vals(uint32_t timer_freq, uint32_t* prescale,
                         uint32_t* period, uint32_t baseclock);

#endif
