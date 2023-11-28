#ifndef _LN_BLE_APP_ERROR_CODE_H_
#define _LN_BLE_APP_ERROR_CODE_H_


typedef enum
{
    BLE_ERR_NONE               = 0u,

    BLE_ERR_INVALID_PARAM      = 10u,
    BLE_ERR_SIZE_EXCEEDED,
    BLE_ERR_MEM,

    BLE_ERR_TIMEOUT            = 20u,
    BLE_ERR_STATE,
} ble_err_code_t;



#endif /* _LN_BLE_APP_ERROR_CODE_H_ */

