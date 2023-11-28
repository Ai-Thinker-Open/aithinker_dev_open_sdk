/**
 * @file     hal_dma.h
 * @author   BSP Team 
 * @brief    This file contains all of DMA functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-15
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_DMA_H
#define __HAL_DMA_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "hal/hal_common.h"
#include "reg_dma.h"


#define WS2811_DATA_REG      (WS2811_BASE + 0x10)
#define SPI0_DATA_REG        (SPI0_BASE   + 0x0c)
#define SPI1_DATA_REG        (SPI1_BASE   + 0x0c)
#define UART0_RX_DATA_REG    (UART0_BASE  + 0x24)
#define UART0_TX_DATA_REG    (UART0_BASE  + 0x28)
#define UART1_RX_DATA_REG    (UART1_BASE  + 0x24)
#define UART1_TX_DATA_REG    (UART1_BASE  + 0x28)
#define UART2_RX_DATA_REG    (UART2_BASE  + 0x24)
#define UART2_TX_DATA_REG    (UART2_BASE  + 0x28)
#define I2C_DATA_REG         (I2C_BASE    + 0x10)

#define ADC_DATA_REG_0       (ADC_BASE    + 0x20)
#define ADC_DATA_REG_1       (ADC_BASE    + 0x24)
#define ADC_DATA_REG_2       (ADC_BASE    + 0x28)
#define ADC_DATA_REG_3       (ADC_BASE    + 0x2C)
#define ADC_DATA_REG_4       (ADC_BASE    + 0x30)
#define ADC_DATA_REG_5       (ADC_BASE    + 0x34)
#define ADC_DATA_REG_6       (ADC_BASE    + 0x38)
#define ADC_DATA_REG_7       (ADC_BASE    + 0x3C)

/*
    DMA_CH1  ->   QSPI_TX
             ->   QSPI_RX

    DMA_CH2  ->   WS2811_OUT
    
    DMA_CH3  ->   SPI1_RX
             ->   SPI0_RX
             
    DMA_CH4  ->   SPI0_TX
             ->   SPI1_TX
             
    DMA_CH5  ->   UART0_RX
             ->   UART1_RX
             ->   UART2_RX
             
    DMA_CH6  ->   UART0_TX
             ->   UART1_TX
             ->   UART2_TX   
             
    DMA_CH7  ->   I2C0
             ->   ADC
             
*/


#define DMA_IT     (DMA_BASE)

#define DMA_CH_1   (DMA_BASE + 0x08 + 0x14 * 0)
#define DMA_CH_2   (DMA_BASE + 0x08 + 0x14 * 1)
#define DMA_CH_3   (DMA_BASE + 0x08 + 0x14 * 2)
#define DMA_CH_4   (DMA_BASE + 0x08 + 0x14 * 3)
#define DMA_CH_5   (DMA_BASE + 0x08 + 0x14 * 4)
#define DMA_CH_6   (DMA_BASE + 0x08 + 0x14 * 5)
#define DMA_CH_7   (DMA_BASE + 0x08 + 0x14 * 6)

#define IS_DMA_IT_BASE(BASE)      ((BASE  == DMA_IT))

#define IS_DMA_ALL_PERIPH(PERIPH) (((PERIPH) == DMA_CH_1)  || ((PERIPH) == DMA_CH_2)   ||  \
                                   ((PERIPH) == DMA_CH_3)  || ((PERIPH) == DMA_CH_4)   ||  \
                                   ((PERIPH) == DMA_CH_5)  || ((PERIPH) == DMA_CH_6)   ||  \
                                   ((PERIPH) == DMA_CH_7))     

typedef enum
{
    DMA_MEM_TO_MEM_DIS     = 0,     
    DMA_MEM_TO_MEM_EN      = 1,        
}dma_mem_to_mem_en_t;

#define IS_DMA_MEM_TO_MEM_EN(EN)  (((EN) == DMA_MEM_TO_MEM_DIS)  || ((EN) == DMA_MEM_TO_MEM_EN)) 

typedef enum
{
    DMA_PRI_LEV_LOW      = 0,     
    DMA_PRI_LEV_MEDIUM   = 1,     
    DMA_PRI_LEV_HIGH     = 2,     
    //DMA_PRI_LEV_HIGH     = 3,          
}dma_pri_lev_t;

#define IS_DMA_PRI_LEV(LEV) (((LEV) == DMA_PRI_LEV_LOW) || ((LEV) == DMA_PRI_LEV_MEDIUM) || ((LEV) == DMA_PRI_LEV_HIGH))

typedef enum
{
    DMA_MEM_SIZE_8_BIT      = 0,     
    DMA_MEM_SIZE_16_BIT     = 1,     
    DMA_MEM_SIZE_32_BIT     = 2,     
    //DMA_MEM_SIZE_RESERVED   = 3,          
}dma_mem_size_t;

#define IS_DMA_MEM_SIZE(SIZE)  (((SIZE) == DMA_MEM_SIZE_8_BIT)  || ((SIZE) == DMA_MEM_SIZE_16_BIT) || \
                                ((SIZE) == DMA_MEM_SIZE_32_BIT))

typedef enum
{
    DMA_P_SIZE_8_BIT      = 0,     
    DMA_P_SIZE_16_BIT     = 1,     
    DMA_P_SIZE_32_BIT     = 2,     
    //DMA_P_SIZE_RESERVED   = 3,          
}dma_p_size_t;

#define IS_DMA_P_SIZE(SIZE)  (((SIZE) == DMA_P_SIZE_8_BIT)  || ((SIZE) == DMA_P_SIZE_16_BIT) || \
                              ((SIZE) == DMA_P_SIZE_32_BIT))

typedef enum
{
    DMA_MEM_INC_DIS     = 0,     
    DMA_MEM_INC_EN      = 1,        
}dma_mem_inc_en_t;

#define IS_DMA_MEM_INC_EN(EN)  (((EN) == DMA_MEM_INC_DIS)  || ((EN) == DMA_MEM_INC_EN)) 

typedef enum
{
    DMA_P_INC_DIS     = 0,     
    DMA_P_INC_EN      = 1,        
}dma_p_inc_en_t;

#define IS_DMA_P_INC_EN(EN)  (((EN) == DMA_P_INC_DIS)  || ((EN) == DMA_P_INC_EN)) 

typedef enum
{
    DMA_CIRC_MODE_DIS     = 0,     
    DMA_CIRC_MODE_EN      = 1,        
}dma_circ_mode_en_t;

#define IS_DMA_CIRC_MODE_EN(EN)  (((EN) == DMA_CIRC_MODE_DIS)  || ((EN) == DMA_CIRC_MODE_EN)) 


typedef enum
{
    DMA_READ_FORM_P         = 0,     
    DMA_READ_FORM_MEM       = 1,        
}dma_dir_t;

#define IS_DMA_DIR(EN)  (((EN) == DMA_READ_FORM_P)  || ((EN) == DMA_READ_FORM_MEM)) 


#define IS_DMA_NDT_VALUE(VALUE)  (((VALUE) <= 0xFFFF)) 



#define IS_DMA_PA_VALUE(VALUE)  (((VALUE) <= 0xFFFFFFFF)) 

#define IS_DMA_MA_VALUE(VALUE)  (((VALUE) <= 0xFFFFFFFF)) 

typedef enum
{
    DMA_IT_FLAG_TRAN_ERR            = 0,     // [4] transfer error flag
    DMA_IT_FLAG_TRAN_HALF           = 1,     // [3] half transfer flag   
    DMA_IT_FLAG_TRAN_COMP           = 2,     // [2] transfer complete flag  
    DMA_IT_FLAG_TRAN_GLOB           = 3,     // [1] global interrupt flag 
}dma_it_flag_t;

#define IS_DMA_IT_FLAG_VALUE(VALUE)  ( ((VALUE) <= 3)) 


typedef enum
{
    DMA_STATUS_FLAG_TRAN_ERR        = 0,     // [4] transfer error flag   
    DMA_STATUS_FLAG_TRAN_HALF       = 1,     // [3] half transfer flag        
    DMA_STATUS_FLAG_TRAN_COMP       = 2,     // [2] transfer complete flag     
    DMA_STATUS_FLAG_TRAN_GLOB       = 3,     // [1] global interrupt flag   
}dma_status_flag_t;

#define IS_STATUS_FLAG_VALUE(VALUE)  (((VALUE) <= 3)) 
typedef struct
{

    dma_mem_to_mem_en_t     dma_mem_to_mem_en;              /*!< Specifies the dma memory to memory mode enable status.
                                                            This parameter can be a value of dma_mem_to_mem_en_t */

    dma_pri_lev_t           dma_pri_lev;                    /*!< Specifies the dma channel priority level.
                                                            This parameter can be a value of dma_pri_lev_t */

    dma_mem_size_t          dma_mem_size;                   /*!< Specifies the dma timer memory size.
                                                            This parameter can be a value of dma_mem_size_t */

    dma_p_size_t            dma_p_size;                     /*!< Specifies the dma peripheral size.
                                                            This parameter can be a value of dma_p_size_t */

    dma_mem_inc_en_t        dma_mem_inc_en;                 /*!< Specifies the dma memory increment mode enable status.
                                                            This parameter can be a value of dma_mem_inc_en_t */

    dma_p_inc_en_t          dma_p_inc_en;                   /*!< Specifies the dma peripheral increment mode enable status.
                                                            The parameter range can be referred to dma_p_inc_en_t*/

    dma_circ_mode_en_t      dma_circ_mode_en;               /*!< Specifies the dma circular mode enable status.
                                                            The parameter range can be referred to dma_circ_mode_en_t*/

    dma_dir_t               dma_dir;                        /*!< Specifies the dma direction.
                                                            The parameter range can be referred to dma_dir_t*/

    uint16_t                dma_data_num;                   /*!< Specifies the dma number of data to transfer.
                                                            The parameter range can be referred to IS_DMA_NDT_VALUE*/

    uint32_t                dma_mem_addr;                   /*!< Specifies the dma memory address.
                                                            The parameter range can be referred to IS_DMA_MA_VALUE*/

    uint32_t                dma_p_addr;                     /*!< Specifies the dma peripheral address.
                                                            The parameter range can be referred to IS_DMA_PA_VALUE*/
}dma_init_t_def;


            //DMA init and config
void        hal_dma_init(uint32_t dma_x_base,dma_init_t_def* dma_init);    
void        hal_dma_deinit(void);
void        hal_dma_en(uint32_t dma_x_base,hal_en_t en);
void        hal_dma_set_dir(uint32_t dma_x_base,dma_dir_t dma_dir);         
void        hal_dma_set_mem_addr(uint32_t dma_x_base,uint32_t dma_mem_addr);         
void        hal_dma_set_p_addr(uint32_t dma_x_base,uint32_t dma_p_addr); 
void        hal_dma_set_data_num(uint32_t dma_x_base,uint16_t dma_data_num);       

void        hal_dma_set_mem_size(uint32_t dma_x_base,dma_mem_size_t dma_mem_size);         
void        hal_dma_set_p_size(uint32_t dma_x_base,dma_p_size_t dma_p_size);         
void        hal_dma_set_pri_lev(uint32_t dma_x_base,dma_pri_lev_t dma_pri_lev);  
void        hal_dma_set_mem_inc_en(uint32_t dma_x_base,dma_mem_inc_en_t dma_mem_inc_en);
void        hal_dma_set_p_inc_en(uint32_t dma_x_base,dma_p_inc_en_t dma_p_inc_en);   

uint16_t    hal_dma_get_data_num(uint32_t dma_x_base);

            //interrupt
void        hal_dma_it_cfg(uint32_t dma_x_base,dma_it_flag_t dma_it_flag,hal_en_t en);
uint8_t     hal_dma_get_it_flag(uint32_t dma_x_base,dma_it_flag_t dma_it_flag);
void        hal_dma_clr_it_flag(uint32_t dma_x_base,dma_it_flag_t dma_it_flag);

uint8_t     hal_dma_get_status_flag(uint32_t dma_x_base,dma_status_flag_t dma_status_flag);
void        hal_dma_clr_status_flag(uint32_t dma_x_base,dma_status_flag_t dma_status_flag);




#ifdef __cplusplus
}
#endif

#endif /* __HAL_DMA_H */
