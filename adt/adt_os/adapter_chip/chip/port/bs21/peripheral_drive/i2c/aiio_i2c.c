#include "aiio_i2c.h"
#include "pinctrl.h"
#include "i2c.h"
#include "soc_osal.h"
#include "app_init.h"

#define AIIO_FUNC_FAIL -3

static errcode_t bs21_master_i2c_write(i2c_bus_t bus, uint8_t *data, uint8_t len, uint16_t addr)
{
    i2c_data_t i2c_send_data = {0};
    i2c_send_data.send_buf = data;                           /* 设置 tx buff */
    i2c_send_data.send_len = len;                            /* 设置 tx buff 长度 */
    return uapi_i2c_master_write(bus, addr >> 1, &i2c_send_data); /* 发送数据 */
}

static errcode_t bs21_master_i2c_writeread(i2c_bus_t bus, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *data, uint8_t len, uint16_t addr)
{
    i2c_data_t i2c_data = {0};
    i2c_data.send_buf = target_addr;                        /* 设置 tx buff */
    i2c_data.send_len = target_addr_len;                    /* 设置 tx buff 长度 */
    i2c_data.receive_buf = data;                            /* 设置 tx buff */
    i2c_data.receive_len = len;                             /* 设置 tx buff 长度 */
    return uapi_i2c_master_writeread(bus, addr >> 1, &i2c_data); /* 发送数据 */
}

int32_t aiio_i2c_master_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg)
{
    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        uapi_pin_set_mode(aiio_i2c_cfg->aiio_gpio_scl_pin, HAL_PIO_I2C0_CLK);
        uapi_pin_set_mode(aiio_i2c_cfg->aiio_gpio_sda_pin, HAL_PIO_I2C0_DATA);
        
        if (uapi_i2c_master_init(I2C_BUS_0, aiio_i2c_cfg->aiio_i2c_period, 0) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }

        break;

    case AIIO_I2C_1:
        uapi_pin_set_mode(aiio_i2c_cfg->aiio_gpio_scl_pin, HAL_PIO_I2C0_CLK);
        uapi_pin_set_mode(aiio_i2c_cfg->aiio_gpio_sda_pin, HAL_PIO_I2C0_DATA);
        if (uapi_i2c_master_init(I2C_BUS_1, aiio_i2c_cfg->aiio_i2c_period, 0) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }
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
    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        if (uapi_i2c_deinit(I2C_BUS_0) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }
        aiio_log_w("aiio_i2c_master_write function AIIO_I2C_0 is nonsupport!\r\n");
        break;

    case AIIO_I2C_1:
        if (uapi_i2c_deinit(I2C_BUS_1) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }
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
    uint16_t total_len = target_addr_len + buf_len;
    uint8_t *send_buf = (uint8_t *)osal_kmalloc(total_len, 0);
    // 拷贝 target_addr 到发送缓冲区
    memcpy_s(send_buf, target_addr_len, target_addr, target_addr_len);
    memcpy_s(send_buf + target_addr_len, buf_len, buf, buf_len);

    timeout = timeout;

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        if (bs21_master_i2c_write(I2C_BUS_0, send_buf, total_len, dev_addr) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }
        break;

    case AIIO_I2C_1:
        if (bs21_master_i2c_write(I2C_BUS_1, send_buf, total_len, dev_addr) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }
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

    osal_kfree(send_buf);

    return AIIO_OK;
}

int32_t aiio_i2c_master_read(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    timeout = timeout;
    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        if(bs21_master_i2c_writeread(I2C_BUS_0, target_addr, target_addr_len, buf, buf_len, dev_addr) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }
        break;

    case AIIO_I2C_1:
        if(bs21_master_i2c_writeread(I2C_BUS_1, target_addr, target_addr_len, buf, buf_len, dev_addr) != OSAL_SUCCESS)
        {
            return AIIO_ERROR;
        }
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
    if (aiio_i2c_master_write(aiio_i2c_port, dev_addr, target_addr, target_addr_len, (uint8_t *)&data,1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_read_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len,uint8_t *data, uint32_t timeout)
{
    if (aiio_i2c_master_read(aiio_i2c_port, dev_addr, target_addr, target_addr_len, data,1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
