#include "cmd_mode.h"
#include "file_mode.h"
#include "hal/hal_flash.h"

#define BOOTRAM_CMD_PROMPT   "\r\nbootram> "
#define BOOTRAM_DUMP_COL_NUM (16)

static bootram_cmd_tbl_t bootram_cmd_list[] = {
    {
        "ferase",
        3,
        cmd_flash_erase,
        "Flash erase command.\r\nCommand formate: ferase flash_offset erase_size\r\n\tExample: ferase 0xD000 0x4000\r\n"
    },

    {
        "ferase_all",
        1,
        cmd_flash_erase_all,
        "Erase the all flash.\r\nCommand formate: ferase_all\r\n\tExample: ferase_all\r\n"
    },

    {
        "fdump",
        3,
        cmd_flash_dump,
        "Flash data dump command.\r\nCommand formate: fdump flash_offset dump_size(dump_size < 0x100)\r\n\tExample: fdump 0xD000 0x100\r\n"
    },

    {
        "upgrade",
        1,
        cmd_flash_upgrade,
        "Download the total flash image by UART0 and burn to flash. After run this command, the PC side can send flash image file through ymode.\r\nCommand formate: upgrade\r\n\tExample: upgrade\r\n"
    },

    {
        "startaddr",
        2,
        cmd_download_startaddr,
        "Download offset.\r\nExample: startaddr 0x1000\r\n"
    },

    {
        "baudrate",
        2,
        cmd_download_baudrate,
        "Set UART baudrate during download.\r\nExample: baudrate 115200"
    },

    {
        "filecount",
        1,
        cmd_download_filecount,
        "Check how many files have been downloaded via ymodem.\r\nExample: filecount \r\n"
    },

    {
        "reboot",
        1,
        cmd_reboot,
        "Chip reboot.\r\n"
    },

    {
        "version",
        1,
        cmd_version,
        "ramcode version.\r\n"
    },
    
    {
        "flash_info",
        1,
        cmd_flash_info,
        "flash_info.\r\n"
    },
    
    {
        "flash_test",
        1,
        cmd_flash_test,
        "flash_test.\r\n"
    },
    

#if (defined(DEBUG_FLASH_ROBUST) && (DEBUG_FLASH_ROBUST == 1))
    {
        "flash_robust",
        1,
        flash_robust,
        "Flash robust test within an infinite loop.\r\nCommand format: flash_robust\r\nExample: flash_robust\r\n"
    },
#endif  // !DEBUG_FLASH_ROBUST
};

bootram_cmd_tbl_t* bootram_find_cmd(const char* cmd)
{
    bootram_cmd_tbl_t* cmdtp      = NULL;
    bootram_cmd_tbl_t* cmdtp_temp = NULL; /*Init value */
    int                i;

    for (i = 0; i < NELEMENTS(bootram_cmd_list); i++) {
        cmdtp = &bootram_cmd_list[i];
        if (strncmp(cmd, cmdtp->cmd, strlen(cmd)) == 0) {
            if (strlen(cmd) == strlen(cmdtp->cmd))
                return cmdtp; /* full match */
            /*
             *record abbreviated command
             */
            cmdtp_temp = cmdtp;
        }
    }

    return cmdtp_temp;
}

static int bootram_parse_line(char* line, char* argv[])
{
    int nargs = 0;

    while (nargs < 4) {
        /* skip any white space */
        while ((*line == ' ') || (*line == '\t')) {
            ++line;
        }

        if (*line == '\0') {
            argv[nargs] = 0;
            return (nargs);
        }

        /* Argument include space should be bracketed by quotation mark */
        if (*line == '\"') {
            /* Skip quotation mark */
            line++;

            /* Begin of argument string */
            argv[nargs++] = line;

            /* Until end of argument */
            while (*line && (*line != '\"')) {
                ++line;
            }
        }
        else {
            argv[nargs++] = line; /* begin of argument string    */

            /* find end of string */
            while (*line && (*line != ' ') && (*line != '\t')) {
                ++line;
            }
        }

        if (*line == '\0') { /* end of line, no more args    */
            argv[nargs] = 0;
            return (nargs);
        }

        *line++ = '\0'; /* terminate current arg     */
    }

    return (nargs);
}

static int bootram_command_execute(bootram_cmd_ctrl_t* cmd_ctrl, char* cmd)
{
    int                ret = -1;
    bootram_cmd_tbl_t* cmdtp;
    char*              argv[CMD_MAXARGS] = {
        NULL,
    };
    int argc;

    /* Extract arguments */
    if ((argc = bootram_parse_line(cmd, argv)) == 0) {
        return ret;
    }

    /* Look up command in command table */
    if ((cmdtp = bootram_find_cmd(argv[0])) == NULL) {
        // bootram_console_printf("\r\nUnknown command '%s' - try 'help'\r\n", argv[0]);
        return ret;
    }

    /* found - check max args */
    if (argc != cmdtp->argc) {
        // bootram_console_printf("\r\n%s\r\n", cmdtp->usage);
        return ret;
    }

    return (cmdtp->cmd_executor)(cmdtp, argc, argv);  // run the cmd
}

int bootram_enter_command_mode(void)
{
    bootram_cmd_ctrl_t* cmd_ctrl = bootram_cmd_get_handle();

    uint8_t ch     = 0;
    int     ret    = -1;
    int     rd_len = 0;

    rd_len = bootram_serial_read(&ch, 1);
    if (rd_len > 0) {
        if (isprint(ch)) {
            bootram_serial_write(&ch, 1);
            // put this char into console_ctrl.console_buffer
            cmd_ctrl->cmd_line[cmd_ctrl->index++] = ch;
            if (cmd_ctrl->index >= CMD_PBSIZE) {
                memset(cmd_ctrl->cmd_line, 0, CMD_PBSIZE);
                cmd_ctrl->index = 0;
            }
        }
        else if (ch == 0x08) {  // for backspace
            if (cmd_ctrl->index > 0) {
                cmd_ctrl->index--;
                cmd_ctrl->cmd_line[cmd_ctrl->index] = 0;
            }
        }
        else if (ch == '\n' || ch == '\r') {  // for '\r' '\n'
            if (strlen(cmd_ctrl->cmd_line) > 0) {
                ret = bootram_command_execute(cmd_ctrl, cmd_ctrl->cmd_line);
                if (ret == 1) {
                    memset(cmd_ctrl->cmd_line, 0, CMD_PBSIZE);
                    cmd_ctrl->index = 0;
                    return ret;
                }
            }

            memset(cmd_ctrl->cmd_line, 0, CMD_PBSIZE);
            cmd_ctrl->index = 0;
        }
    }
    return 0;
}

void echo_result(int isPass)
{
    char  result_pass[] = "\r\npppp\r\n";
    char  result_fail[] = "\r\nffff\r\n";
    char* result        = NULL;

    bootram_cmd_ctrl_t* cmd_ctrl = bootram_cmd_get_handle();

    if (isPass) {
        result = result_pass;
    }
    else {
        result = result_fail;
    }

    // for (int k = 0; k < 3; k++)
    for (int i = 0; i < strlen(result); i++) {
        uint8_t ch = result[i];
        bootram_serial_write(&ch, 1);
    }
}

int cmd_flash_info(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    uint32_t ret        = 0;
    char     buf[50]    = {0};
    uint32_t flash_size = 0;
    
    ret = bootram_flash_info();
    
    if((ret & 0xFF) == 0x15){
        flash_size = 2;
    }else{
        flash_size = 1;
    }
    
    sprintf(buf, "\r\nid:0x%X,flash size:%dM Byte\r\n", ret,flash_size);

    for (int i = 0; i < strlen(buf); i++) {
        uint8_t ch = buf[i];
        bootram_serial_write(&ch, 1);
    }
    
    return 0;
}


int cmd_flash_test(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    uint32_t    ret                = 0;
    char        buf[50]            = {0};
    uint32_t    flash_size         = 0;
    uint8_t     write_buf[4096];
    uint8_t     read_buf[4096];
    uint32_t    err_pos            = 0;

    ret = bootram_flash_info();
    
    if((ret & 0xFF) == 0x15){
        flash_size = 0x200000;
    }else{
        flash_size = 0x100000;
    }
    
    for(int i = 0; i < 4096; i ++)
    {
        write_buf[i] = i;
    }
    
    ret = 0;
    bootram_flash_chiperase();
    for(int i = 0; i < flash_size / 0x1000; i ++)
    {
        bootram_flash_write(i * 0x1000,0x1000,write_buf);
        bootram_flash_read(i * 0x1000,0x1000,read_buf);
        for(int x = 0; x < 4096; x ++)
        {
            if(write_buf[x] != read_buf[x]){
                ret = 1;
                err_pos = i * 0x1000 + x;
                break;
            }
        }
        if(ret != 0)
            break;
    }
    
    if(ret == 0){
        sprintf(buf, "\r\nno err\r\n");
    }else{
        sprintf(buf, "\r\nerr pos:0x%X\r\n", err_pos);
    }
    
    for (int i = 0; i < strlen(buf); i++) {
        uint8_t ch = buf[i];
        bootram_serial_write(&ch, 1);
    }
    
    return 0;
}

int cmd_flash_erase(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    int      ret          = -1;
    uint32_t flash_offset = 0, size = 0;

    if (argv[1]) {
        flash_offset = strtoul(argv[1], NULL, 0);
    }
    if (argv[2]) {
        size = strtoul(argv[2], NULL, 0);
    }
    size = MIN(FALSH_SIZE_MAX - flash_offset, size);
    if (size != 0 && (flash_offset + size) < FALSH_SIZE_MAX) {
        bootram_flash_erase(flash_offset, size);

        echo_result(1);

        return 0;
    }

    // bootram_console_printf("\r\nCommand %s args error!\r\n", argv[0]);
    echo_result(0);

    return ret;
}

int cmd_flash_erase_all(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    bootram_file_ctrl_t* cmd_ctrl     = bootram_file_get_handle();
    upgrade_ctrl_t*      upgrade_ctrl = &(cmd_ctrl->upgrade_ctrl);
    bootram_flash_chiperase();
    upgrade_ctrl->file_count = 0;

    echo_result(1);

    return 0;
}

int cmd_flash_upgrade(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    bootram_fsm_t* fsm    = bootram_fsm_get_handle();
    fsm->bootram_cur_mode = BOOTRAM_MODE_CMD_2_DOWNLOAD;

    return 0;
}

void bootram_hexdump(bootram_output_type_enum_t output_type,
                     uint32_t                   offset,
                     uint8_t*                   buff,
                     uint32_t                   size)
{
    volatile uint32_t i = 0, col = 0, row = 0;
    if (BOOTRAM_CONSOLE_OUTPUT == output_type) {
        bootram_console_printf("\r\n");
    }
    else {
        // LOG(LOG_LVL_INFO, "\r\n");
    }
    for (row = 0; row < (size / BOOTRAM_DUMP_COL_NUM); row++) {
        if (BOOTRAM_CONSOLE_OUTPUT == output_type) {
            bootram_console_printf("0x%04X: ", (offset + BOOTRAM_DUMP_COL_NUM * row));
        }
        else {
            // LOG(LOG_LVL_INFO, "0x%04X: ", (offset + BOOTRAM_DUMP_COL_NUM * row));
        }
        for (col = 0; col < BOOTRAM_DUMP_COL_NUM; col++) {
            i = (BOOTRAM_DUMP_COL_NUM * row + col);
            if (BOOTRAM_CONSOLE_OUTPUT == output_type) {
                bootram_console_printf("%02X ", *(buff + i));
            }
            else {
                // LOG(LOG_LVL_INFO, "%02X ", *(buff + i));
            }
        }
        if (BOOTRAM_CONSOLE_OUTPUT == output_type) {
            bootram_console_printf("\r\n");
        }
        else {
            // LOG(LOG_LVL_INFO, "\r\n");
        }
    }
    if (size % BOOTRAM_DUMP_COL_NUM) {
        if (BOOTRAM_CONSOLE_OUTPUT == output_type) {
            bootram_console_printf("0x%04X: ", (offset + BOOTRAM_DUMP_COL_NUM * row));
        }
        else {
            // LOG(LOG_LVL_INFO, "0x%04X: ", (offset + BOOTRAM_DUMP_COL_NUM * row));
        }
        for (col = 0; col < (size % BOOTRAM_DUMP_COL_NUM); col++) {
            i = (BOOTRAM_DUMP_COL_NUM * row + col);
            if (BOOTRAM_CONSOLE_OUTPUT == output_type) {
                bootram_console_printf("%02X ", *(buff + i));
            }
            else {
                // LOG(LOG_LVL_INFO, "%02X ", *(buff + i));
            }
        }

        if (BOOTRAM_CONSOLE_OUTPUT == output_type) {
            bootram_console_printf("\r\n");
        }
        else {
            // LOG(LOG_LVL_INFO, "\r\n");
        }
    }
}
int bootram_console_stdio_write(char* buf, size_t size)
{
    int                 ret      = 0;
    bootram_cmd_ctrl_t* cmd_ctrl = bootram_cmd_get_handle();

    ret = bootram_serial_write((const void*)buf, size);
    return ret;
}
int cmd_flash_dump(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    int      ret = -1;
    uint8_t  page_buffer[FLASH_PAGE_SIZE + 1];
    uint32_t flash_offset = 0, size = 0, page_num = 0, i = 0;

    if (argv[1]) {
        flash_offset = strtoul(argv[1], NULL, 0);
    }
    if (argv[2]) {
        size = strtoul(argv[2], NULL, 0);
    }
    size = MIN(FALSH_SIZE_MAX - flash_offset, size);
    if (size > 0) {
        page_num = size / FLASH_PAGE_SIZE;
        for (i = 0; i < page_num; i++) {
            bootram_flash_read(flash_offset + i * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE, page_buffer);
            bootram_hexdump(BOOTRAM_CONSOLE_OUTPUT, i * FLASH_PAGE_SIZE, page_buffer,
                            FLASH_PAGE_SIZE);
        }
        if (size % FLASH_PAGE_SIZE) {
            bootram_flash_read(flash_offset + i * FLASH_PAGE_SIZE, size % FLASH_PAGE_SIZE,
                               page_buffer);
            bootram_hexdump(BOOTRAM_CONSOLE_OUTPUT, i * FLASH_PAGE_SIZE, page_buffer,
                            size % FLASH_PAGE_SIZE);
        }

        echo_result(1);

        return 0;
    }
    else {
        // bootram_console_printf("\r\nCommand %s args error!\r\n", argv[0]);
    }

    echo_result(0);

    return ret;
}

/**
 * @brief
 * @param mblock
 * @param _upgrade_ctrl
 * @return int
 */

#if (defined(DEBUG_FLASH_ROBUST) && (DEBUG_FLASH_ROBUST == 1))
static int flash_robust(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    const uint32_t FLASH_ERASE_SIZE_MIN = FALSH_SIZE_4K;
    const uint32_t FLASH_START_ADDRESS  = FLASH_BASE_OFFSET;
    const uint32_t FLASH_STOP_ADDRESS = FLASH_START_ADDRESS + FALSH_SIZE_MAX - FLASH_ERASE_SIZE_MIN;

    uint8_t  page_data[FLASH_PAGE_SIZE] = {0};
    uint8_t  read_back[FLASH_PAGE_SIZE] = {0};
    uint16_t total_page_num             = 0;  // pages in a sector.

    uint32_t offset = FLASH_START_ADDRESS;

    uint32_t page_pass_count  = 0;
    uint32_t total_pass_count = 0;
    uint8_t  flag_continue    = 1;

    for (int i = 0; i < FLASH_PAGE_SIZE; i++) {
        page_data[i] = i;
    }

    while (flag_continue) {
        offset += FLASH_ERASE_SIZE_MIN;
        if (offset > FLASH_STOP_ADDRESS) {
            offset = FLASH_START_ADDRESS;
            total_pass_count += 1;
        }

        // erase a sector (4K)
        bootram_flash_erase(offset, FLASH_ERASE_SIZE_MIN);

        // program a sector
        total_page_num = FLASH_ERASE_SIZE_MIN / FLASH_PAGE_SIZE;
        for (int i = 0; i < total_page_num; i++) {
            bootram_flash_write(offset + i * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE, page_data);

            // read back and compare.
            memset(read_back, 0, FLASH_PAGE_SIZE * sizeof(uint8_t));
            bootram_flash_read(offset + i * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE, read_back);

            if (memcmp(read_back, page_data, FLASH_PAGE_SIZE)) {
                // ERROR!!!
                flag_continue = 0;
                break;
            }
        }

        if (flag_continue) {
            page_pass_count += 1;
            bootram_console_printf("pass count, page = %d, total = %d\r\n", page_pass_count,
                                   total_pass_count);
        }
    }

    bootram_console_printf("\r\nflash_robust run over, page = %d, total = %d\r\n", page_pass_count,
                           total_pass_count);

    return 1;
}
#endif  // !DEBUG_FLASH_ROBUST

/**
 * @brief
 * usage:
 * startaddr 0x0000
 * startaddr 0x1000
 * startaddr 0x80000
 *
 * @param cmdtbl
 * @param argc
 * @param argv
 * @return int
 */
int cmd_download_startaddr(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    bootram_file_ctrl_t* cmd_ctrl     = bootram_file_get_handle();
    upgrade_ctrl_t*      upgrade_ctrl = &(cmd_ctrl->upgrade_ctrl);

    uint32_t sa     = 0;
    char*    endptr = NULL;

    if (argv[1]) {
        sa                               = strtoul(argv[1], &endptr, 16);
        upgrade_ctrl->flash_write_offset = upgrade_ctrl->start_addr = sa;
        echo_result(1);
        return 0;
    }

    upgrade_ctrl->flash_write_offset = upgrade_ctrl->start_addr = sa;
    echo_result(0);
    return -1;
}

/**
 * @brief
 * Usage:
 * baudrate 115200
 *
 * @param cmdtbl
 * @param argc
 * @param argv
 * @return int
 */
int cmd_download_baudrate(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    bootram_cmd_ctrl_t* cmd_ctrl = bootram_cmd_get_handle();

    uint32_t baudrate = CFG_UART_BAUDRATE_CONSOLE;
    char*    endptr   = NULL;
    if (argv[1]) {
        baudrate = strtoul(argv[1], &endptr, 10);
        bootram_serial_setbaudrate(baudrate);
        bootram_serial_flush();
        return 0;
    }

    return -1;
}

/**
 * @brief Echo to PC how many files have been downloaded via ymodem.
 * fc:3
 * fc:8
 * fc:12
 * @param cmdtbl
 * @param argc
 * @param argv
 * @return int
 */
int cmd_download_filecount(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    bootram_file_ctrl_t* cmd_ctrl     = bootram_file_get_handle();
    upgrade_ctrl_t*      upgrade_ctrl = &(cmd_ctrl->upgrade_ctrl);
    char                 buf[10]      = {0};

    sprintf(buf, "\r\nfc:%d\r\n", upgrade_ctrl->file_count);

    for (int i = 0; i < strlen(buf); i++) {
        uint8_t ch = buf[i];
        bootram_serial_write(&ch, 1);
    }

    return 0;
}

int cmd_reboot(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
    echo_result(1);

    bootram_user_reboot();
    return 0;
}

/**
 * @brief ram code version reply.
 *
 * @param cmdtbl unused.
 * @param argc unused.
 * @param argv unused.
 * @return int return 0.
 */
int cmd_version(bootram_cmd_tbl_t* cmdtbl, int argc, char* argv[])
{
#define RAMCODE_VERSION "\r\nRAMCODE\r\n"

    bootram_cmd_ctrl_t* cmd_ctrl = bootram_cmd_get_handle();

    bootram_serial_write(RAMCODE_VERSION, strlen(RAMCODE_VERSION));

    return 0;
}
