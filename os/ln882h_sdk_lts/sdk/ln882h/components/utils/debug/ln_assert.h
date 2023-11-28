#ifndef __LN_ASSERT_H_
#define __LN_ASSERT_H_

#include <assert.h>
#include "proj_config.h"


#if ((defined LN_ASSERT_EN) && (LN_ASSERT_EN))
  #define LN_ASSERT(e)  assert(e)
  void __aeabi_assert (const char *expr, const char *file, int line);
#else
  #define LN_ASSERT(e)
#endif

#endif /* __LN_ASSERT_H_ */
