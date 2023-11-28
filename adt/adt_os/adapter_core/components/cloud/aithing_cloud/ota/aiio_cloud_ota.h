/**
 * @brief   Declare OTA data proccess logic interface
 * 
 * @file    aiio_cloud_ota.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-27          <td>1.0.0            <td>zhuolm             <td> The OTA data proccess logic interface
 */

#ifndef __AIIO_CLOUD_OTA_H_
#define __AIIO_CLOUD_OTA_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_ota_protocol.h"


typedef enum
{
    AIIO_CLOUD_OTA_TYPE_MODULE,                                 /*!< The module ota type*/
    AIIO_CLOUD_OTA_TYPE_THIRD_PARTY,                             /*!< The third-party ota type*/
    AIIO_CLOUD_OTA_TYPE_UNKNOWN,                                 /*!< unkonw-type*/
}aiio_cloud_ota_type_t;

typedef enum
{
    AIIO_CLOUD_OTA_INFO_PARSE_OK,                               /*!< Parse ota information success*/
    AIIO_CLOUD_OTA_INFO_PARSE_FAILE,                            /*!< Parse ota information fail */
    AIIO_CLOUD_OTA_HARDWARE_ERR,                                /*!< Unexpected hardware error*/
    AIIO_CLOUD_OTA_HTTP_ERR,                                    /*!< Unexpected http error*/
    AIIO_CLOUD_OTA_REV_DATA,                                    /*!< Receive ota data from cloud*/
    AIIO_CLOUD_OTA_DATA_CHECK_ERR,                              /*!< Error while passing the md5 check*/
    AIIO_CLOUD_OTA_REV_DATA_TIMEOUT,                            /*!< The OTA data receiving was timed out*/
    AIIO_CLOUD_OTA_FINISH                                       /*!< The OTA data is fully received*/
}aiio_cloud_ota_event_id_t;

typedef struct
{
    char                            *ota_data;                  /*!< Receive ota data from cloud by http*/
    uint32_t                        ota_data_len;               /*!< The length while receiving ota data*/
    uint32_t                        ota_data_progress;          /*!< The progress while receiving ota data*/
    uint32_t                        ota_data_total_len;         /*!< The total length of ota data*/

    int32_t                         ota_err_code;               /*!< The status of ota data process*/
    aiio_ota_protocol_msg_t         *ota_protocol_msg;          /*!< The protocol massage of ota*/
    aiio_cloud_ota_event_id_t       ota_event_id;               /*!< The event of ota data process*/
    aiio_cloud_ota_type_t           cloud_ota_type;             /*!< The type of ota data*/
}aiio_cloud_ota_event_t;


typedef void (*aiio_cloud_ota_receive_event_cb)(aiio_cloud_ota_event_t *cloud_ota_event);


typedef struct 
{
    char                        *device_fw_version;                             /*!< The version of device firmware*/
    char                        *device_product_id;                             /*!< The product id of device*/
    bool                        is_compatible_type;                             /*!< This variable is designed to set up the older version of the compatible OTA mode, it is for zhenmei or kegao customer.
                                                                                    if it set to true, It will be compatible with older versions of the OTA mode, it will upgrade zhenmei or kegao device
                                                                                    by default, it must be false, it will be designed to set up the new ota type. */
    uint32_t                    task_prio;                                      /*!< Priority of the thread task*/
    uint32_t                    task_stack;                                     /*!< The stack for the thread task*/
    uint32_t                    rev_buffer_max_len;                             /*!< The length of recevie buffer */

    aiio_cloud_ota_receive_event_cb  cloud_ota_rev_eve_cb;                      /*!< Receives the callback function for the ota event */

}aiio_cloud_ota_config_t;



/**
 * @brief   Launch ota, and then getting ota data from cloud
 * 
 * @param[in]   ota_info                    The ota protocol message
 * @param[in]   cloud_ota_config            The ota configuration data
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_protocol_code.h or aiio_error.h
 */
int aiio_cloud_ota_start(char *ota_info, aiio_cloud_ota_config_t *cloud_ota_config);

/**
 * @brief   Stop ota, and release resource
 * @note    it must be called when ota is success or fail, it will release resource
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_protocol_code.h or aiio_error.h
 */
int aiio_cloud_ota_stop(void);

/**
 * @brief   Reporting the progress of ota data download
 * 
 * @param[in]   progress                The progress of ota data download
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_protocol_code.h or aiio_error.h
 */
int aiio_cloud_ota_report_download_progress(uint32_t progress);


/**
 * @brief   Reporting the status of ota data installing
 * 
 * @param[in]   err_code                The ota data installing status code
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_protocol_code.h or aiio_error.h
 */
int aiio_cloud_ota_report_install_status(int32_t err_code);


/**
 * @brief   Reporting the status of ota data proccess
 * 
 * @param[in]   err_code                The ota data proccess status code
 * @param[in]   OtaVer                  OTA version to be updated
 * @param[in]   PubId                   The ID of the OTA information release
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_protocol_code.h or aiio_error.h
 */
int aiio_cloud_ota_report_proccess_status(int32_t err_code, char *OtaVer, char *PubId);


#endif
