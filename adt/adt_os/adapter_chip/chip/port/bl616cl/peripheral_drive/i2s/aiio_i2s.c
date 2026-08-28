/** @brief I2S intermediate device port adaptation
 *
 *  @file        aiio_i2s.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/08/01          <td>V1.0.0          <td>johhn           <td>I2S device port adaptation
 *  <table>
 *
 */
#include "bflb_clock.h"
#include "bflb_mtimer.h"
#include "bflb_i2s.h"
#include "bflb_i2c.h"
#include "bflb_dma.h"
#include "bflb_gpio.h"


#include "aiio_i2s.h"

#if (CONFIG_AIIO_I2S)
#define AIIO_FUNC_FAIL -3
struct aiio_i2s_struct
{
    aiio_i2s_enum_t aiio_i2s_port;
    struct bflb_device_s *aiio_i2s_dev;
};

struct aiio_i2s_struct aiio_i2s_list[4];
static uint8_t aiio_i2s_num = 0;

static int32_t aiio_i2s_port_cal(aiio_i2s_enum_t aiio_i2s_port)
{
    uint8_t i = 0;

    for (i = 0; i < aiio_i2s_num; i++)
    {
        if (aiio_i2s_list[i].aiio_i2s_port == aiio_i2s_port)
        {
            return i;
        }
    }

    return AIIO_FUNC_FAIL;
}

struct bflb_device_s *dma0_ch0;
struct bflb_device_s *dma0_ch1;
ATTR_PSRAM_SECTION uint8_t rx_buffer[64000];
//ATTR_NOCACHE_RAM_SECTION uint8_t rx_buffer[32000/2];
static void dma0_ch0_isr(void *arg)
{
    aiio_log_i("tc done");
}

static void dma0_ch1_isr(void *arg)
{
    aiio_log_i("rx done");
}

void i2s_dma_init(struct bflb_device_s *i2s0)
{
    static struct bflb_dma_channel_lli_pool_s tx_llipool[100];
    static struct bflb_dma_channel_lli_transfer_s tx_transfers[1];
    static struct bflb_dma_channel_lli_pool_s rx_llipool[100];
    static struct bflb_dma_channel_lli_transfer_s rx_transfers[1];

    struct bflb_i2s_config_s i2s_cfg = {
        .bclk_freq_hz = 16000 * 16 * 2, /* bclk = Sampling_rate * frame_width * channel_num */
        .role = I2S_ROLE_MASTER,
        .format_mode = I2S_MODE_LEFT_JUSTIFIED,
        .channel_mode = I2S_CHANNEL_MODE_NUM_2,
        .frame_width = I2S_SLOT_WIDTH_16,
        .data_width = I2S_SLOT_WIDTH_16,
        .fs_offset_cycle = 0,

        .tx_fifo_threshold = 0,
        .rx_fifo_threshold = 0,
    };

    struct bflb_dma_channel_config_s tx_config = {
        .direction = DMA_MEMORY_TO_PERIPH,
        .src_req = DMA_REQUEST_NONE,
        .dst_req = DMA_REQUEST_I2S_TX,
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT,
    };

    struct bflb_dma_channel_config_s rx_config = {
        .direction = DMA_PERIPH_TO_MEMORY,
        .src_req = DMA_REQUEST_I2S_RX,
        .dst_req = DMA_REQUEST_NONE,
        .src_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT
    };
    aiio_log_i("i2s init");
    /* i2s init */
    bflb_i2s_init(i2s0, &i2s_cfg);
    /* enable dma */
    bflb_i2s_link_txdma(i2s0, true);
    bflb_i2s_link_rxdma(i2s0, true);

    aiio_log_i("dma init");
    dma0_ch0 = bflb_device_get_by_name("dma0_ch0");
    dma0_ch1 = bflb_device_get_by_name("dma0_ch1");

    bflb_dma_channel_init(dma0_ch0, &tx_config);
    bflb_dma_channel_init(dma0_ch1, &rx_config);

    bflb_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);
    bflb_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL);

    tx_transfers[0].src_addr = (uint32_t)rx_buffer;
    tx_transfers[0].dst_addr = (uint32_t)DMA_ADDR_I2S_TDR;
    tx_transfers[0].nbytes = sizeof(rx_buffer);

    rx_transfers[0].src_addr = (uint32_t)DMA_ADDR_I2S_RDR;
    rx_transfers[0].dst_addr = (uint32_t)rx_buffer;
    rx_transfers[0].nbytes = sizeof(rx_buffer);

    uint32_t num = bflb_dma_channel_lli_reload(dma0_ch0, tx_llipool, 100, tx_transfers, 1);
    bflb_dma_channel_lli_link_head(dma0_ch0, tx_llipool, num);
    aiio_log_i("tx dma lli num: %d \r\n", num);
    num = bflb_dma_channel_lli_reload(dma0_ch1, rx_llipool, 100, rx_transfers, 1);
    aiio_log_i("rx dma lli num: %d \r\n", num);
    bflb_dma_channel_lli_link_head(dma0_ch1, rx_llipool, num);
    bflb_dma_channel_start(dma0_ch0);
    bflb_dma_channel_start(dma0_ch1);

    /* enable i2s tx and rx */
    bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_TX | I2S_CMD_DATA_ENABLE_RX);
}


int aiio_hal_i2s_init(aiio_i2s_enum_t aiio_i2s_port, aiio_i2s_pin_cfg_t *aiio_i2s_cfg)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) != AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_init function aiio_i2s_port is repetition!");
        return AIIO_ERROR;
    }
    switch (aiio_i2s_port)
    {
    case AIIO_I2S_0:
        aiio_log_e("aiio_i2s_cfg->aiio_gpio_fs_pin:%d",aiio_i2s_cfg->aiio_gpio_fs_pin);
        aiio_log_e("aiio_i2s_cfg->aiio_gpio_di_pin:%d",aiio_i2s_cfg->aiio_gpio_di_pin);
        aiio_log_e("aiio_i2s_cfg->aiio_gpio_do_pin:%d",aiio_i2s_cfg->aiio_gpio_do_pin);
        aiio_log_e("aiio_i2s_cfg->aiio_gpio_bclk_pin:%d",aiio_i2s_cfg->aiio_gpio_bclk_pin);
        aiio_log_e("aiio_i2s_cfg->aiio_gpio_mclk_pin:%d",aiio_i2s_cfg->aiio_gpio_mclk_pin);
        aiio_i2s_list[aiio_i2s_num].aiio_i2s_port = aiio_i2s_port;
        aiio_i2s_list[aiio_i2s_num].aiio_i2s_dev=bflb_device_get_by_name("i2s0");
        if (aiio_i2s_list[aiio_i2s_num].aiio_i2s_dev)
        {
            struct bflb_device_s *gpio = bflb_device_get_by_name("gpio");
            /* I2S_FS */
            bflb_gpio_init(gpio, aiio_i2s_cfg->aiio_gpio_fs_pin, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
            /* I2S_DI */
            bflb_gpio_init(gpio, aiio_i2s_cfg->aiio_gpio_di_pin, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
            /* I2S_DO */
            bflb_gpio_init(gpio, aiio_i2s_cfg->aiio_gpio_do_pin, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
            /* I2S_BCLK */
            bflb_gpio_init(gpio, aiio_i2s_cfg->aiio_gpio_bclk_pin, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
            /* MCLK CLKOUT */
            bflb_gpio_init(gpio, aiio_i2s_cfg->aiio_gpio_mclk_pin, GPIO_FUNC_CLKOUT | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
            /* output MCLK,Will change the clock source of i2s,
            It needs to be called before i2s is initialized
            clock source 25M
            */
            GLB_Set_I2S_CLK(1, 2, 0, 0);
            GLB_Set_Chip_Clock_Out2_Sel(1);
            i2s_dma_init(aiio_i2s_list[aiio_i2s_num].aiio_i2s_dev);
        }
        else
        {
            aiio_log_e("i2s device get fail");
            return AIIO_ERROR;
        }
        aiio_i2s_num++;
        break;

    case AIIO_I2S_1:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;

    case AIIO_I2S_2:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;

    case AIIO_I2S_SOFT:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;

    default:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;


    }

    return AIIO_OK;
}

int aiio_hal_i2s_deinit(aiio_i2s_enum_t aiio_i2s_port)
{

    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_i2s_port)
    {
    case AIIO_I2S_0:
        bflb_i2s_deinit(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev);
        break;

    case AIIO_I2S_1:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;

    case AIIO_I2S_2:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;

    case AIIO_I2S_SOFT:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;

    default:
        aiio_log_w("aiio_i2s_port is nonsupport!");
        break;
    }

    return AIIO_OK;

}

int aiio_hal_i2s_link_tx_dma(aiio_i2s_enum_t aiio_i2s_port,bool en)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }
    bflb_i2s_link_txdma(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev, en);
    return AIIO_OK;
     
}
int aiio_hal_i2s_link_rx_dma(aiio_i2s_enum_t aiio_i2s_port,bool en)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }
    bflb_i2s_link_rxdma(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev, en);
    return AIIO_OK;
}
int aiio_hal_i2s_txint_mask(aiio_i2s_enum_t aiio_i2s_port,bool mask)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }
    bflb_i2s_txint_mask(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev, mask);
    return AIIO_OK;
}

int aiio_hal_i2s_rxint_mask(aiio_i2s_enum_t aiio_i2s_port,bool mask)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }
    bflb_i2s_rxint_mask(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev, mask);
    return AIIO_OK;
}
int aiio_hal_i2s_errint_mask(aiio_i2s_enum_t aiio_i2s_port,bool mask)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }
    bflb_i2s_errint_mask(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev, mask);
    return AIIO_OK;
}

uint32_t aiio_hal_i2s_get_intstatus(aiio_i2s_enum_t aiio_i2s_port)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }

    return bflb_i2s_get_intstatus(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev);
}

int aiio_hal_i2s_feature_control(aiio_i2s_enum_t aiio_i2s_port,int cmd, size_t arg)
{
    if (aiio_i2s_port_cal(aiio_i2s_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2s_port is nonsupport!");
        return AIIO_ERROR;
    }
    if(bflb_i2s_feature_control(aiio_i2s_list[aiio_i2s_port_cal(aiio_i2s_port)].aiio_i2s_dev,cmd,arg)!=0)
    {
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

#endif




