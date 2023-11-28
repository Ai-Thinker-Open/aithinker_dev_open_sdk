#include "proj_config.h"
#include "ln882h.h"
#include "utils/wrap_stdio.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define WRAP_STDOUT_BUF_SIZE	(256)
static char stdout_buf[WRAP_STDOUT_BUF_SIZE];

#ifdef __CONFIG_OS_KERNEL
  #include "osal/osal.h"
  OS_Mutex_t stdout_mutex;

  static void printf_mutex_lock(void)
  {
      if (__get_PRIMASK() || \
          __get_FAULTMASK() || \
          __get_IPSR() || \
          !OS_ThreadIsSchedulerRunning())
      {
          return;
      }

      if (!OS_MutexIsValid(&stdout_mutex)) {
          OS_MutexCreate(&stdout_mutex);
      }

      OS_MutexLock(&stdout_mutex, OS_WAIT_FOREVER);
  }

  static void printf_mutex_unlock(void)
  {
      if (__get_PRIMASK() || \
          __get_FAULTMASK() || \
          __get_IPSR() || \
          !OS_ThreadIsSchedulerRunning())
      {
          return;
      }

      if (OS_MutexIsValid(&stdout_mutex)) {
          OS_MutexUnlock(&stdout_mutex);
      }
  }
#else
  static void printf_mutex_lock(void){};
  static void printf_mutex_unlock(void){};
#endif


void __sprintf(const char *tag, stdio_write_fn write, const char *format, va_list args)
{
    printf_mutex_lock();

    vsnprintf(stdout_buf, WRAP_STDOUT_BUF_SIZE, format, args);

    if(write)
    {
        if (tag) {
            write(tag, strlen(tag));
        }
        write(stdout_buf, strlen(stdout_buf));
    }

    printf_mutex_unlock();
}

void __wrap_sprintf(stdio_write_fn write, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    __sprintf(NULL, write, format, args);
    va_end(args);
}

void __wrap_stdio_raw_data_flush(const char *buff, stdio_write_fn write, size_t size)
{
    printf_mutex_lock();
    if(write)
    {
        write(buff, size);
    }
    printf_mutex_unlock();
}
