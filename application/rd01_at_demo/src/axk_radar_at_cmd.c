#include "axk_radar_at_cmd.h"

// #include "aiio_adapter_include.h"
#include "aiio_at_core.h"
#include "aiio_os_port.h"
#include "aiio_radar.h"
#include "aiio_uart.h"
#include "aiio_autoconf.h"
#include "aiio_log.h"
#include "aiio_nvs.h"

#include <stdint.h>

#define RADAR_DEFAULT_MODE  CONFIG_RADAR_CMD_MODE

#define RADAR_AT_MODE_KET   "rd_cmd_mode"

enum
{
    RADAR_MODE,
    AT_MODE,
};

extern aiio_at_ctrl_t at_ctrl_g;
extern void at_cmd_execute(char *buf);
static int32_t aiio_at_radar_state_cmd(int32_t argc, const char **argv, aiio_at_cmd_type_t type);
static int32_t aiio_at_radar_transparent_cmd(int32_t argc, const char **argv, aiio_at_cmd_type_t type);
static int32_t aiio_at_radar_cmd_mode_set(uint8_t mode);

static aiio_os_mutex_handle_t uart_mutex_s;
static uint8_t at_mode_flag = 0;
static uint8_t radar_at_mode_s = RADAR_DEFAULT_MODE;

static aiio_at_cmd_t aiio_at_radar_cmd[] = {
    {"AT+RADARTT", aiio_at_radar_transparent_cmd, "Turn on radar transparent mode"},
    {"AT+RADARSTATE", aiio_at_radar_state_cmd, "Query radar detection state"},
};

static aiio_at_radar_body_t radar_status_s = {
    .status = RADAR_NO_ONE,
    .distance = 0,
};

static int32_t aiio_at_radar_state_cmd(int32_t argc, const char **argv, aiio_at_cmd_type_t type)
{
    switch (type) {
        case AIIO_AT_CMD_TYPE_QUERY:
            aiio_at_printf("\r\n+RADARSTATE:%u,%u", radar_status_s.status, radar_status_s.distance);
            return AIIO_AT_RET_SUCCESS;
        default:
            aiio_at_set_errcode(AIIO_AT_ERR_SYS_OPT_UNSUPPORTED);
            return AIIO_AT_RET_FAILURE;
    }
}

static int32_t aiio_at_radar_transparent_cmd(int32_t argc, const char **argv, aiio_at_cmd_type_t type)
{
    int save_flash;

    switch (type) {
        case AIIO_AT_CMD_TYPE_SET:
            if (argc != 1)
            {
                aiio_at_set_errcode(AIIO_AT_ERR_SYS_PARAM_TOO_LONG);
                return AIIO_AT_RET_FAILURE;
            }
            at_mode_flag = 0;
            radar_at_mode_s = RADAR_MODE;

            save_flash = atoi(argv[0]);

            if (save_flash != 0 && save_flash != 1)
            {
                aiio_at_set_errcode(AIIO_AT_ERR_SYS_PARAM_INVALID);
                return AIIO_AT_RET_FAILURE;
            }

            if (save_flash != 0)
            {
                //保存flash
                uint8_t cmd_mode = RADAR_MODE;
                aiio_at_radar_cmd_mode_set(cmd_mode);
            }

            return AIIO_AT_RET_SUCCESS;
        default:
            aiio_at_set_errcode(AIIO_AT_ERR_SYS_OPT_UNSUPPORTED);
            return AIIO_AT_RET_FAILURE;
    }
}

static void radar_send_data(const uint8_t *data, uint32_t len)
{
    aiio_os_mutex_get(uart_mutex_s, 0xffffffff);

    aiio_uart_send_data(CONFIG_AT_UART_PORT, data, len);

    aiio_os_mutex_put(uart_mutex_s);
}

void radar_cmd_ack_cb(uint8_t *data, uint16_t len)
{
    uint8_t cmd = data[6];

    switch (cmd)
    {
    case 0xB2:
        at_mode_flag = 1;
        break;
    case 0xFE:
        radar_at_mode_s = at_mode_flag;
        break;
    default:
        break;
    }

    radar_send_data(data, len);
}

void radar_data_output_cb(uint8_t *data, uint16_t len)
{
    radar_status_s.status = (aiio_at_radar_body_status_t)data[8];
    radar_status_s.distance = ((uint16_t)data[16] << 8) | data[15];

    aiio_log_d("radar_status_s.status: %u", radar_status_s.status);
    aiio_log_d("radar_status_s.distance: %u", radar_status_s.distance);

    if (radar_at_mode_s == RADAR_MODE)
    {
        radar_send_data(data, len);
    }
}

static void at_reset_exit_passthrough_detect(void)
{
    for (int cnt = 0; cnt < at_ctrl_g.passthrough_cfg.passthrough_cnt; cnt++) {
        if (at_ctrl_g.passthrough_cfg.specific_callback) {
            at_ctrl_g.passthrough_cfg.specific_callback('+');
        }
    }

    at_ctrl_g.passthrough_cfg.passthrough_cnt = 0;
}

static void radar_at_task(void *arg)
{
    static char cmdstr[AIIO_AT_CMD_MAX_LEN];
    static uint16_t cmdlen = 0;
    char ch;
    int32_t idle_flag;
    uint32_t tick;
    uint32_t last_time;

    while (1)
    {
        if (radar_at_mode_s == AT_MODE)
        {
            idle_flag = AIIO_AT_FALSE;

            if (at_ctrl_g.at_state == AT_STATE_TRANSPARENT) {
                aiio_os_tick_count(&tick);
                last_time = aiio_os_tick2ms(tick);

                /* idle detect */
                if (last_time - at_ctrl_g.passthrough_cfg.last_time_ms > AIIO_AT_PASSTHROUGH_IDLE_MS) {
                    idle_flag = AIIO_AT_TRUE;
                }

                if (idle_flag != AIIO_AT_FALSE) {
                    if (at_ctrl_g.passthrough_cfg.passthrough_cnt == 3) {
                        /* exit passthrough */
                        if (at_ctrl_g.passthrough_cfg.exit_callback) {
                            at_ctrl_g.passthrough_cfg.exit_callback();
                        }

                        at_ctrl_g.at_state = AT_STATE_IDLE;
                    } else if (at_ctrl_g.passthrough_cfg.idle_check == AIIO_AT_TRUE) {
                        if (at_ctrl_g.passthrough_cfg.idle_callback) {
                            at_ctrl_g.passthrough_cfg.idle_callback();
                        }

                        /* clear idle detect */
                        at_ctrl_g.passthrough_cfg.idle_check = AIIO_AT_FALSE;
                    }
                }
            }

            if (aiio_at_recv_one_byte((uint8_t *)&ch) != AIIO_OK) {
                aiio_os_tick_dealy(aiio_os_ms2tick(AIIO_AT_TASK_STACK_DELAY_MS));
                continue;
            }

            if (at_ctrl_g.at_state == AT_STATE_TRANSPARENT) {
                do {
                    if (idle_flag != AIIO_AT_FALSE) {
                        if (ch == '+') {
                            /* start +++ detect */
                            if (at_ctrl_g.passthrough_cfg.passthrough_cnt == 0) {
                                at_ctrl_g.passthrough_cfg.passthrough_cnt++;

                                /* eat + */
                                break;
                            } else {
                                at_reset_exit_passthrough_detect();
                            }
                        }
                    } else {
                        if (at_ctrl_g.passthrough_cfg.passthrough_cnt != 0) {
                            if (ch == '+') {
                                at_ctrl_g.passthrough_cfg.passthrough_cnt++;

                                /* eat + */
                                break;
                            }
                        } else {
                            at_reset_exit_passthrough_detect();
                        }
                    }

                    if (at_ctrl_g.passthrough_cfg.specific_callback) {
                        at_ctrl_g.passthrough_cfg.specific_callback(ch);
                    }
                } while(0);

                /* update time for idle detection */
                aiio_os_tick_count(&tick);
                last_time = aiio_os_tick2ms(tick);
                at_ctrl_g.passthrough_cfg.last_time_ms = last_time;

                continue;
            } else if (at_ctrl_g.specific_callback) {
                at_ctrl_g.specific_callback(ch);
                continue;
            }

            *(uint8_t *)(cmdstr + cmdlen) = ch;
            cmdlen += 1;

            if (cmdlen > AIIO_AT_CMD_MAX_LEN) {
                /* input command too long */
                cmdlen = 0;
                memset(cmdstr, 0, AIIO_AT_CMD_MAX_LEN);
                continue;
            }
            if (cmdlen >= 4 && ('\r' == cmdstr[cmdlen - 2]) && ('\n' == cmdstr[cmdlen - 1])) {
                cmdstr[cmdlen - 2] = '\0';

                at_cmd_execute(cmdstr);

                cmdlen = 0;
            }
        }
        else
        {
            //处理雷达数据
            if (aiio_at_recv_one_byte((uint8_t *)&ch) != AIIO_OK) {
                aiio_os_tick_dealy(aiio_os_ms2tick(5));
                continue;
            }

            *(uint8_t *)(cmdstr + cmdlen) = ch;
            cmdlen += 1;

            if (cmdlen > AIIO_AT_CMD_MAX_LEN) {
                /* input command too long */
                cmdlen = 0;
                memset(cmdstr, 0, AIIO_AT_CMD_MAX_LEN);
                continue;
            }
            if ((cmdlen >= 12) && (0x04 == cmdstr[cmdlen - 4]) && (0x03 == cmdstr[cmdlen - 3]) &&
                (0x02 == cmdstr[cmdlen - 2]) && (0x01 == cmdstr[cmdlen - 1]))
            {
                aiio_radar_cmd_process((uint8_t *)cmdstr, cmdlen);
                
                cmdlen = 0;
            }
        }
    }
}

int32_t aiio_radar_at_run(void)
{
    int32_t ret;

    aiio_os_mutex_create(&uart_mutex_s);

    ret = aiio_os_thread_create(NULL, "radar_at", radar_at_task, AIIO_AT_TASK_STACK_SIZE, NULL, AIIO_AT_TASK_PRIO);
    if (ret != AIIO_OK) {
        return ret;
    }

    return AIIO_OK;
}

int32_t aiio_at_radar_cmd_regist(void)
{
    return aiio_at_register_cmd(aiio_at_radar_cmd, sizeof(aiio_at_radar_cmd) / sizeof(aiio_at_radar_cmd[0]));
}

static int32_t aiio_at_radar_cmd_mode_get(uint8_t *mode)
{
    uint8_t cmd_mode;
    size_t len = 0;

    aiio_nvs_get_blob(RADAR_AT_MODE_KET, &cmd_mode, 1, &len);
    if (len > 0)
    {
        if (cmd_mode == 0 || cmd_mode == 1)
        {
            *mode = cmd_mode;

            return AIIO_OK;
        }
    }

    return AIIO_ERROR;
}

static int32_t aiio_at_radar_cmd_mode_set(uint8_t mode)
{
    uint8_t cmd_mode = mode;
    aiio_nvs_err_code_t ret;

    ret = aiio_nvs_set_blob(RADAR_AT_MODE_KET, &cmd_mode, 1);
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set cmd mode failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

void radar_load_nv_param(void)
{
    if (aiio_at_radar_cmd_mode_get(&radar_at_mode_s) != AIIO_OK)
    {
        aiio_at_radar_cmd_mode_set(radar_at_mode_s);
    }
}
