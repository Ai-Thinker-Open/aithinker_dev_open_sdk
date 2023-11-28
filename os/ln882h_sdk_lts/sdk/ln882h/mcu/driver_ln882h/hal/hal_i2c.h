/**
 * @file     hal_i2c.h
 * @author   BSP Team 
 * @brief    This file contains all of I2C functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-11
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_I2C_H
#define __HAL_I2C_H

#ifdef __cplusplus
    extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "hal/hal_common.h"
#include "reg_i2c.h"


#define IS_I2C_ALL_PERIPH(PERIPH) (((PERIPH) == I2C_BASE)) 


typedef enum
{
    I2C_SW_NO_RST    = 0,
    I2C_SW_RST       = 1,
}i2c_sw_rst_t;

#define IS_I2C_SW_RST(RST) (((RST) == I2C_SW_NO_RST) || ((RST) == I2C_SW_RST))

typedef enum
{
    I2C_SM_ALERT_MODEL_1    = 0,    /* Releases SMBA pin high. Alert Response Address Header followed by NACK. */
    I2C_SM_ALERT_MODEL_2    = 1,    /* Drives SMBA pin low. Alert Response Address Header followed by ACK.     */ 
}i2c_smbus_alert_model_t;

#define IS_I2C_SMBUS_ALERT_MODEL(MODEL) (((MODEL) == I2C_SM_ALERT_MODEL_1) || ((MODEL) == I2C_SM_ALERT_MODEL_2))


typedef enum
{
    I2C_PEC_CLOSE      = 0,         /* No PEC transfer.                 */
    I2C_PEC_OPEN       = 1,         /* PEC transfer (in Tx or Rx mode). */
}i2c_pec_t;

#define IS_I2C_PEC(PEC) (((PEC) == I2C_PEC_CLOSE) || ((PEC) == I2C_PEC_OPEN))

typedef enum
{
    I2C_POC_MODEL_1    = 0,         /* ACK bit controls the (N)ACK of the current byte being received in the shift register. 
                                    The PEC bit indicates that current byte in shift register is a PEC.      */
    I2C_POC_MODEL_2    = 1,         /* ACK bit controls the (N)ACK of the next byte which will be received in the shift register. 
                                    The PEC bit indicates that the next byte in the shift register is a PEC. */
}i2c_poc_t;

#define IS_I2C_POC(POC) (((POC) == I2C_POC_MODEL_1) || ((POC) == I2C_POC_MODEL_2))

typedef enum
{
    I2C_ACK_DIS    = 0,
    I2C_ACK_EN     = 1,
}i2c_ack_en_t;

#define IS_I2C_ACK_EN(EN) (((EN) == I2C_ACK_DIS) || ((EN) == I2C_ACK_EN))

typedef enum
{
    I2C_CLOCK_STRETCH_EN    = 0,
    I2C_CLOCK_STRETCH_DIS   = 1,
}i2c_clock_stretch_dis_t;

#define IS_I2C_CLOCK_STRETCH_EN(EN)  (((EN) == I2C_CLOCK_STRETCH_EN) || ((EN) == I2C_CLOCK_STRETCH_DIS))

typedef enum
{
    I2C_GENERAL_CALL_DIS    = 0,    /* General call disabled. Address 00h is NACKed. */
    I2C_GENERAL_CALL_EN     = 1,    /* General call enabled.  Address 00h is ACKed.  */
}i2c_general_call_en_t;

#define IS_I2C_GENERAL_CALL_EN(EN)  (((EN) == I2C_GENERAL_CALL_EN) || ((EN) == I2C_GENERAL_CALL_DIS))

typedef enum
{
    I2C_PEC_DIS    = 0,             /* PEC calculation disabled. */
    I2C_PEC_EN     = 1,             /* PEC calculation enabled.  */ 
}i2c_pec_en_t;

#define IS_I2C_PEC_EN(EN)  (((EN) == I2C_PEC_DIS) || ((EN) == I2C_PEC_EN))


typedef enum
{
    I2C_ARP_DIS     = 0,             
    I2C_ARP_EN      = 1,            
}i2c_arp_en_t;

#define IS_I2C_ARP_EN(EN)  (((EN) == I2C_ARP_DIS) || ((EN) == I2C_ARP_EN))


typedef enum
{
    I2C_SMBUS_DEVICE    = 0,             
    I2C_SMBUS_HOST      = 1,            
}i2c_smbus_type_t;

#define IS_I2C_SMBUS_TYPE(TYPE)  (((TYPE) == I2C_SMBUS_DEVICE) || ((TYPE) == I2C_SMBUS_HOST))


typedef enum
{
    I2C_I2C_MODE        = 0,             
    I2C_SMBUS_MODE      = 1,            
}i2c_mode_t;

#define IS_I2C_MODE(MODE)  (((MODE) == I2C_I2C_MODE) || ((MODE) == I2C_SMBUS_MODE))


typedef enum
{
    I2C_ADD_7BIT_MODE        = 0,             
    I2C_ADD_10BIT_MODE       = 1,            
}i2c_add_mode_t;

#define IS_I2C_ADD_MODE(MODE)  (((MODE) == I2C_ADD_7BIT_MODE) || ((MODE) == I2C_ADD_10BIT_MODE))


typedef enum
{
    I2C_SM_MODE              = 0,             
    I2C_FM_MODE              = 1,            
}i2c_master_mode_sel_t;

#define IS_I2C_MASTER_MODE_SEL(MODE)  (((MODE) == I2C_SM_MODE) || ((MODE) == I2C_FM_MODE))


typedef enum
{
    I2C_FM_MODE_DUTY_CYCLE_1        = 0,          // 0: Fm mode tlow/thigh = 2   
    I2C_FM_MODE_DUTY_CYCLE_2        = 1,          // 1: Fm mode tlow/thigh = 16/9 
}i2c_fm_mode_duty_cycle_t;

#define IS_I2C_FM_MODE_DUTY_CYCLE(DUTY_CYCLE)  (((DUTY_CYCLE) == I2C_FM_MODE_DUTY_CYCLE_1) || ((DUTY_CYCLE) == I2C_FM_MODE_DUTY_CYCLE_2))

#define IS_I2C_CCR(SPEED)                      (((SPEED) >= 1) && ((SPEED) <= 2047))
#define IS_I2C_TRISE(TRISE)                    (((TRISE) >= 1) && ((TRISE) <= 31))
#define IS_I2C_7_bit_ADD(ADD)                  (((ADD)   >= 1) && ((ADD)   <= 127))
#define IS_I2C_10_bit_ADD(ADD)                 (((ADD)   >= 1) && ((ADD)   <= 1023))




typedef enum
{
    //SR1
    I2C_STATUS_FLAG_SB              = 0,              // [0] Start bit (Master mode)                          
    I2C_STATUS_FLAG_ADDR            = 1,              // [1] Address sent (master mode)/matched (slave mode)   
    I2C_STATUS_FLAG_BTF             = 2,              // [2] Byte transfer finished
    I2C_STATUS_FLAG_ADD10           = 3,              // [3] 10-bit header sent (Master mode)
    I2C_STATUS_FLAG_STOPF           = 4,              // [4] Stop detection (slave mode)
                                                      // [5] Reserved
    I2C_STATUS_FLAG_RXNE            = 6,              // [6] Data register not empty (receivers)
    I2C_STATUS_FLAG_TXE             = 7,              // [7] Data register empty (transmitters)
    I2C_STATUS_FLAG_BERR            = 8,              // [8] Bus error
    I2C_STATUS_FLAG_ARLO            = 9,              // [9] Arbitration lost (master mode)
    I2C_STATUS_FLAG_AF              = 10,             // [10] Acknowledge failure
    I2C_STATUS_FLAG_OVR             = 11,             // [11] Overrun/Underrun
    I2C_STATUS_FLAG_PECERR          = 12,             // [12] PEC Error in reception
                                                      // [13] Reserved
    I2C_STATUS_FLAG_TIMEOUT         = 14,             // [14] Timeout or Tlow error
    I2C_STATUS_FLAG_SMBALERT        = 15,             // [15] SMBus alert

    //SR2
    I2C_STATUS_FLAG_MSL             = 16,             // [0] Master/slave, 0: Slave Mode, 1: Master Mode
    I2C_STATUS_FLAG_BUSY            = 17,             // [1] Bus busy
    I2C_STATUS_FLAG_TRA             = 18,             // [2] Transmitter/receiver
                                                      // [3] Reserved
    I2C_STATUS_FLAG_GENCALL         = 19,             // [4] General call address (Slave mode)
    I2C_STATUS_FLAG_SMBDEFAULT      = 20,             // [5] SMBus device default address (Slave mode)
    I2C_STATUS_FLAG_SMBHOST         = 21,             // [6] SMBus host header (Slave mode)
    I2C_STATUS_FLAG_DUALF           = 22,             // [7] Dual flag (Slave mode)
    I2C_STATUS_FLAG_PEC             = 23,             // [15:8]Packet error checking register


}i2c_status_flag_t;

#define IS_I2C_STATUS_FLAG(FLAG)        (( FLAG <= 23 ))


typedef enum
{

    I2C_IT_FLAG_SB              = 0,              // Start bit (Master mode)                           
    I2C_IT_FLAG_ADDR            = 1,              // Address sent (master mode)/matched (slave mode)   
    I2C_IT_FLAG_BTF             = 2,              // Byte transfer finished (Master),BTF = 1 with no TxE or RxNE event
    I2C_IT_FLAG_ADD10           = 3,              // 10-bit header sent (Master mode)
    I2C_IT_FLAG_STOPF           = 4,              // Stop detection (slave mode)
    I2C_IT_FLAG_RXNE            = 5,              // RxNE event to 1 if ITBUFEN = 1
    I2C_IT_FLAG_TXE             = 6,              // TxE event to 1 if ITBUFEN = 1
    I2C_IT_FLAG_BERR            = 7,              // Bus error
    I2C_IT_FLAG_ARLO            = 8,              // Arbitration lost (master mode)
    I2C_IT_FLAG_AF              = 9,              // Acknowledge failure
    I2C_IT_FLAG_OVR             = 10,             // Overrun/Underrun
    I2C_IT_FLAG_PECERR          = 11,             // PEC Error in reception
    I2C_IT_FLAG_TIMEOUT         = 12,             // Timeout or Tlow error
    I2C_IT_FLAG_SMBALERT        = 13,             // SMBus alert

}i2c_it_flag_t;

#define IS_I2C_IT_FLAG(FLAG)        (( FLAG <= 13 ))



typedef struct
{

    uint8_t                     i2c_peripheral_clock_freq;  /*!< Specifies the i2c peripheral_clock_freq.Unit is mhz.
                                                            This parameter must be set to a value lower than 36 mhz */

    i2c_sw_rst_t                i2c_sw_rst;                 /*!< Specifies the i2c reset status.
                                                            This parameter can be a value of i2c_sw_rst_t */

    i2c_smbus_alert_model_t     i2c_smbus_alert_model;      /*!< Specifies the i2c smbus alert model.
                                                            This parameter can be a value of i2c_smbus_alert_model_t */

    i2c_pec_t                   i2c_pec;                    /*!< Specifies whether the package error check mode is turned on.
                                                            This parameter can be a value of i2c_pec_t */

    i2c_poc_t                   i2c_poc;                    /*!< Specifies the package error check position.
                                                            This parameter can be a value of i2c_poc_t */

    i2c_ack_en_t                i2c_ack_en;                 /*!< Specifies the i2c ack enable status.
                                                            This parameter can be a value of i2c_ack_en_t */

    i2c_clock_stretch_dis_t     i2c_clock_stretch_dis;      /*!< Specifies the i2c clock stretch enable status.
                                                            This parameter can be a value of i2c_clock_stretch_dis_t */

    i2c_general_call_en_t       i2c_general_call_en;        /*!< Specifies the i2c general call enable status.
                                                            This parameter can be a value of i2c_general_call_en_t */ 

    i2c_pec_en_t                i2c_pec_en;                 /*!< Specifies the i2c package error check enable status.
                                                            This parameter can be a value of i2c_pec_en_t */      

    i2c_arp_en_t                i2c_arp_en;                 /*!< Specifies the i2c arp enable status.
                                                            This parameter can be a value of i2c_arp_en_t */           

    i2c_smbus_type_t            i2c_smbus_type;             /*!< Specifies the i2c smbus type.
                                                            This parameter can be a value of i2c_smbus_type_t */             

    i2c_mode_t                  i2c_mode;                   /*!< Specifies the i2c smbus mode.
                                                            This parameter can be a value of i2c_mode_t */  

    i2c_add_mode_t              i2c_add_mode;               /*!< Specifies the i2c add1 mode.
                                                            This parameter can be a value of i2c_add_mode_t */  

    i2c_master_mode_sel_t       i2c_master_mode_sel;        /*!< Specifies the i2c master mode.
                                                            This parameter can be a value of i2c_master_mode_sel_t */  

    i2c_fm_mode_duty_cycle_t    i2c_fm_mode_duty_cycle;     /*!< Specifies the i2c fast mode duty cycle.
                                                            This parameter can be a value of i2c_fm_mode_duty_cycle_t */  

    uint16_t                    i2c_ccr;                    /*!< Specifies the i2c SCL clock in master mode.
                                                            This parameter must be set to a value lower than 2048. */

    uint8_t                     i2c_trise;                  /*!< Specifies the i2c maximum rise time in Fm/Sm mode (master mode)
                                                            This parameter must be set to a value lower than 32. */

}i2c_init_t_def;


                //I2C init and config

void            hal_i2c_init(uint32_t i2c_x_base,i2c_init_t_def* i2c_init);              
void            hal_i2c_deinit(void);
void            hal_i2c_set_peripheral_clock_freq(uint32_t i2c_x_base, uint32_t peripheral_clock_freq);
void            hal_i2c_en(uint32_t i2c_x_base,hal_en_t en);
void            hal_i2c_dma_en(uint32_t i2c_x_base,hal_en_t en);
void            hal_i2c_ack_en(uint32_t i2c_x_base,hal_en_t en);


                //I2C master opetation
void            hal_i2c_master_reset(uint32_t i2c_x_base);
uint8_t         hal_i2c_master_start(uint32_t i2c_x_base,uint32_t timeout);
void            hal_i2c_master_stop(uint32_t i2c_x_base);
void            hal_i2c_master_send_data(uint32_t i2c_x_base,uint8_t data);
uint8_t         hal_i2c_master_recv_data(uint32_t i2c_x_base);

uint8_t         hal_i2c_master_wait_addr(uint32_t i2c_x_base,uint32_t timeout);                          
uint8_t         hal_i2c_master_wait_add10(uint32_t i2c_x_base,uint32_t timeout);                        


                //I2C slave opetation
void            hal_i2c_slave_set_add_mode(uint32_t i2c_x_base,i2c_add_mode_t add_mode);
void            hal_i2c_slave_set_add1(uint32_t i2c_x_base,uint16_t add);
void            hal_i2c_slave_set_add2(uint32_t i2c_x_base,uint16_t add);
uint8_t         hal_i2c_slave_wait_addr(uint32_t i2c_x_base,uint32_t timeout);                           //Wait for address matched


                //I2C universal opetation
uint8_t         hal_i2c_wait_txe(uint32_t i2c_x_base,uint32_t timeout);
uint8_t         hal_i2c_wait_rxne(uint32_t i2c_x_base,uint32_t timeout);
uint8_t         hal_i2c_wait_btf(uint32_t i2c_x_base,uint32_t timeout);
uint8_t         hal_i2c_wait_bus_idle(uint32_t i2c_x_base,uint32_t timeout);
void            hal_i2c_clear_sr(uint32_t i2c_x_base);                                                   //Clear SR1 and SR2  


                //interrupt
void            hal_i2c_it_cfg(uint32_t i2c_x_base,i2c_it_flag_t i2c_it_flag,hal_en_t en);
uint8_t         hal_i2c_get_it_flag(uint32_t i2c_x_base,i2c_it_flag_t i2c_it_flag);
void            hal_i2c_clr_it_flag(uint32_t i2c_x_base,i2c_it_flag_t i2c_it_flag);

uint8_t         hal_i2c_get_status_flag(uint32_t i2c_x_base,i2c_status_flag_t i2c_status_flag);
void            hal_i2c_clr_status_flag(uint32_t i2c_x_base,i2c_status_flag_t i2c_status_flag);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_I2C_H */


/**************************************END OF FILE********************************************/
