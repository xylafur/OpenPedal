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
#include <string.h>
#include <stdio.h>

#include "kcb.h"

/*
 *  Initializes the given KCB
 */
status_t kcb_init (kcb_t *buf, uint32_t size) {
    if (!buf) {
        return STATUS_ERROR;
    }

    buf->read = 0;
    buf->write= 0;
    buf->size = BUFFER_SIZE;

    memset(buf->buf, 0, size);

    return STATUS_SUCCESS;
}

/*
 * Returns STATUS_SUCCESS if the buffer is empty.  STATUS_ERROR otherwise
 */
status_t kcb_empty (kcb_t* buf) {
    return buf->read == buf->write ? STATUS_SUCCESS : STATUS_ERROR;
}

/*
 * Returns STATUS_SUCCESS if the buffer is full.  STATUS_ERROR otherwise
 */
status_t kcb_full (kcb_t* buf) {
    return ((buf->write +1) % buf->size) == buf->read ? STATUS_SUCCESS : STATUS_ERROR;
}

/*
 * Reads a value from the buffer.  Does not Increment the read index.
 *
 * val (OUT) The value read from the buffer
 *
 * Returns STATUS_SUCCESS if the entry was read without error, STATUS_ERROR otherwise
 */
status_t kcb_peek (kcb_t* buf, uint32_t *val) {
    if (kcb_empty(buf)) {
        return STATUS_ERROR;
    }

    *val = buf->buf[buf->read];

    return STATUS_SUCCESS;
}

/*
 * Reads a value from the buffer.  Increments the read index.
 *
 * val (OUT) The value read from the buffer
 *
 * Returns STATUS_SUCCESS if the entry was read without error, STATUS_ERROR otherwise
 */
status_t kcb_read (kcb_t* buf, uint32_t *val) {
    status_t st = kcb_peek(buf, val);

    if (SUCCESS(st)) {
        buf->read = (buf->read + 1) % buf->size;
    }

    return st;
}

/*
 * Returns STATUS_SUCCESS if the index to be read is valid, STATUS_ERROR otherwise
 */
static status_t kcb_idx_within_bounds (kcb_t* buf, uint32_t idx) {
    if (kcb_empty(buf)) {
        return STATUS_ERROR;
    }

    if (((buf->read + idx) % buf->size) < buf->write) {
        return STATUS_SUCCESS;
    }

    return STATUS_ERROR;
}

/*
 * Reads a value from the buffer at the given buffer index.
 *
 * val (OUT) The value read from the buffer
 *
 * Returns STATUS_SUCCESS if the index to be read was valid, STATUS_ERROR otherwise
 */
status_t kcb_read_idx (kcb_t* buf, uint32_t idx, uint32_t *val) {
    if (FAIL(kcb_idx_within_bounds(buf, idx))) {
        return STATUS_ERROR;
    }

    *val = buf->buf[buf->read + idx];
    return STATUS_SUCCESS;
}

/*
 * Reads multiple values from the buffer starting at the given buffer index.
 *
 * val (OUT) The value read from the buffer
 *
 * Returns STATUS_SUCCESS if the index to be read was valid and the count was
 * valid (within the bounds of the buffer), STATUS_ERROR otherwise
 */
status_t kcb_read_many (kcb_t* buf, uint32_t start_idx, uint32_t count, uint32_t *val) {
    //uint32_t to_read;
    if (FAIL(kcb_idx_within_bounds(buf, start_idx))) {
        return STATUS_ERROR;
    }

    if (FAIL(kcb_idx_within_bounds(buf, start_idx + count))) {
        return STATUS_ERROR;
    }

    if (start_idx + count >= buf->size) {
        return STATUS_ERROR;
    }

    if (buf->read + start_idx + count > buf->size){
        if (start_idx >= buf->size - buf->read) {
            /* Our read is completley from the beginning of the physical array */
            memcpy(val,
                   buf->buf + (buf->read + start_idx) % buf->size,
                   count);
        } else {
            /* Our read wraps at the end and the begining */

            /* Read size - read pointer entries from end of buffer */
            memcpy(val,
                   buf->buf + buf->read + start_idx,
                   buf->size - buf->read);

            /* Read the remaining number of entries from the begining of the
             * buffer */
            memcpy(val + buf->size - buf->read,
                   buf->buf,
                   count - buf->size - buf->read);
        }

    } else /* this is a very simple read */ {
        memcpy(val, &buf->buf[buf->read+start_idx], count);
    }

    return STATUS_ERROR;
}

/*
 * Returns the most recently added value to the buffer.  Also removes this
 * value from the buffer.  Similar to a stack pop.
 *
 * val (OUT) The value read
 *
 * returns STATUS_SUCCESS if we were able to pop the value, STATUS_ERROR
 * otherwise
 */
status_t kcb_pop (kcb_t* buf, uint32_t* val) {
    if (kcb_empty(buf)) {
        return STATUS_ERROR;
    }

    buf->write = (buf->write - 1) % buf->size;

    *val = buf->buf[buf->write];

    return STATUS_SUCCESS;
}

/*
 * Writes the specified value into the buffer if there is space.
 *
 * returns STATUS_SUCCESS if the value was added into the buffer, STATUS_ERROR
 * otherwise
 */
status_t kcb_write (kcb_t* buf, uint32_t val) {
    if FAIL(kcb_full (buf)) {
        return STATUS_ERROR;
    }

    buf->buf[buf->write] = val;

    buf->write = (buf->write + 1) % buf->size;

    return STATUS_SUCCESS;
}


/*
 *  Print out information about a KCB.
 *
 *  verbose (IN) Print out all of the entries?
 */
void kcb_print(kcb_t* buf, uint32_t verbose) {
    uint32_t read, jj;
    printf("Read Pointer: %lu\n", buf->read);
    printf("Write Pointer: %lu\n", buf->write);
    printf("Buffer Size: %lu\n", buf->size);

    if (verbose) {
        read = buf->read;
        while (read < buf->write) {
            #define VALS_PER_LINE 5
            for (jj = 0; jj < VALS_PER_LINE; jj++) {
                printf("%lu: %lu ", jj, buf->buf[read++]);
                if (read >= buf->write) {
                    break;
                }
            }
            #undef VALS_PER_LINE
            printf("\n");
        }

    }
}
