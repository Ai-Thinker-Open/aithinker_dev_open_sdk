/**
 * @file     bootram_mode.c
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2020-12-31
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

#include "proj_config.h"
#include "mode_ctrl.h"
#include "file_mode.h"
#include "cmd_mode.h"

/*****************************************  variables *********************************************/
static bootram_fsm_t       sg_bootram_fsm;
static bootram_cmd_ctrl_t  sg_bootram_cmd_ctrl;
static bootram_file_ctrl_t sg_bootram_file_ctrl;

/*****************************************  functions *********************************************/
bootram_fsm_t* bootram_fsm_get_handle(void)
{
    return &sg_bootram_fsm;
}

bootram_cmd_ctrl_t* bootram_cmd_get_handle(void)
{
    return &sg_bootram_cmd_ctrl;
}

bootram_file_ctrl_t* bootram_file_get_handle(void)
{
    return &sg_bootram_file_ctrl;
}

void bootram_ctrl_init(void)
{
    bootram_fsm_t*       fsm       = bootram_fsm_get_handle();
    bootram_cmd_ctrl_t*  cmd_ctrl  = bootram_cmd_get_handle();
    bootram_file_ctrl_t* file_ctrl = bootram_file_get_handle();

    // [0] fsm
    fsm->bootram_cur_mode = BOOTRAM_MODE_INIT;

    // [1] cmd_ctrl
    bootram_serial_init();

    memset(cmd_ctrl, 0, sizeof(bootram_cmd_ctrl_t));
    cmd_ctrl->index = 0;
    memset(cmd_ctrl->cmd_line, 0, CMD_PBSIZE);

    // [2] file_ctrl
    memset(&file_ctrl->ymodem_ctrl, 0, sizeof(ymodem_t));
    memset(&file_ctrl->upgrade_ctrl, 0, sizeof(upgrade_ctrl_t));

    file_ctrl->upgrade_ctrl.cache_buffer_arr = g_cache_buffer;
    file_ctrl->upgrade_ctrl.cache_buffer_pos = 0;

    file_ctrl->upgrade_ctrl.cache_tailing_buf = g_cache_tailing;
    file_ctrl->upgrade_ctrl.cache_tailing_pos = 0;

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
    file_ctrl->upgrade_ctrl.verify_buffer_arr = g_cache_read_buf;
#endif
}

void bootram_ctrl_loop(void)
{
    bootram_fsm_t* fsm = bootram_fsm_get_handle();
    while (1) {
        switch (fsm->bootram_cur_mode) {
            case BOOTRAM_MODE_INIT: {
                bootram_serial_flush();
                fsm->bootram_cur_mode = BOOTRAM_MODE_CMD;
                break;
            }
            case BOOTRAM_MODE_CMD: {
                bootram_enter_command_mode();
                break;
            }
            case BOOTRAM_MODE_CMD_2_DOWNLOAD: {
                bootram_serial_flush();
                fsm->bootram_cur_mode = BOOTRAM_MODE_DOWNLOAD;
                break;
            }
            case BOOTRAM_MODE_DOWNLOAD: {
                bootram_enter_file_mode();
                fsm->bootram_cur_mode = BOOTRAM_MODE_DOWNLOAD_2_CMD;
                break;
            }
            case BOOTRAM_MODE_DOWNLOAD_2_CMD: {
                bootram_serial_flush();
                fsm->bootram_cur_mode = BOOTRAM_MODE_CMD;
                break;
            }
            default: break;
        }
    }
}
