/**
 * @file     bootram_command_mode.h
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2020-12-31
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

#ifndef __BOOTRAM_COMMAND_MODE_H__
#define __BOOTRAM_COMMAND_MODE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mode_ctrl.h"

typedef enum {
    BOOTRAM_CONSOLE_OUTPUT,

} bootram_output_type_enum_t;

typedef struct bootram_cmd_tbl
{
    char* cmd;
    int   argc;
    int (*cmd_executor)(struct bootram_cmd_tbl*, int, char*[]);
    char* usage;
} bootram_cmd_tbl_t;

int bootram_enter_command_mode(void);

bootram_cmd_ctrl_t*  bootram_cmd_get_handle(void);
bootram_file_ctrl_t* bootram_file_get_handle(void);

int cmd_reboot(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_version(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_download_filecount(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_download_baudrate(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_download_startaddr(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_flash_upgrade(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_flash_dump(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_flash_erase_all(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_flash_erase(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_flash_info(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);
int cmd_flash_test(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[]);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  /* __BOOTRAM_COMMAND_MODE_H__ */
