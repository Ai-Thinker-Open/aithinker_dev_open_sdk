#define __QYQ_WS2812_APP_C_
#include "qyq_ws2812_app.h"
#include "config.h"
// LED控制块
qyq_triled_control_block_t led_block[150];
extern rtos_queue_t mic_xqueue;
extern rtos_queue_t button_xqueue;
uint32_t colorcnt = 0;

// WS2812 灯光配置
qyq_ws2812_app_type_t ws2812_status = {
    .ws2812_power_status = 0,
    .ws2812_mode = 0,
    .ws2812_work_mode = 0,
    .led_num = 150,
    .normal_color = 0,
    .normal_contrast = 1,
    .normal_luminance = 10,
    .music_luminance = 10,
    .music_color = 150,
    .color_luminance = 10,
    .color_delay = 5,
    .water_luminance = 10,
    .water_delay = 20,
};

#define FLASH_APP_BASE_MODE 0x1EF000 // 工作模式

void bw20_flash_read_byte(uint32_t addr, uint32_t *value)
{
    *value = HAL_READ32(SPI_FLASH_BASE, addr);
}

void bw20_flash_erase(uint32_t addr)
{
    FLASH_Write_Lock();
    FLASH_Erase(EraseSector, addr);
    FLASH_Write_Unlock();
}

void bw20_flash_write_byte(uint32_t addr, uint32_t value)
{
    FLASH_Write_Lock();
    FLASH_TxData(addr, 4, (u8 *)&value);
    FLASH_Write_Unlock();

    DCache_Invalidate(SPI_FLASH_BASE + addr, 4);
}

void bw20_flash_write(uint32_t addr, uint32_t *buf, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        bw20_flash_write_byte(addr + i * 4, buf[i]);
    }
}

void bw20_flash_read(uint32_t addr, uint32_t *buf, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        bw20_flash_read_byte(addr + i * 4, &buf[i]);
    }
}

void bw20_flash_save(void)
{
    bw20_flash_erase(FLASH_APP_BASE_MODE);
    // 保存WS2812状态数据
    bw20_flash_write(FLASH_APP_BASE_MODE, &ws2812_status, sizeof(ws2812_status));
}

static void bw20_ws2812_init(void)
{
    // 加载LED块
    // qyq_triled_drive_init(led_block, 150);
    qyq_triled_drive_init(led_block, 150);

    // 打开全部LED
    qyq_triled_drive_turn_onall();
}

static void qyq_ws2812_music_mode(uint32_t num)
{
    if (num > ws2812_status.led_num)
    {
        num = ws2812_status.led_num;
    }
    qyq_triled_drive_set_hsv_numcolor(num, ws2812_status.music_color, 1, (float)((float)ws2812_status.music_luminance / 100));
}

// 5-105的时间间隔变化
static void qyq_ws2812_colorring_mode(void)
{
    colorcnt++;
    if (colorcnt > 360)
    {
        colorcnt = 0;
    }
    qyq_triled_drive_set_hsv_numcolor(ws2812_status.led_num, colorcnt, 1, (float)((float)ws2812_status.color_luminance / 100));
    // qyq_triled_drive_set_hsv_allcolor(colorcnt, 1, (float)((float)ws2812_status.color_luminance / 100));
    vTaskDelay(ws2812_status.color_delay);
}

// 20 - 150的时间间隔
static void qyq_ws2812_water_mode(void)
{
    if (ws2812_status.led_num > 4)
    {
        colorcnt++;
        if (colorcnt > ws2812_status.led_num - 4)
        {
            colorcnt = 0;
        }
        qyq_triled_drive_set_hsv_numintervals_color(colorcnt, colorcnt + 3, colorcnt * 3 % 360, 1, (float)((float)ws2812_status.water_luminance / 100));
    }

    vTaskDelay(ws2812_status.water_delay);
}

void qyq_ws2812_app_receive_task(void *para)
{
    qyq_button_app_type_t key_value;
    aiio_rev_queue_t rev_queue = {0};
    while (1)
    {
        rtos_queue_receive(button_xqueue, &key_value, 0xffffffffUL);
        // aiio_log_e("button_id:%d button_value:%d \r\n", key_value.button_id, key_value.button_value);
        switch (key_value.button_id)
        {
        case 0:
            if (key_value.button_value == 1)
            {
                if (ws2812_status.ws2812_mode == 1)
                {
                    ws2812_status.color_delay += 10;
                    if (ws2812_status.color_delay > 105)
                    {
                        ws2812_status.color_delay = 105;
                    }
                    aiio_report_int_attibute_status(NULL, NULL, CMD_BREATHING_SPEED, 100 - (ws2812_status.color_delay - 5));
                    aiio_log_e("color_delay:%d \r\n", ws2812_status.color_delay);
                }
                else if (ws2812_status.ws2812_mode == 2)
                {
                    ws2812_status.water_delay += 13;
                    if (ws2812_status.water_delay > 150)
                    {
                        ws2812_status.water_delay = 150;
                    }
                    aiio_report_int_attibute_status(NULL, NULL, CMD_RUNNING_SPEED, 100 - ((ws2812_status.water_delay - 20) * 10 / 13));
                    aiio_log_e("water_delay:%d \r\n", ws2812_status.water_delay);
                }
            }
            break;
        case 1:
            if (key_value.button_value == 1)
            {
                if (ws2812_status.ws2812_work_mode == 0)
                {
                    ws2812_status.ws2812_work_mode = 1;
                    // 上报颜色还是模式
                    aiio_report_int_attibute_status(NULL, NULL, CMD_WORK_MODE, ws2812_status.ws2812_work_mode);
                }
                else
                {
                    ws2812_status.ws2812_mode++;
                    if (ws2812_status.ws2812_mode > 2)
                    {
                        ws2812_status.ws2812_mode = 0;
                        ws2812_status.ws2812_work_mode = 0;
                        aiio_report_int_attibute_status(NULL, NULL, CMD_WORK_MODE, ws2812_status.ws2812_work_mode);
                    }
                    aiio_report_int_attibute_status(NULL, NULL, CMD_MODE, ws2812_status.ws2812_mode);
                }

                aiio_log_e("ws2812_mode:%d \r\n", ws2812_status.ws2812_mode);
            }
            else if (key_value.button_value == 2)
            {
                // 双击按键
                if (ws2812_status.ws2812_power_status)
                {
                    ws2812_status.ws2812_power_status = 0;
                }
                else
                {
                    ws2812_status.ws2812_power_status = 1;
                }
                aiio_report_bool_attibute_status(NULL, NULL, CMD_POWERSTATE, ws2812_status.ws2812_power_status);
                aiio_log_e("double click \r\n");
            }
            else if (key_value.button_value == 3)
            {
                // 长按按键处理
                rev_queue.common_event = REV_CONFIG_START_EVENT;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_e("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            }
            break;
        case 2:
            if (key_value.button_value == 1)
            {
                if (ws2812_status.ws2812_mode == 1)
                {
                    ws2812_status.color_delay -= 10;
                    if (ws2812_status.color_delay < 5)
                    {
                        ws2812_status.color_delay = 5;
                    }
                    aiio_report_int_attibute_status(NULL, NULL, CMD_BREATHING_SPEED, 100 - (ws2812_status.color_delay - 5));
                    aiio_log_e("color_delay:%d \r\n", ws2812_status.color_delay);
                }
                else if (ws2812_status.ws2812_mode == 2)
                {
                    ws2812_status.water_delay -= 13;
                    if (ws2812_status.water_delay < 20)
                    {
                        ws2812_status.water_delay = 20;
                    }
                    aiio_report_int_attibute_status(NULL, NULL, CMD_RUNNING_SPEED, 100 - ((ws2812_status.water_delay - 20) * 10 / 13));
                    aiio_log_e("water_delay:%d \r\n", ws2812_status.water_delay);
                }
            }
            break;
        default:
            break;
        }
        bw20_flash_erase(FLASH_APP_BASE_MODE);
        // 保存WS2812状态数据
        bw20_flash_write(FLASH_APP_BASE_MODE, &ws2812_status, sizeof(ws2812_status));
    }
}

void qyq_ws2812_app_task(void *para)
{
    int32_t powervalue = 0;
    int32_t outvalue = 0;
    int32_t read_buf[20];
    int32_t cushion_buf[20];
    qyq_ws2812_app_type_t tmp_status;

    // WS2812 初始化
    bw20_ws2812_init();

    bw20_flash_read(FLASH_APP_BASE_MODE, &tmp_status, sizeof(ws2812_status));

    aiio_log_e("\r\nRead ws2812_power_status 0x%lx\n", tmp_status.ws2812_power_status);
    aiio_log_e("\r\nRead ws2812_mode 0x%lx\n", tmp_status.ws2812_mode);
    aiio_log_e("\r\nRead led_num 0x%lx\n", tmp_status.led_num);
    aiio_log_e("\r\nRead normal_color 0x%lx\n", tmp_status.normal_color);
    aiio_log_e("\r\nRead normal_luminance 0x%lx\n", tmp_status.normal_luminance);
    aiio_log_e("\r\nRead music_luminance 0x%lx\n", tmp_status.music_luminance);
    aiio_log_e("\r\nRead music_color 0x%lx\n", tmp_status.music_color);
    aiio_log_e("\r\nRead color_luminance 0x%lx\n", tmp_status.color_luminance);
    aiio_log_e("\r\nRead color_delay 0x%lx\n", tmp_status.color_delay);
    aiio_log_e("\r\nRead water_luminance 0x%lx\n", tmp_status.water_luminance);
    aiio_log_e("\r\nRead water_delay 0x%lx\n", tmp_status.water_delay);

    if (((tmp_status.ws2812_mode >= 0) && (tmp_status.ws2812_mode <= 2)) &&
        ((tmp_status.normal_contrast == 1)) &&
        ((0 <= tmp_status.ws2812_power_status) && (tmp_status.ws2812_power_status <= 1)) &&
        ((0 <= tmp_status.ws2812_work_mode) && (tmp_status.ws2812_work_mode <= 1)) &&
        ((0 <= tmp_status.normal_color) && (tmp_status.normal_color < 360)) &&
        ((0 <= tmp_status.led_num) && (tmp_status.led_num <= 150)) &&
        ((0 <= tmp_status.normal_luminance) && (tmp_status.normal_luminance <= 100)) &&
        ((0 <= tmp_status.music_color) && (tmp_status.music_color < 360)) &&
        ((0 <= tmp_status.music_luminance) && (tmp_status.music_luminance <= 100)) &&
        ((0 <= tmp_status.water_luminance) && (tmp_status.water_luminance <= 100)) &&
        ((20 <= tmp_status.water_delay) && (tmp_status.water_delay <= 150)) &&
        ((0 <= tmp_status.color_luminance) && (tmp_status.color_luminance <= 100)) &&
        ((5 <= tmp_status.color_delay) && (tmp_status.color_delay <= 105)))
    {
        ws2812_status.ws2812_power_status = tmp_status.ws2812_power_status;
        ws2812_status.ws2812_mode = tmp_status.ws2812_mode;
        ws2812_status.ws2812_work_mode = tmp_status.ws2812_work_mode;
        ws2812_status.led_num = tmp_status.led_num;
        ws2812_status.normal_color = tmp_status.normal_color;
        ws2812_status.normal_contrast = tmp_status.normal_contrast;
        ws2812_status.normal_luminance = tmp_status.normal_luminance;
        ws2812_status.music_luminance = tmp_status.music_luminance;
        ws2812_status.music_color = tmp_status.music_color;
        ws2812_status.color_luminance = tmp_status.color_luminance;
        ws2812_status.color_delay = tmp_status.color_delay;
        ws2812_status.water_luminance = tmp_status.water_luminance;
        ws2812_status.water_delay = tmp_status.water_delay;

        aiio_log_e("\r\n config read flash success \r\n");
    }

    while (1)
    {
        // 只有电源打开才会进行模式
        if (ws2812_status.ws2812_power_status)
        {
            if (ws2812_status.ws2812_work_mode)
            {
                switch (ws2812_status.ws2812_mode)
                {
                case 0:
                    rtos_queue_receive(mic_xqueue, &powervalue, 0xffffffffUL);
                    // 中位值滤波有限幅的效果
                    qyq_filter_algorithm_recursive_median(powervalue, &outvalue, read_buf, cushion_buf, 20);
                    // aiio_log_e("powervalue: %dmV outvalue: %dmV\n", powervalue, outvalue);

                    qyq_ws2812_music_mode((uint32_t)(outvalue / 10));
                    vTaskDelay(1);
                    break;
                case 1:
                    qyq_ws2812_colorring_mode();
                    break;
                case 2:
                    qyq_ws2812_water_mode();
                    break;
                default:
                    break;
                }
            }
            else
            {
                qyq_triled_drive_set_hsv_numcolor(ws2812_status.led_num, ws2812_status.normal_color, ws2812_status.normal_contrast, (float)((float)ws2812_status.normal_luminance / 100));
                vTaskDelay(100);
            }
        }
        else
        {
            // 关闭所有的LED
            qyq_triled_drive_set_hsv_numcolor(0, 0, 0, 0);
            vTaskDelay(100);
        }
    }
}
