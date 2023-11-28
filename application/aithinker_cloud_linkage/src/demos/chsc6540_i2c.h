/**
 * @file        chsc6540_i2c.h
 * @brief
 * @author      Jimmy (ccolacat@163.com)
 * @version     v1.0.0
 * @date        2023-06-06
 * 
 * @copyright   Copyright (c) 2020-2023 Shenzhen Ai-Thinker Technology Co., Ltd. All Rights Reserved.
 * 
 * Disclaimer:  This program is for informational purposes only, please keep author information and source code license.
 * 
 */
#pragma once

#include "stdint.h"

#define CHSC6540_I2C_SLAVE_ADDR 0x2e

/* Maximum border values of the touchscreen pad that the chip can handle */
#define CHSC6540_MAX_WIDTH  ((uint16_t)240)
#define CHSC6540_MAX_HEIGHT ((uint16_t)320)

/* Max detectable simultaneous touch points */
#define CHSC6540_I2C_MAX_POINT 2

/* Register of the current mode */
#define CHSC6540_DEV_MODE_REG 0x00

/* Possible modes as of CHSC6540_DEV_MODE_REG */
#define CHSC6540_DEV_MODE_WORKING 0x00
#define CHSC6540_DEV_MODE_FACTORY 0x04

#define CHSC6540_DEV_MODE_MASK  0x70
#define CHSC6540_DEV_MODE_SHIFT 4

/* Gesture ID register */
#define CHSC6540_GEST_ID_REG 0x01

/* Possible values returned by CHSC6540_GEST_ID_REG */
#define CHSC6540_GEST_ID_NO_GESTURE 0x00
#define CHSC6540_GEST_ID_MOVE_UP    0x10
#define CHSC6540_GEST_ID_MOVE_RIGHT 0x14
#define CHSC6540_GEST_ID_MOVE_DOWN  0x18
#define CHSC6540_GEST_ID_MOVE_LEFT  0x1C
#define CHSC6540_GEST_ID_ZOOM_IN    0x48
#define CHSC6540_GEST_ID_ZOOM_OUT   0x49

/* Status register: stores number of active touch points (0, 1, 2) */
#define CHSC6540_TD_STAT_REG   0x00
#define CHSC6540_TD_STAT_MASK  0xFF
#define CHSC6540_TD_POINT_NUM  0x02
#define CHSC6540_TD_DATA_NUM   0x0E

/* Touch events */
#define CHSC6540_TOUCH_EVT_FLAG_PRESS_DOWN 0x00
#define CHSC6540_TOUCH_EVT_FLAG_LIFT_UP    0x01
#define CHSC6540_TOUCH_EVT_FLAG_CONTACT    0x02
#define CHSC6540_TOUCH_EVT_FLAG_NO_EVENT   0x03

#define CHSC6540_TOUCH_EVT_FLAG_SHIFT 6
#define CHSC6540_TOUCH_EVT_FLAG_MASK  (3 << CHSC6540_TOUCH_EVT_FLAG_SHIFT)

#define CHSC6540_MSB_MASK  0x0F
#define CHSC6540_MSB_SHIFT 0
#define CHSC6540_LSB_MASK  0xFF
#define CHSC6540_LSB_SHIFT 0

#define CHSC6540_P1_XH_REG 0x03
#define CHSC6540_P1_XL_REG 0x04
#define CHSC6540_P1_YH_REG 0x05
#define CHSC6540_P1_YL_REG 0x06

#define CHSC6540_P1_WEIGHT_REG      0x07 /* Register reporting touch pressure - read only */
#define CHSC6540_TOUCH_WEIGHT_MASK  0xFF
#define CHSC6540_TOUCH_WEIGHT_SHIFT 0

#define CHSC6540_P1_MISC_REG 0x08 /* Touch area register */

#define CHSC6540_TOUCH_AREA_MASK  (0x04 << 4) /* Values related to CHSC6540_Pn_MISC_REG */
#define CHSC6540_TOUCH_AREA_SHIFT 0x04

#define CHSC6540_P2_XH_REG     0x09
#define CHSC6540_P2_XL_REG     0x0A
#define CHSC6540_P2_YH_REG     0x0B
#define CHSC6540_P2_YL_REG     0x0C
#define CHSC6540_P2_WEIGHT_REG 0x0D
#define CHSC6540_P2_MISC_REG   0x0E

/* Threshold for touch detection */
#define CHSC6540_TH_GROUP_REG    0x80
#define CHSC6540_THRESHOLD_MASK  0xFF /* Values CHSC6540_TH_GROUP_REG : threshold related  */
#define CHSC6540_THRESHOLD_SHIFT 0

#define CHSC6540_TH_DIFF_REG 0x85 /* Filter function coefficients */

#define CHSC6540_CTRL_REG 0x86 /* Control register */

#define CHSC6540_CTRL_KEEP_ACTIVE_MODE              0x00 /* Will keep the Active mode when there is no touching */
#define CHSC6540_CTRL_KEEP_AUTO_SWITCH_MONITOR_MODE 0x01 /* Switching from Active mode to Monitor mode automatically when there is no touching */

#define CHSC6540_TIME_ENTER_MONITOR_REG 0x87 /* The time period of switching from Active mode to Monitor mode when there is no touching */

#define CHSC6540_PERIOD_ACTIVE_REG  0x88 /* Report rate in Active mode */
#define CHSC6540_PERIOD_MONITOR_REG 0x89 /* Report rate in Monitor mode */

#define CHSC6540_RADIAN_VALUE_REG 0x91 /* The value of the minimum allowed angle while Rotating gesture mode */

#define CHSC6540_OFFSET_LEFT_RIGHT_REG 0x92 /* Maximum offset while Moving Left and Moving Right gesture */
#define CHSC6540_OFFSET_UP_DOWN_REG    0x93 /* Maximum offset while Moving Up and Moving Down gesture */

#define CHSC6540_DISTANCE_LEFT_RIGHT_REG 0x94 /* Minimum distance while Moving Left and Moving Right gesture */
#define CHSC6540_DISTANCE_UP_DOWN_REG    0x95 /* Minimum distance while Moving Up and Moving Down gesture */

#define CHSC6540_LIB_VER_H_REG 0xA1 /* High 8-bit of LIB Version info */
#define CHSC6540_LIB_VER_L_REG 0xA2 /* Low 8-bit of LIB Version info */

#define CHSC6540_CHIPSELECT_REG 0xA3 /* 0x36 for ft6236; 0x06 for ft6206 */

#define CHSC6540_POWER_MODE_REG  0xA5
#define CHSC6540_FIRMWARE_ID_REG 0xA6
#define CHSC6540_RELEASECODE_REG 0xAF
#define CHSC6540_PANEL_ID_REG    0xA8
#define CHSC6540_OPMODE_REG      0xBC

typedef struct
{
    uint16_t coord_x;
    uint16_t coord_y;
} touch_coord_t;

int chsc6540_i2c_init(void);
int chsc6540_i2c_read(uint8_t *point_num, touch_coord_t *touch_coord, uint8_t max_num);

/************************ Copyright (c) 2023 Shenzhen Ai-Thinker Technology Co., Ltd. ************************/
