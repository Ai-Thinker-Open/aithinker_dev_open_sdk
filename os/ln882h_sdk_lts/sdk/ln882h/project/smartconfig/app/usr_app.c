#include "osal/osal.h"
#include "utils/debug/log.h"
#include "wifi.h"
#include "wifi_port.h"
#include "netif/ethernetif.h"
#include "wifi_manager.h"
#include "lwip/tcpip.h"
#include "drv_adc_measure.h"
#include "utils/debug/ln_assert.h"
#include "utils/system_parameter.h"
#include "utils/sysparam_factory_setting.h"
#include "utils/ln_psk_calc.h"
#include "utils/power_mgmt/ln_pm.h"
#include "hal/hal_adc.h"
#include "ln_nvds.h"
#include "ln_wifi_err.h"
#include "ln_misc.h"
#include "ln882h.h"
#include "usr_app.h"
#include "ln_smartcfg_api.h"

#define PM_DEFAULT_SLEEP_MODE             (ACTIVE)
#define PM_WIFI_DEFAULT_PS_MODE           (WIFI_NO_POWERSAVE)
#define WIFI_TEMP_CALIBRATE               (1)

#define USR_APP_TASK_STACK_SIZE           (6*256) //Byte

#if WIFI_TEMP_CALIBRATE
static OS_Thread_t g_temp_cal_thread;
#define TEMP_APP_TASK_STACK_SIZE          (4*256) //Byte
#endif

static OS_Thread_t g_usr_app_thread;

/* declaration */
static void wifi_init_sta(void);
static void local_connect_assigned_ap(uint8_t *ssid, uint8_t *pwd);
static void usr_app_task_entry(void *params);
static void temp_cal_app_task_entry(void *params);

static uint8_t mac_addr[6]        = {0x00, 0x50, 0xC2, 0x5E, 0x88, 0x99};
static uint8_t psk_value[40]      = {0x0};

static void wifi_init_sta(void)
{
    sta_ps_mode_t ps_mode = PM_WIFI_DEFAULT_PS_MODE;

    //1. sta mac get
     if (SYSPARAM_ERR_NONE != sysparam_sta_mac_get(mac_addr)) {
        LOG(LOG_LVL_ERROR, "[%s]sta mac get filed!!!\r\n", __func__);
        return;
    }
    if (mac_addr[0] == STA_MAC_ADDR0 &&
        mac_addr[1] == STA_MAC_ADDR1 &&
        mac_addr[2] == STA_MAC_ADDR2 &&
        mac_addr[3] == STA_MAC_ADDR3 &&
        mac_addr[4] == STA_MAC_ADDR4 &&
        mac_addr[5] == STA_MAC_ADDR5) {
        ln_generate_random_mac(mac_addr);
        sysparam_sta_mac_update((const uint8_t *)mac_addr);
    }

    //2. net device(lwip)
    netdev_set_mac_addr(NETIF_IDX_STA, mac_addr);
    netdev_set_active(NETIF_IDX_STA);

    //3. wifi start
    if(WIFI_ERR_NONE != wifi_sta_start(mac_addr, ps_mode)){
        LOG(LOG_LVL_ERROR, "[%s]wifi sta start filed!!!\r\n", __func__);
    }
}

static void local_connect_assigned_ap(uint8_t *ssid, uint8_t *pwd)
{
    wifi_sta_connect_t connect = {
        .ssid    = ssid,
        .pwd     = ((pwd == NULL) ? "" : pwd),
        .bssid   = NULL,
        .psk_value = NULL,
    };

    wifi_scan_cfg_t scan_cfg = {
        .channel   = 0,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = 20,
    };

    if (!ssid) {
        return;
    }

    if (pwd && strlen(pwd) != 0) {
        if (0 == ln_psk_calc(connect.ssid, connect.pwd, psk_value, sizeof (psk_value))) {
            connect.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }

    wifi_sta_connect(&connect, &scan_cfg);
}

void usr_app_task_entry(void *params)
{
    uint16_t channel_map = 0;
    uint8_t *ssid = NULL;
    uint8_t *pwd = NULL;
    LN_UNUSED(params);

    wifi_manager_init();
    wifi_init_sta();

    channel_map = CHANNEL1_MASK | CHANNEL2_MASK | CHANNEL3_MASK | CHANNEL4_MASK | \
                  CHANNEL5_MASK | CHANNEL6_MASK | CHANNEL7_MASK | CHANNEL8_MASK | \
                  CHANNEL9_MASK | CHANNEL10_MASK | CHANNEL11_MASK | CHANNEL12_MASK | CHANNEL13_MASK;

    if (LN_TRUE != ln_smartconfig_start(channel_map)) {
        LOG(LOG_LVL_INFO, "failed to start smartconfig...\r\n");
    }
    LOG(LOG_LVL_INFO, "smartconfig is started\r\n");

    while (LN_TRUE != ln_smartconfig_is_complete()) {
        OS_MsDelay(300);
    }

    ln_smartconfig_stop();

    ssid = ln_smartconfig_get_ssid();
    pwd  = ln_smartconfig_get_pwd();
    LOG(LOG_LVL_INFO, "smartconfig received ssid:%s; pwd:%s\r\n", ssid, pwd);

    local_connect_assigned_ap(ssid, pwd);

    //Wait for network link up
    while (!netdev_got_ip()) {
        OS_MsDelay(1000);
    }

    ln_smartconfig_send_ack();
    LOG(LOG_LVL_INFO, "smartconfig ack finish!\r\n");

    while(1)
    {
        OS_MsDelay(200);
    }
}

void temp_cal_app_task_entry(void *params)
{
    LN_UNUSED(params);
    uint8_t cnt = 0;
    int8_t cap_comp = 0;
    uint16_t adc_val = 0;
    int16_t curr_adc = 0;

    if (NVDS_ERR_OK == ln_nvds_get_xtal_comp_val((uint8_t *)&cap_comp)) {
        if ((uint8_t)cap_comp == 0xFF) {
            cap_comp = 0;
        }
    }

    drv_adc_init();

    wifi_temp_cal_init(drv_adc_read(ADC_CH0), cap_comp);

    while (1)
    {
        OS_MsDelay(1000);

        adc_val = drv_adc_read(ADC_CH0);
        wifi_do_temp_cal_period(adc_val);

        curr_adc = (adc_val & 0xFFF);

        cnt++;
        if ((cnt % 60) == 0) {
            LOG(LOG_LVL_INFO, "adc raw: %4d, temp_IC: %4d\r\n",
                    curr_adc, (int16_t)(25 + (curr_adc - 770) / 2.54f));
            LOG(LOG_LVL_INFO, "Total:%d; Free:%ld;\r\n", 
                    OS_HeapSizeGet(), OS_GetFreeHeapSize());
        }
    }
}

void creat_usr_app_task(void)
{
    {
        ln_pm_sleep_mode_set(PM_DEFAULT_SLEEP_MODE);

        /**
         * CLK_G_EFUSE: For wifi temp calibration
         * CLK_G_BLE  CLK_G_I2S  CLK_G_WS2811  CLK_G_DBGH  CLK_G_SDIO  CLK_G_EFUSE  CLK_G_AES
        */
        ln_pm_always_clk_disable_select(CLK_G_I2S | CLK_G_WS2811 | CLK_G_SDIO | CLK_G_AES);

        /**
         * ADC0: For wifi temp calibration
         * TIM3: For wifi pvtcmd evm test
         * CLK_G_ADC  CLK_G_GPIOA  CLK_G_GPIOB  CLK_G_SPI0  CLK_G_SPI1  CLK_G_I2C0  CLK_G_UART1  CLK_G_UART2
         * CLK_G_WDT  CLK_G_TIM_REG  CLK_G_TIM1  CLK_G_TIM2  CLK_G_TIM3  CLK_G_TIM4  CLK_G_MAC  CLK_G_DMA
         * CLK_G_RF  CLK_G_ADV_TIMER  CLK_G_TRNG
        */
        ln_pm_lightsleep_clk_disable_select(CLK_G_GPIOA | CLK_G_GPIOB | CLK_G_SPI0 | CLK_G_SPI1 | CLK_G_I2C0 |
                                            CLK_G_UART1 | CLK_G_UART2 | CLK_G_WDT | CLK_G_TIM1 | CLK_G_TIM2 | CLK_G_MAC | CLK_G_DMA | CLK_G_RF | CLK_G_ADV_TIMER| CLK_G_TRNG);
    }

    if(OS_OK != OS_ThreadCreate(&g_usr_app_thread, "UsrAPP", usr_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, USR_APP_TASK_STACK_SIZE)) {
        LN_ASSERT(1);
    }

#if  WIFI_TEMP_CALIBRATE
    if(OS_OK != OS_ThreadCreate(&g_temp_cal_thread, "TempAPP", temp_cal_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, TEMP_APP_TASK_STACK_SIZE)) {
        LN_ASSERT(1);
    }
#endif

    /* print sdk version */
    {
        LOG(LOG_LVL_INFO, "LN882H SDK Ver: %s [build time:%s][0x%08x]\r\n",
                LN882H_SDK_VERSION_STRING, LN882H_SDK_BUILD_DATE_TIME, LN882H_SDK_VERSION);
    }
}
