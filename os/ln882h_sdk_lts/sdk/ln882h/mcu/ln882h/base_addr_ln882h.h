#ifndef __BASE_ADDR_LN882H_H_
#define __BASE_ADDR_LN882H_H_

#define    DMA_BASE                   0x20100000
#define    CACHE_BASE                 0x20200000


/*    APB0  */
#define    SYSC_CMP_BASE              0x40000000
#define    ADC_BASE                   0x40000800

#define    PWM_BASE                   0x40001000
#define    EFUSE_BASE                 0x40001800

#define    UART0_BASE                 0x40002000
#define    UART1_BASE                 0x40003000
#define    UART2_BASE                 0x40004000

#define    SPI0_BASE                  0x40005000
#define    SPI1_BASE                  0x40006000

#define    WS2811_BASE                0x40007000

#define    I2C_BASE                   0x40008000
#define    I2S_BASE                   0x40009000

#define    TIMER_BASE                 0x4000A000
#define    WDT_BASE                   0x4000B000
#define    GPIOA_BASE                 0x4000C000
#define    GPIOB_BASE                 0x4000C400
#define    TRNG_BASE                  0x4000C800

#define    BLE_MDM_BASE               0x4000D000

#define    MAC_PCU_REG_BASE           0x40010000
#define    MAC_CE_REG_BASE            0x40011000
#define    RF_P0_BASE                 0x40012000
#define    RF_P1_BASE                 0x40012400


/* APB1 */
#define    SYSC_AWO_BASE              0x40100000
#define    RTC_BASE                   0x40101000


#define    QSPI_BASE                  0x40200000
#define    SDIO_BASE                  0x40300000
#define    AES_BASE                   0x40400000


#define    BLE_BASEBAND_BASE          0x40420000
#define    BLE_EM_BASE                0x40430000

#endif /* __BASE_ADDR_LN882H_H_ */
