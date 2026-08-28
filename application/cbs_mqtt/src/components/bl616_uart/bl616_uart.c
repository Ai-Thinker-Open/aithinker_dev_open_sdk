#define __BL616_UART_C_
#include "bl616_uart.h"
#include "qyq_frame_at.h"

qyq_frame_at_type_t qyq_frame_at;
qyq_frame_at_config_t qyq_frame_at_config;
static TaskHandle_t bl618_uart_handle;
static TaskHandle_t bl618_uart_tick_handle;
static TaskHandle_t bl618_uart_rec_handle;
volatile struct bflb_device_s *uartx;
volatile struct bflb_device_s *gpio;
static uint8_t uart1_read_buf[1024];

void qyq_frame_at_write(uint8_t *buf, uint16_t length)
{
    uint16_t i = 0;
    for (i = 0; i < length; i++)
    {
        bflb_uart_putchar(uartx, buf[i]);
    }
}

int8_t bl616_wifi_sta_set(unsigned int argc, const char **argv)
{
    if (argc != 2)
    {
        return -1;
    }

    uint8_t write_ssid_buf[128] = {0};
    uint8_t write_pass_buf[128] = {0};

    for (int i = 0; i < strlen(argv[0]); i++)
    {
        write_ssid_buf[i] = argv[0][i];
    }

    for (int i = 0; i < strlen(argv[1]) - 2; i++)
    {
        write_pass_buf[i] = argv[1][i];
    }

    // 擦除闪存区域
    bflb_flash_erase(FLASH_RW_START_ADDR, sizeof(write_ssid_buf) + sizeof(write_pass_buf));

    // 写入值到闪存
    bflb_flash_write(FLASH_RW_START_ADDR, write_ssid_buf, sizeof(write_ssid_buf));
    bflb_flash_write(FLASH_RW_START_ADDR + sizeof(write_ssid_buf), write_pass_buf, sizeof(write_pass_buf));

    return 0;
}

void uart_mac_to_string_with_prefix(const uint8_t mac[6], char *mac_str)
{
    // 定义前缀
    const char *prefix = "AiPi-CBS-Kit/";

    // 将前缀复制到输出字符串中
    strcpy(mac_str, prefix);

    // 将MAC地址转换为字符串并追加到前缀后面
    sprintf(mac_str + strlen(prefix), "%02X%02X%02X%02X%02X%02X\r\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int8_t bl616_printf_topic(unsigned int argc, const char **argv)
{
    uint8_t mac_addr[6];
    uint8_t top_str[100];

    aiio_wifi_sta_mac_get(mac_addr);

    uart_mac_to_string_with_prefix(mac_addr, top_str);

    qyq_frame_at_write(top_str, strlen(top_str));

    return 0;
}

qyq_frame_at_list_t qyq_frame_at_list[] = {
    {"+STACONFIG", 10, null, null, bl616_wifi_sta_set, null},
    {"+GPIOHIGH", 9, null, null, null, null},
    {"+TOPIC", 6, null, null, null, bl616_printf_topic},
};

// 1ms定时时钟
void bl616_uart1_tick_task(void *msg)
{
    while (1)
    {
        qyq_frame_at.qyq_frame_at_tick(&qyq_frame_at);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return NULL;
}

void bl616_uart1_at_rec_task(void *msg)
{
    int ch;

    while (1)
    {
        ch = bflb_uart_getchar(uartx);
        if (ch != -1)
        {
            qyq_frame_at.qyq_frame_at_recv(&qyq_frame_at, ch);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void bl616_at_init(void)
{
    qyq_frame_at_config.qyq_frame_at_list = qyq_frame_at_list;
    qyq_frame_at_config.qyq_frame_at_list_size = (sizeof(qyq_frame_at_list) / sizeof(qyq_frame_at_list_t));

    qyq_frame_at_config.qyq_frame_at_rx_buf = uart1_read_buf;
    qyq_frame_at_config.qyq_frame_at_rx_size = sizeof(uart1_read_buf);
    qyq_frame_at_config.qyq_frame_at_write = qyq_frame_at_write;

    if (qyq_frame_at_create(&qyq_frame_at, &qyq_frame_at_config) != 0)
    {
        aiio_log_e("qyq_frame_at_create is fail!\r\n");
    }

    if (qyq_frame_at.qyq_frame_at_init(&qyq_frame_at) != 0)
    {
        aiio_log_e("qyq_frame_at_init is fail!\r\n");
    }

    xTaskCreate(bl616_uart1_tick_task, (char *)"bl616_uart1_tick_task", 1152, NULL, 15, &bl618_uart_tick_handle);
    xTaskCreate(bl616_uart1_at_rec_task, (char *)"bl616_uart1_at_rec_task", 1152, NULL, 14, &bl618_uart_rec_handle);
}

void bl616_uart1_init(void)
{
    gpio = bflb_device_get_by_name("gpio");

    bflb_gpio_init(gpio, GPIO_PIN_16, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_uart_init(gpio, GPIO_PIN_21, GPIO_UART_FUNC_UART1_TX);
    bflb_gpio_uart_init(gpio, GPIO_PIN_22, GPIO_UART_FUNC_UART1_RX);

    uartx = bflb_device_get_by_name("uart1");

    struct bflb_uart_config_s cfg;

    cfg.baudrate = 9600;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    bflb_uart_init(uartx, &cfg);
}

// 串口运行任务
void bl616_uart_task(void)
{
    // AT 框架初始化
    bl616_at_init();

    // 串口初始化
    bl616_uart1_init();

    while (1)
    {
        qyq_frame_at.qyq_frame_at_run(&qyq_frame_at);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void bl616_uart_init(void)
{
    xTaskCreate(bl616_uart_task, (char *)"bl616_uart_task", 512, NULL, 20, &bl618_uart_handle);
}
