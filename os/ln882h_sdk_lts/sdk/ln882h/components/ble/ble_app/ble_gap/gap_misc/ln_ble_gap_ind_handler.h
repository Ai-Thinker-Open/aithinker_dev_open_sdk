/**
 ****************************************************************************************
 *
 * @file ln_ble_gap_ind_handler.h
 *
 * @brief GAP API.
 *
 * Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef _LN_APP_GAP_IND_HANDLER_H_
#define _LN_APP_GAP_IND_HANDLER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "ke_task.h"


/*
 * DEFINES
 ****************************************************************************************
 */

extern struct ke_msg_handler app_gap_general_msg_tab[];

int app_gap_msg_size(void);

#endif

