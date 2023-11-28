#include "log.h"
#include "ln882h.h"
#include "utils/debug/ln_assert.h"

#if ((defined LN_ASSERT_EN) && (LN_ASSERT_EN))
void __aeabi_assert(const char *expr,const char *file,int line)
{
    __disable_irq();
    LOG(LOG_LVL_ERROR,"Assertion Failed: file %s, line %d, %s\n",file,line,expr);
    __BKPT(0);
    while(1);
}
#endif
