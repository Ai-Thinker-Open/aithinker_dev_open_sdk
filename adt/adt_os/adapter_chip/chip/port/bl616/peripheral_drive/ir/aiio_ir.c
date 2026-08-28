#include "aiio_ir.h"
#include "bflb_core.h"
#include "bflb_gpio.h"
#include "bl616_glb.h"
#include "bflb_ir.h"

aiio_err_t aiio_ir_rx_gpio_init(uint32_t port, uint16_t pin)
{
    (void)(port);

    struct bflb_device_s *gpio;

    gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, pin, GPIO_INPUT | GPIO_SMT_EN | GPIO_DRV_0);
    GLB_IR_RX_GPIO_Sel((GLB_GPIO_Type)pin);

    return AIIO_OK;
}

aiio_err_t aiio_ir_rx_init(aiio_ir_rx_cfg_t *cfg)
{
    if (cfg == NULL)
    {
        return AIIO_ERROR;
    }

    struct bflb_ir_rx_config_s rx_cfg;
    struct bflb_device_s *irrx;

    irrx = bflb_device_get_by_name("irrx");

    /* RX init */
    rx_cfg.rx_mode = cfg->mode;
    rx_cfg.input_inverse = cfg->input_inverse;
    rx_cfg.deglitch_enable = cfg->deglitch_enable;
    bflb_ir_rx_init(irrx, &rx_cfg);

    return AIIO_OK;
}

aiio_err_t aiio_ir_rx_enable(uint8_t enable)
{
    struct bflb_device_s *irrx = bflb_device_get_by_name("irrx");

    bflb_ir_rx_enable(irrx, enable);

    return AIIO_OK;
}

aiio_err_t aiio_ir_receive(uint64_t *rx_data, uint8_t *bit_cnt)
{
    struct bflb_device_s *irrx = bflb_device_get_by_name("irrx");

    *bit_cnt = bflb_ir_receive(irrx, rx_data);

    return AIIO_OK;
}
