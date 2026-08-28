#include "axk_http_server_scan.h"
#include "aiio_adapter_include.h"

#define WIFI_MGMR_SCAN_ITEMS (50)

static int32_t axk_scan_init(void);
static uint32_t axk_scan_get_ap_num(void);
static aiio_wifi_ap_item_t *axk_scan_get_ap_item(void);

axk_http_server_scan_t g_axk_http_server_scan = {
    .init = axk_scan_init,
    .get_ap_num = axk_scan_get_ap_num,
    .get_ap_item = axk_scan_get_ap_item,
};

static uint32_t ap_count = 0;
aiio_wifi_ap_item_t *ap_ary_p = NULL;

static void scan_item_cb(aiio_wifi_ap_item_t *env, uint32_t *param1, aiio_wifi_ap_item_t *item)
{
    uint32_t counter = (*param1);

    /*scan overflow check*/
    if (counter >= WIFI_MGMR_SCAN_ITEMS)
    {
        /*XXX maybe we should warning here*/
        return;
    }
    aiio_wifi_ap_item_t *ap_ary_ptr = (aiio_wifi_ap_item_t *)env + counter; // get and move the ptr to the next item
    counter++;

    /*copy out scan data*/
    ap_ary_ptr->channel = item->channel;
    ap_ary_ptr->rssi = item->rssi;
    memcpy(ap_ary_ptr->bssid, item->bssid, sizeof(ap_ary_ptr->bssid));
    memcpy(ap_ary_ptr->ssid, item->ssid, sizeof(ap_ary_ptr->ssid)); // XXX boundary attack, copy too much data from wifi manager internal?
    ap_ary_ptr->ssid_tail[0] = '\0';
    ap_ary_ptr->ssid_len = strlen((char *)ap_ary_ptr->ssid);
    ap_ary_ptr->auth = item->auth;
    ap_ary_ptr->cipher = item->cipher;

    // aiio_log_i("index[%02d]: channel %02u, bssid %02X:%02X:%02X:%02X:%02X:%02X, rssi %3d, auth %d, SSID %s",
    //            counter,
    //            ap_ary_ptr->channel,
    //            ap_ary_ptr->bssid[0],
    //            ap_ary_ptr->bssid[1],
    //            ap_ary_ptr->bssid[2],
    //            ap_ary_ptr->bssid[3],
    //            ap_ary_ptr->bssid[4],
    //            ap_ary_ptr->bssid[5],
    //            ap_ary_ptr->rssi,
    //            ap_ary_ptr->auth,
    //            ap_ary_ptr->ssid);
    /*store back counter*/
    (*param1) = counter;
}

static void cb_scan_complete(void *data, void *param)
{
    ap_count = 0;

    aiio_wifi_scan_ap_all(ap_ary_p, &ap_count, scan_item_cb);

    aiio_log_i("cb_scan_complete:%ld \r\n", ap_count);

    // if (ap_ary_p != NULL)
    // {
    //     aiio_os_free(ap_ary_p);
    //     ap_ary_p = NULL;
    // }
}

static void axk_scan_task(void *param)
{
    (void)(param);

    if (ap_ary_p == NULL)
    {
        ap_ary_p = (aiio_wifi_ap_item_t *)aiio_os_malloc(sizeof(aiio_wifi_ap_item_t) * WIFI_MGMR_SCAN_ITEMS);
        if (ap_ary_p == NULL)
        {
            aiio_log_e("malloc falied");
            goto __exit;
        }

        memset(ap_ary_p, 0, sizeof(aiio_wifi_ap_item_t) * WIFI_MGMR_SCAN_ITEMS);
    }

    // memset(ap_ary_p, 0, sizeof(aiio_wifi_ap_item_t) * WIFI_MGMR_SCAN_ITEMS);

    aiio_wifi_scan(ap_ary_p, cb_scan_complete, 1);
    
__exit:
    aiio_os_thread_delete(NULL);
}

static int32_t axk_scan_init(void)
{
    return aiio_os_thread_create(NULL, "scan", axk_scan_task, 2048, NULL, 13);
}

static uint32_t axk_scan_get_ap_num(void)
{
    return ap_count;
}

static aiio_wifi_ap_item_t *axk_scan_get_ap_item(void)
{
    return ap_ary_p;
}
