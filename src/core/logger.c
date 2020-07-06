#include "core.h"
#include <stdio.h>


uint32_t log_level = LOG_LEVEL_ERROR;
uint32_t log_intelligent = LOG_INTELLIGENT;

void set_log_level(uint32_t level) {
    printf("Setting log level to %lu\n\r", level);
    log_level = level;
}

void set_log_intelligent(uint32_t intelligent) {
    printf("Setting log intelligent to: %lu\n\r", intelligent);
    log_intelligent = intelligent;
}

static void __log(char * msg, uint32_t level) {
    if (level <= log_level) {
        switch (level) {
        case LOG_LEVEL_ERROR:
            printf("error> ");
        case LOG_LEVEL_WARN:
            printf("warn> ");
        case LOG_LEVEL_INFO:
            printf("info> ");
        }

        printf("%s", msg);
    }
}

//#define MAX_LOG_MSG 256
//static void _log(char *fmt, uint32_t level, va_list ap) {
//    char* msg;
//
//    //msg = malloc(MAX_LOG_MSG);
//
//    vsscanf(msg, fmt, ap);
//    __log(msg, level);
//}

void log_info(char * msg) {
    //va_list ap;
    //va_start(ap, msg);

    //_log(msg, LOG_LEVEL_INFO, ap);
    __log(msg, LOG_LEVEL_INFO);

    //va_end(ap);
}
