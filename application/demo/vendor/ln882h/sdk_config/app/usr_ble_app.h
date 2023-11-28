/**
 ****************************************************************************************
 *
 * @file usr_ble_app.h
 *
 * @brief define  API.
 *
 *Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef _LN_DEF_H_
#define _LN_DEF_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * 
 *
 * @brief define API.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>

#define CHAR_VAL_MAX_LEN    1024

enum trans_svc_att_idx
{
    DATA_TRANS_DECL_SVC = 0,
    DATA_TRANS_DECL_CHAR_RX,
    DATA_TRANS_DECL_CHAR_RX_VAL,
    DATA_TRANS_DECL_CHAR_TX,
    DATA_TRANS_DECL_CHAR_TX_VAL,
    DATA_TRANS_DECL_CHAR_TX_CCC,

    DATA_TRANS_IDX_MAX,
};

/*
 * DEFINES
 ****************************************************************************************
 */

#define LN_MALLOC_CHECK(cond)                                                        \
     do {                                                                            \
         if (!(cond)) {                                                              \
                LOG(LOG_LVL_ERROR, "malloc error  %s  %d \r\n",__FILE__ ,__LINE__);  \
         }                                                                           \
     } while(0) 

void ble_creat_usr_app_task(void);

#endif // _APP_GAP_H_
