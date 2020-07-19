#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "core.h"
#include "logger.h"

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2

#define LOG_INTELLIGENT 0
#define LOG_UNINTELLIGENT 1


uint32_t log_level = LOG_LEVEL_ERROR;
uint32_t log_intelligent = LOG_INTELLIGENT;

void set_log_level(uint32_t level) {
    printf("Setting log level to %lu\n\r", level);
    log_level = level;
}

/* TODO: If the log is set to intelligent mode, put the message into a buffer
 * instead of printing it
 *
 * Print the messages from the buffer whenever the user manually does so
 *
 */
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

#define MAX_LOG_MSG 256
static void _log(char *fmt, uint32_t level, va_list ap) {
    char msg [MAX_LOG_MSG];

    /* If we recieve a null message just bail out */
    if (!fmt) {
        return;
    }

    /* Still try to print user's string even if it is too long */
    if (strlen(fmt) > MAX_LOG_MSG) {
        fmt[MAX_LOG_MSG - 1] = '\0';
    }

    vsscanf(msg, fmt, ap);
    __log(msg, level);
}

void log_info(char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);

    _log(fmt, LOG_LEVEL_INFO, ap);

    va_end(ap);
}

void log_warn(char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);

    _log(fmt, LOG_LEVEL_WARN, ap);

    va_end(ap);
}

void log_err(char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);

    _log(fmt, LOG_LEVEL_ERROR, ap);

    va_end(ap);
}
