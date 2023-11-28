#include "aiio_ir.h"
#include "bl602_glb.h"
#include "bl602_ir.h"

aiio_err_t aiio_ir_rx_gpio_init(uint32_t port, uint16_t pin)
{
    (void)(port);

    GLB_GPIO_Type gpioPin = pin;
    GLB_GPIO_FUNC_Type gpioFun = GPIO_FUN_SWGPIO;

    GLB_GPIO_Func_Init(gpioFun, &gpioPin, 1);

    GLB_IR_RX_GPIO_Sel(pin);

    return AIIO_OK;
}

aiio_err_t aiio_ir_rx_init(aiio_ir_rx_cfg_t *cfg)
{
    IR_RxCfg_Type rxCfg = {0};

    if (cfg == NULL)
    {
        return AIIO_ERROR;
    }

    IR_Disable(IR_RX);

    rxCfg.rxMode = cfg->mode;
    rxCfg.inputInverse = cfg->input_inverse;
    rxCfg.endThreshold = cfg->end_threshold;
    rxCfg.dataThreshold = cfg->data_threshold;
    rxCfg.DeglitchCnt = cfg->deglitch_cnt;
    rxCfg.rxDeglitch = cfg->deglitch_enable;

    /* IR rx init */
    IR_RxInit(&rxCfg);

    return AIIO_OK;
}

aiio_err_t aiio_ir_rx_enable(uint8_t enable)
{
    if (enable != 0)
    {
        IR_Enable(IR_RX);
    }
    else
    {
        IR_Disable(IR_RX);
    }

    return AIIO_OK;
}

aiio_err_t aiio_ir_receive(uint64_t *rx_data, uint8_t *bit_cnt)
{
    IR_Enable(IR_RX);

    /* Wait for rx interrupt */
    if (SET != IR_GetIntStatus(IR_INT_RX))
    {
        return AIIO_ERROR;
    }

    /* Disable ir rx */
    IR_Disable(IR_RX);
    
    /* Clear rx interrupt */
    IR_ClrIntStatus(IR_INT_RX);

    *bit_cnt = IR_GetRxDataBitCount();
    *rx_data = IR_ReceiveData(IR_WORD_0);

    return AIIO_OK;
}
