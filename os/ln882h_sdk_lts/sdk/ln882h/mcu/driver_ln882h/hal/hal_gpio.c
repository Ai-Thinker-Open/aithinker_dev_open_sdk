/**
 * @file     hal_gpio.c
 * @author   BSP Team 
 * @brief    This file provides GPIO function.
 * @version  0.0.0.1
 * @date     2021-08-17
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#include "hal/hal_gpio.h"

void hal_gpio_init(uint32_t gpio_base, gpio_init_t_def *gpio_init)
{
    sysc_cmp_gpioa_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();
    sysc_cmp_gpiob_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    gpio_pin_t pin = gpio_init->pin;
    
    hal_gpio_pin_pull_set(gpio_base, pin, gpio_init->pull);
    hal_gpio_pin_speed_set(gpio_base, pin, gpio_init->speed);
    hal_gpio_pin_mode_set(gpio_base, pin, gpio_init->mode);
    hal_gpio_pin_direction_set(gpio_base, pin, gpio_init->dir);
}

void hal_gpio_deinit(uint32_t gpio_base)
{
    if(gpio_base ==  GPIOA_BASE)
    {
        sysc_cmp_srstn_gpioa_setf(0);
        sysc_cmp_srstn_gpioa_setf(1);
    }
    else if(gpio_base ==  GPIOB_BASE)
    {
        sysc_cmp_srstn_gpiob_setf(0);
        sysc_cmp_srstn_gpiob_setf(1);
    }
}

void hal_gpio_pin_pull_set(uint32_t gpio_base, gpio_pin_t pin, gpio_pull_t pull)
{
    uint16_t pu_tmp = gpio_pu_getf(gpio_base);
    uint16_t pd_tmp = gpio_pd_getf(gpio_base);
    
    switch(pull)
    {
        case GPIO_PULL_UP:
            pu_tmp |= pin;  //set
            pd_tmp &= ~pin; //reset
            break;
        case GPIO_PULL_DOWN:
            pu_tmp &= ~pin; //reset
            pd_tmp |= pin;  //set
           break;
        case GPIO_PULL_NONE:
            pu_tmp &= ~pin; //reset
            pd_tmp &= ~pin; //reset
            break;
        case GPIO_PULL_UP_AND_PULL_DOWN:        //Do not use this.
            pu_tmp |= pin;  //set
            pd_tmp |= pin;  //set
        default:
            break;
    }
    gpio_pu_setf(gpio_base, pu_tmp);
    gpio_pd_setf(gpio_base, pd_tmp);
}

void hal_gpio_pin_speed_set(uint32_t gpio_base, gpio_pin_t pin, gpio_speed_t speed)
{
    uint16_t speed_tmp = gpio_hp_getf(gpio_base);
    
    switch(speed)
    {
        case GPIO_HIGH_SPEED:
            speed_tmp |= pin;  //set
            break;
        case GPIO_NORMAL_SPEED:
            speed_tmp &= ~pin; //reset
           break;
        default:
            break;
    }
    gpio_hp_setf(gpio_base, speed_tmp);
}

void hal_gpio_pin_mode_set(uint32_t gpio_base, gpio_pin_t pin, gpio_mode_t mode)
{
    uint16_t mode_tmp = gpio_dsel_getf(gpio_base);
    
    switch(mode)
    {
        case GPIO_MODE_DIGITAL:
            mode_tmp |= pin;  //set
            break;
        case GPIO_MODE_ANALOG:
            mode_tmp &= ~pin; //reset
           break;
        
        case GPIO_MODE_AFIO:
            //TODO: 
            break;

        default:
            break;
    }
    gpio_dsel_setf(gpio_base, mode_tmp);
}

void hal_gpio_pin_direction_set(uint32_t gpio_base, gpio_pin_t pin, gpio_direction_t dir)
{
    uint16_t dir_tmp = gpio_ddr_getf(gpio_base);
    
    switch(dir)
    {
        case GPIO_OUTPUT:
            dir_tmp |= pin;  //set
            break;
        case GPIO_INPUT:
            dir_tmp &= ~pin; //reset
           break;
        default:
            break;
    }
    gpio_ddr_setf(gpio_base, dir_tmp);
}

uint16_t hal_gpio_port_input_read(uint32_t gpio_base)
{
    return gpio_idr_getf(gpio_base);
}

uint16_t hal_gpio_port_output_read(uint32_t gpio_base)
{
    return gpio_odr_getf(gpio_base);
}

uint8_t hal_gpio_pin_input_read(uint32_t gpio_base, gpio_pin_t pin)
{
    uint16_t port_input = gpio_idr_getf(gpio_base);
    return (port_input & pin) ? HAL_SET : HAL_RESET;
}

uint8_t hal_gpio_pin_output_read(uint32_t gpio_base, gpio_pin_t pin)
{
    uint16_t port_output = gpio_odr_getf(gpio_base);
    return (port_output & pin) ? HAL_SET : HAL_RESET;
}

uint8_t hal_gpio_pin_read(uint32_t gpio_base, gpio_pin_t pin)
{
    uint16_t port_dir = gpio_ddr_getf(gpio_base);
    uint16_t port_value = 0;
    if((port_dir & pin) == pin)
    {
        port_value = gpio_odr_getf(gpio_base);
    }
    else
    {
        port_value = gpio_idr_getf(gpio_base);
    }
    return (port_value & pin) ? HAL_SET : HAL_RESET;
}

void hal_gpio_port_output_write(uint32_t gpio_base, uint16_t port_val)
{
    gpio_odr_setf(gpio_base, port_val);
}

void hal_gpio_pin_set(uint32_t gpio_base, gpio_pin_t pin)
{
    gpio_bsrr_set(gpio_base, pin);
}

void hal_gpio_pin_reset(uint32_t gpio_base, gpio_pin_t pin)
{
    uint32_t pin_reset = ((uint32_t)pin) << 16;
    gpio_bsrr_set(gpio_base, pin_reset);
}
void hal_gpio_pin_toggle(uint32_t gpio_base, gpio_pin_t pin)
{
    if(hal_gpio_pin_output_read(gpio_base,pin) == 1)
    {
        hal_gpio_pin_reset(gpio_base,pin);                
    }
    else
    {
        hal_gpio_pin_set(gpio_base,pin);
    }
}

void hal_gpio_pin_it_en(uint32_t gpio_base, gpio_pin_t pin, hal_en_t en)
{
    uint16_t en_tmp = gpio_inten_getf(gpio_base);
    
    switch(en)
    {
        case HAL_ENABLE:
            en_tmp |= pin;  //set
            break;
        case HAL_DISABLE:
            en_tmp &= ~pin; //reset
           break;
        default:
            break;
    }
    gpio_inten_setf(gpio_base, en_tmp);
}

void hal_gpio_pin_it_cfg(uint32_t gpio_base, gpio_pin_t pin, gpio_int_type_t type)
{
    uint16_t rise_tmp = gpio_rise_getf(gpio_base);
    uint16_t fall_tmp = gpio_fall_getf(gpio_base);
    
    switch(type)
    {
        case GPIO_INT_RISING:
            rise_tmp |= pin;    //set
            fall_tmp &= ~pin;   //reset
            break;
        case GPIO_INT_FALLING:
            rise_tmp &= ~pin;   //reset
            fall_tmp |= pin;    //set
           break;
        case GPIO_INT_RISING_FALLING:
            rise_tmp |= pin;    //set
            fall_tmp |= pin;    //set
           break;
        default:
            break;
    }
    gpio_rise_setf(gpio_base, rise_tmp);
    gpio_fall_setf(gpio_base, fall_tmp);
}

uint8_t hal_gpio_pin_get_it_flag(uint32_t gpio_base, gpio_pin_t pin)
{
    uint16_t st_tmp = gpio_int_st_getf(gpio_base);
    
    return ((st_tmp & pin) ? HAL_SET : HAL_RESET);
}

void hal_gpio_pin_clr_it_flag(uint32_t gpio_base, gpio_pin_t pin)
{    
    gpio_int_st_setf(gpio_base, pin);
}


void hal_gpio_pin_afio_select(uint32_t gpio_base, gpio_pin_t pin, afio_function_t fun)
{
    if(gpio_base == GPIOA_BASE)
    {
        hal_assert(IS_GPIOA_AFIO(pin));
        switch(pin)
        {
            case GPIO_PIN_0 :
                sysc_cmp_func_io00_sel_setf(fun); 
                break;
            case GPIO_PIN_1 :    
                sysc_cmp_func_io01_sel_setf(fun); 
                break;
            case GPIO_PIN_2 :    
                sysc_cmp_func_io02_sel_setf(fun); 
                break;
            case GPIO_PIN_3 :
                sysc_cmp_func_io03_sel_setf(fun); 
                break;
            case GPIO_PIN_4 :    
                sysc_cmp_func_io04_sel_setf(fun); 
                break;
            case GPIO_PIN_5 :    
                sysc_cmp_func_io05_sel_setf(fun); 
                break;
            case GPIO_PIN_6 :    
                sysc_cmp_func_io06_sel_setf(fun); 
                break;
            case GPIO_PIN_7 :    
                sysc_cmp_func_io07_sel_setf(fun); 
                break;
            case GPIO_PIN_8 :    
                sysc_cmp_func_io08_sel_setf(fun); 
                break;
            case GPIO_PIN_9 :    
                sysc_cmp_func_io09_sel_setf(fun); 
                break;
            case GPIO_PIN_10:    
                sysc_cmp_func_io10_sel_setf(fun); 
                break;
            case GPIO_PIN_11:    
                sysc_cmp_func_io11_sel_setf(fun); 
                break;
            case GPIO_PIN_12:    
                sysc_cmp_func_io12_sel_setf(fun); 
                break;
            default:
                break;
        }
    }
    else if(gpio_base == GPIOB_BASE)
    {
        hal_assert(IS_GPIOB_AFIO(pin));
        
        switch(pin)
        {
            case GPIO_PIN_3 :
                sysc_cmp_func_io13_sel_setf(fun); 
                break;
            case GPIO_PIN_4 :    
                sysc_cmp_func_io14_sel_setf(fun); 
                break;
            case GPIO_PIN_5 :    
                sysc_cmp_func_io15_sel_setf(fun); 
                break;
            case GPIO_PIN_6 :    
                sysc_cmp_func_io16_sel_setf(fun); 
                break;
            case GPIO_PIN_7 :    
                sysc_cmp_func_io17_sel_setf(fun); 
                break;
            case GPIO_PIN_8 :    
                sysc_cmp_func_io18_sel_setf(fun); 
                break;
            case GPIO_PIN_9 :    
                sysc_cmp_func_io19_sel_setf(fun); 
                break;
            default:
                break;
        }
    }
}

void hal_gpio_pin_afio_en(uint32_t gpio_base, gpio_pin_t pin, hal_en_t en)
{
    uint32_t en_tmp = sysc_cmp_func_ien_getf();

    if(en == HAL_ENABLE)
    {
        //if you want to enable the fullmux,you must set the gpio to input.
        hal_gpio_pin_direction_set(gpio_base,pin,GPIO_INPUT);
    }

    if(gpio_base == GPIOA_BASE)
    {
        hal_assert(IS_GPIOA_AFIO(pin));
        
        if (HAL_ENABLE == en) {
            en_tmp |= pin;
        } else {
            en_tmp &= ~pin;
        }
    }
    else if(gpio_base == GPIOB_BASE)
    {
        hal_assert(IS_GPIOB_AFIO(pin));
        
        if (HAL_ENABLE == en) {
            en_tmp |= (pin << 10);
        } else {
            en_tmp &= ~(pin << 10);
        }
    }
    sysc_cmp_func_ien_setf(en_tmp);
}



