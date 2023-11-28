#ifndef LN_AT_MQTT_TOPIC_H
#define LN_AT_MQTT_TOPIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "osal/osal.h"
#include "utils/ln_list.h"

#define LN_MQTT_LIST_NODE_MAX				10
#define LN_MQTT_TOPIC_MAX_LENGTH			128

enum
{
	LN_MQTT_OK = 0,
	LN_MQTT_FAIL,
	LN_MQTT_EXIST,
	LN_MQTT_NOT_EXIST,
	LN_MQTT_FULL
};




typedef struct {
    ln_list_t       list;
	char 			topic[LN_MQTT_TOPIC_MAX_LENGTH + 1];
	int 			qos;
} mqtt_topic_node_t;

typedef struct {
    ln_list_t       list;
	uint8_t         node_count;
	
	OS_Mutex_t      lock;
	int             enable;
} mqtt_topic_list_ctrl_t;

extern void ln_mqtt_topic_init(void);
extern void ln_mqtt_topic_list_cleanup(void);
extern int ln_mqtt_topic_get_list(ln_list_t **list, uint8_t * node_count);
extern int ln_mqtt_topic_list_update_enable(int en);
extern int ln_mqtt_topic_list_update(const mqtt_topic_node_t *info);
extern int ln_mqtt_topic_list_rm_node(mqtt_topic_node_t* pTopic);


#ifdef __cplusplus
}
#endif

#endif /* LN_AT_MQTT_TOPIC_H */
