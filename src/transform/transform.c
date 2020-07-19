#include "transform.h"

#include <math.h>

/******************************************************************************
 * Globals
 *****************************************************************************/

#define BUFFER_DEPTH 128

uint32_t buffer [BUFFER_DEPTH];
uint32_t buffer_read, buffer_write;

transform_t transform_type;

/* The frequency at which the generation module will ask the transform module
 * for a new value */
uint32_t generation_frequency;

uint32_t ticks_per_440;
uint32_t ticks_per_update;

//(sin(2 * pi * n / 128) + 1) * 128 for n = [0..127]
uint32_t SIN_TABLE [BUFFER_DEPTH] = {
    128, 134, 140, 146, 152, 159, 165, 171, 176, 182, 188, 193, 199, 204, 209,
    213, 218, 222, 226, 230, 234, 237, 240, 243, 246, 248, 250, 252, 253, 254,
    255, 255, 256, 255, 255, 254, 253, 252, 250, 248, 246, 243, 240, 237, 234,
    230, 226, 222, 218, 213, 209, 204, 199, 193, 188, 182, 176, 171, 165, 159,
    152, 146, 140, 134, 128, 121, 115, 109, 103, 96, 90, 84, 79, 73, 67, 62,
    56, 51, 46, 42, 37, 33, 29, 25, 21, 18, 15, 12, 9, 7, 5, 3, 2, 1, 0, 0, 0,
    0, 0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 21, 25, 29, 33, 37, 42, 46, 51, 56, 62,
    67, 73, 79, 84, 90, 96, 103, 109, 115, 121
};

/******************************************************************************
 * Private Prototypes
 *****************************************************************************/
status_t check_buffer();
status_t buffer_full();
status_t get_constant_value(uint32_t* val);

/******************************************************************************
 * Public
 *****************************************************************************/
status_t init_transform(uint32_t generate_frequency, transform_t _transform_type) {
    buffer_read = 0;
    buffer_write = 0;

    transform_type = _transform_type;
    generation_frequency = generate_frequency;

    ticks_per_440 = generate_frequency / 440;
    ticks_per_update = ticks_per_440 / 128;

    return STATUS_SUCCESS;
}

status_t get_value(uint32_t* val) {
    if (transform_type == TRANSFORM_CONSTANT) {
        return get_constant_value(val);
    }

    /* Other transform types are not yet supported.. */
    DEBUG();
    return STATUS_SUCCESS;
}

status_t read_buffer(uint32_t* val) {
    if (FAIL(check_buffer())) {
        return STATUS_ERROR;
    }

    if (val == 0) {
        return STATUS_ERROR;
    }

    *val = buffer[buffer_read];
    buffer_read = (buffer_read + 1) % BUFFER_DEPTH;

    return STATUS_SUCCESS;
}

status_t write_buffer(uint32_t val) {
    if (SUCCESS(buffer_full())) {
        return STATUS_ERROR;
    }

    buffer[buffer_write] = val;
    buffer_write = (buffer_write + 1) % BUFFER_DEPTH;

    return STATUS_SUCCESS;
}

uint32_t get_max_value() {
    if (transform_type == TRANSFORM_CONSTANT) {
        return 256;
    } else {
        /* TODO: Max value that ADC can give back */
        return 0;
    }
}

/******************************************************************************
 * Private
 *****************************************************************************/
/*  Is there room to put something into the buffer
 */
status_t check_buffer() {
    return buffer_read != buffer_write ? STATUS_SUCCESS : STATUS_ERROR;
}

/*  Return's STATUS_SUCCESS if the buffer is full, STATUS_ERROR otherwise
 */
status_t buffer_full() {
    return ((buffer_write + 1) % BUFFER_DEPTH) == buffer_read ? STATUS_SUCCESS : STATUS_ERROR;
}

status_t get_constant_value(uint32_t* val) {
    static uint32_t ii = 0;
    static uint32_t jj = 0;

    if ((++ii) == ticks_per_update) {
        *val = SIN_TABLE[(jj++) % BUFFER_DEPTH];

        ii = 0;
        return STATUS_SUCCESS;
    }

    return STATUS_ERROR;
}
