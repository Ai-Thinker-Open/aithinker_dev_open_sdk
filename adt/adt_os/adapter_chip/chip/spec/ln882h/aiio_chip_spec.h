#ifndef __AIIO_CHIP_SPEC_H__
#define __AIIO_CHIP_SPEC_H__

#include "aiio_chip_spec_config.h"

/* INCN SPEC */
#include "hal/hal_gpio.h"
#include "base_addr_ln882h.h"
/* TYPE SPEC */
#ifdef      __AIIO_DEFAULT_DATE_TYPE

#endif

#ifdef      __AIIO_DATA_TYPE

#endif

/* RTOS SPEC */
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "task.h"
#include <lwip/netdb.h>
#include "lwip/sockets.h"
#include "lwip/errno.h"
#ifdef configMAX_PRIORITIES
/**< The current OS maximum priority is configMAX_PRIORITIES, and the user-available range is (configMAX_PRIORITIES -1)*/
#define     __AIIO_OS_PRIORITY_DEFAULT  (configMAX_PRIORITIES/2-1)
#define     __AIIO_OS_PRIORITY_FREE  (__AIIO_OS_PRIORITY_DEFAULT-1)
#else
#define     __AIIO_OS_PRIORITY_DEFAULT  (3)
#define     __AIIO_OS_PRIORITY_FREE  (__AIIO_OS_PRIORITY_DEFAULT-1)
#endif

/* CHIP INFO */
#define __AIIO_CHIP_NAME    "LN882H"


/* GPIO SPEC */
#define     __AIIO_GPIO_P_ENABLE        (1)

#if(__AIIO_GPIO_P_ENABLE == 1)
    #define     __AIIO_GPIO_A               (GPIOA_BASE)
    #define     __AIIO_GPIO_B               (GPIOB_BASE)
#else
    #define     __AIIO_GPIO_A               (0)
    #define     __AIIO_GPIO_B               (0)
#endif

#define     __AIIO_GPIO_NUM_0           GPIO_PIN_0     /*!< GPIO0, input and output */
#define     __AIIO_GPIO_NUM_1           GPIO_PIN_1     /*!< GPIO1, input and output */
#define     __AIIO_GPIO_NUM_2           GPIO_PIN_2     /*!< GPIO2, input and output */
#define     __AIIO_GPIO_NUM_3           GPIO_PIN_3     /*!< GPIO3, input and output */
#define     __AIIO_GPIO_NUM_4           GPIO_PIN_4     /*!< GPIO4, input and output */
#define     __AIIO_GPIO_NUM_5           GPIO_PIN_5     /*!< GPIO5, input and output */
#define     __AIIO_GPIO_NUM_6           GPIO_PIN_6     /*!< GPIO7, input and output */
#define     __AIIO_GPIO_NUM_7           GPIO_PIN_7     /*!< GPIO7, input and output */
#define     __AIIO_GPIO_NUM_8           GPIO_PIN_8     /*!< GPIO8, input and output */
#define     __AIIO_GPIO_NUM_9           GPIO_PIN_9     /*!< GPIO7, input and output */
#define     __AIIO_GPIO_NUM_10          GPIO_PIN_10     /*!< GPIO7, input and output */
#define     __AIIO_GPIO_NUM_11          GPIO_PIN_11   /*!< GPIO11, input and output */
#define     __AIIO_GPIO_NUM_12          GPIO_PIN_12   /*!< GPIO12, input and output */
#define     __AIIO_GPIO_NUM_13          GPIO_PIN_13     /*!< GPIO7, input and output */
#define     __AIIO_GPIO_NUM_14          GPIO_PIN_14   /*!< GPIO14, input and output */
#define     __AIIO_GPIO_NUM_15          GPIO_PIN_15     /*!< GPIO7, input and output */
#define     __AIIO_GPIO_NUM_MAX         (16)   /*!< max gpio */

#define     __BOOT_NUM_  __AIIO_GPIO_NUM_9   //pa9
#define     __FACTORY_LED_RED_NUM_      __AIIO_GPIO_NUM_12
#define     __FACTORY_LED_GREEN_NUM_    __AIIO_GPIO_NUM_14
#define     __FACTORY_LED_BLUE_NUM_     __AIIO_GPIO_NUM_15
/* UART SPEC */
#define __AIIO_UART0    0
#define __AIIO_UART1    1
#define __AIIO_UART2    2

#define     __AIIO_UART0_TX_P           __AIIO_GPIO_B
#define     __AIIO_UART0_TX             __AIIO_GPIO_NUM_9
#define     __AIIO_UART0_RX_P           __AIIO_GPIO_B
#define     __AIIO_UART0_RX             __AIIO_GPIO_NUM_8

/* UART SPEC */
#define     __AIIO_UART1_TX_P           __AIIO_GPIO_A
#define     __AIIO_UART1_TX             __AIIO_GPIO_NUM_2
#define     __AIIO_UART1_RX_P           __AIIO_GPIO_A
#define     __AIIO_UART1_RX             __AIIO_GPIO_NUM_3

/* UART SPEC */
#define     __AIIO_UART2_TX_P           __AIIO_GPIO_A
#define     __AIIO_UART2_TX             __AIIO_GPIO_NUM_11
#define     __AIIO_UART2_RX_P           __AIIO_GPIO_A
#define     __AIIO_UART2_RX             __AIIO_GPIO_NUM_12


/* RTOS SPEC */

/*  */

#define     __MAIN_ENTRY()          void main(void)

#endif
