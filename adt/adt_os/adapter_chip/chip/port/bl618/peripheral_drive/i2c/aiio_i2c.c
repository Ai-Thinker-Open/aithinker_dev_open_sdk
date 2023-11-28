/** @brief      I2C application interface.
 *
 *  @file       aiio_i2c.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>I2C Init
 *  </table>
 *
 */
#include "aiio_i2c.h"

#include "bflb_gpio.h"
#include "bflb_i2c.h"



#define AIIO_FUNC_FAIL -3

struct aiio_i2c_struct
{
    aiio_i2c_enum_t aiio_i2c_port;
    struct bflb_device_s *aiio_i2c_dev;
};

struct aiio_i2c_struct aiio_i2c_list[4];
static uint8_t aiio_i2c_num = 0;

static int32_t aiio_i2c_port_cal(aiio_i2c_enum_t aiio_i2c_port)
{
    uint8_t i = 0;

    for (i = 0; i < aiio_i2c_num; i++)
    {
        if (aiio_i2c_list[i].aiio_i2c_port == aiio_i2c_port)
        {
            return i;
        }
    }

    return AIIO_FUNC_FAIL;
}

int32_t aiio_i2c_master_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) != AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_init function aiio_i2c_port is repetition!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        aiio_log_w("aiio_gpio_scl_pin:%d",aiio_i2c_cfg->aiio_gpio_scl_pin);
        aiio_log_w("aiio_gpio_sda_pin:%d",aiio_i2c_cfg->aiio_gpio_sda_pin);
        aiio_i2c_list[aiio_i2c_num].aiio_i2c_port = aiio_i2c_port;
        aiio_i2c_list[aiio_i2c_num].aiio_i2c_dev=bflb_device_get_by_name("i2c0");
        if (aiio_i2c_list[aiio_i2c_num].aiio_i2c_dev) 
        {
             /* i2c_gpio_init */
            struct bflb_device_s *i2c_gpio = NULL;
            i2c_gpio = bflb_device_get_by_name("gpio");
            /* I2C0_SCL */
            bflb_gpio_init(i2c_gpio, aiio_i2c_cfg->aiio_gpio_scl_pin, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
            /* I2C0_SDA */
            bflb_gpio_init(i2c_gpio, aiio_i2c_cfg->aiio_gpio_sda_pin, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);

            /* init i2c*/
            bflb_i2c_init(aiio_i2c_list[aiio_i2c_num].aiio_i2c_dev, aiio_i2c_cfg->aiio_i2c_period);
            aiio_log_w("bflb_i2c_init finish ");
        } 
        else 
        {
            aiio_log_e("i2c device get fail");
            return AIIO_ERROR;
        }

        aiio_i2c_num++;
        break;

    case AIIO_I2C_1:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_1 is nonsupport!");
        break;

    case AIIO_I2C_2:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_2 is nonsupport!");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_SOFT is nonsupport!");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_deinit(aiio_i2c_enum_t aiio_i2c_port)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        bflb_i2c_deinit(aiio_i2c_list[aiio_i2c_port_cal(aiio_i2c_port)].aiio_i2c_dev);
        //aiio_log_w("aiio_i2c_master_write function AIIO_I2C_0 is nonsupport!\r\n");
        break;

    case AIIO_I2C_1:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_1 is nonsupport!\r\n");
        break;

    case AIIO_I2C_2:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_2 is nonsupport!\r\n");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_write(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    int ret = 0;
    struct bflb_i2c_msg_s msg[2];
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
       
        msg[0].addr = dev_addr>>1;
        msg[0].flags = I2C_M_NOSTOP;//7BIT
        msg[0].buffer = target_addr;
        msg[0].length = target_addr_len;

        msg[1].addr = dev_addr>>1;
        msg[1].flags = 0;//write
        msg[1].buffer = buf;
        msg[1].length = buf_len;
        ret=bflb_i2c_transfer(aiio_i2c_list[aiio_i2c_port_cal(aiio_i2c_port)].aiio_i2c_dev,msg,2);
        bflb_mtimer_delay_ms(100);
        if (ret != 0)
        {
            aiio_log_e("hosal i2c write failed! ret:%d",ret);
            return AIIO_ERROR;
        }
        

        break;

    case AIIO_I2C_1:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_1 is nonsupport!\r\n");
        break;

    case AIIO_I2C_2:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_2 is nonsupport!\r\n");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_read(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    int ret = 0;
    struct bflb_i2c_msg_s msg[2];
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:        
        msg[0].addr = dev_addr>>1;
        msg[0].flags = I2C_M_NOSTOP;//7BIT
        msg[0].buffer = target_addr;
        msg[0].length = target_addr_len;

        msg[1].addr = dev_addr>>1;
        msg[1].flags = I2C_M_READ;     
        msg[1].buffer = buf;
        msg[1].length = buf_len;
        ret = bflb_i2c_transfer(aiio_i2c_list[aiio_i2c_port_cal(aiio_i2c_port)].aiio_i2c_dev,msg,2);
        if (ret != 0)
        {
            aiio_log_e("hosal i2c read failed! ret:%d",ret);
            return AIIO_ERROR;
        }

        break;

    case AIIO_I2C_1:
        aiio_log_w("aiio_i2c_master_read function AIIO_I2C_1 is nonsupport!\r\n");
        break;

    case AIIO_I2C_2:
        aiio_log_w("aiio_i2c_master_read function AIIO_I2C_2 is nonsupport!\r\n");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_w("aiio_i2c_master_read function AIIO_I2C_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_write_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t data, uint32_t timeout)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_i2c_master_write(aiio_i2c_port, dev_addr, target_addr, target_addr_len, data,1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_read_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len,uint8_t *data, uint32_t timeout)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_i2c_master_read(aiio_i2c_port, dev_addr, target_addr, target_addr_len, data,1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
