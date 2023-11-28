/** @brief      radar application interface.
 *
 *  @file       aiio_radar.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       radar application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/11/08      <td>1.0.0       <td>hongjz      <td>Define radar API
 *  </table>
 *
 */

#ifndef _AIIO_RADAR_H_
#define _AIIO_RADAR_H_

#include "aiio_autoconf.h"

#include "aiio_adapter_include.h"

typedef void(*rd01_data_output_t)(uint8_t *data, uint16_t len);

#if CONFIG_AIIO_RADAR

#define RADAR_CMD_RECV_BUF_SIZE   (64)

typedef enum FlagStatus 
{
    STOP    =   0,
    START   =   1,
    MAX_FLAGSTATUS
}T_FlagStatus;

typedef enum DETECTSTATUS
{
    NOBODY = 0,
    BODYMOTION = 1,
    BODYMOTIONLESS = 2,
    BOTH_STATUS = 3,
    ENERGYSATURATION = 4
}DetStatus;

typedef struct RESULT
{
    DetStatus status;
	uint16_t Distance;
    uint16_t Motion_Distance;
    uint8_t Motion_MaxVal;
    uint16_t MotionLess_Distance;
    uint8_t MotionLess_MaxVal;
    uint16_t offtime;
    uint8_t bNeedReport;
}Result;

#pragma pack(4) 
typedef struct COMPLEXFLOAT
{
    float real;
    float imag;
}COMPLEXFLOAT_T;
#pragma pack() 

typedef struct COMPLEX16
{
    int16_t real;
    int16_t imag;
}COMPLEX16_T;

typedef struct COMPLEX32
{
    int32_t real;
    int32_t imag;
}COMPLEX32_T;

typedef struct COMPLEX64
{
    int64_t real;
    int64_t imag;
}COMPLEX64_T;

typedef enum  
{
	DATA_STATE_HEAD = 0,
    DATA_STATE_ID,
    DATA_STATE_INDEX1,
    DATA_STATE_INDEX2,
    DATA_STATE_DATA,
    DATA_STATE_TAIL0,
    DATA_STATE_TAIL1,
    DATA_STATE_TAIL2,
    DATA_STATE_TAIL3
}dataStateEnum;

typedef enum
{
    BUF_0        = 0,
    BUF_1        = 1,
    BUF_MAX
}BufEnum;

typedef struct CMD_RECV
{
    uint8_t buf[BUF_MAX][RADAR_CMD_RECV_BUF_SIZE];
    volatile uint8_t bufRecv;
    volatile uint8_t bufProc;
    volatile uint8_t cmdReady;
    volatile uint8_t idleCnt;
    volatile uint16_t curIndex;
    volatile uint16_t bufLen;
}CMD_RECV_T;

typedef struct RADAR_REG
{
    uint16_t addr;
    uint16_t val;
}RADAR_REG_T;

typedef struct RADAR_PARA
{
    uint8_t dataType;
    uint16_t dataLen;
    uint16_t chirpNum;
}RADAR_PARA_T;

typedef enum
{
    DATA_TYPE_FFT         = 0,
    DATA_TYPE_DFFT        = 1,
    DATA_TYPE_DFFT_PEAK   = 2,
	DATA_TYPE_DSRAW		  = 3,
    DATA_TYPE_MAX
}dataTypeEnum;

typedef enum  
{
	RAW_POINT_64 = 64,
    RAW_POINT_128 = 128,
    RAW_POINT_256 = 256,
    RAW_POINT_512 = 512,
    RAW_POINT_1024 = 1024,
}rawPointEnum;

typedef struct RADAR_DATA_PARSE
{
    uint8_t  buf[264];
    uint8_t  state;
    uint8_t  channelId;
    uint8_t  chirpIndex;
    uint16_t frameCnt;
    uint16_t curIndex;
    uint16_t needCopyLen;
    uint16_t dfft_peak_data_len;
}RADAR_DATA_PARSE_T;

/** @brief Rd-01 init function
 *
 *  @param[in]
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_rd01_init(void);

/** @brief Rd-01 data ouput callback register
 *
 *  @param[in]      cb              Callback function
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_rd01_data_output_cb_register(rd01_data_output_t cb);

#endif

#endif
