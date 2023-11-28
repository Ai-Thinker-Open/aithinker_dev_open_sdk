#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define AIIO_ADC_CHANNEL      CONFIG_ADC_CHANNEL
#define AIIO_ADC_MODE         CONFIG_ADC_MODE
#define AIIO_ADC_RESOLUTION   CONFIG_ADC_RESOLUTION
#define AIIO_ADC_VREF         CONFIG_ADC_VREF

void aiio_main(void *params)
{
    uint32_t value = 0;

    aiio_adc_cfg_t init = {
        .change = AIIO_ADC_CHANNEL,
        .mode = AIIO_ADC_MODE,
        .resolution = AIIO_ADC_RESOLUTION,  
        .vref = AIIO_ADC_VREF,
    };

    aiio_adc_init(&init);

    while (1)
    {
        value = aiio_adc_get_value(AIIO_ADC_CHANNEL);

        aiio_log_w("adc value is %d", value);
        aiio_os_tick_dealy(aiio_os_ms2tick(500));
    }
}