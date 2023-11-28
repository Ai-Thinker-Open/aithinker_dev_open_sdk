#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#include "ble_qiot_common.h"
#include "ble_qiot_import.h"
#include "ble_qiot_log.h"

#include "mac/ble/hl/api/gattm_task.h"
#include "ble_api/gap_gatt/api/ln_app_gatt.h"
#include "ble_api/gap_gatt/api/ln_app_gap.h"

#include "freertos_common.h"
#include "ble_arch/arch.h"
#include "hal/hal_flash.h"

/**************************************************************************************/
/*                            LLSync LN882H                                           */
/**************************************************************************************/
enum {
    LLSYNC_DECL_CHAR_DEVICE_INFO = 0, /* write(micro_program -> ln882h)  */
    LLSYNC_CHAR_VAL_DEVICE_INFO  = 1,
    
    LLSYNC_DECL_CHAR_EVENT       = 2, /* notify(ln882h -> micro_program) */
    LLSYNC_CHAR_VAL_EVENT        = 3,
    
    LLSYNC_CLIENT_CHAR_CFG_DSCR  = 4, /* CCCD (micro_program -> ln882h)  */
};

#define DEVICE_NAME                  ("q")
#define DEVICE_NAME_LEN              (sizeof(DEVICE_NAME))
#define ADV_DATA_MAX_LENGTH          (31)

extern uint8_t  adv_actv_idx;
uint8_t  llsync_conid  = 0xFF;
uint16_t llsync_mtu    = 23; // TODO: 128

static struct gattm_att_desc g_gap_service_atts[] = {
    [LLSYNC_DECL_CHAR_DEVICE_INFO] = {
        .uuid = { 0x03, 0x28 },
        .perm = PERM_MASK_RD,
        .max_len = 0,
        .ext_perm = (0 <<  PERM_POS_UUID_LEN),
    },
    [LLSYNC_CHAR_VAL_DEVICE_INFO] = {
        .uuid = { IOT_BLE_UUID_DEVICE_INFO & 0xFF, (IOT_BLE_UUID_DEVICE_INFO >> 8) & 0xFF},  //0xFF, 0xE1
        .perm = PERM_MASK_WRITE_REQ,
        .max_len = BLE_QIOT_EVENT_MAX_SIZE, // TODO:???
        .ext_perm = (2 << PERM_POS_UUID_LEN),
    },

    [LLSYNC_DECL_CHAR_EVENT] = {
        .uuid = { 0x03, 0x28 },
        .perm = PERM_MASK_RD,
        .max_len = 0,
        .ext_perm =  (0 << PERM_POS_UUID_LEN),
    },
    [LLSYNC_CHAR_VAL_EVENT] = {
        .uuid = {IOT_BLE_UUID_EVENT & 0xFF, (IOT_BLE_UUID_EVENT >> 8) & 0xFF},   // 0xFFE3
        .perm = PERM_MASK_NTF,
        .max_len = BLE_QIOT_EVENT_MAX_SIZE,
        .ext_perm =  (2 << PERM_POS_UUID_LEN),
    },

    [LLSYNC_CLIENT_CHAR_CFG_DSCR] = {
        .uuid = { 0x02, 0x29 },
        .perm = PERM_MASK_WRITE_REQ|PERM_MASK_RD,
        .max_len = 0,
        .ext_perm = 0,
    },
};


static uint8_t ch_write_handle   = 0;
static uint8_t ch_notify_handle  = 0;
static uint8_t ch_cccd_handle    = 0;
static uint8_t ch_cccd_enable    = 0;

void _char_svc_handle_set(uint8_t start_hdl)
{
    ch_write_handle  = start_hdl + LLSYNC_CHAR_VAL_DEVICE_INFO + 1;
    ch_notify_handle = start_hdl + LLSYNC_CHAR_VAL_EVENT + 1;
    ch_cccd_handle   = start_hdl + LLSYNC_CLIENT_CHAR_CFG_DSCR + 1;
}

uint8_t _is_char_cccd_handle(uint8_t hdl)
{
    return (hdl == ch_cccd_handle);
}
uint8_t _is_char_write_handle(uint8_t hdl)
{
    return (hdl == ch_write_handle);
}

uint8_t _char_write_handle_get(void)
{
    return ch_write_handle;
}
uint8_t _char_notify_handle_get(void)
{
    return ch_notify_handle;
}
uint8_t _cccd_handle_get(void)
{
    return ch_cccd_handle;
}

void _char_cccd_enable_set(uint8_t en)
{
    ch_cccd_enable = en;
}

static void _app_create_adv_activity(void)
{
    #define APP_ADV_CHMAP                (0x07) // Advertising channel map - 37, 38, 39
    #define APP_ADV_INT_MIN              (80)   // Advertising minimum interval - (0.625ms * 80)
    #define APP_ADV_INT_MAX              (100)  // Advertising maximum interval - (0.625ms * 100)

    struct ln_gapm_activity_create_adv_cmd  adv_creat_param = {0};

    adv_creat_param.own_addr_type                     = GAPM_STATIC_ADDR;
    adv_creat_param.adv_param.type                    = GAPM_ADV_TYPE_LEGACY;//GAPM_ADV_TYPE_EXTENDED;//GAPM_ADV_TYPE_LEGACY;
    adv_creat_param.adv_param.filter_pol              = ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_creat_param.adv_param.prim_cfg.chnl_map       = APP_ADV_CHMAP;
    adv_creat_param.adv_param.prim_cfg.phy            = GAP_PHY_1MBPS;
    adv_creat_param.adv_param.prop                    = GAPM_ADV_PROP_UNDIR_CONN_MASK;//GAPM_ADV_PROP_NON_CONN_SCAN_MASK;
    adv_creat_param.adv_param.disc_mode               = GAPM_ADV_MODE_GEN_DISC;
    adv_creat_param.adv_param.prim_cfg.adv_intv_min   = APP_ADV_INT_MIN;
    adv_creat_param.adv_param.prim_cfg.adv_intv_max   = APP_ADV_INT_MAX;
    adv_creat_param.adv_param.max_tx_pwr              = 0;
    //adv_creat_param.adv_param.second_cfg.phy        = GAP_PHY_1MBPS;//GAP_PHY_1MBPS;//GAP_PHY_CODED;
    adv_creat_param.adv_param.second_cfg.max_skip     = 0x00;
    adv_creat_param.adv_param.second_cfg.phy          = 0x01;
    adv_creat_param.adv_param.second_cfg.adv_sid      = 0x00;
    adv_creat_param.adv_param.period_cfg.adv_intv_min = 0x0400;
    adv_creat_param.adv_param.period_cfg.adv_intv_max = 0x0400;
    ln_app_advertise_creat(&adv_creat_param);
}

static void _app_set_adv_data(adv_info_s *adv)
{
    //adv data: adv length--adv type--adv string ASCII
    uint8_t raw_adv_data[ADV_DATA_MAX_LENGTH] = {
            /* flags */
            // 0x02, 0x01, 0x06,
            /* service UUID */
            0x03, 0x03, 0xF0, 0xFF
    };

    uint8_t temp_data_buf[ADV_DATA_MAX_LENGTH] = {0};
    uint8_t temp_data_len = 0;
    uint8_t index = 0;

    /* 1. temp data */
    memcpy(temp_data_buf, &adv->manufacturer_info.company_identifier, sizeof(uint16_t));
    temp_data_len = sizeof(uint16_t);
    memcpy(temp_data_buf + temp_data_len, adv->manufacturer_info.adv_data, adv->manufacturer_info.adv_data_len);
    temp_data_len += adv->manufacturer_info.adv_data_len;

    /* 2. raw data */
    index                 = 4;
    raw_adv_data[index++] = temp_data_len + 1;
    raw_adv_data[index++] = 0xFF;
    memcpy(raw_adv_data + index, temp_data_buf, temp_data_len);
    index += temp_data_len;

    raw_adv_data[index++] = strlen(DEVICE_NAME) + 1;
    raw_adv_data[index++] = 0x09;
    memcpy(raw_adv_data + index, DEVICE_NAME, strlen(DEVICE_NAME));
    index += strlen(DEVICE_NAME);

    ble_qiot_log_hex(BLE_QIOT_LOG_LEVEL_INFO, "BLE_ADV raw data", (const char *)raw_adv_data, index);

    struct ln_gapm_set_adv_data_cmd adv_data_param;
    adv_data_param.actv_idx = adv_actv_idx;
    adv_data_param.length   = index;
    adv_data_param.data     = raw_adv_data;
    ln_app_set_adv_data(&adv_data_param);
}

static void _app_start_adv(void)
{
    struct ln_gapm_activity_start_cmd  adv_start_param = {0};
    adv_start_param.actv_idx = adv_actv_idx;
    adv_start_param.u_param.adv_add_param.duration = 0;
    adv_start_param.u_param.adv_add_param.max_adv_evt = 0;
    ln_app_advertise_start(&adv_start_param);
}


/**************************************************************************************/
/*                            LLSync (ble_qiot_import.h)                              */
/**************************************************************************************/
int ble_get_mac(char *mac)
{
    ln_ble_mac_get((uint8_t *)mac);
    return 0;
}

void ble_services_add(const qiot_service_init_s *p_service)
{
    int nb_att = sizeof(g_gap_service_atts)/sizeof(g_gap_service_atts[0]);
    struct ln_gattm_add_svc_req p_svc_desc;
    p_svc_desc.svc_desc.start_hdl = 0;
    p_svc_desc.svc_desc.perm = (2 << 5);

    // service UUID (128bit) 0xFFF0
    memcpy(p_svc_desc.svc_desc.uuid, p_service->service_uuid128, 16);
    p_svc_desc.svc_desc.uuid[12] = p_service->service_uuid16 & 0xFF;
    p_svc_desc.svc_desc.uuid[13] = (p_service->service_uuid16 >> 8) & 0xFF;

    // characteristic (write) -- UUID 0xFFE1
    memcpy(g_gap_service_atts[LLSYNC_CHAR_VAL_DEVICE_INFO].uuid, p_service->service_uuid128, 16);
    g_gap_service_atts[LLSYNC_CHAR_VAL_DEVICE_INFO].uuid[12] = p_service->device_info.uuid16 & 0xFF;
    g_gap_service_atts[LLSYNC_CHAR_VAL_DEVICE_INFO].uuid[13] = (p_service->device_info.uuid16 >> 8) & 0xFF;

    // characteristic (notify) -- UUID 0xFFE3
    memcpy(g_gap_service_atts[LLSYNC_CHAR_VAL_EVENT].uuid, p_service->service_uuid128, 16);
    g_gap_service_atts[LLSYNC_CHAR_VAL_EVENT].uuid[12] = p_service->event.uuid16 & 0xFF;
    g_gap_service_atts[LLSYNC_CHAR_VAL_EVENT].uuid[13] = (p_service->event.uuid16 >> 8) & 0xFF;

    p_svc_desc.svc_desc.nb_att = nb_att;
    p_svc_desc.svc_desc.atts = g_gap_service_atts;
    ln_app_gatt_add_svc(&p_svc_desc);
}

ble_qiot_ret_status_t ble_advertising_start(adv_info_s *adv)
{
    _app_create_adv_activity();
    _app_set_adv_data(adv);
    _app_start_adv();
    return BLE_QIOT_RS_OK;
}

ble_qiot_ret_status_t ble_advertising_stop(void)
{
    ln_app_activity_stop(adv_actv_idx);
    return BLE_QIOT_RS_OK;
}

uint16_t ble_get_user_data_mtu_size(e_system type)
{
    return llsync_mtu;
}

ble_qiot_ret_status_t ble_send_notify(uint8_t *buf, uint8_t len)
{
    struct ln_gattc_send_evt_cmd param;
    param.handle = _char_notify_handle_get();
    param.length = len;
    param.value  = buf;

    ln_app_gatt_send_ntf(llsync_conid, &param);
    return BLE_QIOT_RS_OK;
}

#if (BLE_QIOT_BUTTON_BROADCAST || BLE_QIOT_SUPPORT_OTA)
  ble_timer_t ble_timer_create(uint8_t type, ble_timer_cb timeout_handle) {
      return (ble_timer_t)NULL;
  }
  
  ble_qiot_ret_status_t ble_timer_start(ble_timer_t timer_id, uint32_t period) {
      return BLE_QIOT_RS_OK;
  }
  
  ble_qiot_ret_status_t ble_timer_stop(ble_timer_t timer_id) {
      return BLE_QIOT_RS_OK;
  }
  
  ble_qiot_ret_status_t ble_timer_delete(ble_timer_t timer_id) {
      return BLE_QIOT_RS_OK;
  }
#endif /* (BLE_QIOT_BUTTON_BROADCAST || BLE_QIOT_SUPPORT_OTA) */

#ifdef BLE_QIOT_LLSYNC_STANDARD
  DeviceInfo sg_temp_devinfo = {0};

  int ble_get_product_id(char *product_id)
  {
      int ret = 0;
      memset(&sg_temp_devinfo, 0, sizeof(DeviceInfo));
      ret = HAL_GetDevInfo(&sg_temp_devinfo);
      if (ret != QCLOUD_RET_SUCCESS) {
          return -1;
      }

      memcpy(product_id, sg_temp_devinfo.product_id, BLE_QIOT_PRODUCT_ID_LEN);
      return 0;
  }

  int ble_get_device_name(char *device_name)
  {
      int ret = 0;
      memset(&sg_temp_devinfo, 0, sizeof(DeviceInfo));  
      ret = HAL_GetDevInfo(&sg_temp_devinfo);
      if (ret != QCLOUD_RET_SUCCESS) {
          return -1;
      }
  
      memcpy(device_name, sg_temp_devinfo.device_name, strlen(sg_temp_devinfo.device_name));
      return strlen(sg_temp_devinfo.device_name);
  }

  int ble_get_psk(char *psk)
  {
      int ret = 0;
      memset(&sg_temp_devinfo, 0, sizeof(DeviceInfo));  
      ret = HAL_GetDevInfo(&sg_temp_devinfo);
      strcpy(psk, sg_temp_devinfo.product_secret);
      return 0;  
  }

  #if BLE_QIOT_DYNREG_ENABLE

    int ble_set_psk(const char *psk, uint8_t len)
    {
        return 0;
    }

    int ble_get_product_key(char *product_secret)
    {
        return 0;
    }
  #endif /* BLE_QIOT_DYNREG_ENABLE */
    
  int ble_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len)
  {
      hal_flash_program(flash_addr, write_len, (uint8_t *)write_buf);
      return write_len;
  }

  int ble_read_flash(uint32_t flash_addr, char *read_buf, uint16_t read_len)
  {
      hal_flash_read_by_cache(flash_addr, read_len, (uint8_t *)read_buf);
      return read_len;
  }

  #if BLE_QIOT_SECURE_BIND

    void ble_secure_bind_user_cb(void)
    {
        return 0;
    }

    void ble_secure_bind_user_notify(uint8_t result)
    {
        return 0;
    }
  #endif /* BLE_QIOT_SECURE_BIND */

  #if BLE_QIOT_SUPPORT_OTA

    uint8_t ble_ota_is_enable(const char *version)
    {
        return 0;
    }

    uint32_t ble_ota_get_download_addr(void)
    {
        return 0;
    }

    int ble_ota_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len)
    {
        return 0;
    }
  #endif /* BLE_QIOT_SUPPORT_OTA */

  void ble_qiot_dev_start(void)
  {
//      extern void ble_app_init(void);
//      ble_app_init();
  }
#endif  /* BLE_QIOT_LLSYNC_STANDARD */


#if BLE_QIOT_LLSYNC_CONFIG_NET
  #include "qcloud_wifi_config.h"
  
  extern int   port_wifi_set_connected_param(char *ssid, uint8_t ssid_len, char *pwd, uint8_t pwd_len);
  extern char *port_wifi_get_connected_ap_ssid(void);
  extern char *port_wifi_get_connected_ap_pwd(void);

  ble_qiot_ret_status_t ble_combo_wifi_mode_set(BLE_WIFI_MODE mode)
  {
      //TODO:
      //1. set wifi mode
      
      //2. report to MicroProgrom
      ble_event_report_wifi_mode(0);
      return BLE_QIOT_RS_OK;
  }
  
  ble_qiot_ret_status_t ble_combo_wifi_info_set(const char *ssid, uint8_t ssid_len, const char *passwd, uint8_t passwd_len)
  {
      //TODO:
      //1. set wifi config info
      port_wifi_set_connected_param((char *)ssid, ssid_len, (char *)passwd, passwd_len);
      
      //2. report to MicroProgrom
      ble_event_report_wifi_info(0);
      
      //3. BLE WIFI_CONFIG_SUCCESS
      set_bt_combo_config_state(WIFI_CONFIG_SUCCESS);
      return BLE_QIOT_RS_OK;
  }
  
  ble_qiot_ret_status_t ble_combo_wifi_connect(void)
  {
      const char * ssid = port_wifi_get_connected_ap_ssid();
      const char * psw  = port_wifi_get_connected_ap_pwd();
      HAL_Wifi_StaConnect(ssid, psw, 0);
      return BLE_QIOT_RS_OK;
  }
  
  ble_qiot_ret_status_t ble_combo_wifi_token_set(const char *token, uint16_t token_len)
  {
      qiot_device_bind_set_token(token);
      return BLE_QIOT_RS_OK;
  }

  ble_qiot_ret_status_t ble_combo_wifi_log_get(void)
  {
      app_send_ble_dev_log();
      app_send_ble_error_log();
      return BLE_QIOT_RS_OK;
  }
#endif  /* BLE_QIOT_LLSYNC_CONFIG_NET */

