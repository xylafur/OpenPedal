/*
 *  Kesley's Circular Buffer (KCB)
 *
 *  A simple circular buffer implementation
 *
 *
 *  Below is a quick and dirty design explaination
 *
 *      read     write
 *     v        v
 *  -------------------------------
 *    |x |x |x |  |  |  |  |  |  |
 *  -------------------------------
 *     0  1  2  3  4  5  6  7  8
 *
 *  The buffer is implemented as a queue, so a call to the buffer read function
 *  will read the oldest element in the buffer.
 *
 *  The indexes are always determined by the positions of the read and write
 *  pointers as detailed by the below diagram
 *
 *              read     write
 *             v        v
 *  ---------------------------------------
 *   |  |  |  |x |x |x |  |  |  |  |  |  |
 *  ---------------------------------------
 *             0  1  2  3  4  5  6  7  8  9
 *
 */
#ifndef __BUFFER_HEADER__
#define __BUFFER_HEADER__

#include <stdint.h>
#include "core.h"

#define BUFFER_SIZE 128

typedef struct kcb_s {
    uint32_t read;
    uint32_t write;
    uint32_t size;

    /* Making size static for now.. maybe it can be a compile time option?
     * Optionally use the heap?
     */
    uint32_t buf [BUFFER_SIZE];
} kcb_t;

status_t kcb_init (kcb_t* buf, uint32_t size);

status_t kcb_empty (kcb_t *buf);
status_t kcb_full (kcb_t* buf);

status_t kcb_pop (kcb_t* buf, uint32_t* val);
status_t kcb_peek (kcb_t* buf, uint32_t *val);

status_t kcb_read (kcb_t* buf, uint32_t *val);
status_t kcb_read_idx (kcb_t* buf, uint32_t idx, uint32_t *val);
status_t kcb_read_many (kcb_t* buf, uint32_t start_idx, uint32_t count, uint32_t *val);

status_t kcb_write (kcb_t* buf, uint32_t val);

#endif
