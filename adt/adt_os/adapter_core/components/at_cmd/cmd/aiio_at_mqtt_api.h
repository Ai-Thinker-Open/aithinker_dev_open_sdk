/** @brief      AT mqtt command api.
 *
 *  @file       aiio_at_mqtt_api.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>wusen       <td>First version
 *  <tr><td>2023/04/07      <td>1.0.1       <td>wusen       <td>add at-mqtt api
 *  </table>
 *
 */

#ifndef __AIIO_AT_MQTT_API_H__
#define __AIIO_AT_MQTT_API_H__
#include <stdint.h>
#include "aiio_type.h"


#define AIIO_MQTT_MAX_SBUSCRIBE	5	//Maximum number of subscriptions
#define AIIO_MQTT_SENDBUF_LEN	(2048 + 17)	
#define AIIO_MQTT_READBUF_LEN	(2048 + 17)	


typedef struct{
	uint8_t scheme;	//Connection mode 1：Using tcp connection  2: Using ssl connection
	char username[128];	
	char password[256];	
	char clientId[128];	
	char host_name[128];	
	int32_t port;	
	//Testamentary message
	char LWTTopic[64];
	uint8_t LWTQos;	//0/1/2
	uint8_t LWTRetained;	//0/1
	char LWTPayload[128];
}aiio_at_mqtt_config_t;

//mqtt status return value (mainly external)
//state machine string description aiio_MqttStatusStr one-to-one correspondence
typedef enum{
	AIIO_MQTT_STATUS_START		= 0,	
	AIIO_MQTT_STATUS_CONNECT	= 1,	
	AIIO_MQTT_STATUS_SUBTOPIC	= 2,	
	AIIO_MQTT_STATUS_RUNNING	= 3		//Successful connection
}aiio_mqtt_status_t;

extern aiio_at_mqtt_config_t at_mqtt_config;

/** @brief MQTT task
 *
 *  @param[in]      	pvParameters          task param.
 *  @return                         
 *  @retval         
 *  @retval         
 *  @note
 *  @see
 */
CORE_API void aiio_mqtt_prv_task(void *pvParameters);

/** @brief 	Reconnect to the MQTT server (This only works when connected)
 *
 *  @return                         
 *  @retval         
 *  @retval         
 *  @note
 *  @see
 */
CORE_API void aiio_mqtt_reconnect(void);

/** @brief 	Gets the MQTT connection status
 *
 *  @return				return connection status                        
 *  @retval         	
 *  @retval         
 *  @note
 *  @see
 */
CORE_API aiio_mqtt_status_t aiio_get_mqtt_status(void);

/** @brief 	Print the subscription topic
 *
 *  @return				                        
 *  @retval         	
 *  @retval         
 *  @note
 *  @see
 */
CORE_API void aiio_mqtt_print_subscribelist(void);

/** @brief 	Publish message (app level send, this only applies to the case of only one mqtt link, 
 * 			only after aiio_mqtt_prv_task is started, MQTT connection is successful)
 *	@param[in]      	topic          		Topics to publish.
 *	@param[in]      	qos          		qos level (0,1,2)
 *	@param[in]      	retained         	Whether it is Retained information.(0:Ordinary message  1:retained message)
 *	@param[in]      	p_payload          	Load message.
 *	@param[in]      	payloadLen          Load message len.
 *  @return				return at error code                        
 *  @retval         	0: The release is successful  
 *  @retval         	!=0: Publishing failed
 *  @note		
 *  @see				aiio_at_errcode.h
 */
CORE_API uint8_t aiio_mqtt_app_publish_message(char *topic,uint8_t qos,uint8_t retained,uint8_t *p_payload,uint16_t payloadLen);

/** @brief 	Subscribe to a subject (app level send, this only applies to the case of only one mqtt link, 
 * 			only after aiio_mqtt_prv_task is started, MQTT connection is successful)
 *	@param[in]      	topic          		Subscribed subject
 *	@param[in]      	qos          		qos level (0,1,2)
 *  @return				return at error code                        
 *  @retval         	0: The subscribe is successful  
 *  @retval         	!=0: subscribe failed
 *  @note		
 *  @see				aiio_at_errcode.h
 */
CORE_API uint8_t aiio_mqtt_app_subscribe_topic(char *topic,uint8_t qos);

/** @brief 	Unsubscribe to a subject (app level send, this only applies to the case of only one mqtt link, 
 * 			only after aiio_mqtt_prv_task is started, MQTT connection is successful)
 *	@param[in]      	topic          		Unsubscribe subject
 *	@param[in]      	qos          		qos level (0,1,2)
 *  @return				return at error code                        
 *  @retval         	0: The Unsubscribe is successful  
 *  @retval         	!=0: Unsubscribe failed
 *  @note		
 *  @see				aiio_at_errcode.h
 */
CORE_API uint8_t aiio_mqtt_app_unsubscribe_topic(const char*topic);





#endif