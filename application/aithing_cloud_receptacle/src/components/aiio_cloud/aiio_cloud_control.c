/**
 * @brief   Define the cloud  control the attribute state interface
 *
 * @file    aiio_cloud_control.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note    This is mainly destribing the attribute state that is received from cloud, And the different type is come from different command interface
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-16          <td>1.0.0            <td>zhuolm             <td> The  different command interface of control attribute state.
 */
#include "aiio_adapter_include.h"
#include "aiio_common.h"
#include "aiio_cloud_led.h"
#include "config.h"

uint8_t powerstatus = 1;
uint8_t status = 1;
uint8_t ledstatus = 1;
double powerconsumption = 0;
double electriccurrent = 0;
double voltage = 0;
double electricpower = 0;
extern bool old_led_status_change_enable; /*!< switch variable of led twinkling*/
extern uint8_t old_led_triggle_status;

uint8_t control_led_count = 0;

static void aiio_receive_bool_cmd(char *msgMid, char *from, uint8_t dpid, bool cmd);
static void aiio_receive_string_cmd(char *msgMid, char *from, uint8_t dpid, char *cmd);
static void aiio_receive_value_cmd(char *msgMid, char *from, uint8_t dpid, int value);
static void aiio_receive_array_cmd(char *msgMid, char *from, uint8_t dpid, cJSON *cmd);
static void aiio_receive_object_cmd(char *msgMid, char *from, uint8_t dpid, cJSON *cmd);

// 用于数据解析
void aiio_parse_control_data(char *msgMid, char *from, char *control_data)
{
    cJSON *cjson_root = NULL;
    cJSON *cjson_control = NULL;
    uint16_t cmd_num = 0;
    cJSON *cjson_value = NULL;
    char str[10] = {0};

    if (control_data == NULL)
    {
        aiio_log_e("param err \r\n");
        return;
    }

    aiio_log_d("control_data = %s \r\n", control_data);
    cjson_root = cJSON_Parse(control_data);
    if (cjson_root == NULL)
    {
        aiio_log_e("json parse err \r\n");
        return;
    }

    cjson_control = cJSON_GetObjectItem(cjson_root, PROTOCOL_STR_CONTROL);
    if (cjson_control == NULL)
    {
        aiio_log_e("cjson_control is NULL \r\n");
        cJSON_Delete(cjson_root);
        return;
    }

    if (cjson_control)
    {
        for (cmd_num = 1; cmd_num < 256; cmd_num++) /*  Check the attribute command by polling that is received from cloud, The range of attribute command value is [1 - 255]*/
        {
            snprintf(str, 10, "%d", cmd_num);
            // aiio_log_d("dpid = %s \r\n", str);

            cjson_value = cJSON_GetObjectItem(cjson_control, str);
            if (cjson_value)
            {
                switch (cjson_value->type)
                {
                case cJSON_Number:
                {
                    aiio_log_i("receive value data\r\n");
                    aiio_receive_value_cmd(msgMid, from, cmd_num, cJSON_GetNumberValue(cjson_value));
                }
                break;

                case cJSON_String:
                {
                    aiio_log_i("receive string data\r\n");
                    aiio_receive_string_cmd(msgMid, from, cmd_num, cJSON_GetStringValue(cjson_value));
                }
                break;

                case cJSON_Array:
                {
                    aiio_log_i("receive array data \r\n");
                    aiio_receive_array_cmd(msgMid, from, cmd_num, cjson_value);
                }
                break;

                case cJSON_Object:
                {
                    aiio_log_i("receive object data \r\n");
                    aiio_receive_object_cmd(msgMid, from, cmd_num, cjson_value);
                }
                break;

                case cJSON_True:
                {
                    aiio_log_i("receive bool data \r\n");
                    aiio_receive_bool_cmd(msgMid, from, cmd_num, true);
                }
                break;

                case cJSON_False:
                {
                    aiio_log_i("receive bool data \r\n");
                    aiio_receive_bool_cmd(msgMid, from, cmd_num, false);
                }
                break;

                default:
                    aiio_log_e("can't find cjson type \r\n");
                    break;
                }
            }
        }
    }

    cJSON_Delete(cjson_root);
}

#ifdef CONFIG_CLOUD_CONTROL_LED_MODULE
void aiio_entry_start_pair(void)
{
    aiio_rev_queue_t rev_queue = {0};

    aiio_flash_clear_config_data();
    rev_queue.common_event = REV_CONFIG_START_EVENT;
    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
    {
        aiio_log_e("queue send failed\r\n");
    }
    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
}

#endif

static void aiio_receive_bool_cmd(char *msgMid, char *from, uint8_t dpid, bool cmd)
{
    switch (dpid)
    {
    case CMD_POWERSTATE:
    {
        aiio_log_e("dpid[%d] cmd:%d", dpid, cmd);
        if (cmd)
        {
            // aiio_turn_on_led_status(ALL_LED_TURN_ON);
            // powerstate_status = true;
            powerstatus = 1;
        }
        else
        {
            // aiio_turn_on_led_status(ALL_LED_TURN_OFF);
            // powerstate_status = false;
            powerstatus = 0;
        }
        if (powerstatus == 0)
        {
            old_led_status_change_enable = aiio_cloud_receptacle_led_get_enable();
            old_led_triggle_status = aiio_cloud_receptacle_led_get_triggle();
            // 如果闪烁使能
            aiio_cloud_receptacle_led_enable(false);
            aiio_cloud_receptacle_set_led_status(ALL_LED_TURN_OFF);
        }
        else
        {
            aiio_cloud_receptacle_led_enable(old_led_status_change_enable);
            aiio_cloud_receptacle_set_led_status(old_led_triggle_status);
        }

        // aiio_blue_led_triggle();
        aiio_report_all_attibute_status(NULL, NULL);
        // 上报数据
        // aiio_report_bool_attibute_status(NULL, NULL, CMD_POWERSTATE, powerstatus);
        // aiio_report_bool_attibute_status(msgMid, from, dpid, cmd);
    }
    break;

    default:
        aiio_log_e("can't find dpid[%d]", dpid);
        break;
    }
}

static void aiio_receive_string_cmd(char *msgMid, char *from, uint8_t dpid, char *cmd)
{
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    switch (dpid)
    {
        // case CMD_PLAY_LIST:
        // {
        //     aiio_log_i("receive :%s \r\n", cmd);
        //     aiio_report_string_attibute_status(msgMid, from, dpid, cmd);
        // }
        // break;

    default:
        aiio_log_e("can't find dpid[%d]", dpid);
        break;
    }
#endif
}

static void aiio_receive_value_cmd(char *msgMid, char *from, uint8_t dpid, int value)
{
    switch (dpid)
    {
    case CMD_POWERSTATE:
    {
        aiio_log_i("receive :%d \r\n", value);
        // spray_mode_status = value;
        aiio_report_int_attibute_status(msgMid, from, dpid, value);
    }
    break;

    default:
        aiio_log_e("can't find dpid[%d]", dpid);
        break;
    }
}

static void aiio_receive_array_cmd(char *msgMid, char *from, uint8_t dpid, cJSON *cmd)
{
}

static void aiio_receive_object_cmd(char *msgMid, char *from, uint8_t dpid, cJSON *cmd)
{
}

void aiio_report_bool_attibute_status(char *msgMid, char *from, uint8_t dpid, bool status)
{
    cJSON *json_device = NULL;
    char dpid_str[10] = {0};
    char *json_str = NULL;

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    if (status)
    {
        cJSON_AddTrueToObject(json_device, dpid_str);
    }
    else
    {
        cJSON_AddFalseToObject(json_device, dpid_str);
    }

    json_str = cJSON_PrintUnformatted(json_device);
    if (json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return;
    }
#ifdef CONFIG_WAN_NETWORK_ENABLE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
}

void aiio_report_string_attibute_status(char *msgMid, char *from, uint8_t dpid, char *value)
{
    cJSON *json_device = NULL;
    char dpid_str[10] = {0};
    char *json_str = NULL;

    if (value == NULL)
    {
        aiio_log_e("param err \r\n");
        return;
    }

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddStringToObject(json_device, dpid_str, value);

    json_str = cJSON_PrintUnformatted(json_device);
    if (json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return;
    }
#ifdef CONFIG_WAN_NETWORK_ENABLE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
}

void aiio_report_int_attibute_status(char *msgMid, char *from, uint8_t dpid, int value)
{
    cJSON *json_device = NULL;
    char dpid_str[10] = {0};
    char *json_str = NULL;

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddNumberToObject(json_device, dpid_str, value);

    json_str = cJSON_PrintUnformatted(json_device);
    if (json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return;
    }
#ifdef CONFIG_WAN_NETWORK_ENABLE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
}

static int32_t aiio_packet_json_bool_value(cJSON *json_root, uint8_t dpid, bool value)
{
    char dpid_str[10] = {0};

    if (json_root == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    if (value)
    {
        cJSON_AddTrueToObject(json_root, dpid_str);
    }
    else
    {
        cJSON_AddFalseToObject(json_root, dpid_str);
    }

    return 0;
}

static int32_t aiio_packet_json_int_value(cJSON *json_root, uint8_t dpid, int value)
{
    char dpid_str[10] = {0};

    if (json_root == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }
    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddNumberToObject(json_root, dpid_str, value);

    return 0;
}

static int32_t aiio_packet_json_string_value(cJSON *json_root, uint8_t dpid, char *value)
{
    char dpid_str[10] = {0};

    if (json_root == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }
    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddStringToObject(json_root, dpid_str, value);

    return 0;
}

void aiio_set_powerstate_status(bool status)
{
    // powerstate_status = status;
}

void aiio_report_all_attibute_status(char *msgMid, char *from)
{
    cJSON *json_device = NULL;
    char *json_str = NULL;

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return;
    }

    aiio_packet_json_bool_value(json_device, CMD_POWERSTATE, powerstatus);
    aiio_packet_json_int_value(json_device, CMD_STATUS, status);
    aiio_packet_json_bool_value(json_device, CMD_LEDSTATUS, ledstatus);

    aiio_packet_json_int_value(json_device, CMD_POWERCONSUMPTION, powerconsumption);
    aiio_packet_json_int_value(json_device, CMD_ELECTRICCURRENT, electriccurrent);
    aiio_packet_json_int_value(json_device, CMD_VOLTAGE, voltage);
    aiio_packet_json_int_value(json_device, CMD_ELECTRICPOWER, electricpower);

    json_str = cJSON_PrintUnformatted(json_device);
    if (json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return;
    }
#ifdef CONFIG_WAN_NETWORK_ENABLE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
}
