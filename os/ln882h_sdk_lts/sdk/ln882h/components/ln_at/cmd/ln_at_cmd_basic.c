/**
 * @file   ln_at_cmd_basic.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#include "ln_at.h"
#include "ln_utils.h"

/* declarations */
ln_at_err_t ln_at_basic_at_exec(const char *name);
ln_at_err_t ln_at_basic_ate0_exec(const char *name);
ln_at_err_t ln_at_basic_ate1_exec(const char *name);

ln_at_err_t ln_at_basic_at_exec(const char *name)
{
    LN_UNUSED(name);
    LN_AT_LOG_D("AT EXEC [%s:%d].\r\n", __func__, __LINE__);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(AT, NULL, NULL, NULL, ln_at_basic_at_exec);

ln_at_err_t ln_at_basic_ate0_exec(const char *name)
{
    LN_UNUSED(name);

    LN_AT_LOG_D("AT EXEC [%s:%d].\r\n", __func__, __LINE__);
    ln_at_echo_enable(0);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(ATE0, NULL, NULL, NULL, ln_at_basic_ate0_exec);

ln_at_err_t ln_at_basic_ate1_exec(const char *name)
{
    LN_UNUSED(name);

    LN_AT_LOG_D("AT EXEC [%s:%d].\r\n", __func__, __LINE__);
    ln_at_echo_enable(1);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(ATE1, NULL, NULL, NULL, ln_at_basic_ate1_exec);

static ln_at_err_t ln_at_self_test_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);

    LN_AT_LOG_I("=== AT self test <%s> ===\r\n", name);
    LN_AT_LOG_I("param cnt:%d\r\n", para_num);

    ln_at_param_dump();

    LN_AT_LOG_I("=== AT self test end ===\r\n");
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(SELF_TEST, NULL, ln_at_self_test_set, NULL, NULL);
