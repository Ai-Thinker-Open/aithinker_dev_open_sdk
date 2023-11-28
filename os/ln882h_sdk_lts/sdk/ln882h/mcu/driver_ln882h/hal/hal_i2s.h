/**
 * @file     hal_i2s.h
 * @author   BSP Team 
 * @brief    This file contains all of I2S functions prototypes.
 * @version  0.0.0.1
 * @date     2021-02-19
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __HAL_I2S_H
#define __HAL_I2S_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "hal/hal_common.h"
#include "reg_i2s.h"


#define IS_I2S_ALL_PERIPH(PERIPH) (((PERIPH) == I2S_BASE) ) 


typedef enum
{
    I2S_REC_CONFIG_IGNORE_LENGTH    = 0,
    I2S_REC_CONFIG_12_BIT           = 1,
    I2S_REC_CONFIG_16_BIT           = 2,
    I2S_REC_CONFIG_20_BIT           = 3,
    I2S_REC_CONFIG_24_BIT           = 4,
    I2S_REC_CONFIG_32_BIT           = 5,
}i2s_rec_config_t;

#define IS_I2S_REC_CONFIG(CONFIG)  (((CONFIG) == I2S_REC_CONFIG_IGNORE_LENGTH)  || ((CONFIG) == I2S_REC_CONFIG_12_BIT) || \
                                    ((CONFIG) == I2S_REC_CONFIG_16_BIT)         || ((CONFIG) == I2S_REC_CONFIG_20_BIT) || \
                                    ((CONFIG) == I2S_REC_CONFIG_24_BIT)         || ((CONFIG) == I2S_REC_CONFIG_32_BIT))

typedef enum
{
    I2S_TRA_CONFIG_IGNORE_LENGTH    = 0,
    I2S_TRA_CONFIG_12_BIT           = 1,
    I2S_TRA_CONFIG_16_BIT           = 2,
    I2S_TRA_CONFIG_20_BIT           = 3,
    I2S_TRA_CONFIG_24_BIT           = 4,
    I2S_TRA_CONFIG_32_BIT           = 5,
}i2s_tra_config_t;

#define IS_I2S_TRA_CONFIG(CONFIG)  (((CONFIG) == I2S_TRA_CONFIG_IGNORE_LENGTH)  || ((CONFIG) == I2S_TRA_CONFIG_12_BIT) || \
                                    ((CONFIG) == I2S_TRA_CONFIG_16_BIT)         || ((CONFIG) == I2S_TRA_CONFIG_20_BIT) || \
                                    ((CONFIG) == I2S_TRA_CONFIG_24_BIT)         || ((CONFIG) == I2S_TRA_CONFIG_32_BIT))

#define IS_I2S_REC_FIFO_CONFIG(CONFIG)  (CONFIG <= 7)
#define IS_I2S_TRA_FIFO_CONFIG(CONFIG)  (CONFIG <= 7)

typedef enum
{
    I2S_IT_FLAG_TXFO             = 0,              // Attempt to write to full TX FIFO.               
    I2S_IT_FLAG_TXFE             = 1,              // trigger level reached.
    I2S_IT_FLAG_RXFO             = 2,              /* Status of Data Overrun interrupt for the RX channel. Incoming data lost due to a full RX FIFO.
                                                        0: RX FIFO write valid
                                                        1: RX FIFO write overrun
                                                   */
    I2S_IT_FLAG_RXDA             = 3,              /* Status of Receive Data Available interrupt. RX FIFO data available.
                                                        1: trigger level reached
                                                        0: trigger level not reached
                                                   */
}i2s_it_flag_t;

#define IS_I2S_IT_FLAG(FLAG)        (( FLAG <= 3 ))

typedef struct
{
    i2s_rec_config_t    i2s_rec_config;                 /*!< Specifies the i2s receive data length.
                                                        This parameter can be a value of i2s_rec_config_t */ 

    i2s_tra_config_t    i2s_tra_config;                 /*!< Specifies the i2s transmit data length.
                                                        This parameter can be a value of i2s_tra_config_t */  

    uint8_t             rec_fifo_trig_lev;              /*!< Specifies the i2s receive fifo trigger level .
                                                        This parameter can be a value of IS_I2S_REC_FIFO_CONFIG */  

    uint8_t             tra_fifo_trig_lev;              /*!< Specifies the i2s transmit fifo trigger level .
                                                        This parameter can be a value of IS_I2S_TRA_FIFO_CONFIG */                                                      
}i2s_init_t_def;

            //I2S init and config
void        hal_i2s_init(uint32_t i2s_base,i2s_init_t_def* i2s_init);   
void        hal_i2s_deinit(void);
void        hal_i2s_rx_en(uint32_t i2s_base,hal_en_t en);           
void        hal_i2s_tx_en(uint32_t i2s_base,hal_en_t en);           
void        hal_i2s_en(uint32_t i2s_base,hal_en_t en);
void        hal_i2s_tx_fifo_flush(uint32_t i2s_base);
void        hal_i2s_rx_fifo_flush(uint32_t i2s_base);
void        hal_i2s_send_data(uint32_t i2s_base,uint32_t left_data,uint32_t right_data);
void        hal_i2s_recv_data(uint32_t i2s_base,uint32_t *left_data,uint32_t *right_data);

            //interrupt
void        hal_i2s_it_cfg(uint32_t i2s_base,i2s_it_flag_t i2s_it_flag,hal_en_t en);
uint8_t     hal_i2s_get_it_flag(uint32_t i2s_base,i2s_it_flag_t i2s_it_flag);
void        hal_i2s_clr_it_flag(uint32_t i2s_base,i2s_it_flag_t i2s_it_flag);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_I2S_H */


/**************************************END OF FILE********************************************/
