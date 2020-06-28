#include "transform.h"

#include "math.h"

/******************************************************************************
 * Globals
 *****************************************************************************/

#define BUFFER_DEPTH 128


#define SIN_FREQUENCY 440
#define SIN_SAMPLE_FREQUENCY (SIN_FREQUENCY/BUFFER_DEPTH)

uint32_t buffer [BUFFER_DEPTH];
uint32_t buffer_read, buffer_write;

transform_t transform_type;

/* The frequency at which the generation module will ask the transform module
 * for a new value */
uint32_t generation_frequency;

uint32_t ticks_per_440;
uint32_t ticks_per_update;

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
        return 2;
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

    //if (ii++ == ticks_per_update) {
        *val = (uint32_t)sinf(2 * 3.14 * ((float)jj / BUFFER_DEPTH)) + 1;

        ii = 0;
        jj = (jj + 1) % BUFFER_DEPTH;
        return STATUS_SUCCESS;
    //}

    return STATUS_ERROR;
}
