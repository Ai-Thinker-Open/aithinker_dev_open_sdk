/**
 * @file     hal_ext.h
 * @author   BSP Team 
 * @brief    This file contains all of EXT functions prototypes.
 * @version  0.0.0.1
 * @date     2021-02-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __HAL_EXT_H
#define __HAL_EXT_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "hal/hal_common.h"

typedef enum
{
    EXT_INT_SENSE_0 = 0,    //PA0
    EXT_INT_SENSE_1 = 1,    //PA1
    EXT_INT_SENSE_2 = 2,    //PA2
    EXT_INT_SENSE_3 = 3,    //PA3
    EXT_INT_SENSE_4 = 4,    //PA5
    EXT_INT_SENSE_5 = 5,    //PA6
    EXT_INT_SENSE_6 = 6,    //PA7
    EXT_INT_SENSE_7 = 7,    //PB9

}ext_int_sense_t;

#define IS_EXT_INT_SENSE(SENSE)  (((SENSE) == EXT_INT_SENSE_0)  || ((SENSE) == EXT_INT_SENSE_1) || \
                                  ((SENSE) == EXT_INT_SENSE_2)  || ((SENSE) == EXT_INT_SENSE_3) || \
                                  ((SENSE) == EXT_INT_SENSE_4)  || ((SENSE) == EXT_INT_SENSE_5) || \
                                  ((SENSE) == EXT_INT_SENSE_6)  || ((SENSE) == EXT_INT_SENSE_7) ) 


typedef enum
{
    EXT_INT_HIGH_LEVEL = 0,
    EXT_INT_LOW_LEVEL  = 1,
    EXT_INT_POSEDEG    = 2,
    EXT_INT_NEGEDGE    = 3,
}ext_trig_mode_t;

#define IS_EXT_TRIG_MODE(MODE)   (((MODE) == EXT_INT_HIGH_LEVEL)  || ((MODE) == EXT_INT_LOW_LEVEL) || \
                                  ((MODE) == EXT_INT_POSEDEG)     || ((MODE) == EXT_INT_NEGEDGE) )
          

typedef enum
{
    EXT_INT_SENSE_0_IT_FLAG = 0,
    EXT_INT_SENSE_1_IT_FLAG  = 1,
    EXT_INT_SENSE_2_IT_FLAG  = 2,
    EXT_INT_SENSE_3_IT_FLAG  = 3,
    EXT_INT_SENSE_4_IT_FLAG  = 4,
    EXT_INT_SENSE_5_IT_FLAG  = 5,
    EXT_INT_SENSE_6_IT_FLAG  = 6,
    EXT_INT_SENSE_7_IT_FLAG  = 7, 
}ext_it_flag_t;

#define IS_EXT_IT_FLAG(FLAG)     (((FLAG) == EXT_INT_SENSE_0_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_1_IT_FLAG) || \
                                  ((FLAG) == EXT_INT_SENSE_2_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_3_IT_FLAG) || \
                                  ((FLAG) == EXT_INT_SENSE_4_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_5_IT_FLAG) || \
                                  ((FLAG) == EXT_INT_SENSE_6_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_7_IT_FLAG) ) 

typedef enum
{
    EXT_INT_SENSE_0_RAW_IT_FLAG = 0,
    EXT_INT_SENSE_1_RAW_IT_FLAG = 1,
    EXT_INT_SENSE_2_RAW_IT_FLAG = 2,
    EXT_INT_SENSE_3_RAW_IT_FLAG = 3,
    EXT_INT_SENSE_4_RAW_IT_FLAG = 4,
    EXT_INT_SENSE_5_RAW_IT_FLAG = 5,
    EXT_INT_SENSE_6_RAW_IT_FLAG = 6,
    EXT_INT_SENSE_7_RAW_IT_FLAG = 7, 
}ext_it_raw_flag_t;

#define IS_EXT_RAW_IT_FLAG(FLAG) (((FLAG) == EXT_INT_SENSE_0_RAW_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_1_RAW_IT_FLAG) || \
                                  ((FLAG) == EXT_INT_SENSE_2_RAW_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_3_RAW_IT_FLAG) || \
                                  ((FLAG) == EXT_INT_SENSE_4_RAW_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_5_RAW_IT_FLAG) || \
                                  ((FLAG) == EXT_INT_SENSE_6_RAW_IT_FLAG)  || ((FLAG) == EXT_INT_SENSE_7_RAW_IT_FLAG) ) 

//          init the ext 
void        hal_ext_init(ext_int_sense_t ext_int_sense,ext_trig_mode_t ext_trig_mode,hal_en_t en);
void        hal_ext_deinit(void);
//          ext interrupt configuration
uint8_t     hal_ext_get_it_flag(ext_it_flag_t ext_it_flag);
uint8_t     hal_ext_get_raw_it_flag(ext_it_raw_flag_t ext_raw_it_flag);
void        hal_ext_clr_it_flag(ext_it_flag_t ext_it_flag);



#ifdef __cplusplus
}
#endif

#endif /* __HAL_EXT_H */
