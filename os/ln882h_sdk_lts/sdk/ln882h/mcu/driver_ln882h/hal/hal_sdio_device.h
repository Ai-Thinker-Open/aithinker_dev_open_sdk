/**
 * @file     hal_sdio_device.h
 * @author   BSP Team 
 * @brief    This file contains all of SDIO DEVICE functions prototypes.
 * @version  0.0.0.1
 * @date     2021-03-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __HAL_SDIO_DEVICE_H
#define __HAL_SDIO_DEVICE_H

#ifdef __cpluscplus
    extern "C" {
#endif // __cpluscplus


#include "hal/hal_common.h"
#include "reg_sdio.h"


#define IS_SDIO_DEV_ALL_PERIPH(PERIPH)  (((PERIPH) == SDIO_BASE)) 

#define REVISION_REG_SD                 (0x232)     /* SDIO_CCCR    [0:11]  sdio card version      */
#define SDIO_HIGH_SPEED_EN              (1)         /* SDIO_CCCR    [21]    high speed mode enable */
#define SDIO_IO_READY                   (1)         
#define SDIO_DEVICE_READY               (1 << 0)    /* SDIO_PROGREG [0]     device ready,       */
#define SDIO_CARD_READY                 (1 << 1)    /* SDIO_PROGREG [1]     card ready,         */
#define SDIO_CPU_IN_ACTIVE              (1 << 2)    /* SDIO_PROGREG [2]     cpu active,         */
#define SDIO_FBR1_REG                   (0xFF)      /* SDIO_FBR_REG1[4:11]  extended standard sdio funcation interface code */
#define SDIO_INTERFACE_CODE             (0xF)       /* SDIO_PROGREG [0:3]   Standard SDIO Function interface code           */
#define SDIO_HOST_IIR_REG_CLEAR         (0)         /* SDIO_FN1_SPEC[24]    clear register      */
#define SDIO_HOST_IIR_REG_SET           (1)         /* SDIO_FN1_SPEC[24]    set DATA1 interrupt */
#define SDIO_SUPPORT_FUNC_NUM           (1 << 4)    /* SDIO_PROGREG [4:6]   support function number available 0~7, will be filled in R4 for CMD5  */

#define SDIO_FUN_CIS_LENGTH_OFFSET       4
#define SDIO_FUNC_CIS_CHECKSUM_OFFSET    6
#define SDIO_FUNC_BLOCK_SIZE             512
#define SET_CISTPL_CHECKSUM              0          //If Host check CIS tuple checksum, set this to 1.

typedef enum {
    SDIO_FUNC0 = 0,
    SDIO_FUNC1 = 1,
    SDIO_FUNC2 = 2,
    SDIO_FUNC3 = 3,
    SDIO_FUNC4 = 4,
    SDIO_FUNC5 = 5,
    SDIO_FUNC6 = 6,
    SDIO_FUNC7 = 7
}sdio_dev_func_num_t;

#define IS_SDIO_DEV_FUNC_NUM(NUM)  (((NUM) == SDIO_FUNC0) || ((NUM) == SDIO_FUNC1) || \
                                    ((NUM) == SDIO_FUNC2) || ((NUM) == SDIO_FUNC3) || \
                                    ((NUM) == SDIO_FUNC4) || ((NUM) == SDIO_FUNC5) || \
                                    ((NUM) == SDIO_FUNC6) || ((NUM) == SDIO_FUNC7))

typedef enum {
    FN1_CSA_SUPPORT = (1 << 0),
    FN2_CSA_SUPPORT = (1 << 1),
    FN3_CSA_SUPPORT = (1 << 2),
    FN4_CSA_SUPPORT = (1 << 3),
    FN5_CSA_SUPPORT = (1 << 4),
    FN6_CSA_SUPPORT = (1 << 5),
    FN7_CSA_SUPPORT = (1 << 6),
}sdio_dev_csa_support_t;

#define IS_SDIO_DEV_CSA_SUPPORT(FUN_NUM)   (FUN_NUM <= 0x7F)

typedef enum {
    SDIO_CCCR_CAP_SCSI_0 = 0,  
    SDIO_CCCR_CAP_SCSI_1 = 1,            /* Support Continuous SPI interrupt */
}sdio_dev_scsi_t;

#define IS_SDIO_DEV_SCSI(STATUS)         (((STATUS) == SDIO_CCCR_CAP_SCSI_0) || ((STATUS) == SDIO_CCCR_CAP_SCSI_1)) 

typedef enum {
    SDIO_CCCR_CAP_SDC_0 = 0,  
    SDIO_CCCR_CAP_SDC_1 = 1,             /* Card Supports Direct Commands(CMD52) during data transfer.  */
}sdio_dev_sdc_t;

#define IS_SDIO_DEV_SDC(STATUS)         (((STATUS) == SDIO_CCCR_CAP_SDC_0) || ((STATUS) == SDIO_CCCR_CAP_SDC_1)) 

typedef enum {
    SDIO_CCCR_CAP_SMB_0 = 0,  
    SDIO_CCCR_CAP_SMB_1 = 1,             /* Card Supports Multi-Block. (CMD53) */
}sdio_dev_smb_t;

#define IS_SDIO_DEV_SMB(STATUS)         (((STATUS) == SDIO_CCCR_CAP_SMB_0) || ((STATUS) == SDIO_CCCR_CAP_SMB_1)) 

typedef enum {
    SDIO_CCCR_CAP_SRW_0 = 0,  
    SDIO_CCCR_CAP_SRW_1 = 1,             /* Card Supports Read Wait. */
}sdio_dev_srw_t;

#define IS_SDIO_DEV_SRW(STATUS)         (((STATUS) == SDIO_CCCR_CAP_SRW_0) || ((STATUS) == SDIO_CCCR_CAP_SRW_1)) 

typedef enum {
    SDIO_CCCR_CAP_SBS_0 = 0,  
    SDIO_CCCR_CAP_SBS_1 = 1,             /* Card supports Suspend/Resume. */
}sdio_dev_sbs_t;

#define IS_SDIO_DEV_SBS(STATUS)         (((STATUS) == SDIO_CCCR_CAP_SBS_0) || ((STATUS) == SDIO_CCCR_CAP_SBS_1)) 

typedef enum {
    SDIO_CCCR_CAP_S4MI_0 = 0,  
    SDIO_CCCR_CAP_S4MI_1 = 1,            /* Supports interrupt between blocks of data in 4-bit SD mode.  */
}sdio_dev_s4mi_t;

#define IS_SDIO_DEV_S4MI(STATUS)         (((STATUS) == SDIO_CCCR_CAP_S4MI_0) || ((STATUS) == SDIO_CCCR_CAP_S4MI_1)) 

typedef enum {
    SDIO_CCCR_CAP_E4MI_0 = 0,  
    SDIO_CCCR_CAP_E4MI_1 = 1,            /* Enable interrupt between blocks of data in 4-bit SD mode. */
}sdio_dev_e4mi_t;

#define IS_SDIO_DEV_E4MI(STATUS)         (((STATUS) == SDIO_CCCR_CAP_E4MI_0) || ((STATUS) == SDIO_CCCR_CAP_E4MI_1)) 

typedef enum {
    SDIO_CCCR_CAP_LSC_0 = 0,  
    SDIO_CCCR_CAP_LSC_1 = 1,             /* Card is a Low-Speed card. */
}sdio_dev_lsc_t;

#define IS_SDIO_DEV_LSC(STATUS)         (((STATUS) == SDIO_CCCR_CAP_LSC_0) || ((STATUS) == SDIO_CCCR_CAP_LSC_1)) 

typedef enum {
    SDIO_CCCR_CAP_4BLS_0 = 0,  
    SDIO_CCCR_CAP_4BLS_1 = 1,            /* 4-bit support for Low-Speed cards. */
}sdio_dev_4bls_t;

#define IS_SDIO_DEV_4BLS(STATUS)         (((STATUS) == SDIO_CCCR_CAP_4BLS_0) || ((STATUS) == SDIO_CCCR_CAP_4BLS_1)) 


typedef enum {
    FN1_WRITE_OVER_INTERRPT     = 1 << 0,  
    FN1_READ_OVER_INTERRPT      = 1 << 1,  
    READ_ERROR_FN1_INTERRPT     = 1 << 2,  
    WRITE_ERROR_FN1_INTERRPT    = 1 << 3,  
    WRITE_ABORT_FN1_INTERRPT    = 1 << 4,  
    RESET_FN1_INTERRPT          = 1 << 5, 
    FN1_ENABLE_INTERRPT         = 1 << 6,  
    FN1_STATUS_PCRRT_INTERRPT   = 1 << 7,  
    FN1_STATUS_PCWRT_INTERRPT   = 1 << 8,  
    FN1_RTC_SET_INTERRPT        = 1 << 9,  
    FN1_CLINTRD_INTERRPT        = 1 << 10,  
    FN1_INT_EN_UP_INTERRPT      = 1 << 11,  
    FN1_M2S_INT_INTERRPT        = 1 << 20,  
}sdio_dev_int_flag_t;

#define IS_SDIO_DEV_INT_FLAG(STATUS)         (((STATUS) <= 0xFFFFF) ) 


typedef struct {
    sdio_dev_func_num_t             sdio_dev_func_num;          /*!< Specifies the sdio device funcation number.
                                                                This parameter can be a value of sdio_dev_func_num_t */
                                                                
    uint8_t                         sdio_dev_csa_support;       /*!< Specifies the sdio device csa support.
                                                                This parameter can be a value of sdio_dev_csa_support_t */
                                                                
    sdio_dev_scsi_t                 sdio_dev_scsi;              /*!< Specifies the sdio device support continuous SPI interrupt.
                                                                This parameter can be a value of sdio_dev_scsi_t */
                                                                
    sdio_dev_sdc_t                  sdio_dev_sdc;               /*!< Specifies the sdio device card supports direct commands(CMD52) during data transfer..
                                                                This parameter can be a value of sdio_dev_sdc_t */
                                                                
    sdio_dev_smb_t                  sdio_dev_smb;               /*!< Specifies the sdio device card supports multi-Block.
                                                                This parameter can be a value of sdio_dev_smb_t */
                                                                
    sdio_dev_srw_t                  sdio_dev_srw;               /*!< Specifies the sdio device card supports read wait.
                                                                This parameter can be a value of sdio_dev_srw_t */
                                                                
    sdio_dev_sbs_t                  sdio_dev_sbs;               /*!< Specifies the sdio device card supports suspend/resume.
                                                                This parameter can be a value of sdio_dev_sbs_t */
                                                                
    sdio_dev_s4mi_t                 sdio_dev_s4mi;              /*!< Specifies the sdio device supports interrupt between blocks of data in 4-bit SD mode.
                                                                This parameter can be a value of sdio_dev_s4mi_t */
                                                                
    sdio_dev_e4mi_t                 sdio_dev_e4mi;              /*!< Specifies the sdio device enable interrupt between blocks of data in 4-bit SD mode..
                                                                This parameter can be a value of sdio_dev_e4mi_t */
                                                                
    sdio_dev_lsc_t                  sdio_dev_lsc;               /*!< Specifies the sdio device card is a low-speed card..
                                                                This parameter can be a value of sdio_dev_lsc_t */
                                                                
    sdio_dev_4bls_t                 sdio_dev_4bls;              /*!< Specifies the sdio device 4-bit support for Low-Speed cards..
                                                                This parameter can be a value of sdio_dev_4bls_t */
                                                                
    
    uint8_t                        *cis_fn0_base;               /*!< Specifies the sdio device cis f0 base address.
                                                                This parameter can be a value of ram addr */
                                                                
    uint8_t                        *cis_fn1_base;               /*!< Specifies the sdio device cis f1 base address.
                                                                This parameter can be a value of ram addr */
                                                                
    uint8_t                        *recv_buff;                  /*!< Specifies the sdio device receive buffer address
                                                                This parameter can be a value of ram addr */
                                                                
} sdio_init_t;



void      hal_sdio_device_init(sdio_init_t *sdio_init);
void      hal_sdio_device_deinit(void);
void      hal_sdio_device_clear_busy(void);
void      hal_sdio_device_set_busy(void);
uint32_t* hal_sdio_device_get_recv_buf_addr(void);
void      hal_sdio_device_set_recv_buf_addr(uint8_t * addr);
uint16_t  hal_sdio_device_get_recv_buf_size(void);
uint32_t* hal_sdio_device_get_send_buf_addr(void);
void      hal_sdio_device_set_send_buf_addr(uint8_t *addr);
void      hal_sdio_device_set_send_buf_size(uint32_t size);
void      hal_sdio_device_trig_host_data1_int(void);
void      hal_sdio_device_trig_host_s2m1_int(void);
uint8_t*  hal_sdio_device_cis_func_get(sdio_dev_func_num_t fn);
uint8_t   hal_sdio_device_cis_fn_set(sdio_dev_func_num_t fn,uint32_t offset,uint8_t value);
void      hal_sdio_device_cis_init(void);

void      hal_sdio_device_it_cfg(uint32_t sdio_dev_int_flag,hal_en_t en);
uint8_t   hal_sdio_device_it_get_flag(sdio_dev_int_flag_t sdio_dev_int_flag);
void      hal_sdio_device_it_clr_flag(sdio_dev_int_flag_t sdio_dev_int_flag);




#ifdef __cpluscplus
}
#endif // __cpluscplus

#endif // __HAL_SDIO_DEVICE_H

