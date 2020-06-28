#ifndef __TRANSFORM_HEADER__
#define __TRANSFORM_HEADER__

#include "core.h"

typedef enum {
    TRANSFORM_CONSTANT,
    TRANSFORM_SAMPLE,
} transform_t;

status_t init_transform(uint32_t generate_frequency, transform_t transform_type);

status_t read_buffer(uint32_t* val);
status_t write_buffer(uint32_t val);
status_t get_value(uint32_t* val);
uint32_t get_max_value();

#endif
