#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ln_types.h"
#include "utils/ln_list.h"
#include "wifi.h"

#define AP_INFO_LIFETIME    (8)//uints:second
#define AP_LIST_NODE_MAX    (40)

typedef enum 
{
    WIFI_MGR_EVENT_STA_STARTUP           = 0, /**<  STA startup */ 
    WIFI_MGR_EVENT_STA_CONNECTED         = 1, /**<  STA connected target AP */
    WIFI_MGR_EVENT_STA_DISCONNECTED      = 2, /**<  STA disconnected target AP */
    WIFI_MGR_EVENT_STA_SCAN_COMPLETE     = 3, /**<  STA scan complete */
    WIFI_MGR_EVENT_STA_CONNECT_FAILED    = 4, /**<  STA connect torget AP failed */

    WIFI_MGR_EVENT_SOFTAP_STARTUP        = 5, /**<  softAP startup */
    WIFI_MGR_EVENT_SOFTAP_ASSOCIATED     = 6, /**<  a station connected to softAP */
    WIFI_MGR_EVENT_SOFTAP_DISASSOCIATED  = 7, /**<  a station disconnected to softAP */
    WIFI_MGR_EVENT_SOFTAP_ASSOCIAT_FILED = 8, /**<  a station connected to softAP failed */
    WIFI_MGR_EVENT_MAX,  
} wifi_mgr_event_t;

typedef void (*wifi_mgr_event_cb_t)(void * arg);
 
typedef enum {
    NORMAL_SORT     = 0,
    RSSI_SORT       = 1,
} sort_rule_t;

typedef struct {
    ln_list_t       list;
    ap_info_t       info;
    uint32_t        life_ticks;
} ap_info_node_t;


int  wifi_manager_init(void);
int  wifi_manager_deinit(void);
int  wifi_manager_reg_event_callback(wifi_mgr_event_t event, wifi_mgr_event_cb_t cb);
int  wifi_manager_get_ap_list(ln_list_t **list, uint8_t * node_count);
int  wifi_manager_set_ap_list_sort_rule(sort_rule_t rule);
int  wifi_manager_ap_list_update_enable(int en);
void wifi_manager_cleanup_scan_results(void);


#ifdef __cplusplus
}
#endif

#endif /* __WIFI_MANAGER_H__ */


