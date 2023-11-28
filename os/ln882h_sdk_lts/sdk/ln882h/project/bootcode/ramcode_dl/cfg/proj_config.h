#ifndef _PROJ_CONFIG_H_
#define _PROJ_CONFIG_H_

#define DISABLE    0
#define ENABLE     1

#define PLL_CLOCK  (160000000)
#define XTAL_CLOCK (40000000)

/*
 * System clock config
 */
#define USE_PLL    DISABLE

#if (USE_PLL == ENABLE)
#  define SOURCE_CLOCK PLL_CLOCK
#else
#  define SOURCE_CLOCK XTAL_CLOCK
#endif

#define SYSTEM_CLOCK (SOURCE_CLOCK)
#define AHBUS_CLOCK  (SYSTEM_CLOCK)
#define APBUS0_CLOCK (AHBUS_CLOCK / 2)
#define APBUS1_CLOCK (AHBUS_CLOCK / 4)
#define QSPI_CLK     (AHBUS_CLOCK / 4)

#if ((AHBUS_CLOCK % APBUS0_CLOCK) || (AHBUS_CLOCK % APBUS1_CLOCK))
#  error "AHBUS_CLOCK % APBUS0_CLOCK != 0 or AHBUS_CLOCK % APBUS1_CLOCK != 0"
#endif

#define FLASH_XIP                 DISABLE
#define HAL_ASSERT_EN             ENABLE
#define LN_ASSERT_EN              ENABLE

/*
 * Hardware config
 */
#define CFG_UART0_TX_BUF_SIZE     (512)
#define CFG_UART0_RX_BUF_SIZE     (512 * 3)
#define CFG_UART1_TX_BUF_SIZE     256
#define CFG_UART1_RX_BUF_SIZE     256
#define CFG_UART2_TX_BUF_SIZE     256
#define CFG_UART2_RX_BUF_SIZE     256

#define CFG_UART_BAUDRATE_LOG     921600
#define CFG_UART_BAUDRATE_CONSOLE 115200

// the flash actual size on chip
#define FLASH_MAX_SIZE_ON_CHIP    (1024 * 1024 * 4)

// verify data which has been programmed into flash.
#define FLASH_VERIFY_ENABLE       (1)

#endif /* _PROJ_CONFIG_H_ */
