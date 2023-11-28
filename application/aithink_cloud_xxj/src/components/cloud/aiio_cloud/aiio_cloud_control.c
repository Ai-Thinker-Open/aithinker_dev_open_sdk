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
#include "aiio_cloud_control.h"
#include "aiio_cloud_led.h"
#include "aiio_log.h"
#include "aiio_wan_service.h"
#include "config.h"
#include "aiio_common.h"
#include "gui_guider.h"

static void aiio_receive_bool_cmd(char *msgMid, char *from, uint8_t dpid, bool cmd);
static void aiio_receive_string_cmd(char *msgMid, char *from, uint8_t dpid, char *cmd);
static void aiio_receive_value_cmd(char *msgMid, char *from, uint8_t dpid, int value);
static void aiio_receive_array_cmd(char *msgMid, char *from, uint8_t dpid, cJSON *cmd);
static void aiio_receive_object_cmd(char *msgMid, char *from, uint8_t dpid, cJSON *cmd);



void aiio_parse_control_data(char *msgMid, char *from, char *control_data)
{
    cJSON *cjson_root   = NULL;
    cJSON *cjson_control = NULL;
    uint16_t cmd_num = 0;
    cJSON *cjson_value   = NULL;
    char  str[10] = {0};

    if(control_data == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    aiio_log_d("control_data = %s \r\n", control_data);
    cjson_root = cJSON_Parse(control_data);
    if(cjson_root == NULL)
    {
        aiio_log_e("json parse err \r\n");
        return ;
    }

    cjson_control = cJSON_GetObjectItem(cjson_root, PROTOCOL_STR_CONTROL);
    if(cjson_control == NULL)
    {
        aiio_log_e("cjson_control is NULL \r\n");
        cJSON_Delete(cjson_root);
        return ;
    }

    if(cjson_control)
    {
        for(cmd_num = 1; cmd_num < 256; cmd_num++)                              /*  Check the attribute command by polling that is received from cloud, The range of attribute command value is [1 - 255]*/
        {
            snprintf(str, 10, "%d", cmd_num);
            // aiio_log_d("dpid = %s \r\n", str);

            cjson_value = cJSON_GetObjectItem(cjson_control, str);
            if(cjson_value)
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


static void aiio_receive_string_cmd(char *msgMid, char *from, uint8_t dpid, char *cmd)
{
    switch (dpid)
    {
        default:
            aiio_log_e("can't find dpid[%d]", dpid);
            break;
    }
}


static void aiio_receive_value_cmd(char *msgMid, char *from, uint8_t dpid, int value)
{
    switch (dpid)
    {
        case CMD_SPRAY_MODE:
        {
            aiio_log_i("receive :%d \r\n", value);
            Fragrance.spray_size = value;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
            level_color_screen(Fragrance.spray_size);
            aiio_report_int_attibute_status(msgMid, from, dpid, value);
        }
        break;
        case CMD_BOTANY_MODE:
        {
            aiio_log_i("receive :%d \r\n", value);
            Fragrance.work_mode = value;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
            mode_color_screen(Fragrance.work_mode);
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

static void aiio_receive_bool_cmd(char *msgMid, char *from, uint8_t dpid, bool cmd)
{
    switch (dpid)
    {
        case CMD_POWERSTATE:
        {
            if(cmd)
            {
                Fragrance.switch_control = 1;
                aiio_pwm_control(Fragrance.spray_size);
                aiio_flash_save_device_params(&Fragrance);
                lv_obj_add_state(guider_ui.screen_2_sw_1, LV_STATE_CHECKED);
            }
            else
            {
                Fragrance.switch_control = 0;
                aiio_pwm_control(XXJ_SIZE_OFF);
                aiio_flash_save_device_params(&Fragrance);
                lv_obj_clear_state(guider_ui.screen_2_sw_1, LV_STATE_CHECKED);
            }
            aiio_report_bool_attibute_status(msgMid, from, dpid, cmd);
        }
        break;
        default:
            aiio_log_e("can't find dpid[%d]", dpid);
            break;
    }
}


void aiio_report_bool_attibute_status(char *msgMid, char *from, uint8_t dpid, bool status)
{
    cJSON  *json_device = NULL;
    char dpid_str[10] = {0};
    char  *json_str = NULL;

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return ;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    if(status)
    {
        cJSON_AddTrueToObject(json_device, dpid_str);
    }
    else
    {
        cJSON_AddFalseToObject(json_device, dpid_str);
    }

    json_str = cJSON_PrintUnformatted(json_device);
    if(json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return ;
    }
#ifdef CONFIG_WAN_NETWORK_MODULE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
}


void aiio_report_string_attibute_status(char *msgMid, char *from, uint8_t dpid, char* value)
{
    cJSON  *json_device = NULL;
    char dpid_str[10] = {0};
    char  *json_str = NULL;

    if(value == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return ;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddStringToObject(json_device, dpid_str, value);

    json_str = cJSON_PrintUnformatted(json_device);
    if(json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return ;
    }
#ifdef CONFIG_WAN_NETWORK_MODULE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
}

void aiio_report_int_attibute_status(char *msgMid, char *from, uint8_t dpid, int value)
{
    cJSON  *json_device = NULL;
    char dpid_str[10] = {0};
    char  *json_str = NULL;

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return ;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddNumberToObject(json_device, dpid_str, value);

    json_str = cJSON_PrintUnformatted(json_device);
    if(json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return ;
    }
#ifdef CONFIG_WAN_NETWORK_MODULE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
}


static int32_t aiio_packet_json_bool_value(cJSON * json_root, uint8_t dpid, bool value)
{
    char dpid_str[10] = {0};

    if(json_root == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }

    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    if(value)
    {
        cJSON_AddTrueToObject(json_root, dpid_str);
    }
    else
    {
        cJSON_AddFalseToObject(json_root, dpid_str);
    }

    return 0;
}


static int32_t aiio_packet_json_int_value(cJSON * json_root, uint8_t dpid, int value)
{
    char dpid_str[10] = {0};

    if(json_root == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }
    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddNumberToObject(json_root, dpid_str, value);

    return 0;
}


static int32_t aiio_packet_json_string_value(cJSON * json_root, uint8_t dpid, char* value)
{
    char dpid_str[10] = {0};

    if(json_root == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }
    snprintf(dpid_str, sizeof(dpid_str), "%d", dpid);

    cJSON_AddStringToObject(json_root, dpid_str, value);

    return 0;
}


void aiio_report_all_attibute_status(char *msgMid, char *from)
{
    cJSON  *json_device = NULL;
    char  *json_str = NULL;

    json_device = cJSON_CreateObject();
    if (!json_device)
    {
        aiio_log_e("json create object fail \r\n");
        return ;
    }

    aiio_packet_json_bool_value(json_device, CMD_POWERSTATE, Fragrance.switch_control);
    aiio_packet_json_int_value(json_device, CMD_SPRAY_MODE, Fragrance.spray_size);
    aiio_packet_json_int_value(json_device, CMD_BOTANY_MODE, Fragrance.work_mode);

    json_str = cJSON_PrintUnformatted(json_device);
    if(json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_device);
        return ;
    }
#ifdef CONFIG_WAN_NETWORK_MODULE
    aiio_service_report_attribute(msgMid, from, json_str);
#endif
    cJSON_Delete(json_device);
    cJSON_free(json_str);
    
}
