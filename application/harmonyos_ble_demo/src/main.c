#include "aiio_adapter_include.h"
#include "aiio_os_port.h"
#include "aiio_ble.h"
#include "aiio_gpio.h"
#include "aiio_harmony_ble.h"

#define KEY_PORT    CONFIG_KEY_PORT
#define KEY_PIN     CONFIG_KEY_PIN
#define LED_PORT    CONFIG_GPIO_R_LED_PORT
#define LED_PIN     CONFIG_GPIO_R_LED_PIN


static uint8_t led_status = 1;
static int seq = 0;

static int32_t _device_ctrl(uint8_t *data, uint16_t len)
{
    if (strstr((const char *)data, "\"on\":0"))
    {
        //关灯
        led_status = 0;
        
    }
    else if (strstr((const char *)data, "\"on\":1"))
    {
        //开灯
        led_status = 1;
    }

    aiio_hal_gpio_set(LED_PORT, LED_PIN, led_status);

    return 0;
}

static void _harmony_connected(void)
{
    uint8_t buff[128];

    aiio_log_i("harmony connected");

    snprintf((char *)buff, sizeof(buff), "{\"vendor\":{\"sid\":\"switch\",\"data\":{\"on\":%d}},\"seq\":%d}", led_status, seq);
    aiio_harmony_ble_data_report(buff, strlen((const char *)buff));
}

void ble_main(void)
{
    aiio_hamony_dev_info_t dev = {
        .pid = "2NGN",
        .custom_name = "LIGHT",
        .ver = "1.0.0",
        .menu = "axk",
    };
    aiio_harmony_ble_dev_info_set(&dev);

    aiio_harmony_ble_conn_cb_reg(_harmony_connected);
    aiio_harmony_ble_recv_remote_data_cb_reg(_device_ctrl);
    aiio_harmony_ble_init();
}

static void _key_detect(void)
{
    uint16_t cnt = 0;
    uint8_t buff[256];

    for (;;)
    {
        if (aiio_hal_gpio_get(KEY_PORT, KEY_PIN) == 1)
        {
            cnt ++;
            if (cnt >= 100)
            {
                aiio_log_i("key long press");
                for (size_t i = 0; i < 15; i++)
                {
                    led_status = i % 2;
                    aiio_hal_gpio_set(LED_PORT, LED_PIN, led_status);
                    aiio_os_tick_dealy(100);
                }
                aiio_harmony_ble_restore();
            }
        }
        else
        {
            if (cnt >= 2 && cnt < 100)
            {
                aiio_log_i("key press");
                led_status = !led_status;
                aiio_hal_gpio_set(LED_PORT, LED_PIN, led_status);
                snprintf((char *)buff, sizeof(buff), "{\"vendor\":{\"sid\":\"switch\",\"data\":{\"on\":%d}},\"seq\":%d}", led_status, seq);
                aiio_log_i("report: %s", buff);
                aiio_harmony_ble_data_report(buff, strlen((const char *)buff));
                (seq >= 0xFFFF) ? (seq = 0) : (seq++);
            }

            cnt = 0;
        }

        aiio_os_tick_dealy(15);
    }
}

void _device_init(void)
{
    led_status = 1;
    aiio_hal_gpio_init(LED_PORT, LED_PIN);
    aiio_hal_gpio_pin_direction_set(LED_PORT, LED_PIN, AIIO_GPIO_OUTPUT);

    aiio_hal_gpio_init(CONFIG_GPIO_G_LED_PORT, CONFIG_GPIO_G_LED_PIN);
    aiio_hal_gpio_pin_direction_set(CONFIG_GPIO_G_LED_PORT, CONFIG_GPIO_G_LED_PIN, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_set(CONFIG_GPIO_G_LED_PORT, CONFIG_GPIO_G_LED_PIN, 0);

    aiio_hal_gpio_init(CONFIG_GPIO_B_LED_PORT, CONFIG_GPIO_B_LED_PIN);
    aiio_hal_gpio_pin_direction_set(CONFIG_GPIO_B_LED_PORT, CONFIG_GPIO_B_LED_PIN, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_set(CONFIG_GPIO_B_LED_PORT, CONFIG_GPIO_B_LED_PIN, 0);


    aiio_hal_gpio_init(KEY_PORT, KEY_PIN);
    aiio_hal_gpio_pin_direction_set(KEY_PORT, KEY_PIN, AIIO_GPIO_INPUT);

    aiio_hal_gpio_set(LED_PORT, LED_PIN, led_status);

    aiio_os_thread_create(NULL, "key", _key_detect, 4096, NULL, 8);
}

void aiio_main(void *params)
{
    aiio_log_i("Harmonyos BLE DEMO!");

    aiio_nvs_init();

    _device_init();

    ble_main();

    aiio_os_thread_delete(NULL);
}

