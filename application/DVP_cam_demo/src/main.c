#include "aiio_adapter_include.h"
#include "user_common.h"

#ifdef CONFIG_CHERRYUSB
#include "usbh_core.h"
#include "usbh_uvc_uac.h"
#else
volatile uint32_t g_uvc_fps = 0;
#endif

#define DBG_TAG "MAIN"

/****************************************************************************
 * user Definitions
 ****************************************************************************/
#define USE_SD_FATFS    0

#define USER_AP_NAME "aiio_cam"
#define USER_AP_PASSWORD "123456789"

//fatfs
#include "fatfs_diskio_register.h"
#include "ff.h"

FATFS fs;
__attribute((aligned(8))) static uint32_t workbuf[4096];

MKFS_PARM fs_para = {
    .fmt = FM_FAT32,     /* Format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD) */
    .n_fat = 1,          /* Number of FATs */
    .align = 0,          /* Data area alignment (sector) */
    .n_root = 1,         /* Number of root directory entries */
    .au_size = 512 * 32, /* Cluster size (byte) */
};
static uint8_t is_fatfs_init_ok = 0;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WIFI_STACK_SIZE  (1536)
#define TASK_PRIORITY_FW (16)


/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct bflb_device_s *gpio;

static TaskHandle_t wifi_fw_task;

static wifi_conf_t conf = {
    .country_code = "CN",
};

extern void shell_init_with_task(struct bflb_device_s *shell);

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Functions
 ****************************************************************************/

static int wifi_start_firmware_task(void)
{
    aiio_log_i("Starting wifi ...\r\n");

    /* enable wifi clock */

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IP_WIFI_PHY | GLB_AHB_CLOCK_IP_WIFI_MAC_PHY | GLB_AHB_CLOCK_IP_WIFI_PLATFORM);
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_WIFI);

    /* set ble controller EM Size */

    GLB_Set_EM_Sel(GLB_WRAM160KB_EM0KB);

    if (0 != rfparam_init(0, NULL, 0)) {
        aiio_log_i("PHY RF init failed!\r\n");
        return 0;
    }

    aiio_log_i("PHY RF init success!\r\n");

    /* Enable wifi irq */

    extern void interrupt0_handler(void);
    bflb_irq_attach(WIFI_IRQn, (irq_callback)interrupt0_handler, NULL);
    bflb_irq_enable(WIFI_IRQn);

    xTaskCreate(wifi_main, (char *)"fw", WIFI_STACK_SIZE, NULL, TASK_PRIORITY_FW, &wifi_fw_task);

    return 0;
}

static void wifi_event_handler(uint32_t code)
{
    switch (code) {
        case CODE_WIFI_ON_INIT_DONE: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_INIT_DONE\r\n", __func__);
            wifi_mgmr_init(&conf);
        } break;
        case CODE_WIFI_ON_MGMR_DONE: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_MGMR_DONE\r\n", __func__);
#ifdef CONFIG_CHERRYUSB
#ifndef SIMU_DATA_TEST
            aiio_log_i("[USB] uvc/uav start\r\n");
            usbh_video_test();
            //usbh_audio_test();
#endif
#endif
        } break;
        case CODE_WIFI_ON_SCAN_DONE: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_SCAN_DONE\r\n", __func__);
            wifi_mgmr_sta_scanlist();
        } break;
        case CODE_WIFI_ON_CONNECTED: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_CONNECTED\r\n", __func__);
            void mm_sec_keydump();
            mm_sec_keydump();
        } break;
        case CODE_WIFI_ON_GOT_IP: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_GOT_IP\r\n", __func__);
        } break;
        case CODE_WIFI_ON_DISCONNECT: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_DISCONNECT\r\n", __func__);
        } break;
        case CODE_WIFI_ON_AP_STARTED: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_AP_STARTED\r\n", __func__);
        } break;
        case CODE_WIFI_ON_AP_STOPPED: {
            aiio_log_i("[APP] [EVT] %s, CODE_WIFI_ON_AP_STOPPED\r\n", __func__);
        } break;
        case CODE_WIFI_ON_AP_STA_ADD: {
            aiio_log_i("[APP] [EVT] [AP] [ADD] %lld\r\n", xTaskGetTickCount());
        } break;
        case CODE_WIFI_ON_AP_STA_DEL: {
            aiio_log_i("[APP] [EVT] [AP] [DEL] %lld\r\n", xTaskGetTickCount());
        } break;
        default: {
            aiio_log_i("[APP] [EVT] Unknown code %u \r\n", code);
        }
    }
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>

#define WIFI_HTTP_SERVER_STACK_SIZE  (1024 * 5)
#define HTTP_SERVERTASK_PRIORITY (15)
#define CAM_PROCESS_STACK_SIZE  (1024 * 1)
#define CAM_PROCESS_PRIORITY (HTTP_SERVERTASK_PRIORITY - 1)
#define button_PROCESS_STACK_SIZE  (1024)
#define button_PROCESS_PRIORITY (14)
static TaskHandle_t http_server_task_hd;
static TaskHandle_t cam_process_task_hd;
static TaskHandle_t button_process_task_hd;
extern char *datajpeg_buf;
extern uint32_t datajpeg_len;
void * MuxSem_Handle = NULL;

static void fps_printf_task(void *pvParameters)
{
    // extern volatile uint32_t g_dbi_disp_fps;
    // extern volatile uint32_t g_dvp_fps;
    extern volatile uint32_t g_mjpeg_fps;
    extern volatile uint32_t g_uvc_fps;

    static char info_buffer[1024];

    while (1) {
        for (uint32_t i = 0; i < 5; i++) {
            aiio_log_i("FPS ");
            // aiio_log_i("lcd: %2d, ", g_dbi_disp_fps);
            // aiio_log_i("dvp: %2d, ", g_dvp_fps);
            aiio_log_i("mjpeg: %2d, ", g_mjpeg_fps);
            aiio_log_i("uvc: %2d", g_uvc_fps);
            aiio_log_i("\r\n");
            vTaskDelay(1000);
        }

        vTaskGetRunTimeStats(info_buffer);
        aiio_log_i("\r\n<taskname>   <runtime>   <cpu load>\r\n%s\r\n", info_buffer);
    }
}



static void start_ap(void)
{
    wifi_mgmr_ap_params_t config = {0};

	config.channel = 3;
	config.key = USER_AP_PASSWORD;
	config.ssid = USER_AP_NAME;
	config.use_dhcpd = 1;

	if(wifi_mgmr_conf_max_sta(2) != 0){
		return 5;
	}
	if(wifi_mgmr_ap_start(&config) == 0){
		return 0;
	}
}

void http_server_task(void *param)
{
    start_ap();
    mhttp_server_init();
}

void button_process_task(void *param)
{
    static uint32_t press_10ms_cnt = 0;
    uint32_t press_mode = 0;
    static bool led_ctrl = 0;
    uint32_t frames = 1;
    uint8_t *pBuf = NULL;
    uint32_t len;
    uint8_t *pStr2 = NULL;
    uint32_t len2;

    bflb_gpio_init(gpio, GPIO_PIN_2, GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, GPIO_PIN_16, GPIO_OUTPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_1);
    // bflb_gpio_reset(gpio, GPIO_PIN_16);
    while(1){
        if(bflb_gpio_read(gpio, GPIO_PIN_2) == 1) {
            press_10ms_cnt++;
        }
        else if(bflb_gpio_read(gpio, GPIO_PIN_2) == 0){
            if(press_10ms_cnt > 10 && press_10ms_cnt < 100){
                press_mode = 1;
            }
            else if(press_10ms_cnt > 100){
                press_mode = 2;
            }
            press_10ms_cnt = 0;
        }

        if(1 == press_mode){
            press_mode = 0;
            aiio_log_i("1 == press_mode\r\n");
#if USE_SD_FATFS
            if(is_fatfs_init_ok == 1) {
                xSemaphoreTake(MuxSem_Handle, portMAX_DELAY);
                if(bl_cam_frame_fifo_get(&frames, &pBuf, &len, &pStr2, &len2, 2) == 0)
                {
                    take_photo(pBuf, len);
                    bl_cam_frame_pop();
                    // aiio_log_i("io2 pBuf:0x%x, len:%d\r\n", pBuf, len);
                }
                xSemaphoreGive(MuxSem_Handle);
            }
#endif
        }
        else if(2 == press_mode){
            press_mode = 0;
            led_ctrl = !led_ctrl;
            aiio_log_i("led_ctrl:%d\r\n", led_ctrl);
            if(led_ctrl){
                bflb_gpio_set(gpio, GPIO_PIN_16);
            }
            else{
                bflb_gpio_reset(gpio, GPIO_PIN_16);
            }
        }

        vTaskDelay(10);
    }
}

static void cam_process_task(void *param)
{
    uint32_t frames = 1;
    uint8_t *pBuf = NULL;
    uint32_t len;
    uint8_t *pStr2 = NULL;
    uint32_t len2;

    while(1)
    {
        xSemaphoreTake(MuxSem_Handle, portMAX_DELAY);
        if(bl_cam_frame_fifo_get(&frames, &pBuf, &len, &pStr2, &len2, 2) == 0)
        {
            bl_cam_frame_pop();
            datajpeg_buf = pBuf;
            datajpeg_len = len;
            // aiio_log_i("pBuf:0x%x, len:%d\r\n", pBuf, len);
        }else{
            aiio_log_i("ERR: pBuf:0x%x, len:%d\r\n", pBuf, len);
        }
        xSemaphoreGive(MuxSem_Handle);

        vTaskDelay(10);
    }
}
void create_http_server_task(void)
{
    MuxSem_Handle = xSemaphoreCreateMutex();
	if (NULL != MuxSem_Handle)
	{
		aiio_log_i("MuxSem_Handle creat success!\r\n");
	}

    xTaskCreate(http_server_task, (char *)"fw", WIFI_HTTP_SERVER_STACK_SIZE, NULL, HTTP_SERVERTASK_PRIORITY, &http_server_task_hd);
    xTaskCreate(cam_process_task, (char *)"cam_proc_task", CAM_PROCESS_STACK_SIZE, NULL, CAM_PROCESS_PRIORITY, &cam_process_task_hd);
    xTaskCreate(button_process_task, (char *)"button_proc_task", button_PROCESS_STACK_SIZE, NULL, button_PROCESS_PRIORITY, &button_process_task_hd);
}


int filesystem_init(void)
{
    FRESULT ret;

    board_sdh_gpio_init();

    fatfs_sdh_driver_register();

    ret = f_mount(&fs, "/sd", 1);
    // ret = FR_NO_FILESYSTEM;

    if (ret == FR_NO_FILESYSTEM) {
        aiio_log_i("No filesystem yet, try to be formatted...\r\n");

        ret = f_mkfs("/sd", &fs_para, workbuf, sizeof(workbuf));

        if (ret != FR_OK) {
            aiio_log_i("fail to make filesystem %d\r\n", ret);
            // _CALL_ERROR();
            return ret;
        }

        if (ret == FR_OK) {
            aiio_log_i("done with formatting.\r\n");
            aiio_log_i("first start to unmount.\r\n");
            ret = f_mount(NULL, "/sd", 1);
            aiio_log_i("then start to remount.\r\n");
        }
    } else if (ret != FR_OK) {
        aiio_log_i("fail to mount filesystem,error= %d\r\n", ret);
        aiio_log_i("SD card might fail to initialise.\r\n");
        // _CALL_ERROR();
        return ret;
    } else {
        aiio_log_i("Succeed to mount filesystem\r\n");
    }

    if (ret == FR_OK) {
        aiio_log_i("FileSystem cluster size:%d-sectors (%d-Byte)\r\n", fs.csize, fs.csize * 512);
    }

    is_fatfs_init_ok = 1;
    return 0;
}

int take_photo(uint8_t *Buf, uint32_t len)
{
    FRESULT ret;
    FIL fnew;
    UINT fnum;
    uint32_t time_node;
    static creat_file_cnt = 0;
    #define PHOTO_NAME_LEN 30
    uint8_t file_name[PHOTO_NAME_LEN] = {0};

    memset(file_name, 0, PHOTO_NAME_LEN);
    snprintf(file_name, PHOTO_NAME_LEN, "/sd/%d.jpg", creat_file_cnt);
    creat_file_cnt++;
    ret = f_open(&fnew, file_name, FA_CREATE_ALWAYS | FA_WRITE);
    // ret = f_open(&fnew, "/sd/test_file.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (ret == FR_OK) {
        time_node = (uint32_t)bflb_mtimer_get_time_ms();
        /*write into file*/
        ret = f_write(&fnew, Buf, len, &fnum);


        /* close file */
        ret |= f_close(&fnew);
        /* get time */
        time_node = (uint32_t)bflb_mtimer_get_time_ms() - time_node;

        if (ret == FR_OK) {
            aiio_log_i("Write Succeed! photo cnt:%d\r\n", creat_file_cnt-1);
            aiio_log_i("Write data size:%d Byte, written size:%d KB\r\n", len, fnum);
            // aiio_log_i("Time:%dms, Write Speed:%d KB/s \r\n", time_node, ((sizeof(RW_Buffer) * i) >> 10) * 1000 / time_node);
        } else {
            aiio_log_i("Fail to write files(%d)\n", ret);
            return;
        }
    } else {
        aiio_log_i("Fail to open or create files: %d.\r\n", ret);
        return;
    }
}

void user_test_func(void)
{
#if USE_SD_FATFS
    filesystem_init();
#endif
    
    bflb_irq_set_nlbits(4);
    bflb_irq_set_priority(37, 3, 0);
    bflb_irq_set_priority(WIFI_IRQn, 1, 0);

    gpio = bflb_device_get_by_name("gpio");

#ifndef SIMU_DATA_TEST

    /* queue init, Do not delete! */
    picture_frame_init();
    jpeg_frame_init();

    /*  */
    dbi_disp_free_init();

    /* mjpeg init */
    mjpeg_init(640, 480, 50);
    // mjpeg_init(1600, 1200, 50);
#ifndef CONFIG_CHERRYUSB
    /* dvp cam init */
    dvp_cam_init();
#endif

#endif

    tcpip_init(NULL, NULL);
    wifi_start_firmware_task();

#ifdef CONFIG_CHERRYUSB
    aiio_log_i("Starting usb host task...\r\n");
    /* usb video init */
    usbh_initialize();

#endif
    create_http_server_task();

}

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    user_test_func();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
