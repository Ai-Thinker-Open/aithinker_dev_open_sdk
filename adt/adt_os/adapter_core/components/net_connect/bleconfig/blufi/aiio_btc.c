/** @brief      blufi btc.
 *
 *  @file       aiio_btc.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/18      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#include <stdlib.h>
#include <string.h>
#include "aiio_btc.h"
#include "blufi_prf.h"

void *g_btc_profile_cb = NULL;

void aiio_profile_cb_reset(void)
{
    g_btc_profile_cb = NULL;
}

int32_t btc_profile_cb_set(void *cb)
{

    g_btc_profile_cb = cb;
    return AIIO_OK;
}

void *btc_profile_cb_get()
{
    return g_btc_profile_cb;
}


static void btc_thread_handler(void *arg)
{
    btc_msg_t *msg = (btc_msg_t *)arg;

    aiio_log_i("[blufi] %s msg %u %u %p\n", __func__, msg->sig, msg->act, msg->arg);
    switch (msg->sig) {
    case BTC_SIG_API_CALL:
        btc_blufi_call_handler(msg);
        break;
    case BTC_SIG_API_CB:
        btc_blufi_cb_handler(msg);
        break;
    default:
        break;
    }

    if (msg->arg) {
        free(msg->arg);
    }
    //free(msg);
}

bt_status_t btc_transfer_context(btc_msg_t *msg, void *arg, int32_t arg_len, btc_arg_deep_copy_t copy_func)
{
     btc_msg_t lmsg;

    if (msg == NULL) {
        return BT_STATUS_PARM_INVALID;
    }

    aiio_log_i("[blufi]%s msg %u %u %p\n", __func__, msg->sig, msg->act, arg);

    memcpy(&lmsg, msg, sizeof(btc_msg_t));
    if (arg) {
        lmsg.arg = (void *)malloc(arg_len);
        if (lmsg.arg == NULL) {
            return BT_STATUS_NOMEM;
        }
        memset(lmsg.arg, 0x00, arg_len);    //important, avoid arg which have no length
        memcpy(lmsg.arg, arg, arg_len);
        if (copy_func) {
            copy_func(&lmsg, lmsg.arg, arg);
        }
    } else {
        lmsg.arg = NULL;
    }

    btc_thread_handler(&lmsg);

    return AIIO_OK;
}