#ifndef _WRAP_STDIO_H_
#define _WRAP_STDIO_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*stdio_write_fn)(const char *buf, int len);

#if defined (__GNUC__) || defined (__clang__)
void __wrap_sprintf(stdio_write_fn write, const char *format, ...)
        __attribute__((format(printf, 2, 3)));
#else
void __wrap_sprintf(stdio_write_fn write, const char *format, ...);
#endif
void __wrap_stdio_raw_data_flush(const char *buff, stdio_write_fn write, size_t size);
void __sprintf(const char *tag, stdio_write_fn write, const char *format, va_list args);

#ifdef __cplusplus
}
#endif
#endif /* _WRAP_STDIO_H_ */
