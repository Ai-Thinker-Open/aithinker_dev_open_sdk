/** @brief      Ai-Thinker configurable network software local control interface
 *
 *  @file       aiio_local_control.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Ai-Thinker configurable network software local control interface, used for NFC swiping and swiping local control
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/24      <td>1.0.0       <td>yanch       <td>define local control API
 *  </table>
 *
 */

#include "aiio_local_control.h"
#include "cJSON.h"
#include "aiio_os_port.h"
#include "aiio_utils_filling.h"
#include "aiio_gpio.h"
#include "aiio_system.h"

#define GPIO_OUT_TEST_PORT  CONFIG_TEST_PORT_OUT
#define GPIO_OUT_TEST_GPIO  CONFIG_TEST_OUT_GPIO

static uint32_t id = 0;
static int led_status = 0;
static int len_init = 0;

static aiio_ret_t aiio_led_set(uint16_t state);

static aiio_ret_t aiio_led_init(void)
{
    aiio_hal_gpio_init(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO);
    aiio_hal_gpio_pin_pull_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO, AIIO_GPIO_PULL_UP);
    aiio_hal_gpio_pin_direction_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO, AIIO_GPIO_OUTPUT);
    aiio_led_set(0);
    return AIIO_OK;
}

static aiio_ret_t aiio_led_set(uint16_t state)
{
    led_status = state;
    aiio_hal_gpio_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO,led_status);
    return AIIO_OK;
}

static aiio_ret_t aiio_led_get(uint16_t *state)
{
    *state = led_status;
    return AIIO_OK;
}

CORE_API aiio_ret_t aiio_local_control_protocol_analysis(udp_socket_client_t *udp_client, uint8_t *data, uint16_t len)
{
    if (!len_init)
    {
        len_init = 1;
        aiio_led_init();
    }

    cJSON_Hooks memoryHook;

    memoryHook.malloc_fn = (void *(*)(size_t))aiio_os_malloc;
    memoryHook.free_fn = (void (*)(void *))aiio_os_free;

    cJSON_InitHooks(&memoryHook);

    cJSON *root = cJSON_Parse((char *)data);
    if (!root)
    {
        aiio_log_e("Parsing JSON Error: [%s]", cJSON_GetErrorPtr());

        return AIIO_ERROR;
    }

    cJSON *method_field = cJSON_GetObjectItem(root, "method");
    if (method_field == NULL || !cJSON_IsString(method_field))
    {
        aiio_log_w("method field does not exist or has the wrong type");
        cJSON_Delete(root);
        return AIIO_ERROR;
    }
    const char *method = method_field->valuestring;

    cJSON *id_field = cJSON_GetObjectItem(root, "id");
    if (id_field == NULL || !cJSON_IsNumber(id_field))
    {
        aiio_log_w("id field does not exist or has the wrong type");
        cJSON_Delete(root);
        return AIIO_ERROR;
    }
    id = id_field->valueint;

    if (0 == strcmp("control", method))
    {
        cJSON *params_field = cJSON_GetObjectItem(root, "params");
        if (params_field == NULL || !cJSON_IsObject(params_field))
        {
            aiio_log_w("params field does not exist or has the wrong type");
            cJSON_Delete(root);
            return AIIO_ERROR;
        }
        cJSON *switch_field = cJSON_GetObjectItem(params_field, "switch");
        if (switch_field == NULL || !cJSON_IsNumber(switch_field))
        {
            aiio_log_w("params.switch field does not exist or has the wrong type");
            cJSON_Delete(root);
            return AIIO_ERROR;
        }
        int switch_value = switch_field->valueint;

        aiio_log_i("id=%d, method=%s, params.switch=%d", id, method, switch_value);

        int led_status = 0;
        aiio_led_set(switch_value);
        aiio_led_get(&led_status);
        if (led_status == switch_value)
        {
            aiio_local_control_send_response_control(udp_client, 0);
        }
        else
        {
            aiio_local_control_send_response_control(udp_client, -1);
        }
    }
    else if (0 == strcmp("get_status", method))
    {
        cJSON *type_field = cJSON_GetObjectItem(root, "type");
        if (type_field == NULL || !cJSON_IsString(type_field))
        {
            aiio_log_w("params field does not exist or has the wrong type");
            cJSON_Delete(root);
            return AIIO_ERROR;
        }
        const char *type = type_field->valuestring;

        if (0 == strcmp("report", type))
        {
            int led_value = 0;
            aiio_led_get(&led_value);
            aiio_local_control_send_device_status(udp_client, led_value);
        }

        aiio_log_i("id=%d, method=%s, type=%s", id, method, type);
    }
    else if (0 == strcmp("unbind_device", method))
    {
        aiio_local_control_send_unbind_results(udp_client, 0);
        aiio_restart();
    }
    else
    {
        aiio_log_w("wrong command");
    }

    cJSON_Delete(root);

    return 0;
}

CORE_API aiio_ret_t aiio_local_control_send_response_control(udp_socket_client_t *udp_client, uint8_t result)
{
    cJSON_Hooks memoryHook;

    memoryHook.malloc_fn = (void *(*)(size_t))aiio_os_malloc;
    memoryHook.free_fn = (void (*)(void *))aiio_os_free;

    cJSON_InitHooks(&memoryHook);

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        aiio_log_w("Error creating cJSON object.");
        return AIIO_ERROR;
    }

    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddStringToObject(root, "method", "control_reply");
    cJSON_AddNumberToObject(root, "code", result);

    if (result != 0)
    {
        cJSON_AddStringToObject(root, "status", "control failure");
    }

    char *json_str = cJSON_PrintUnformatted(root);

    aiio_log_d("%s", json_str);

    uint8_t array[1024] = {0};
    strcpy((char *)array, json_str);
    pkcs7_padding((char *)array, sizeof(array), strlen((char *)array));
    uint16_t len = 0;
    uint8_t encryption_data[1024] = {0};

    aiio_encryption_app_data(array, strlen((char *)array), encryption_data, &len);

    aiio_udp_write(udp_client, (uint8_t *)encryption_data, len);

    cJSON_Delete(root);
    aiio_os_free(json_str);

    return AIIO_OK;
}

CORE_API aiio_ret_t aiio_local_control_send_device_status(udp_socket_client_t *udp_client, uint8_t status)
{
    cJSON_Hooks memoryHook;

    memoryHook.malloc_fn = (void *(*)(size_t))aiio_os_malloc;
    memoryHook.free_fn = (void (*)(void *))aiio_os_free;

    cJSON_InitHooks(&memoryHook);

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        aiio_log_w("Error creating cJSON object.");
        return 1;
    }

    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddStringToObject(root, "method", "get_status_reply");
    cJSON_AddNumberToObject(root, "code", 0);

    cJSON *params = cJSON_CreateObject();
    if (params == NULL)
    {
        printf("Error creating cJSON object.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON *reported = cJSON_CreateObject();
    if (reported == NULL)
    {
        printf("Error creating cJSON object.");
        cJSON_Delete(root);
        cJSON_Delete(params);
        return 1;
    }

    cJSON_AddNumberToObject(reported, "switch", status);
    cJSON_AddItemToObject(params, "reported", reported);
    cJSON_AddItemToObject(root, "params", params);

    char *json_str = cJSON_PrintUnformatted(root);
    aiio_log_d("%s", json_str);

    uint8_t array[1024] = {0};
    uint16_t len = 0;
    uint8_t encryption_data[1024] = {0};

    strcpy((char *)array, json_str);
    pkcs7_padding((char *)array, sizeof(array), strlen((char *)array));
    aiio_encryption_app_data(array, strlen((char *)array), encryption_data, &len);

    aiio_udp_write(udp_client, (uint8_t *)encryption_data, len);

    cJSON_Delete(root);
    aiio_os_free(json_str);

    return AIIO_OK;
}

CORE_API aiio_ret_t aiio_local_control_send_unbind_results(udp_socket_client_t *udp_client, uint8_t result)
{

    cJSON_Hooks memoryHook;

    memoryHook.malloc_fn = (void *(*)(size_t))aiio_os_malloc;
    memoryHook.free_fn = (void (*)(void *))aiio_os_free;

    cJSON_InitHooks(&memoryHook);

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        aiio_log_w("Error creating cJSON object.");
        return AIIO_ERROR;
    }

    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddStringToObject(root, "method", "unbind_device");
    cJSON_AddNumberToObject(root, "code", result);

    if (result != 0)
    {
        cJSON_AddStringToObject(root, "status", "unbind failure");
    }

    char *json_str = cJSON_PrintUnformatted(root);

    aiio_log_d("%s", json_str);

    uint8_t array[1024] = {0};
    uint16_t len = 0;
    uint8_t encryption_data[1024] = {0};

    strcpy((char *)array, json_str);
    pkcs7_padding((char *)array, sizeof(array), strlen((char *)array));
    aiio_encryption_app_data(array, strlen((char *)array), encryption_data, &len);

    aiio_udp_write(udp_client, (uint8_t *)encryption_data, len);

    cJSON_Delete(root);
    aiio_os_free(json_str);

    return AIIO_OK;
}
