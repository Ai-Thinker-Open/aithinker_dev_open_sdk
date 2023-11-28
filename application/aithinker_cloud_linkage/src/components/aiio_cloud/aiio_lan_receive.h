/**
 * @brief   Declare custom LAN communication interface
 * 
 * @file    aiio_lan_receive.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-06          <td>1.0.0            <td>zhuolm             <td> The custom LAN communication interface
 */
#ifndef __AIIO_LAN_RECEIVE_H_
#define __AIIO_LAN_RECEIVE_H_


/**
 * @brief   The configuration structure of lan information
 */
typedef struct 
{
    char *device_id;                /*!< Device id*/
    char *local_ip;             /*!< The local network address*/
    uint16_t lan_port;             /*!< The port number of LAN network, if not set, LAN will use default value*/
}aiio_user_lan_config_t;


void aiio_user_lan_service_init(aiio_user_lan_config_t *lan_config);

void aiio_user_lan_service_deinit(void);



#endif
