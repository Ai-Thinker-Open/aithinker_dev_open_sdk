/**
 ****************************************************************************************
 *
 * @file ln_ble_test.c
 *
 * @brief ble test interface
 *
 * Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>

#include "rwip_config.h"             // SW configuration
#include "rwip_task.h"      // Task definitions
#include "ke_task.h"        // Kernel Task
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "osal/osal.h"

#include "utils/debug/log.h"

#include "ln_ble_test.h"
#include "ln_ble_app_default_cfg.h"


void app_le_test_tx_start_cmd(uint8_t tx_ch, uint8_t phy, uint8_t tx_len, uint8_t pkt)
{
    // Prepare the GAPM_LE_TEST_MODE_CTRL_CMD message
    struct gapm_le_test_mode_ctrl_cmd *p_cmd = KE_MSG_ALLOC(GAPM_LE_TEST_MODE_CTRL_CMD,
            TASK_GAPM, TASK_APP,
            gapm_le_test_mode_ctrl_cmd);

    // Set operation code
    p_cmd->operation = GAPM_LE_TEST_TX_START;
    p_cmd->channel = tx_ch;
    p_cmd->phy = phy;
    p_cmd->tx_data_length = tx_len;
    p_cmd->tx_pkt_payload = pkt;

    ke_msg_send(p_cmd);
}

void app_le_test_rx_start_cmd(uint8_t rx_ch, uint8_t phy, uint8_t mod_index)
{
    // Prepare the GAPM_LE_TEST_MODE_CTRL_CMD message
    struct gapm_le_test_mode_ctrl_cmd *p_cmd = KE_MSG_ALLOC(GAPM_LE_TEST_MODE_CTRL_CMD,
            TASK_GAPM, TASK_APP,
            gapm_le_test_mode_ctrl_cmd);

    // Set operation code
    p_cmd->operation = GAPM_LE_TEST_RX_START;
    p_cmd->channel = rx_ch;
    p_cmd->phy = phy;
    p_cmd->modulation_idx = mod_index;

    ke_msg_send(p_cmd);
}

void app_le_test_stop_cmd(void)
{
    // Prepare the GAPM_LE_TEST_MODE_CTRL_CMD message
    struct gapm_le_test_mode_ctrl_cmd *p_cmd = KE_MSG_ALLOC(GAPM_LE_TEST_MODE_CTRL_CMD,
            TASK_GAPM, TASK_APP,
            gapm_le_test_mode_ctrl_cmd);

    // Set operation code
    p_cmd->operation = GAPM_LE_TEST_STOP;

    ke_msg_send(p_cmd);
}

