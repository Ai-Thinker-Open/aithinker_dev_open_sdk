/*
 * 1. This mqtt demo use tencent iot server, please refer this url to set your own mqtt model
 *    (https://console.cloud.tencent.com/);
 * 2. You can use mqtt.fx to verify your mqtt model firstly. 
 *    (https://cloud.tencent.com/document/product/634/14630)
 * 3. Please refer to this url(https://cloud.tencent.com/document/product/634/32546) to get the 
 *    username & password, and note that these data have to be updated; 
 */
 
#ifndef LN_AT_MQTT_H
#define LN_AT_MQTT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/apps/mqtt.h"
#include <stdbool.h>
#include "ln_at_mqtt_topic.h"

#define MQTT_CLIENT_ID		"admin"
#define MQTT_CLIENT_USR		"admin"
#define MQTT_CLIENT_PAS		"admin"
#define MQTT_TOPIC_TEST		"ln_light_01"
#define MQTT_DEFAULT_PORT	7788

#define LN_MQTT_CLIENT_ID_MAX_LENGTH			256
#define LN_MQTT_USERNAME_MAX_LENGTH				64
#define LN_MQTT_PASSWORD_MAX_LENGTH				64
#define LN_MQTT_PATH_MAX_LENGTH					32
#define LN_MQTT_HOST_MAX_LENGTH					128
#define LN_MQTT_TOPIC_MAX_LENGTH				128
#define LN_MQTT_MSG_MAX_LENGTH					64

#define LN_MQTT_RXBUF_LENGTH					1024

enum{
	LN_MQTT_SCANF_FALSE = 0,
	LN_MQTT_SCANF_CLIENTID,
	LN_MQTT_SCANF_USRNAME,
	LN_MQTT_SCANF_PASSWORD,
	LN_MQTT_SCANF_PUBRAW
};

enum{
	LN_MQTT_UNINIT = 0,
	LN_MQTT_SET_USRCFG,
	LN_MQTT_SET_CONNCFG,
	LN_MQTT_DISCONNECTED,
	LN_MQTT_CONNECTED,
	LN_MQTT_NO_TOPIC,
	LN_MQTT_HAS_TOPIC
};

typedef struct ln_mqtt_data_cfg
{
	char client_id[LN_MQTT_CLIENT_ID_MAX_LENGTH + 1];
	char username[LN_MQTT_USERNAME_MAX_LENGTH + 1];
	char password[LN_MQTT_PASSWORD_MAX_LENGTH + 1];
	char path[LN_MQTT_PATH_MAX_LENGTH + 1];
	
	char rxbuf[LN_MQTT_RXBUF_LENGTH];
	char pubraw[LN_MQTT_RXBUF_LENGTH];
	
	int iScanf;
	int scanfLen;
	int pubrawQos;
	int pubrawRetain;
	char pubrawTopic[LN_MQTT_TOPIC_MAX_LENGTH];
	
	char will_topic[LN_MQTT_TOPIC_MAX_LENGTH];
	char will_msg[LN_MQTT_MSG_MAX_LENGTH];
	struct mqtt_connect_client_info_t mqtt_client_info;
	
	int linkId;
	char mqtt_host[LN_MQTT_HOST_MAX_LENGTH + 1];
	ip_addr_t mqtt_ip;
	int port;
	bool reconnect;
	bool is_usr_cfg_init;
	int state;
	int scheme;
	
	mqtt_client_t* mqtt_client;
	
}ln_mqtt_data_cfg_t;

typedef struct ln_mqtt_cfg
{
	
	ln_mqtt_data_cfg_t mqtt_data;
	
	void (*init)(void);
	
	void (*dis_tmr)(void);
	
	void (*scanf)(char _ch, int is_empty);
	
	void (*topic_init)(void);
	void (*mqtt_cleanup)(void);
	int (*topic_list)(ln_list_t **list, uint8_t * node_count);
	int (*topic_en)(int en);
	int (*topic_update)(const mqtt_topic_node_t *info);
	int (*topic_rm)(mqtt_topic_node_t* pTopic);

    void (*reconnect_cb)(void);
}ln_mqtt_cfg_t;

extern ln_mqtt_cfg_t* ln_get_mqtt_handle(void);

void mqtt_example_init(void);

#ifdef __cplusplus
}
#endif

#endif /* LN_AT_MQTT_H */
