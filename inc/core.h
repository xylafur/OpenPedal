#ifndef __CORE_HEADER__
#define __CORE_HEADER__

typedef uint32_t status_t;

#define STATUS_SUCCESS 0
#define STATUS_ERROR 1

#define FAIL(st) (st != 0)

#define KHZ(val) (val * 1000)
#define MHZ(val) (val * 1000000)

#endif
