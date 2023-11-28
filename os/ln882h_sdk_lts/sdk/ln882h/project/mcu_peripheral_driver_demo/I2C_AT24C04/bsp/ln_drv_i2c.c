/**
 * @file     ln_drv_i2c.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-03
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#include "ln_drv_i2c.h"

#define TIMEOUT_CYCLE 1000

/**
 * @brief 初始化I2C，配置I2C的引脚和时钟
 * 
 * @param gpio_scl_port 设置I2C SCL端口
 * @param gpio_scl_pin  设置I2C SCL引脚
 * @param gpio_sda_port 设置I2C SDA端口
 * @param gpio_sda_pin  设置I2C SDA引脚
 * @param i2c_period    I2C时钟周期，单位hz,范围 1k - 400k
 */
void i2c_init(gpio_port_t gpio_scl_port,gpio_pin_t gpio_scl_pin,
              gpio_port_t gpio_sda_port,gpio_pin_t gpio_sda_pin,
              uint32_t i2c_period)
{
    uint32_t gpio_scl_base = 0;
    uint32_t gpio_sda_base = 0;

    if(gpio_scl_port == GPIO_A)
        gpio_scl_base = GPIOA_BASE;
    else if(gpio_scl_port == GPIO_B)
        gpio_scl_base = GPIOB_BASE;

    if(gpio_sda_port == GPIO_A)
        gpio_sda_base = GPIOA_BASE;
    else if(gpio_sda_port == GPIO_B)
        gpio_sda_base = GPIOB_BASE;

    //初始化I2C引脚
    hal_gpio_pin_afio_select(gpio_scl_base, gpio_scl_pin, I2C0_SCL); //初始化I2C引脚
    hal_gpio_pin_afio_select(gpio_sda_base, gpio_sda_pin, I2C0_SDA);
    hal_gpio_pin_afio_en(gpio_scl_base, gpio_scl_pin, HAL_ENABLE);   //使能功能复用
    hal_gpio_pin_afio_en(gpio_sda_base, gpio_sda_pin, HAL_ENABLE);

    /* disable the i2c */
    hal_i2c_en(I2C_BASE, HAL_DISABLE); //配置参数前要先失能I2C

    /* init the i2c */
    i2c_init_t_def i2c_init;
    memset(&i2c_init, 0, sizeof(i2c_init));

    i2c_init.i2c_peripheral_clock_freq = 4;                     //时钟设置，默认4就可以（滤波使用）
    i2c_init.i2c_master_mode_sel = I2C_FM_MODE;                 //fs   = 1
    i2c_init.i2c_fm_mode_duty_cycle = I2C_FM_MODE_DUTY_CYCLE_2; //duty = 1
    /*  
        配置时钟举例：
        快速模式下：400k SCL周期为2.5us
        外设工作时钟为80MHZ 所以Tpckl = 1/80M = 0.0125us
        
        duty = 1;fs = 1
        Thigh = 9  x CCR x TPCLK1 
        Tlow  = 16 x CCR x TPCLK1
        Thigh + Tlow = 2.5us
        ccr =  8
    */
    i2c_init.i2c_ccr = (1.0f / i2c_period) / 25 * hal_clock_get_apb0_clk();
    i2c_init.i2c_trise = 0xF; //推荐默认值为0xF;

    hal_i2c_init(I2C_BASE, &i2c_init);
    hal_i2c_en(I2C_BASE, HAL_ENABLE);
}



/**
 * @brief I2C 写数据到目标设备的寄存器（设备地址7bit）
 * 
 * @param dev_addr        设备地址
 * @param target_addr     目标设备寄存器地址
 * @param target_addr_len 寄存器地址长度
 * @param buf             要写入的数据
 * @param buf_len         要写入数据的长度
 * @return uint8_t        是否写入成功，0：不成功，1：成功
 */
uint8_t i2c_master_7bit_write_target_address(uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, 
                                            uint8_t *buf, uint16_t buf_len)
{
    //1. check busy
    if (hal_i2c_wait_bus_idle(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
    {
        hal_i2c_master_reset(I2C_BASE);
        return HAL_RESET;
    }

    //2. send start
    if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //3. send addr
    hal_i2c_master_send_data(I2C_BASE, dev_addr);

    //4. wait send complete
    if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //5. clear addr flag
    hal_i2c_clear_sr(I2C_BASE);

    //6. send target address
    for (uint16_t i = 0; i < target_addr_len; i++)
    {
        //wait tx empty flag
        if (hal_i2c_wait_txe(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(I2C_BASE, target_addr[i]);
        }
    }

    //7. wait send complete.
    if (hal_i2c_wait_btf(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //8. send data
    for (uint32_t i = 0; i < buf_len; i++)
    {
        //wait tx empty flag
        if (hal_i2c_wait_txe(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(I2C_BASE, buf[i]);
        }
    }

    //9. wait send complete.
    if (hal_i2c_wait_btf(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //10. stop the i2c.
    hal_i2c_master_stop(I2C_BASE);

    return HAL_SET;
}

/**
 * @brief I2C 读目标设备的寄存器（设备地址7bit）     
 * 
 * @param dev_addr          设备地址
 * @param target_addr       目标设备寄存器地址
 * @param target_addr_len   目标设备寄存器地址长度
 * @param buf               读取的数据
 * @param buf_len           读取的数据长度
 * @return uint8_t          是否读取成功，0：不成功，1：成功
 */
uint8_t i2c_master_7bit_read_target_address(uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, 
                                            uint8_t *buf, uint16_t buf_len)
{
    //1. check busy
    if (hal_i2c_wait_bus_idle(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
    {
        hal_i2c_master_reset(I2C_BASE);
        return HAL_RESET;
    }

    //2. send start
    if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //3. send addr
    hal_i2c_master_send_data(I2C_BASE, dev_addr);

    //4. wait send complete
    if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //5. clear addr flag
    hal_i2c_clear_sr(I2C_BASE);

    //6. send target address
    for (uint16_t i = 0; i < target_addr_len; i++)
    {
        //wait tx empty flag
        if (hal_i2c_wait_txe(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(I2C_BASE, target_addr[i]);
        }
    }

    //7. wait send complete.
    if (hal_i2c_wait_btf(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //8. reset the bus,if not do this,the bus can not run start operation
    hal_i2c_master_reset(I2C_BASE);

    //9. send start
    if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //10. send addr (+1 is read operation)
    hal_i2c_master_send_data(I2C_BASE, dev_addr + 1);

    //11. Wait for an ack after sending the address
    if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //12. clear addr flag
    hal_i2c_clear_sr(I2C_BASE);

    //13. clear the DR
    hal_i2c_master_recv_data(I2C_BASE);

    //14. receive data
    for (int i = buf_len; i > 0; i--)
    {
        //when reading the last byte,do not send the ack
        if (buf_len == 1)
        {
            //do not send the ack
            hal_i2c_ack_en(I2C_BASE, HAL_DISABLE);

            ///wait rx not empty
            if (hal_i2c_wait_rxne(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            {
                return HAL_RESET;
            }
            else
            {
                *buf = hal_i2c_master_recv_data(I2C_BASE);
            }
            //read data
        }
        else
        {
            //send ack
            hal_i2c_ack_en(I2C_BASE, HAL_ENABLE);

            //wait rx not empty
            if (hal_i2c_wait_rxne(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            {
                return HAL_RESET;
            }
            else
            {
                *buf = hal_i2c_master_recv_data(I2C_BASE);
            }
        }
        buf_len--;
        buf++;
    }

    //15. stop the i2c.
    hal_i2c_master_stop(I2C_BASE);

    return HAL_SET;
}

/**
 * @brief 扫描接在I2C总线上的设备地址
 * 
 * @param address     地址指针，存放扫描到的地址
 * @param address_num 地址数量
 */
void i2c_address_scan(uint8_t *address,uint16_t *address_num)
{
    hal_i2c_ack_en(I2C_BASE, HAL_DISABLE);

    //scan the iic address
    for (uint32_t dev_addr = 0; dev_addr <= 254; dev_addr = dev_addr + 2)
    {
        //1. check busy
        if (hal_i2c_wait_bus_idle(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            hal_i2c_master_reset(I2C_BASE);
            ln_delay_ms(20);
            continue;
        }

        //2. send start
        if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            continue;

        //3. send addr
        hal_i2c_master_send_data(I2C_BASE, dev_addr);

        //4. wait send complete
        if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            continue;
            
        ln_delay_ms(1);

        //5. check ack
        if (hal_i2c_get_status_flag(I2C_BASE,I2C_STATUS_FLAG_AF) == HAL_SET)
        {
            hal_i2c_master_reset(I2C_BASE);
            hal_i2c_clear_sr(I2C_BASE);
            hal_i2c_master_stop(I2C_BASE);
            ln_delay_ms(20);
            
        }
        else
        {
            hal_i2c_master_reset(I2C_BASE);
            hal_i2c_clear_sr(I2C_BASE);

            address[*address_num] = dev_addr;
            (*address_num)++;

            ln_delay_ms(20);
        }
    }
    // clear addr flag
    hal_i2c_clear_sr(I2C_BASE);

    hal_i2c_master_stop(I2C_BASE);
}
