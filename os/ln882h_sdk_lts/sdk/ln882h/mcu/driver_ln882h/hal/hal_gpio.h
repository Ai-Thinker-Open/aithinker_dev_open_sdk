/**
 * @file     hal_gpio.h
 * @author   BSP Team 
 * @brief    This file contains all of GPIO functions prototypes.
 * @version  0.0.0.1
 * @date     2021-08-17
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "reg_gpio.h"
      
typedef enum
{
    GPIO_PULL_UP               = 0,
    GPIO_PULL_DOWN             = 1,
    GPIO_PULL_NONE             = 2,
    GPIO_PULL_UP_AND_PULL_DOWN = 3,
} gpio_pull_t;

typedef enum
{
    GPIO_NORMAL_SPEED          = 0,
    GPIO_HIGH_SPEED            = 1,
} gpio_speed_t;

typedef enum
{
    GPIO_MODE_DIGITAL          = 0,
    GPIO_MODE_ANALOG           = 1,
    GPIO_MODE_AFIO             = 2, //alternate function
} gpio_mode_t;

typedef enum
{
    GPIO_INPUT                 = 0,
    GPIO_OUTPUT                = 1,
} gpio_direction_t;

typedef enum
{
    GPIO_INT_RISING            = 0,
    GPIO_INT_FALLING           = 1,
    GPIO_INT_RISING_FALLING    = 2,
} gpio_int_type_t;

typedef enum
{
    I2C0_SCL                   = 0 , //  B         
    I2C0_SDA                   = 1 , //  B    

                                         
    UART0_TX                   = 2 , //  O    
    UART0_RX                   = 3 , //  I    
    UART0_RTS                  = 4 , //  O    
    UART0_CTS                  = 5 , //  I    
    UART1_TX                   = 6 , //  O    
    UART1_RX                   = 7 , //  I    
    UART2_TX                   = 8 , //  O    
    UART2_RX                   = 9 , //  I    
                               
    TIMER0_PWM                 = 10, //  O    
    TIMER1_PWM                 = 11, //  O    
    TIMER2_PWM                 = 12, //  O    
    TIMER3_PWM                 = 13, //  O    
                               
    ADV_TIMER_PWM0             = 14, //  B    
    ADV_TIMER_PWM1             = 15, //  O    
    ADV_TIMER_PWM2             = 16, //  B    
    ADV_TIMER_PWM3             = 17, //  O    
    ADV_TIMER_PWM4             = 18, //  B    
    ADV_TIMER_PWM5             = 19, //  O    
    ADV_TIMER_PWM6             = 20, //  B    
    ADV_TIMER_PWM7             = 21, //  O    
    ADV_TIMER_PWM8             = 22, //  B    
    ADV_TIMER_PWM9             = 23, //  O    
    ADV_TIMER_PWM10            = 24, //  B    
    ADV_TIMER_PWM11            = 25, //  O    
                               
                               
    SPI0_CLK                   = 26, //  O    
    SPI0_CSN                   = 27, //  O    
    SPI0_MOSI                  = 28, //  O    
    SPI0_MISO                  = 29, //  I    

    SPI1_CLK                   = 30, //  O    
    SPI1_CSN                   = 31, //  O    
    SPI1_MOSI                  = 32, //  O    
    SPI1_MISO                  = 33, //  I    
                               
    WS2811_OUT                 = 34, //  O
                               
    CLK_TEST0                  = 35, //  O
    CLK_TEST1                  = 36, //  O 
    TX_IND                     = 37, //  O
    RX_IND                     = 38, //  O
    WIFI_BLE_IND               = 39, //  O
} afio_function_t;

#define IS_GPIOA_AFIO(IO)     ((IO) <= GPIO_PIN_12)
#define IS_GPIOB_AFIO(IO)     ((IO) >= GPIO_PIN_3 && (IO) <= GPIO_PIN_9)

typedef enum
{
    GPIO_PIN_0                 = ((uint16_t)0x0001),         
    GPIO_PIN_1                 = ((uint16_t)0x0002),    
    GPIO_PIN_2                 = ((uint16_t)0x0004),    
    GPIO_PIN_3                 = ((uint16_t)0x0008),
    GPIO_PIN_4                 = ((uint16_t)0x0010),    
    GPIO_PIN_5                 = ((uint16_t)0x0020),    
    GPIO_PIN_6                 = ((uint16_t)0x0040),    
    GPIO_PIN_7                 = ((uint16_t)0x0080),    
    GPIO_PIN_8                 = ((uint16_t)0x0100),    
    GPIO_PIN_9                 = ((uint16_t)0x0200),    
    GPIO_PIN_10                = ((uint16_t)0x0400),    
    GPIO_PIN_11                = ((uint16_t)0x0800),    
    GPIO_PIN_12                = ((uint16_t)0x1000),    
    GPIO_PIN_13                = ((uint16_t)0x2000),    
    GPIO_PIN_14                = ((uint16_t)0x4000),    
    GPIO_PIN_15                = ((uint16_t)0x8000),    
    GPIO_PIN_All               = ((uint16_t)0xFFFF),
} gpio_pin_t;

typedef struct
{
    gpio_pin_t                 pin;
    gpio_pull_t                pull;
    gpio_speed_t               speed;
    gpio_mode_t                mode;
    gpio_direction_t           dir;    
} gpio_init_t_def;


/**=======================================hal gpio api============================================**/
//              gpio init
void            hal_gpio_init(uint32_t gpio_base, gpio_init_t_def *gpio_init);
void            hal_gpio_deinit(uint32_t gpio_base);
void            hal_gpio_pin_pull_set(uint32_t gpio_base, gpio_pin_t pin, gpio_pull_t pull);
void            hal_gpio_pin_speed_set(uint32_t gpio_base, gpio_pin_t pin, gpio_speed_t speed);
void            hal_gpio_pin_mode_set(uint32_t gpio_base, gpio_pin_t pin, gpio_mode_t mode);
void            hal_gpio_pin_direction_set(uint32_t gpio_base, gpio_pin_t pin, gpio_direction_t dir);
                
//              gpio read/write set/reset
uint16_t        hal_gpio_port_input_read(uint32_t gpio_base);
uint16_t        hal_gpio_port_output_read(uint32_t gpio_base);
uint8_t         hal_gpio_pin_input_read(uint32_t gpio_base, gpio_pin_t pin);
uint8_t         hal_gpio_pin_output_read(uint32_t gpio_base, gpio_pin_t pin);
uint8_t         hal_gpio_pin_read(uint32_t gpio_base, gpio_pin_t pin);
void            hal_gpio_port_output_write(uint32_t gpio_base, uint16_t port_val);
void            hal_gpio_pin_set(uint32_t gpio_base, gpio_pin_t pin);
void            hal_gpio_pin_reset(uint32_t gpio_base, gpio_pin_t pin);
void            hal_gpio_pin_toggle(uint32_t gpio_base, gpio_pin_t pin);

//              gpio interrupt
void            hal_gpio_pin_it_en(uint32_t gpio_base, gpio_pin_t pin, hal_en_t en);
void            hal_gpio_pin_it_cfg(uint32_t gpio_base, gpio_pin_t pin, gpio_int_type_t type);
uint8_t         hal_gpio_pin_get_it_flag(uint32_t gpio_base, gpio_pin_t pin);
void            hal_gpio_pin_clr_it_flag(uint32_t gpio_base, gpio_pin_t pin);
                
//              alternate function
void            hal_gpio_pin_afio_select(uint32_t gpio_base, gpio_pin_t pin, afio_function_t fun);
void            hal_gpio_pin_afio_en(uint32_t gpio_base, gpio_pin_t pin, hal_en_t en);




#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HAL_GPIO_H */

