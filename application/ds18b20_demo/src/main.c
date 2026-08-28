#include "module_single.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

module_single_type_t module_single;
module_single_config_t module_single_config;

static volatile int cnt = 0;

#ifndef CONFIG_GPIO_DS18B20_DELAY
#define CONFIG_GPIO_DS18B20_DELAY (105)
#endif
#define AIIO_GPIO_DS18B20_PORT CONFIG_GPIO_DS18B20_PORT
#define AIIO_GPIO_DS18B20_PIN CONFIG_GPIO_DS18B20_PIN
#define AIIO_GPIO_DS18B20_DELAY CONFIG_GPIO_DS18B20_DELAY

void module_single_config_write(uint8_t status)
{
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN, AIIO_GPIO_OUTPUT);
    if (status)
    {
        aiio_hal_gpio_set(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN, 1);
    }
    else
    {
        aiio_hal_gpio_set(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN, 0);
    }
}
uint8_t module_single_config_read(void)
{
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN, AIIO_GPIO_INPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN, AIIO_GPIO_PULL_UP);

    if (aiio_hal_gpio_get(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN) == 0)
    {
        return 0;
    }
    return 1;
}
void module_single_config_delay(void)
{
    cnt = AIIO_GPIO_DS18B20_DELAY;
    while (cnt--)
        ;
}

uint16_t ds18b20_readtemp() // 读取的温度有两个字节，分别用tml和tmh存储两个字节
{
    uint16_t temp = 0;
    module_single_dat_t module_single_dat;
    module_single_dat_t module_single_dat1;
    uint8_t buf[2] = {0xcc, 0x44};
    uint8_t buf1[2];
    module_single_dat.buf = buf;
    module_single_dat.buf_len = 2;
    module_single_dat1.buf = buf1;
    module_single_dat1.buf_len = 2;
    buf[0] = 0xCC;
    buf[1] = 0x44;
    module_single.module_single_reset(&module_single);
    module_single.module_single_write(&module_single, &module_single_dat);
    aiio_os_tick_dealy(aiio_os_ms2tick(500));
    buf[0] = 0xCC;
    buf[1] = 0xbe;
    module_single.module_single_reset(&module_single);
    module_single.module_single_write(&module_single, &module_single_dat);
    module_single.module_single_read(&module_single, &module_single_dat1);
    aiio_os_tick_dealy(aiio_os_ms2tick(500));

    aiio_log_w("temph:0x%02x templ:0x%02x",buf1[1],buf1[0]);

    temp = buf1[1]; // 先存高字节，后面通过移位存储低字节
    temp <<= 8;
    temp |= buf1[0];
    return temp;
}

int Temper_change()
{
    int temper;
    float tp;
		temper=ds18b20_readtemp();
    if(temper<0)    //考虑负温度的情况
    {
        temper=temper-1;
        temper=~temper;
        tp=temper*0.0625;  //16位温度转换成10进制的温度
        temper=tp*100+0.5;   //留两个小数点，并四舍五入
    }
    else
    {
        tp=temper*0.0625;  //16位温度转换成10进制的温度
        temper=tp*100+0.5;  //留两个小数点，并四舍五入
    }
    return temper;
}

void aiio_main(void *params)
{
    uint16_t temp = 0;
    aiio_hal_gpio_init(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_DS18B20_PORT, AIIO_GPIO_DS18B20_PIN, AIIO_GPIO_OUTPUT);

    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    module_single_config.module_single_config_write = module_single_config_write;
    module_single_config.module_single_config_read = module_single_config_read;
    module_single_config.module_single_config_delay = module_single_config_delay;

    module_single_create(&module_single, &module_single_config);

    aiio_os_tick_dealy(aiio_os_ms2tick(1));

    while (1)
    {
        temp = ds18b20_readtemp();
        aiio_log_w("temp %f", temp*0.0625);
    }
}
