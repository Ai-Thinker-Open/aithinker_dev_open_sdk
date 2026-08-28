#define __QYQ_MIC_APP_C_
#include "qyq_mic_app.h"

extern rtos_queue_t mic_xqueue;

static void bw20_adc_init(void)
{
    ADC_InitTypeDef ADC_InitStruct; // ADC初始化结构体

    /* 启用ADC时钟和功能 */
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    //  PB18引脚
    Pinmux_Config(ADC_CH1_PIN, PINMUX_FUNCTION_ADC);  // 配置ADC引脚为ADC功能
    PAD_PullCtrl(ADC_CH1_PIN, GPIO_PuPd_NOPULL);      // 设置引脚无上拉下拉
    PAD_SleepPullCtrl(ADC_CH1_PIN, GPIO_PuPd_NOPULL); // 设置睡眠状态下无上拉下拉
    PAD_InputCtrl(ADC_CH1_PIN, DISABLE);              // 禁用引脚输入

    /* 初始化ADC为自动模式以获取连续采样数据 */
    ADC_StructInit(&ADC_InitStruct);           // 结构体初始化
    ADC_InitStruct.ADC_OpMode = ADC_AUTO_MODE; // 设置为自动模式
    ADC_InitStruct.ADC_CvlistLen = 0;          // 通道列表长度应为通道数量减1
    ADC_InitStruct.ADC_Cvlist[0] = ADC_CH1;    // 设置通道1
    ADC_Init(&ADC_InitStruct);                 // 初始化ADC
    ADC_Cmd(ENABLE);                           // 启用ADC
}

void qyq_mic_app_task(void *para)
{
    uint32_t sampbuf[20];
    uint32_t inputbuf[20];
    uint32_t adc_data;
    int32_t adc_vol; // ADC电压值

    // ADC 初始化
    bw20_adc_init();

    while (1)
    {
        ADC_ReceiveBuf(sampbuf, 20); // 接收ADC数据到缓冲区

        for (uint32_t i = 0; i < 20; i++) // 遍历接收的ADC数据
        {
            inputbuf[i] = ADC_GET_DATA_GLOBAL(sampbuf[i]); // 获取ADC数据
        }
        // 中位值平均滤波算法
        qyq_filter_algorithm_median_average(inputbuf, &adc_data, 20, 4);

        // 电压值
        adc_vol = ADC_GetVoltage(adc_data); // 将ADC数据转换为电压值

        if (adc_vol < 0)
        {
            adc_vol = 0;
        }
        else
        {
            if (adc_vol > 1650)
            {
                adc_vol = adc_vol - 1650;
            }
            else
            {
                adc_vol = 1650 - adc_vol;
            }
        }

        rtos_queue_send(mic_xqueue, &adc_vol, 0xffffffffUL);
        vTaskDelay(1);
    }
}
