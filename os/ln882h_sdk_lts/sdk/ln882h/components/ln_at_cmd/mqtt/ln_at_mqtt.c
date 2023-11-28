#include "ln_at_mqtt.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "ln_at.h"
#include "ln_utils.h"
#include "netif/ethernetif.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"


#include "usr_ctrl.h"

static OS_Timer_t s_disScanTmr;

static void ln_mqtt_init(void);
static void ln_mqtt_cleanup(void);
static void ln_mqtt_disconnect_tmr(void);
void ln_mqtt_scanf_data(char _ch, int is_empty);

static void ln_mqtt_reconnect(void);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);


static ln_mqtt_cfg_t s_mqttCfg = {	
	.init = ln_mqtt_init,
	
	.scanf = ln_mqtt_scanf_data,
	
	.dis_tmr = ln_mqtt_disconnect_tmr,
	
	.topic_init = ln_mqtt_topic_init,
	.mqtt_cleanup = ln_mqtt_cleanup,
	.topic_list = ln_mqtt_topic_get_list,
	.topic_en = ln_mqtt_topic_list_update_enable,
	.topic_update = ln_mqtt_topic_list_update,
	.topic_rm = ln_mqtt_topic_list_rm_node,
	.reconnect_cb = ln_mqtt_reconnect,
};

ln_mqtt_cfg_t* ln_get_mqtt_handle()
{
	return &s_mqttCfg;
}

static void ln_mqtt_data_init(void)
{
	memset(&ln_get_mqtt_handle()->mqtt_data, 0, sizeof(ln_mqtt_data_cfg_t));
    
	ln_get_mqtt_handle()->mqtt_data.iScanf = LN_MQTT_SCANF_FALSE;
	ln_get_mqtt_handle()->mqtt_data.mqtt_client = mqtt_client_new();
    if(ln_get_mqtt_handle()->mqtt_data.reconnect)
        ln_get_mqtt_handle()->reconnect_cb = ln_mqtt_reconnect;
    else
        ln_get_mqtt_handle()->reconnect_cb = NULL;
	
	struct mqtt_connect_client_info_t* pMqttInfo = &ln_get_mqtt_handle()->mqtt_data.mqtt_client_info;
	pMqttInfo->client_id = ln_get_mqtt_handle()->mqtt_data.client_id;
	pMqttInfo->client_pass = ln_get_mqtt_handle()->mqtt_data.password;
	pMqttInfo->client_user = ln_get_mqtt_handle()->mqtt_data.username;
	pMqttInfo->will_topic = ln_get_mqtt_handle()->mqtt_data.will_topic;
	pMqttInfo->will_msg = ln_get_mqtt_handle()->mqtt_data.will_msg;	
}

void ln_mqtt_init(void)
{
	ln_get_mqtt_handle()->topic_init();
	ln_mqtt_data_init();
}

static void ln_mqtt_reconnect(void)
{
    err_t ret = ERR_OK;
    ln_mqtt_cfg_t *pcfg = ln_get_mqtt_handle();
    ret = mqtt_client_connect(pcfg->mqtt_data.mqtt_client, \
        &pcfg->mqtt_data.mqtt_ip, \
        pcfg->mqtt_data.port, \
        mqtt_connection_cb, \
        LWIP_CONST_CAST(void*, &pcfg->mqtt_data.mqtt_client_info), \
        &pcfg->mqtt_data.mqtt_client_info);
    
    if (ret != ERR_OK)
    {
        Log_e("mqtt_client_connect return %d", ret);
    }

    mqtt_set_inpub_callback(pcfg->mqtt_data.mqtt_client, \
    mqtt_incoming_publish_cb, \
    mqtt_incoming_data_cb, \
    LWIP_CONST_CAST(void*, &pcfg->mqtt_data.mqtt_client_info));	

    pcfg->dis_tmr();
}
static void ln_mqtt_cleanup(void)
{
	mqtt_client_free(ln_get_mqtt_handle()->mqtt_data.mqtt_client);
	ln_mqtt_topic_list_cleanup();
	ln_mqtt_data_init();
}

static void ln_mqtt_dis_scan_tmr_cb(void *arg)
{
    static bool first_disconn = false;
    ln_mqtt_cfg_t *pcfg = ln_get_mqtt_handle();
    
	if (mqtt_client_is_connected(ln_get_mqtt_handle()->mqtt_data.mqtt_client))
	{
        first_disconn = false;
		return;
	}
	
	OS_TimerStop(&s_disScanTmr);
	OS_TimerDelete(&s_disScanTmr);
	if (ln_get_mqtt_handle()->mqtt_data.mqtt_host[0] == 0)
	{
		return;
	}
    
    if(first_disconn == false)
    {
        first_disconn = true;
        ln_at_printf("+MQTTDISCONNECTED:0\r\n");
    }
        
	ln_get_mqtt_handle()->mqtt_data.state = LN_MQTT_DISCONNECTED;

    if(pcfg->mqtt_data.reconnect)
    {
        pcfg->reconnect_cb();
    }
    
}

static void ln_mqtt_disconnect_tmr(void)
{
	Log_i("start %s timer ... ", __func__);
	ln_start_tmr(&s_disScanTmr, OS_TIMER_PERIODIC, ln_mqtt_dis_scan_tmr_cb, NULL, 1000);
	Log_i("start %s timer ... suc!", __func__);
}

static void 
mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
	LWIP_UNUSED_ARG(data);

	Log_i("MQTT client \"%s\" data cb: len %d, flags %d", client_info->client_id, (int)len, (int)flags);
	
	memset(p->mqtt_data.rxbuf, 0, LN_MQTT_RXBUF_LENGTH);
	memcpy(p->mqtt_data.rxbuf, data, len);
	//Log_i("%s", p->mqtt_data.rxbuf);
	hexdump(LOG_LVL_INFO, "MQTT INCOMING", p->mqtt_data.rxbuf, len);
	ln_at_printf(",%d,", len);
	ln_at_write(p->mqtt_data.rxbuf, len);
	ln_at_printf("\r\n");
}

static void
mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
	const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;

	Log_i("MQTT client \"%s\" publish cb: topic %s, len %d", client_info->client_id,
          topic, (int)tot_len);
		
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	ln_at_printf("+MQTTSUBRECV:%d,\"%s\"", p->mqtt_data.linkId, topic);
}

static void
mqtt_request_cb(void *arg, err_t err)
{
	const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;

	Log_i("MQTT client \"%s\" request cb: err %d", client_info->client_id, (int)err);
}

static void
mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
	LWIP_UNUSED_ARG(client);

	Log_i("MQTT client \"%s\" connection cb: status %d", client_info->client_id, (int)status);
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	p->mqtt_data.state = LN_MQTT_CONNECTED;
	if (status == MQTT_CONNECT_ACCEPTED) {
		ln_at_printf("+MQTTCONNECTED:%d,%d,\"%s\",\"%d\",\"%s\",%d\r\n"
				, p->mqtt_data.linkId
				, p->mqtt_data.scheme
				, p->mqtt_data.mqtt_host
				, p->mqtt_data.port
				, p->mqtt_data.path
				, p->mqtt_data.reconnect);
		
		ln_at_printf(LN_AT_RET_OK_STR);
	}
	else if (status == MQTT_CONNECT_DISCONNECTED){
        p->mqtt_data.state = LN_MQTT_DISCONNECTED;
        //ln_at_printf("+MQTTDISCONNECTED:0\r\n");
	}
}

ln_at_err_t ln_mqtt_at_mqttconn_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	char* pHost = NULL;
	ip_addr_t dnsaddr;
	bool bRes = false;
	err_t ret = ERR_OK;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	//check connect stat
    if(NETDEV_LINK_UP != netdev_get_link_state(netdev_get_active())){
        goto __err;
    }
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	p->mqtt_data.linkId = int_param;
	
	//host
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pHost))
    {
        goto __err;
    }

    if (is_default || !pHost)
    {
        goto __err;
    }
	
    for (size_t i = 0; i < 5; i++)
    {
        if (dns_gethostbyname((char *)pHost, &(dnsaddr), NULL, NULL) == ERR_OK)
        {
            bRes = true;
            break;
        }
        OS_MsDelay(500);
    }
	
	if (!bRes)
	{
		goto __err;
	}
	strcpy(p->mqtt_data.mqtt_host, pHost);
	
	//port
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 0 || int_param > 65535)
	{
		goto __err;
	}
	p->mqtt_data.port = int_param;

	//reconnect
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0 && int_param != 1)
	{
		goto __err;
	}
	p->mqtt_data.reconnect = int_param;
    //reconnect cb
    if(p->mqtt_data.reconnect)
        p->reconnect_cb = ln_mqtt_reconnect;
    else
        p->reconnect_cb = NULL;
    
	//if default para
	if (p->mqtt_data.mqtt_client_info.keep_alive == 0)
	{
		p->mqtt_data.mqtt_client_info.keep_alive = 120;
		p->mqtt_data.mqtt_client_info.will_msg = NULL;
		p->mqtt_data.mqtt_client_info.will_topic = NULL;
		p->mqtt_data.mqtt_client_info.will_qos = 0;
		p->mqtt_data.mqtt_client_info.will_retain = 0;
	}
	
	//connect
	//inet_aton(p->mqtt_data.mqtt_host, &p->mqtt_data.mqtt_ip);
	memcpy(&p->mqtt_data.mqtt_ip, &dnsaddr, sizeof(ip_addr_t));
	Log_i("host ip   : %s %d", p->mqtt_data.mqtt_host, p->mqtt_data.mqtt_ip.addr);
	Log_i("host port : %d", p->mqtt_data.port);
	Log_i("client id : %s", p->mqtt_data.client_id);
	Log_i("client usr: %s", p->mqtt_data.username);
	Log_i("client pwd: %s", p->mqtt_data.password);
	
	ret = mqtt_client_connect(p->mqtt_data.mqtt_client,
						&p->mqtt_data.mqtt_ip, 
						p->mqtt_data.port,
						mqtt_connection_cb, 
						LWIP_CONST_CAST(void*, &p->mqtt_data.mqtt_client_info),
						&p->mqtt_data.mqtt_client_info);
						
	if (ret != ERR_OK)
	{
		Log_e("mqtt_client_connect return %d", ret);
		goto __err;
	}
		  
	mqtt_set_inpub_callback(p->mqtt_data.mqtt_client,
							mqtt_incoming_publish_cb,
							mqtt_incoming_data_cb,
							LWIP_CONST_CAST(void*, &p->mqtt_data.mqtt_client_info));	
	
	p->dis_tmr();
//    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_mqtt_at_mqttconn_get(const char *name)
{
	char strPort[12] = {0};
    ln_mqtt_cfg_t* p = ln_get_mqtt_handle();

	if (0 != p->mqtt_data.port)
	{
		sprintf(strPort, "%d", p->mqtt_data.port);
	}
	ln_at_printf("+MQTTCONN:%d,%d,%d,\"%s\",\"%s\",\"%s\",%d\r\n",
							p->mqtt_data.linkId,
							p->mqtt_data.state,
							p->mqtt_data.scheme,
							p->mqtt_data.mqtt_host,
							strPort,
							p->mqtt_data.path,
							p->mqtt_data.reconnect);
	
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(MQTTCONN, ln_mqtt_at_mqttconn_get, ln_mqtt_at_mqttconn_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqttclean_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	//check connect stat
    if(NETDEV_LINK_UP != netdev_get_link_state(netdev_get_active())){
        goto __err;
    }
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	//error if cleanup
	if (p->mqtt_data.mqtt_host[0] == 0)
	{
		goto __err;
	}
	
	//disconnect
	mqtt_disconnect(p->mqtt_data.mqtt_client);
	p->mqtt_cleanup();
	//p->mqtt_data.state = LN_MQTT_DISCONNECTED;
	
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(MQTTCLEAN, NULL, ln_mqtt_at_mqttclean_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqttsub_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	char *pTopic = NULL;
	mqtt_topic_node_t node;
	int ret = LN_MQTT_OK;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	//check connect stat
    if(NETDEV_LINK_UP != netdev_get_link_state(netdev_get_active())){
        goto __err;
    }
	
	//check stat
	if (p->mqtt_data.state < LN_MQTT_CONNECTED)
	{
		goto __err;
	}
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	//topic
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pTopic))
    {
        goto __err;
    }
	
	if (is_default || !pTopic)
    {
        goto __err;
    }
	
	//qos
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	node.qos = int_param;
	strcpy(node.topic, pTopic);
	
	//update
	ret = p->topic_update(&node);
	if (ret == LN_OK)
	{
		mqtt_sub_unsub(p->mqtt_data.mqtt_client,
				node.topic, node.qos,
				mqtt_request_cb, LWIP_CONST_CAST(void*, &p->mqtt_data.mqtt_client_info),
				1);
				
		p->mqtt_data.state = LN_MQTT_HAS_TOPIC;
	}
	else if (ret == LN_MQTT_EXIST)
	{
		ln_at_printf("ALREADY SUBSCRIBE\r\n");
		return LN_AT_ERR_NONE;
	}
	else
	{
		goto __err;
	}
	
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_mqtt_at_mqttsub_get(const char *name)
{
    ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	ln_list_t *list;
    uint8_t node_count = 0;
    mqtt_topic_node_t *pnode;
	
	p->topic_en(LN_FALSE);
	
	// 1.get ap info list.
    p->topic_list(&list, &node_count);
	
	if (node_count == 0)
	{
		ln_at_printf("+MQTTSUB:0,0,\"\",0\r\n");
	}
	else
	{
		// 2.print all ap info in the list.
		LN_LIST_FOR_EACH_ENTRY(pnode, mqtt_topic_node_t, list, list)
		{
			ln_at_printf("+MQTTSUB:%d,%d,\"%s\",%d\r\n",
								p->mqtt_data.linkId,
								p->mqtt_data.state,
								pnode->topic,
								pnode->qos);
		}
	}
	
	p->topic_en(LN_TRUE);
	
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(MQTTSUB, ln_mqtt_at_mqttsub_get, ln_mqtt_at_mqttsub_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqttunsub_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	char *pTopic = NULL;
	mqtt_topic_node_t node;
	int ret = LN_MQTT_OK;
	
	ln_list_t *list;
	uint8_t node_count = 0;

	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	//check connect stat
    if(NETDEV_LINK_UP != netdev_get_link_state(netdev_get_active())){
        goto __err;
    }
	
	//check stat
	if (p->mqtt_data.state < LN_MQTT_CONNECTED)
	{
		goto __err;
	}
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	//topic
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pTopic))
    {
        goto __err;
    }
	
	if (is_default || !pTopic)
    {
        goto __err;
    }
	
	//rm node
	strcpy(node.topic, pTopic);
	ret = p->topic_rm(&node);
	if (ret == LN_MQTT_OK)
	{
		mqtt_sub_unsub(p->mqtt_data.mqtt_client,
				node.topic, node.qos,
				mqtt_request_cb, LWIP_CONST_CAST(void*, &p->mqtt_data.mqtt_client_info),
				0);
		
		p->topic_list(&list, &node_count);
		if (node_count == 0)
		{
			p->mqtt_data.state = LN_MQTT_NO_TOPIC;
		}
	}		
	else if (ret == LN_MQTT_NOT_EXIST)
	{
		ln_at_printf("NO UNSUBSCRIBE\r\n");
	}
	else
	{
		goto __err;
	}
	
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(MQTTUNSUB, NULL, ln_mqtt_at_mqttunsub_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqttpub_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	int qos = 0;
	int retain = 0;
	char *pData = NULL;
	char *pTopic = NULL;
	err_t ret = ERR_OK;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	//check connect stat
    if(NETDEV_LINK_UP != netdev_get_link_state(netdev_get_active())){
        goto __err;
    }
	
	//check stat
	if (p->mqtt_data.state < LN_MQTT_CONNECTED)
	{
		goto __err;
	}
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	//topic
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pTopic))
    {
        goto __err;
    }
	
	if (is_default || !pTopic)
    {
        goto __err;
    }
	
	//data
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pData))
    {
        goto __err;
    }
	
	if (is_default || !pData)
    {
        goto __err;
    }
	
	//qos
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &qos))
    {
        qos = 0;
    }
	
	//retain
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &retain))
    {
        retain = 0;
    }
	
	//pub
	hexdump(LOG_LVL_INFO, "PUB", pData, strlen(pData));
	ret = mqtt_publish(p->mqtt_data.mqtt_client, pTopic, pData, strlen(pData), qos, retain, NULL, NULL);
	Log_i("mqtt_publish return: %d", ret);
	
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(MQTTPUB, NULL, ln_mqtt_at_mqttpub_set, NULL, NULL);

void ln_mqtt_scanf_data(char _ch, int is_empty)
{
	static int idx = 0;
	char *pBuf = NULL;
	err_t ret = ERR_OK;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	Log_i("is_empty:%d", is_empty);
	if (p->mqtt_data.iScanf == LN_MQTT_SCANF_FALSE)
	{
		return;
	}
	
	switch(p->mqtt_data.iScanf)
	{
		case LN_MQTT_SCANF_CLIENTID:
			pBuf = p->mqtt_data.client_id;
			break;
		case LN_MQTT_SCANF_USRNAME:
			pBuf = p->mqtt_data.username;
			break;
		case LN_MQTT_SCANF_PASSWORD:
			pBuf = p->mqtt_data.password;
			break;
		case LN_MQTT_SCANF_PUBRAW:
			pBuf = p->mqtt_data.pubraw;
			break;
		default:
			Log_e("Unknown mqtt scanf flag: %d", p->mqtt_data.iScanf);
			break;
	}
	
	pBuf[idx++] = _ch;
	if (--p->mqtt_data.scanfLen == 0)
	{
		Log_i("is_empty:%d", is_empty);
		if (is_empty == 0)
		{
			Log_w("is_empty: %d", is_empty);
			ln_at_printf("\r\nERROR\r\n\r\n");
		}
		
		pBuf[idx] = 0;
		Log_i("%d:%s", p->mqtt_data.iScanf, pBuf);
		
		if (LN_MQTT_SCANF_PUBRAW == p->mqtt_data.iScanf)
		{
			hexdump(LOG_LVL_INFO, "PUBRAW", pBuf, idx);
			ret = mqtt_publish(p->mqtt_data.mqtt_client, 
								p->mqtt_data.pubrawTopic, 
								pBuf, 
								idx, 
								p->mqtt_data.pubrawQos, 
								p->mqtt_data.pubrawRetain, 
								NULL, 
								NULL);
			
			if (ret == ERR_OK)
			{
				ln_at_printf("+MQTTPUB:OK\r\n");
			}
			else
			{
				ln_at_printf("+MQTTPUB:FAIL\r\n");
			}
		}
		else
		{
			p->mqtt_data.state = LN_MQTT_SET_USRCFG;
			ln_at_printf(LN_AT_RET_OK_STR);
		}
		
		p->mqtt_data.iScanf = LN_MQTT_SCANF_FALSE;		
		idx  = 0;
	}
}

ln_at_err_t ln_mqtt_at_mqtt_scanf_set(uint8_t para_num, const char *name, int iflag)
{
    //LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
		char *scan_type = NULL;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	if(p->mqtt_data.is_usr_cfg_init != true || p->mqtt_data.state > LN_MQTT_SET_CONNCFG)
	{
		goto __err;
	}
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &scan_type))
	{
		goto __err;
	}	
//refere to ESP32-C3 v2.4.0.0
	switch(iflag)
	{
		case LN_MQTT_SCANF_CLIENTID:
		{
			//str must <= 256 - 20(AT+MQTTCLIENTID=1,\r\n)=236
			if(strlen(scan_type) > 236)
			{
				Log_e("AT MQTT client id is over max at char number!\r\n");
				goto __err;
			}
			else
			{
				memcpy(p->mqtt_data.client_id, scan_type, strlen(scan_type));
			}
			
		}break;
		
		case LN_MQTT_SCANF_PASSWORD:
		{
			//str must <= 256 - 20(AT+MQTTCLIENTID=1,\r\n)=236
			if(strlen(scan_type) > LN_MQTT_PASSWORD_MAX_LENGTH)
			{
				Log_e("AT MQTT password is over len\r\n");
				goto __err;
			}
			else
			{
				memcpy(p->mqtt_data.password, scan_type, strlen(scan_type));
			}			
		}break;
		
		case LN_MQTT_SCANF_USRNAME:
		{
			//str must <= 256 - 20(AT+MQTTCLIENTID=1,\r\n)=236
			if(strlen(scan_type) > LN_MQTT_USERNAME_MAX_LENGTH)
			{
				Log_e("AT MQTT usrname is over len\r\n");
				goto __err;
			}
			else
			{
				memcpy(p->mqtt_data.username, scan_type, strlen(scan_type));
			}						
		}break;
		
		default:
			break;
	}

//	//length
//	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
//    {
//        goto __err;
//    }
//	
//	if (int_param < 0 || int_param > 1024)
//	{
//		goto __err;
//	}
//	
//	p->mqtt_data.scanfLen = int_param;
	p->mqtt_data.iScanf = iflag;
	
	ln_at_printf(LN_AT_RET_OK_STR);
//    ln_at_printf("\r\n>");
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

ln_at_err_t ln_mqtt_at_mqtt_client_id_set(uint8_t para_num, const char *name)
{
    return ln_mqtt_at_mqtt_scanf_set(para_num, name, LN_MQTT_SCANF_CLIENTID);
}
LN_AT_CMD_REG(MQTTCLIENTID, NULL, ln_mqtt_at_mqtt_client_id_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqtt_username_set(uint8_t para_num, const char *name)
{
    return ln_mqtt_at_mqtt_scanf_set(para_num, name, LN_MQTT_SCANF_USRNAME);
}
LN_AT_CMD_REG(MQTTUSERNAME, NULL, ln_mqtt_at_mqtt_username_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqtt_password_set(uint8_t para_num, const char *name)
{
    return ln_mqtt_at_mqtt_scanf_set(para_num, name, LN_MQTT_SCANF_PASSWORD);
}
LN_AT_CMD_REG(MQTTPASSWORD, NULL, ln_mqtt_at_mqtt_password_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqtt_pubraw_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	char *pTopic = NULL;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	//check stat
	if (p->mqtt_data.state < LN_MQTT_CONNECTED)
	{
		goto __err;
	}
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	//topic
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pTopic))
    {
        goto __err;
    }
	
	if (is_default || !pTopic)
    {
        goto __err;
    }
	strcpy(p->mqtt_data.pubrawTopic, pTopic);

	//length
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 0 || int_param > 1024)
	{
		goto __err;
	}
	p->mqtt_data.scanfLen = int_param;
	
	//qos
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 0 || int_param > 1024)
	{
		goto __err;
	}
	p->mqtt_data.pubrawQos = int_param;
	
	//retain
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 0 || int_param > 1024)
	{
		goto __err;
	}
	p->mqtt_data.pubrawRetain = int_param;
	
	p->mqtt_data.iScanf = LN_MQTT_SCANF_PUBRAW;
	
	ln_at_printf(LN_AT_RET_OK_STR);
    ln_at_printf("\r\n>");
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(MQTTPUBRAW, NULL, ln_mqtt_at_mqtt_pubraw_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqtt_conncfg_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	char *pTopic = NULL;
	char *pMsg = NULL;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	//check stat
	if (p->mqtt_data.state < LN_MQTT_SET_USRCFG || p->mqtt_data.state >= LN_MQTT_DISCONNECTED)
	{
		goto __err;
	}
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}

	//keepalive
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 0 || int_param > 7200)
	{
		goto __err;
	}
	p->mqtt_data.mqtt_client_info.keep_alive = int_param == 0 ? 120 : int_param;
	
	//disable_clean_session  Todo: cannot find this para in lwip-mqtt
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }	
	
	//will topic
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pTopic))
    {
        goto __err;
    }
	
	if (!pTopic)
    {
        goto __err;
    }
	
	if (*pTopic == 0)
	{
		p->mqtt_data.mqtt_client_info.will_topic = NULL;
	}
	else{
		p->mqtt_data.mqtt_client_info.will_topic = p->mqtt_data.will_topic;
		strcpy(p->mqtt_data.will_topic, pTopic);
	}
	
	//will msg
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pMsg))
    {
        goto __err;
    }
	
	if (!pMsg)
    {
        goto __err;
    }
	
	if (*pTopic == 0)
	{
		p->mqtt_data.mqtt_client_info.will_msg = NULL;
	}
	else{
		p->mqtt_data.mqtt_client_info.will_msg = p->mqtt_data.will_msg;
		strcpy(p->mqtt_data.will_msg, pMsg);
	}
	
	//qos
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 0 || int_param > 2)
	{
		goto __err;
	}
	p->mqtt_data.mqtt_client_info.will_qos = int_param;
	
	//retain
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 0 || int_param > 1)
	{
		goto __err;
	}
	p->mqtt_data.mqtt_client_info.will_retain = int_param;
	
	if(p->mqtt_data.state < LN_MQTT_CONNECTED)
	{
		p->mqtt_data.state = LN_MQTT_SET_CONNCFG;
	}
	
	ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(MQTTCONNCFG, NULL, ln_mqtt_at_mqtt_conncfg_set, NULL, NULL);

ln_at_err_t ln_mqtt_at_mqtt_usercfg_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	char *pClientId = NULL;
	char *pUsrname = NULL;
	char *pPwd = NULL;
	char *pPath = NULL;
	ln_mqtt_cfg_t* p = ln_get_mqtt_handle();
	
	if(p->mqtt_data.state > LN_MQTT_SET_CONNCFG)
	{
		goto __err;
	}
	
	//linkID
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}

	//scheme only tcp
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param < 1 || int_param > 10)
	{
		goto __err;
	}
	p->mqtt_data.scheme = int_param;
	
	//ClientId
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pClientId))
    {
        goto __err;
    }
	
	if (!pClientId)
    {
        goto __err;
    }
	strcpy(p->mqtt_data.client_id, pClientId);
	
	//username
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pUsrname))
    {
        goto __err;
    }
	
	if (!pUsrname)
    {
        goto __err;
    }
	strcpy(p->mqtt_data.username, pUsrname);

	//password
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pPwd))
    {
        goto __err;
    }
	
	if (!pPwd)
    {
        goto __err;
    }
	strcpy(p->mqtt_data.password, pPwd);
	
	//cert key id
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	//ca id
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
	
	if (int_param != 0)
	{
		goto __err;
	}
	
	//path
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pPath))
    {
        goto __err;
    }
	
	if (!pPath)
    {
        goto __err;
    }
	strcpy(p->mqtt_data.path, pPath);
		
	if(p->mqtt_data.state < LN_MQTT_CONNECTED)
	{
		p->mqtt_data.state = LN_MQTT_SET_USRCFG;
		p->mqtt_data.is_usr_cfg_init = true;
	}
	ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(MQTTUSERCFG, NULL, ln_mqtt_at_mqtt_usercfg_set, NULL, NULL);
