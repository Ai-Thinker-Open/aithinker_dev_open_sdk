#ifndef _AXK_MQTT_H_
#define _AXK_MQTT_H_

#include <stdint.h>
#include <stdio.h>

typedef struct axk_mqtt_info
{
    uint8_t uri[128];
    uint8_t user_name[64];
    uint8_t password[64];
    // uint8_t client_id[64];
    uint8_t flashlight_topic[128];
} axk_mqtt_info_t;

typedef struct axk_mqtt
{
    int32_t (*init)(void);
    int32_t (*deinit)(void);
    int32_t (*modify_uri)(uint8_t *uri);
    int32_t (*modify_username)(uint8_t *username);
    int32_t (*modify_password)(uint8_t *password);
    int32_t (*modify_client_id)(uint8_t *id);
    int32_t (*modify_user_topic)(uint8_t *topic);
    uint8_t (*get_start_flag)(void);
    uint8_t (*get_info_modify_flag)(void);
    void (*set_info_modify_flag)(uint8_t ctrl);
    void (*set_info_default)(void);
    axk_mqtt_info_t *(*get_conn_info)(void);
} axk_mqtt_t;

typedef struct axk_mqtt_publish
{
    /*Currently only link ID 0 is supported */
    // uint8_t link_id;
    /*Topic, maximum length: 128 bytes.*/
    // char topic[128];
    int buff_index;
    /*Mqtt message length. The maximum length of different esp32-c3 devices is limited by available memory*/
    int length;
    /* QoS. The parameters can be 0, 1 or 2. The default value is 0*/
    // uint8_t qos;
    /* retain. The parameter can be 0 or 1. The default value is 0.*/
    // uint8_t retain;
    /* data buffer */
    uint8_t* buff;
    /* used to record the current position */
    uint16_t pos;
} axk_mqtt_publish_t;

int at_setup_mqtt_rawpub_cmd(uint32_t argc, const char** argv);
int axk_setup_mqtt_sub_cmd(uint32_t argc, const char** argv);
int axk_setup_mqtt_pub_cmd(uint32_t argc, const char** argv);

extern axk_mqtt_t g_axk_mqtt;

#endif //_AXK_MQTT_H_
