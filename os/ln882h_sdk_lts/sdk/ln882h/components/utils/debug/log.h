#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdint.h>
#include "proj_config.h"
#include "utils/wrap_stdio.h"


#define LOG_LVL_EMERG               0
#define LOG_LVL_ERROR               (LOG_LVL_EMERG + 1)
#define LOG_LVL_WARN                (LOG_LVL_EMERG + 2)
#define LOG_LVL_INFO                (LOG_LVL_EMERG + 3)
#define LOG_LVL_DEBUG               (LOG_LVL_EMERG + 4)
#define LOG_LVL_TRACE               (LOG_LVL_EMERG + 5)
#define LOG_LVL_CTRL                LOG_LVL_INFO     //By modifying this value, control the different levels of log printing

void log_init(void);
void log_deinit(void);

void hexdump(uint8_t level, const char *info, void *buff, uint32_t count);
int log_stdio_write(char *buf, size_t size);

#define log_printf(...)     __wrap_sprintf((stdio_write_fn)log_stdio_write, __VA_ARGS__)
#define log_raw_data_flush(data, len) __wrap_stdio_raw_data_flush((const char *)data, (stdio_write_fn)log_stdio_write, (size_t)len)

#if (PRINTF_OMIT == DISABLE)
        #define LOG(level, ...)                  \
                do{                              \
                    if(level <= LOG_LVL_CTRL){   \
                        log_printf(__VA_ARGS__); \
                    }                            \
                }while(0)
#else
    #define LOG(level,...);
#endif


#endif /* __LOG_H__ */
