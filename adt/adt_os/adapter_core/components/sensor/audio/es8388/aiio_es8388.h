/** @brief      es8388 application interface.
 *
 *  @file       aiio_es8388.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/08/01      <td>1.0.0       <td>johhn       <td>I2C Init
 *  </table>
 *
 */
#ifndef __AIIO_ES8388_H__
#define __AIIO_ES8388_H__
#include "aiio_i2c.h"
#include "aiio_i2s.h"


/**
 *  @brief ES8388 Role Matser Or Slave
 */
typedef enum {
    ES8388_MASTER, /*!< Master Mode */
    ES8388_SLAVE,  /*!< Slave Mode */
} aiio_es8388_role_type_enum_t;

/**
 *  @brief ES8388 Work Mode
 */
typedef enum {
    ES8388_CODEC_MDOE,     /*!< ES8388 work at codec mode */
    ES8388_RECORDING_MODE, /*!< ES8388 work at recording mode */
    ES8388_PLAY_BACK_MODE, /*!< ES8388 work at paly back mode */
    ES8388_BY_PASS_MODE,   /*!< ES8388 work at by pass mode */
} aiio_es8388_work_mode_enum_t;

/**
 *  @brief ES8388 Microphone input type
 */
typedef enum {
    ES8388_SINGLE_ENDED_MIC, /*!< Mic Single Input Mode */
    ES8388_DIFF_ENDED_MIC,   /*!< Mic Different Input Mode */
} aiio_es8388_mic_input_type_enum_t;

/**
 *  @brief ES8388 Microphone pga sel type
 */
typedef enum {
    ES8388_MIC_PGA_0DB,  /*!< Mic PGA as 0db */
    ES8388_MIC_PGA_3DB,  /*!< Mic PGA as 3db */
    ES8388_MIC_PGA_6DB,  /*!< Mic PGA as 6db */
    ES8388_MIC_PGA_9DB,  /*!< Mic PGA as 9db */
    ES8388_MIC_PGA_12DB, /*!< Mic PGA as 12db */
    ES8388_MIC_PGA_15DB, /*!< Mic PGA as 15db */
    ES8388_MIC_PGA_18DB, /*!< Mic PGA as 18db */
    ES8388_MIC_PGA_21DB, /*!< Mic PGA as 21db */
    ES8388_MIC_PGA_24DB, /*!< Mic PGA as 24db */
} aiio_es8388_mic_input_pga_type_enum_t;

/**
 *  @brief ES8388 I2S Frame Type
 */
typedef enum {
    ES8388_STD_I2S_FRAME,       /*!< Standard I2S Frame */
    ES8388_LEFT_JUSTIFY_FRAME,  /*!< Left  Justify Frame */
    ES8388_RIGHT_JUSTIFY_FRAME, /*!< Right Justify Frame */
    ES8388_DSP_FRAME,           /*!< DSP Frame */
} aiio_es8388_i2s_frame_type_enum_t;

/**
 *  @brief ES8388 I2S Data Len Type
 */
typedef enum {
    ES8388_DATA_LEN_24, /*!< I2S Auido Data Len 24 */
    ES8388_DATA_LEN_20, /*!< I2S Auido Data Len 20 */
    ES8388_DATA_LEN_18, /*!< I2S Auido Data Len 18 */
    ES8388_DATA_LEN_16, /*!< I2S Auido Data Len 16 */
    ES8388_DATA_LEN_32, /*!< I2S Auido Data Len 32 */
} aiio_es8388_i2s_data_with_enum_t;

/**
 *  @brief ES8388_Cfg_Type
 */
typedef struct
{
    aiio_es8388_work_mode_enum_t work_mode;           /*!< ES8388 work mode */
    aiio_es8388_role_type_enum_t role;                /*!< ES8388 role */
    aiio_es8388_mic_input_type_enum_t mic_input_mode; /*!< ES8388 mic input mode */
    aiio_es8388_mic_input_pga_type_enum_t mic_pga;    /*!< ES8388 mic PGA */
    aiio_es8388_i2s_frame_type_enum_t i2s_frame;      /*!< ES8388 I2S frame */
    aiio_es8388_i2s_data_with_enum_t data_width;     /*!< ES8388 I2S dataWitdh */
} aiio_es8388_cfg_type_t;

/*@} end of group ES8388_Public_Types */

/** @defgroup  ES8388_Public_Constants
 *  @{
 */

/** @defgroup  ES8388_ROLE_TYPE
 *  @{
 */
#define IS_ES8388_ROLE_TYPE(type) (((type) == ES8388_MASTER) || \
                                   ((type) == ES8388_SLAVE))

/** @defgroup  ES8388_WORK_MODE
 *  @{
 */
#define IS_ES8388_WORK_MODE(type) (((type) == ES8388_CODEC_MDOE) ||     \
                                   ((type) == ES8388_RECORDING_MODE) || \
                                   ((type) == ES8388_PLAY_BACK_MODE) || \
                                   ((type) == ES8388_BY_PASS_MODE))

/** @defgroup  ES8388_MIC_INPUT_TYPE
 *  @{
 */
#define IS_ES8388_MIC_INPUT_TYPE(type) (((type) == ES8388_SINGLE_ENDED_MIC) || \
                                        ((type) == ES8388_DIFF_ENDED_MIC))

/** @defgroup  ES8388_MIC_INPUT_PGA_TYPE
 *  @{
 */
#define IS_ES8388_MIC_INPUT_PGA_TYPE(type) (((type) == ES8388_MIC_PGA_0DB) ||  \
                                            ((type) == ES8388_MIC_PGA_3DB) ||  \
                                            ((type) == ES8388_MIC_PGA_6DB) ||  \
                                            ((type) == ES8388_MIC_PGA_9DB) ||  \
                                            ((type) == ES8388_MIC_PGA_12DB) || \
                                            ((type) == ES8388_MIC_PGA_15DB) || \
                                            ((type) == ES8388_MIC_PGA_18DB) || \
                                            ((type) == ES8388_MIC_PGA_21DB) || \
                                            ((type) == ES8388_MIC_PGA_24DB))

/** @defgroup  ES8388_I2S_FRAME_TYPE
 *  @{
 */
#define IS_ES8388_I2S_FRAME_TYPE(type) (((type) == ES8388_STD_I2S_FRAME) ||       \
                                        ((type) == ES8388_LEFT_JUSTIFY_FRAME) ||  \
                                        ((type) == ES8388_RIGHT_JUSTIFY_FRAME) || \
                                        ((type) == ES8388_DSP_FRAME))

/** @defgroup  ES8388_I2S_DATA_WIDTH
 *  @{
 */
#define IS_ES8388_I2S_DATA_WIDTH(type) (((type) == ES8388_DATA_LEN_24) || \
                                        ((type) == ES8388_DATA_LEN_20) || \
                                        ((type) == ES8388_DATA_LEN_18) || \
                                        ((type) == ES8388_DATA_LEN_16) || \
                                        ((type) == ES8388_DATA_LEN_32))

/*@} end of group ES8388_Public_Constants */

/** @defgroup  ES8388_Public_Macros
 *  @{
 */

/*@} end of group ES8388_Public_Macros */

/** @defgroup  ES8388_Public_Functions
 *  @{
 */

/** @brief ES8388 Init function
 *
 *  @param[in]      aiio_i2c_cfg                 I2C cfg
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
int32_t aiio_es8388_init(aiio_i2c_pin_cfg_t *aiio_i2c_cfg,aiio_i2s_pin_cfg_t*aiio_i2s_cfg);


#endif
