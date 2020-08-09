#ifndef __OUTPUT_HEADER__
#define __OUTPUT_HEADER__

#include "core.h"

typedef enum {
    OUTPUT_PWM,
    OUTPUT_INVALID,
} output_t;

#define PWM_FREQ (SystemCoreClock/10)

status_t output_buffer_write(uint32_t val, uint32_t max);
status_t output_init(output_t type, uint32_t update_frequency);
void output_print_info(uint32_t verbose);

#endif
