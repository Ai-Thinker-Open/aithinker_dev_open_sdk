#ifndef _PROJ_CONFIG_H_
#define _PROJ_CONFIG_H_

#define DISABLE                                 (0)
#define ENABLE                                  (1)

/*
 * Clock settings section
 * Note:
 *      
 */
#define XTAL_CLOCK                              (40000000)
#define RCO_CLOCK                               (32000)
#define PLL_CLOCK                               (160000000)


/*
 * Module enable/disable control
 */
#define FULL_ASSERT   DISABLE
#define PRINTF_OMIT   DISABLE     // when release software, set 1 to omit all printf logs
#define PRINT_TO_RTT  DISABLE     // 1: print to segger rtt   0: print to uart1

#define FLASH_XIP     ENABLE

/*
 * flash image settings
 */
#define FLASH_IMAGE_VER_MAJOR       0
#define FLASH_IMAGE_VER_MINOR       1
#define SOC_CRP_FLAG                0



/*
 * Hardware config
 */
#define CFG_UART0_TX_BUF_SIZE      256
#define CFG_UART0_RX_BUF_SIZE      512

#define CFG_UART_BAUDRATE_LOG      115200



#endif /* _PROJ_CONFIG_H_ */


