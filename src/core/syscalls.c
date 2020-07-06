#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f0xx_usart.h"


/* TODO: Do I really need errno? */
#undef errno
extern int errno;

int _fstat (int fd, struct stat *pStat) {
    pStat->st_mode = S_IFCHR;
   return 0;
}

int _close(int file) {
   return -1;
}

int _write (int fd, char *ptr, int len) {
    int n;
    for (n = 0; n < len; n++) {
        while ((USART2->ISR & USART_FLAG_TC) == (uint16_t)RESET) {}
        USART2->TDR = (*ptr++ & (uint16_t)0x01FF);
    }

    return len;
}

int _isatty (int fd) {
   return 1;
}

int _lseek(int a, int b, int c) {
   return -1;
}

int _read (int fd, char *ptr, int len) {
    int n;
    int num = 0;
    for (n = 0; n < len; n++) {
        while ((USART2->ISR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
        char c = (char)(USART2->RDR & (uint16_t)0x01FF);
        *ptr++ = c;
        num++;
    }
    return num;
}

caddr_t _sbrk(int incr) {
   extern char _ebss;
   static char *heap_end;
   char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_ebss;
    }

    prev_heap_end = heap_end;

    char * stack = (char*) __get_MSP();
    if (heap_end + incr >  stack) {
        _write (STDERR_FILENO, "Heap and stack collision\n", 25);
        errno = ENOMEM;
        return  (caddr_t) -1;
    }

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

int _getpid() {
    return 1;
}

void _exit(int status) {
    _write(1, "exit", 4);
    while (1) {}
}
