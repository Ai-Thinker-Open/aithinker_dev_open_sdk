/** @brief      SPI application interface.
 *
 *  @file       aiio_spi.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       SPI application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2025/08/21      <td>1.0.0       <td>hly         <td>Adapts to SPI interface
 *  </table>
 *
 */
#include <stdio.h>
#include <stdint.h>
#include "spi_api.h"
#include "aiio_spi.h"
#include "ameba_soc.h"
#include "device.h"
#include "diag.h"
#include "os_wrapper.h"
#include "PinNames.h"

#define SPI1_CS _PB_21

#define AIIO_FUNC_FAIL   -3
#define SPI_MODE0         0

#define PIN_CALCULATE(a, b) ((a)<<5|(b))

GPIO_InitTypeDef GPIO_InitStruct_Temp;

typedef struct
{
    aiio_spi_enum_t aiio_spi_port;
    aiio_spi_pin_cfg_t aiio_spi_cfg;
} aiio_spi_struct;

static aiio_spi_struct aiio_spi_list[4];
static uint8_t aiio_spi_num = 0;

spi_t spi_master;
PinName cs, clk, mosi, miso;

static int32_t aiio_spi_port_cal(aiio_spi_enum_t aiio_spi_port)
{
    uint8_t i = 0;

    for (i = 0; i < aiio_spi_num; i++)
    {
        if (aiio_spi_list[i].aiio_spi_port == aiio_spi_port)
        {
            return i;
        }
    }

    return AIIO_FUNC_FAIL;
}


static void aiio_spi_master_css(uint8_t status) ///< SPI主机CS控制函数指针
{
    if(status)
    {
        GPIO_WriteBit(cs, 1);

    }
    else
    {
        GPIO_WriteBit(cs, 0);
    }
}

int32_t aiio_spi_master_init(aiio_spi_enum_t aiio_spi_port, aiio_spi_pin_cfg_t *aiio_spi_cfg) {
    if (aiio_spi_port_cal(aiio_spi_port) != AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_init function aiio_spi_port is repetition!");
        return AIIO_ERROR;
    }

    switch (aiio_spi_port)
    {
        case AIIO_SPI_0:
            aiio_log_w("aiio_spi_master_init function AIIO_SPI_0 is nonsupport!");
            break;

        case AIIO_SPI_1:
            {
                cs = PIN_CALCULATE(aiio_spi_cfg->aiio_gpio_cs_port, aiio_spi_cfg->aiio_gpio_cs_pin);
                clk = PIN_CALCULATE(aiio_spi_cfg->aiio_gpio_clk_port, aiio_spi_cfg->aiio_gpio_clk_pin);
                mosi = PIN_CALCULATE(aiio_spi_cfg->aiio_gpio_mosi_port, aiio_spi_cfg->aiio_gpio_mosi_pin);
                miso = PIN_CALCULATE(aiio_spi_cfg->aiio_gpio_miso_port, aiio_spi_cfg->aiio_gpio_miso_pin);

                GPIO_InitStruct_Temp.GPIO_Pin = cs;
                GPIO_InitStruct_Temp.GPIO_Mode = GPIO_Mode_OUT;
                GPIO_Init(&GPIO_InitStruct_Temp);
                GPIO_WriteBit(cs, 1);
                /* SPI1 is as Master */
                spi_master.spi_idx = MBED_SPI1;
                spi_init(&spi_master, mosi, miso, clk, SPI1_CS);
                // 配置格式：8位数据，模式0，主机模式
                spi_format(&spi_master, 8, SPI_MODE0, 0);
                
                // 设置频率为1MHz
                spi_frequency(&spi_master, 1000000);   
                
                break;
            }
        case AIIO_SPI_2:
            aiio_log_w("aiio_spi_master_init function AIIO_SPI_2 is nonsupport!");
            break;

        case AIIO_SPI_SOFT:
            aiio_log_w("aiio_spi_master_init function AIIO_SPI_SOFT is nonsupport!");
            break;

        default:
            break;
    }
    aiio_spi_list[aiio_spi_num].aiio_spi_port = aiio_spi_port;
    aiio_spi_num++;

    return AIIO_OK;
}

int32_t aiio_spi_master_deinit(aiio_spi_enum_t aiio_spi_port) {
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_deinit function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        aiio_log_w("aiio_spi_master_deinit function AIIO_SPI_0 is nonsupport!\r\n");
        break;

    case AIIO_SPI_1:
        {
            spi_free(&spi_master);
            aiio_spi_num --;
            break;
        }
    case AIIO_SPI_2:
        aiio_log_w("aiio_spi_master_deinit function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_w("aiio_spi_master_deinit function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *send_data, uint32_t data_len, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    uint8_t *s_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    // uint8_t *r_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    for (int i = 0; i < addr_len; i++)
    {
        s_buf[i] = addr[i];
    }
    for (int i = 0; i < data_len; i++)
    {
        s_buf[addr_len + i] = send_data[i];
    }

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        aiio_log_e("aiio_spi_master_write function AIIO_SPI_0 is nonsupport!\r\n");
        break;

    case AIIO_SPI_1:
        {
            aiio_spi_master_css(0);
            for(uint32_t i = 0;i < (data_len + addr_len);i ++)
            {
                spi_master_write(&spi_master, s_buf[i]);
            }
            aiio_spi_master_css(1);
            break;
        }
       
    case AIIO_SPI_2:
        aiio_log_e("aiio_spi_master_write function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_e("aiio_spi_master_write function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_SOFT is nonsupport!");
        break;
    }

    free(s_buf);
    // free(r_buf);
    s_buf = NULL;
    // r_buf = NULL;

    return AIIO_OK;
}

int32_t aiio_spi_master_read(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t data_len, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_read function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    uint8_t *s_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    uint8_t *r_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    for (int i = 0; i < addr_len; i++)
    {
        s_buf[i] = addr[i];
    }

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_0 is nonsupport!");
        break;

    case AIIO_SPI_1:
        {
            aiio_spi_master_css(0);
            for(uint32_t i = 0;i < (data_len + addr_len);i ++)
            {
                r_buf[i] = spi_master_write(&spi_master, s_buf[i]);
            }
            for (int i = 0; i < data_len; i++)
            {
                rec_data[i] = r_buf[i + addr_len];
            }
            aiio_spi_master_css(1);
            break;
        }

    case AIIO_SPI_2:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_2 is nonsupport!");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_SOFT is nonsupport!");
        break;

    default:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_SOFT is nonsupport!");
        break;
    }

    free(s_buf);
    free(r_buf);
    s_buf = NULL;
    r_buf = NULL;

    return AIIO_OK;
}

void qyq_spi_master_write(uint8_t *buf, uint32_t length) ///< SPI主机写入函数指针
{
    for(uint32_t i = 0;i < length;i ++)
    {
        spi_master_write(&spi_master,buf[i]);
    }
}

void qyq_spi_master_read(uint8_t *buf, uint32_t length) ///< SPI主机读取函数指针
{
    for(uint32_t i = 0;i < length;i ++)
    {
        buf[i] = spi_master_write(&spi_master,buf[i]);
    }
}

int32_t aiio_spi_master_write_and_read(aiio_spi_enum_t aiio_spi_port, uint8_t *send_data, uint8_t *rec_data, uint32_t data_len, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write_and_read function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }
    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        aiio_log_w("aiio_spi_master_write_and_read function AIIO_SPI_0 is nonsupport!\r\n");
        break;

    case AIIO_SPI_1:
        {
            aiio_spi_master_css(0);
            qyq_spi_master_write(send_data, data_len);
	        qyq_spi_master_read(rec_data, data_len);

            // for(int i=0; i<data_len; i++){
            //     rec_data[i] = spi_master_write(&spi_master, send_data[i]);  // 同步发送接收

            // }
            aiio_spi_master_css(1);
            break;
        }

    case AIIO_SPI_2:
        aiio_log_w("aiio_spi_master_write_and_read function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_w("aiio_spi_master_write_and_read function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t send_data, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write_byte function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_spi_master_write(aiio_spi_port, addr, addr_len, &send_data, 1, timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_read_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_read_byte function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_spi_master_read(aiio_spi_port, addr, addr_len, rec_data, 1, timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

