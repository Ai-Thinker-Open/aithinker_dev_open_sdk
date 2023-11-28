#ifndef  __LN_KV_PORT_H__
#define  __LN_KV_PORT_H__

#include "proj_config.h"
#include "ln_types.h"
#include "ln_kv_err.h"
#include "ln_compiler.h"
#include "mem_map_ln882h.h"


#ifndef __CONFIG_OS_KERNEL
  #define KV_CONFIG_OS_NONE
#endif
#define KV_CONFIG_FLASH_SECT_SIZE_LOG2    (12)


#if defined(KV_CONFIG_OS_NONE)
  #include "stdlib.h"
  
  __STATIC_INLINE__ void *ln_kv_malloc(size_t size)
  {
      return malloc(size);
  }
  
  __STATIC_INLINE__ void  ln_kv_free(void *ptr)
  {
      free(ptr);
  }
  
#else  /* KV_CONFIG_OS_NONE */
  #include "osal/osal.h"
  
  typedef OS_Mutex_t    kv_mutex_t;
   
  __STATIC_INLINE__ int ln_kv_is_os_running(void)
  {
      if (OS_ThreadIsSchedulerRunning()){
          return LN_TRUE;
      }
      return LN_FALSE;
  }
  
  __STATIC_INLINE__ int ln_kv_mutex_create(kv_mutex_t *mutex)
  {
      if (OS_OK == OS_MutexCreate(mutex)) {
          return LN_TRUE;
      }
      return LN_FALSE;
  }
  
  __STATIC_INLINE__ void ln_kv_mutex_delete(kv_mutex_t *mutex)
  {
      OS_MutexDelete(mutex);
  }
  
  __STATIC_INLINE__ void ln_kv_mutex_lock(kv_mutex_t *mutex)
  {
      OS_MutexLock(mutex, OS_WAIT_FOREVER);
  }
  
  __STATIC_INLINE__ void ln_kv_mutex_unlock(kv_mutex_t *mutex)
  {
      OS_MutexUnlock(mutex);
  }
  
  __STATIC_INLINE__ void *ln_kv_malloc(size_t size)
  {
      return OS_Malloc(size);
  }
  
  __STATIC_INLINE__ void  ln_kv_free(void *ptr)
  {
      OS_Free(ptr);
  }
#endif /* KV_CONFIG_OS_NONE */


#if (0)
    #include "utils/debug/log.h"
    #define log_a(...)  LOG(LOG_LVL_ERROR, __VA_ARGS__)
    #define log_e(...)  LOG(LOG_LVL_ERROR, __VA_ARGS__)
    #define log_w(...)  LOG(LOG_LVL_WARN, __VA_ARGS__)
    #define log_i(...)  LOG(LOG_LVL_INFO, __VA_ARGS__)
    #define log_d(...)  LOG(LOG_LVL_DEBUG, __VA_ARGS__)
#else
    #define log_a(...)
    #define log_e(...)
    #define log_w(...)
    #define log_i(...)
    #define log_d(...)
#endif
  
#endif /* __LN_KV_PORT_H__ */

