/**
 ****************************************************************************************
 *
 * @file ln_ble_gatt_ind_handler.h
 *
 * @brief GATT API.
 *
 * Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef _LN_APP_GATT_IND_HANDLER_H_
#define _LN_APP_GATT_IND_HANDLER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "ke_task.h"


/*
 * DEFINES
 ****************************************************************************************
 */

extern struct ke_msg_handler app_gatt_general_msg_tab[];

int  app_gatt_msg_size(void);

#endif
