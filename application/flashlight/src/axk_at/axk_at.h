#ifndef _AXK_AT_H_
#define _AXK_AT_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#define AT_YIELD_SLEEP_TIME_MS (100)
#define AT_CMD_MAX_LEN 512
#define AT_CMD_MAX_PARAS 64
#define AT_CMD_LIST_NUM 5
#define AT_CMD_DEBUG_ENABLE 0
#define AT_CMD_RINGBUFF_SIZE     2 * 1024

#define AT_RESPONSE_OK           \
    do                           \
    {                            \
        axk_at_printf("OK\r\n"); \
    } while (0)

#define AT_RESPONSE_OK_WITH_ENTER \
do                           \
{                            \
    axk_at_printf("\r\nOK\r\n"); \
} while (0)

#define AT_RESPONSE_ERROR           \
    do                              \
    {                               \
        axk_at_printf("ERROR\r\n"); \
    } while (0)

#define AT_RESPONSE_ERROR_WITH_ENTER\
    do                              \
    {                               \
        axk_at_printf("\r\nERROR\r\n"); \
    } while (0)

#define AT_ENTER               \
    do                         \
    {                          \
        axk_at_printf("\r\n"); \
    } while (0)

#define AXK_TRUE 1
#define AXK_FALSE 0

typedef int (*at_call_back_func)(int32_t argc, const char **argv);
typedef void (*at_port_specific_callback_t) (char data);

typedef struct
{
    char *at_cmd_name;
    int8_t at_cmd_len;
    at_call_back_func at_test_cmd;
    at_call_back_func at_query_cmd;
    at_call_back_func at_setup_cmd;
    at_call_back_func at_exe_cmd;
} at_cmd_func;

typedef struct axk_at_data
{
    char *data;
    uint32_t data_len;

} axk_at_data_t;

typedef enum
{
    AT_ERR_SUCCESS = 0,
    AT_ERR_FAILURE = -1,
    AT_ERR_FAILURE_WITH_ENTER = -4,
    AT_INVALID_PARAMETER = -2,
    AT_ERR_NAME_OR_FUNC_REPEAT_REGISTERED3 = -3,
    AT_ERR_RECVING = 2,
    AT_ERR_BUSY = 3,
    AT_ERR_TRANSPARENT = 4
} AXK_ERRNO;

typedef enum
{
    AT_IDLE,
    AT_CMD_PROCESS,
    AT_DATA_RECVING,
    AT_DATA_SENDING,
    AT_TRANSPARENT,
} at_state_machine;

typedef struct
{
    at_state_machine at_state;
    uint16_t send_len;
    uint16_t trans_len;
    bool is_first_recv_data;
    bool is_first_over_data;
    uint16_t is_recv_end_char_flag;
    at_port_specific_callback_t specific_callback;
} at_cmd_ctrl;

typedef enum
{
    AT_CMD_TYPE_TEST = 1,
    AT_CMD_TYPE_QUERY = 2,
    AT_CMD_TYPE_SETUP = 3,
    AT_CMD_TYPE_EXE = 4,
} at_cmd_type;

typedef struct
{
    int8_t at_cmd_len;
    char cmd_name[AT_CMD_MAX_LEN];
    at_cmd_type at_cmd_type;
    int32_t at_param_cnt; /* command actual para num  */
    uint32_t param_array[AT_CMD_MAX_PARAS];
} at_cmd_attr;

typedef struct
{
    const at_cmd_func *at_cmd_list[AT_CMD_LIST_NUM]; /* user input at cmd list */
    uint16_t at_cmd_num[AT_CMD_LIST_NUM];            /* command number */
} at_cmd_func_list;

extern at_cmd_ctrl g_at_ctrl;



void axk_at_cmd_execute(char *buf);

int at_port_enter_specific(at_port_specific_callback_t callback);
int at_port_is_specific_mode(void);
void at_port_exit_specific();
// void at_port_exit_passthrough();

/**
 * @brief Whether to print URC data
 * 
 * @return return 0 means don't print URC data. 
 */
// int at_check_print_urc(void);

#if ((defined BL602) || (defined W800))
#define at_log printf
#else
void at_log(char *fmt, ...);
#endif

void axk_at_printf(char *fmt, ...);
void axk_at_send_data(char *data, int len);
int at_uart_mutex_init(void);

int axk_at_cmd_parse(char *cmd_line, at_cmd_attr *cmd_parsed);

at_cmd_func_list *at_get_list(void);

uint32_t integer_check(const char *val);

int axk_at_register_cmd(const at_cmd_func *cmd_tbl, uint16_t cmd_num);

#endif //_AXK_AT_H_
