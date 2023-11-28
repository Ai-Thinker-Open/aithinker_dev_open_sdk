#ifndef __WIFI_H__
#define __WIFI_H__

#include "ln_types.h"

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#define SSID_MAX_LEN                         (33)
#define PASSWORD_MAX_LEN                     (64)
#ifndef MAC_ADDRESS_LEN
    #define MAC_ADDRESS_LEN                  (6)
#endif /* MAC_ADDRESS_LEN */
#define BSSID_LEN                            (MAC_ADDRESS_LEN)

#define WIFI_SNIFFER_FILTER_MASK_MGMT        (1UL << 0U)     /**< WIFI_PKT_MGMT packet filter mask */
#define WIFI_SNIFFER_FILTER_MASK_CTRL        (1UL << 1U)     /**< WIFI_PKT_CTRL packet filter mask */
#define WIFI_SNIFFER_FILTER_MASK_DATA        (1UL << 2U)     /**< WIFI_PKT_DATA packet filter mask */
#define WIFI_SNIFFER_FILTER_MASK_ALL         (0xFFFFFFFF)    /**< all packets filter mask */

typedef enum
{
    WIFI_PKT_TYPE_MGMT            = 0,  /**< sniffer packet is management frame */
    WIFI_PKT_TYPE_CTRL            = 1,  /**< sniffer packet is control frame */
    WIFI_PKT_TYPE_DATA            = 2,  /**< sniffer packet is data frame */
} wifi_pkt_type_t;

typedef enum {
    STATION_IF                    = 0,  /**<  station interface */
    SOFT_AP_IF                    = 1,  /**<  soft-AP interface */
    WIFI_IF_NUM                   = 2,
} wifi_interface_t;

typedef enum {
    WIFI_MODE_STATION             = 0,  /**<  station mode */
    WIFI_MODE_AP                  = 1,  /**<  SoftAP mode */
    WIFI_MODE_AP_STATION          = 2,  /**<  Station + SoftAP mode, not support */
	WIFI_MODE_MAX,
	WIFI_MODE_OFF
} wifi_mode_t;

typedef enum {
    WIFI_STA_STATUS_STARTUP       = 0,
    WIFI_STA_STATUS_SCANING       = 1,
    WIFI_STA_STATUS_CONNECTING    = 2,
    WIFI_STA_STATUS_CONNECTED     = 3,
    WIFI_STA_STATUS_DISCONNECTING = 4,
    WIFI_STA_STATUS_DISCONNECTED  = 5,
} wifi_sta_status_t;

typedef enum {
    WIFI_SCAN_TYPE_ACTIVE         = 0,   /**<  active scan */
	WIFI_SCAN_TYPE_PASSIVE        = 1,   /**<  passive scan */
} wifi_scan_type_t;

typedef enum {
    WIFI_AUTH_OPEN                = 0,   /**<  authenticate mode : open */
    WIFI_AUTH_WEP                 = 1,   /**<  authenticate mode : WEP */
    WIFI_AUTH_WPA_PSK             = 2,   /**<  authenticate mode : WPA_PSK */
    WIFI_AUTH_WPA2_PSK            = 3,   /**<  authenticate mode : WPA2_PSK */
    WIFI_AUTH_WPA_WPA2_PSK        = 4,   /**<  authenticate mode : WPA_WPA2_PSK */
    WIFI_AUTH_WPA2_ENTERPRISE     = 5,   /**<  authenticate mode : WPA2_ENTERPRISE */
    WIFI_AUTH_MAX
} wifi_auth_mode_t;

typedef enum {
	CTRY_CODE_CN                  = 0,
	CTRY_CODE_US                  = 1,
	CTRY_CODE_JP                  = 2,
	CTRY_CODE_ISR                 = 3,
	CTRY_CODE_MAX                 = 4,
} wifi_country_code_t;

typedef enum
{
    WIFI_NO_POWERSAVE             = 0,
    WIFI_MIN_POWERSAVE            = 1,
    WIFI_MAX_POWERSAVE            = 2,
} sta_ps_mode_t;

typedef enum {
    WIFI_CONN_WRONG_PWD           = 1,
    WIFI_CONN_TARGET_AP_NOT_FOUND = 2,
    WIFI_CONN_TIMEOUT             = 3,
} sta_connect_failed_reason_t;



typedef struct {
    uint8_t                      channel;         /**<  channel of target AP.  If the channel of AP is unknown, set it to 0.*/
    wifi_scan_type_t             scan_type;       /**<  scan type, active or passive */
    uint16_t                     scan_time;       /**<  scan time per channel */
} wifi_scan_cfg_t;

typedef struct {
    char                       * ssid;            /**<  SSID of target AP */
    char                       * pwd;             /**<  password of target AP */
    uint8_t                    * bssid;           /**<  BSSID of target AP */
    uint8_t                    * psk_value;
} wifi_sta_connect_t;

typedef struct {
    uint8_t                      channel;         /**<  Channel of softAP */
		uint8_t											 max_conn;
    wifi_auth_mode_t             authmode;        /**<  Auth mode of softAP. Do not support AUTH_WEP in softAP mode */
    uint8_t                      ssid_hidden;     /**<  Broadcast SSID or not, default 0, broadcast the SSID */
    uint16_t                     beacon_interval; /**<  Beacon interval, 100 ~ 60000 ms, default 100 ms */
    uint8_t                    * psk_value;
} wifi_softap_ext_cfg_t;

typedef struct {
    char                       * ssid;            /**<  SSID of softAP */
    char                       * pwd;             /**<  Password of softAP */
    uint8_t                    * bssid;           /**<  softAP's own MAC address*/
    wifi_softap_ext_cfg_t        ext_cfg;
} wifi_softap_cfg_t;

typedef struct {
    uint8_t                      bssid[BSSID_LEN];
    char                         ssid[SSID_MAX_LEN];
    uint8_t                      channel;
    wifi_auth_mode_t             authmode;
    uint8_t                      imode;
    int8_t                       rssi;
    int16_t                      freq_offset;
    uint8_t                      bgn;
    uint8_t                      wps_en;
    uint8_t                      is_hidden;
} ap_info_t;

typedef struct {
    uint8_t  mac_addr[6];
} sta_info_t;


typedef void (*sta_startup_cb_t)(void);
typedef void (*sta_connected_cb_t)(void);
typedef void (*sta_disconnected_cb_t)(void);
typedef void (*sta_scan_complete_cb_t)(void);
typedef void (*sta_scan_report_cb_t)(const ap_info_t *ap_info);
//TODO: typedef void (*sta_connect_failed_cb_t)(sta_connect_failed_reason_t reason);

typedef void (*ap_startup_cb_t)(void);
typedef void (*ap_associated_cb_t)(const uint8_t *mac_addr);
typedef void (*ap_disassociated_cb_t)(const uint8_t *mac_addr);
//TODO: typedef void (*ap_associat_failed_cb_t)(uint8_t *mac_addr);

typedef void (*wifi_if_recv_cb_t)(uint8_t *data, uint16_t len);
typedef void (*wifi_sniffer_cb_t)(void *buf, uint16_t len, wifi_pkt_type_t pkt_type, int8_t rssi_db);
typedef void (*wifi_recv_ethernet_pkt_cb_t)(uint8_t *data, uint16_t len);

typedef void (*wifi_mgnt_cb_t)(uint8_t *buf, uint32_t len);

typedef struct {
    sta_startup_cb_t           startup;
    sta_connected_cb_t         connected;
    sta_disconnected_cb_t      disconnected;
    sta_scan_complete_cb_t     scan_complete;
    sta_scan_report_cb_t       scan_report;
} sta_cb_t;

typedef struct {
    ap_startup_cb_t            startup;
    ap_associated_cb_t         associated;
    ap_disassociated_cb_t      disassociated;
} ap_cb_t;

typedef struct {
    int                        enable;
    int                        type;         // 0: sniffer mode; 1: monitor mode
    uint32_t                   filter_mask;
} sniffer_cfg_t;


//wifi general-purpose API
int     wifi_init(void);
int     wifi_deinit(void);

int     wifi_sta_start(uint8_t * mac_addr, sta_ps_mode_t ps_mode);
int     wifi_softap_start(wifi_softap_cfg_t * ap_cfg);
int     wifi_stop(void);

wifi_mode_t wifi_current_mode_get(void);

int     wifi_sta_scan(wifi_scan_cfg_t * scan_cfg);
int     wifi_sta_connect(wifi_sta_connect_t * connect, wifi_scan_cfg_t * scan_cfg);//if scan_cfg = NULL, uase default scan param.
int     wifi_sta_disconnect(void);

int     wifi_get_sta_scan_cfg(wifi_scan_cfg_t *scan_cfg);
int     wifi_get_sta_status(wifi_sta_status_t *status);
int     wifi_get_sta_conn_info(const char **ssid, const uint8_t **bssid);

/**
 * @brief wifi_get_psk_info: get psk info for fast connect.
 * 
 * @param ssid     out param, do not modify the value of pointer
 * @param pwd      out param, do not modify the value of pointer
 * @param pwd_len  out param
 * @param psk      out param, do not modify the value of pointer
 * @return int     0 -> success; !0 -> failed
 */
int     wifi_get_psk_info(const char **ssid, const uint8_t **pwd, uint8_t *pwd_len, const uint8_t **psk);

int     wifi_set_macaddr(wifi_interface_t if_index, const uint8_t *mac_addr);
int     wifi_get_macaddr(wifi_interface_t if_index, uint8_t *mac_addr);

int     wifi_sta_set_powersave(sta_ps_mode_t ps_mode);
int     wifi_sta_get_powersave(sta_ps_mode_t * ps_mode);

int     wifi_set_country_code(wifi_country_code_t country_code);
int     wifi_get_country_code(wifi_country_code_t *country_code);
int     wifi_set_channel(uint8_t channel);
int     wifi_get_channel(uint8_t *channel);

int     wifi_set_11n_enable(uint8_t ht_enable);
int     wifi_set_allow_cpu_sleep_flag(uint8_t allow);

int     wifi_sta_set_listen_interval(int interval);
int     wifi_sta_get_rssi(int8_t *rssi);

int     wifi_softap_set_dtim_period(int period);

int     wifi_softap_deauth_all(void);
int     wifi_softap_deauth(uint8_t * mac_addr);
int     wifi_softap_set_max_supp_sta_num(uint8_t num);
int     wifi_softap_get_max_supp_sta_num(uint8_t *num);

int     wifi_softap_scan(wifi_scan_cfg_t *cfg, void *buff, int items, void (*cb)(void *arg));
void    wifi_softap_scan_results_get(ap_info_t **buff, int *items);

int     wifi_sta_reg_callback(sta_cb_t *sta_cb);
int     wifi_softap_reg_callback(ap_cb_t *ap_cb);

int     wifi_if_reg_recv_ethernet_pkt_callback(wifi_if_recv_cb_t recv_cb);
void    wifi_if_send_ethernet_pkt(uint8_t *data, uint16_t len);


//wifi sniffer API
int     wifi_sta_set_sniffer_cfg(sniffer_cfg_t *cfg, wifi_sniffer_cb_t cb);
int     wifi_sta_get_sniffer_cfg(sniffer_cfg_t *cfg);


//wifi advanced API
int     wifi_send_80211_mgmt_raw_frame(uint8_t *data, int len);
int     wifi_recv_ethernet_pkt_callback_reg(wifi_recv_ethernet_pkt_cb_t recv_cb);
int     wifi_send_ethernet_pkt(uint8_t *data, int len, uint16_t retry_max, uint8_t retry_timeout);
int     wifi_private_command(char *pvtcmd);

void    wifi_disable_sw_duration_cfg(void);
int     wifi_rts_threshold_set(int val);

/**
 * @param type
 *      0: G_SELF_CTS_PROT
 *      1: G_RTS_CTS_PROT
*/
int     wifi_erp_prot_set(uint8_t type);

/**
 * wifi_rf_calibration
 * 
 * Merge function wifi_rf_preprocess() and function wifi_rf_image_cal(),
 * and finally close RF!
 * RF will be turned on after mac phy initialization is complete.
*/
void    wifi_rf_calibration(void);
void    wifi_temp_cal_init(uint16_t adc_ch0_val,int8_t cap_comp);
void    wifi_do_temp_cal_period(uint16_t adc_ch0_val);

void ln_wifi_mgnt_cb_set(wifi_mgnt_cb_t cb);

// wifi lib version
uint32_t    wifi_lib_version_number_get(void);
const char *wifi_lib_version_string_get(void);
const char *wifi_lib_build_time_get(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __WIFI_H__ */
