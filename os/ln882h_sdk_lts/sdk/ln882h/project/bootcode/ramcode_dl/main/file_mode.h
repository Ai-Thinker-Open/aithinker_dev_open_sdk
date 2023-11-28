/**
 * @file     bootram_file_mode.h
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2020-12-31
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

#ifndef __BOOTRAM_FILE_MODE_H
#define __BOOTRAM_FILE_MODE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int      bootram_enter_file_mode(void);
uint32_t bootram_save_block_data_to_flash(ymodem_t* mblock);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // __BOOTRAM_FILE_MODE_H
