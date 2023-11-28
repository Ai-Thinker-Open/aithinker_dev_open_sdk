#include "ln_at_mqtt_topic.h"
#include "usr_ctrl.h"

static mqtt_topic_list_ctrl_t mqtt_topic_list_ctrl = {0,};

__STATIC_INLINE__ int mqtt_topic_list_init(void)
{
    mqtt_topic_list_ctrl_t *list_ctrl = &mqtt_topic_list_ctrl;
    
    memset(list_ctrl, 0, sizeof(mqtt_topic_list_ctrl_t));
    ln_list_init(&list_ctrl->list);
	list_ctrl->enable = LN_TRUE;
    
    if (OS_OK != OS_MutexCreate(&list_ctrl->lock)) {
        return LN_FAIL;
    }
    return LN_OK;
}

void ln_mqtt_topic_init(void)
{
	mqtt_topic_list_init();
}

void ln_mqtt_topic_list_cleanup(void)
{
    mqtt_topic_list_ctrl_t *list_ctrl = &mqtt_topic_list_ctrl;
    mqtt_topic_node_t *pnode, *tmp;

    OS_MutexLock(&list_ctrl->lock, OS_WAIT_FOREVER);
    LN_LIST_FOR_EACH_ENTRY_SAFE(pnode, tmp, mqtt_topic_node_t, list, &list_ctrl->list) {
        ln_list_rm(&pnode->list);
        OS_Free(pnode);
        list_ctrl->node_count--;
    }

    OS_MutexUnlock(&list_ctrl->lock);
}

int ln_mqtt_topic_get_list(ln_list_t **list, uint8_t * node_count)
{
    if (!list || !node_count) {
        return LN_FAIL;
    }
    
    OS_MutexLock(&mqtt_topic_list_ctrl.lock, OS_WAIT_FOREVER);
    mqtt_topic_list_ctrl_t * list_ctrl = &mqtt_topic_list_ctrl;
    *list = &list_ctrl->list;
    *node_count = list_ctrl->node_count;
    OS_MutexUnlock(&mqtt_topic_list_ctrl.lock);
    
    return LN_OK;
}

int ln_mqtt_topic_list_update_enable(int en)
{
    mqtt_topic_list_ctrl_t * list_ctrl = &mqtt_topic_list_ctrl;
    OS_MutexLock(&list_ctrl->lock, OS_WAIT_FOREVER);
    list_ctrl->enable = en;
    OS_MutexUnlock(&list_ctrl->lock);
    return LN_OK;
}

__STATIC_INLINE__ int mqtt_topic_info_list_update(const mqtt_topic_node_t *info)
{
    int ret = LN_MQTT_OK;
    mqtt_topic_list_ctrl_t * list_ctrl = &mqtt_topic_list_ctrl;
    mqtt_topic_node_t *pnode, *tmp, *node_new = NULL;

    OS_MutexLock(&list_ctrl->lock, OS_WAIT_FOREVER);
    if (list_ctrl->enable != LN_TRUE) {
        ret = LN_MQTT_FAIL;
        goto __exit;
    }

    // creat new info node.
    if (NULL == (node_new = OS_Malloc(sizeof(mqtt_topic_node_t)))) {
        ret = LN_MQTT_FAIL;
        goto __exit;
    }
    memcpy(node_new, info, sizeof(mqtt_topic_node_t));

    if (ln_is_list_empty(&list_ctrl->list)) {
        ln_list_add(&list_ctrl->list, &node_new->list);
        list_ctrl->node_count++;
    } else {
        //list full,remove a node.
        if (list_ctrl->node_count >= LN_MQTT_LIST_NODE_MAX) {
            ret = LN_MQTT_FULL;
			OS_Free(node_new);
			goto __exit;
        }

        // lookup the same info node, romve it.
        LN_LIST_FOR_EACH_ENTRY_SAFE(pnode, tmp, mqtt_topic_node_t, list, &list_ctrl->list) 
        {
            if (!strcmp(pnode->topic, info->topic)) {
                ret = LN_MQTT_EXIST;
				OS_Free(node_new);
				goto __exit;
            }
        }

		//add new node
		ln_list_add(&list_ctrl->list, &node_new->list);
        list_ctrl->node_count++;
    }

__exit:
    OS_MutexUnlock(&list_ctrl->lock);
    return ret;
}

int ln_mqtt_topic_list_update(const mqtt_topic_node_t *info)
{
    return mqtt_topic_info_list_update(info);
}

__STATIC_INLINE__ int mqtt_topic_list_rm_node(mqtt_topic_node_t * info)
{
	int ret = LN_MQTT_NOT_EXIST;
    mqtt_topic_list_ctrl_t * list_ctrl = &mqtt_topic_list_ctrl;
    mqtt_topic_node_t *pnode, *tmp;
    
    LN_LIST_FOR_EACH_ENTRY_SAFE(pnode, tmp, mqtt_topic_node_t, list, &list_ctrl->list) {
        if (!strcmp(info->topic, pnode->topic)) {
			info->qos = pnode->qos;
            ln_list_rm(&pnode->list);
            OS_Free(pnode);
            list_ctrl->node_count--;
			ret = LN_MQTT_OK;
        }
    }
	
	return ret;
}

int ln_mqtt_topic_list_rm_node(mqtt_topic_node_t* info)
{
	return mqtt_topic_list_rm_node(info);
}
