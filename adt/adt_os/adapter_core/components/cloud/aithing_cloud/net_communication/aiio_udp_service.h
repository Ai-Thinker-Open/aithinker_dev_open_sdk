/**
 * @brief   The interface of udp communication
 * 
 * @file    aiio_udp_service.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        Declare UDP communication interface , variable and structure.
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-04          <td>1.0.0            <td>zhuolm             <td> The UDP communication interface
 */
#ifndef __AIIO_UDP_SERVICE_H_
#define __AIIO_UDP_SERVICE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_code.h"



typedef struct 
{
    uint32_t                    task_prio;                                      /*!< Priority of the thread task*/
    uint32_t                    task_stack;                                     /*!< The stack for the thread task*/
    uint16_t                    udp_port;                                       /*!< The port number of udp service*/
    uint32_t                    rev_buffer_max_len;                             /*!< The length of recevie buffer */

    void (*rev_udp_data_cb)(uint8_t *data, uint32_t data_len);                  /*!< The callback function of receiving udp data*/
}aiio_udp_service_config_t;


/**
 * @brief   Initialize UDP service communication
 * 
 * @param[in]   config     The paramter of configuration
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_udp_service_init(aiio_udp_service_config_t *config);


/**
 * @brief   Deinitilize udp service communication, and release memory resource
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_udp_service_deinit(void);


/**
 * @brief   Response data to udp client
 * 
 * @param[in]   data                The data to be responsed
 * @param[in]   data_len            The length of the data to be responsed
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_udp_service_send(uint8_t *data, uint32_t data_len);



#endif
