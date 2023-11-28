#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define EXAMPLE_AXK_WIFI_STA_SSID               CONFIG_AXK_WIFI_STA_SSID
#define EXAMPLE_AXK_WIFI_STA_PASS               CONFIG_AXK_WIFI_STA_PASS
#define EXAMPLE_AXK_WIFI_STA_CHANNEL            CONFIG_AXK_WIFI_STA_CHANNEL
#define EXAMPLE_AXK_WIFI_AP_SSID                CONFIG_AXK_WIFI_AP_SSID
#define EXAMPLE_AXK_WIFI_AP_PASS                CONFIG_AXK_WIFI_AP_PASS
#define EXAMPLE_AXK_WIFI_MAX_STA_CONN           CONFIG_AXK_WIFI_MAX_STA_CONN
#define EXAMPLE_AXK_WIFI_AP_CHANNEL             CONFIG_AXK_WIFI_AP_CHANNEL
#define EXAMPLE_AXK_WIFI_AP_IP                  CONFIG_AXK_WIFI_AP_IP
#ifdef CONFIG_AXK_WIFI_SSID_HIDDEN_ENABLE
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     1
#else
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     0
#endif


static aiio_os_thread_handle_t *aiio_test_ap_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_http_server_thread = NULL;
static aiio_httpd_handle_t http_server = NULL;

static int32_t ret=0;
static int32_t rssi=0;
static int32_t state=0;
static int32_t ap_stop_flag=0;
static uint8_t sta_num=0;
uint8_t mac[MAC_LEN];
aiio_wifi_mode_t cur_wifi_mode=AIIO_WIFI_MODE_NULL;
aiio_wifi_sta_basic_info_t sta_info_test[CONFIG_MAX_STA_CONN_LIMIT]={0};
aiio_wifi_ip_params_t ap_ip_pra={0};

static int http_404_error_handler(aiio_httpd_req_t *req, aiio_httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0)
    {
        aiio_httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return 0 to keep underlying socket open */
        return 0;
    }
    else if (strcmp("/echo", req->uri) == 0)
    {
        aiio_httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return -1 to close underlying socket */
        return -1;
    }
    /* For any other URI send 404 and close socket */
    aiio_httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return -1;
}

/* An HTTP GET handler */
static int hello_get_handler(aiio_httpd_req_t *req)
{
    char *buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = aiio_httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (aiio_httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == 0)
        {
            aiio_log_i("Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = aiio_httpd_req_get_hdr_value_len(req, "Accept-Language") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (aiio_httpd_req_get_hdr_value_str(req, "Accept-Language", buf, buf_len) == 0)
        {
            aiio_log_i("Found header => Accept-Language: %s", buf);
        }
        free(buf);
    }

    buf_len = aiio_httpd_req_get_hdr_value_len(req, "Connection") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (aiio_httpd_req_get_hdr_value_str(req, "Connection", buf, buf_len) == 0)
        {
            aiio_log_i("Found header => Connection: %s", buf);
        }
        free(buf);
    }

    /* Set some custom headers */
    aiio_httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    aiio_httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char *resp_str = (const char *)req->user_ctx;
    aiio_httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (aiio_httpd_req_get_hdr_value_len(req, "Host") == 0)
    {
        aiio_log_i("Request headers lost");
    }
    return 0;
}

static const aiio_httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

/* An HTTP POST handler */
static int echo_post_handler(aiio_httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = aiio_httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return -1;
        }

        /* Send back the same data */
        aiio_httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        aiio_log_i("=========== RECEIVED DATA ==========");
        aiio_log_i("%.*s", ret, buf);
        aiio_log_i("====================================");
    }

    // End response
    aiio_httpd_resp_send_chunk(req, NULL, 0);
    return 0;
}

static const aiio_httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
static int ctrl_put_handler(aiio_httpd_req_t *req)
{
    char buf;
    int ret;

    if ((ret = aiio_httpd_req_recv(req, &buf, 1)) <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            aiio_httpd_resp_send_408(req);
        }
        return -1;
    }

    if (buf == '0')
    {
        /* URI handlers can be unregistered using the uri string */
        aiio_log_i("Unregistering /hello and /echo URIs");
        aiio_httpd_unregister_uri(req->handle, "/hello");
        aiio_httpd_unregister_uri(req->handle, "/echo");
        /* Register the custom error handler */
        aiio_httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else
    {
        aiio_log_i("Registering /hello and /echo URIs");
        aiio_httpd_register_uri_handler(req->handle, &hello);
        aiio_httpd_register_uri_handler(req->handle, &echo);
        /* Unregister custom error handler */
        aiio_httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
    }

    /* Respond with empty body */
    aiio_httpd_resp_send(req, NULL, 0);
    return 0;
}

static const aiio_httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};

static aiio_httpd_handle_t start_webserver(void)
{
    aiio_httpd_handle_t server = NULL;
    aiio_httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.lru_purge_enable = true;

    // Start the httpd server
    aiio_log_i("Starting server on port: '%d'", config.server_port);
    if (aiio_httpd_start(&server, &config) == AIIO_OK) 
    {
        // Set URI handlers
        aiio_log_i("Registering URI handlers");
        aiio_httpd_register_uri_handler(server, &hello);
        aiio_httpd_register_uri_handler(server, &echo);
        aiio_httpd_register_uri_handler(server, &ctrl);
        return server;
    }

    aiio_log_i("Error starting server!");
    return NULL;
}

static void http_server_task(void *param)
{
    (void)(param);

    http_server = start_webserver();
    aiio_os_thread_delete(NULL);
}

static void cb_wifi_event(aiio_input_event_t *event, void* data)
{
    int32_t ret=0;
    // uint8_t bssid[6] = {0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5};
    switch (event->code) {
        case AIIO_WIFI_EVENT_WIFI_READY:
            aiio_log_d("<<<<<<<<<  WIFI INIT OK <<<<<<<<<<");
            break;
        case AIIO_WIFI_EVENT_SCAN_DONE:
            aiio_log_d("<<<<<<<<<  SCAN DONE OK <<<<<<<<<<");
            break;
        case AIIO_WIFI_EVENT_STA_CONNECTING:
            aiio_log_d("<<<<<<<<< STA_CONNECTING <<<<<<<<<<<");
            break;
        case AIIO_WIFI_EVENT_STA_CONNECTED:
            aiio_log_d("<<<<<<<<< CONNECTED<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_STA_DISCONNECTED:
            aiio_log_d("<<<<<<<<<  DISCONNECTED <<<<<<<<<<");
            if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE ||
                    event->value == AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE){
                //connect timeout
                aiio_log_d("connect timeout");
            }else if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE ||
                    event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE ||
                    event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION){
                //password error
                aiio_log_d("password error");
            }else if (event->value == AIIO_WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL){
                //not found AP
                aiio_log_d("not found AP");
            }else if ((event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION) || (event->value == AIIO_WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH)){
                //wifi disconnect
                aiio_log_d("wifi disconnect");
            }else{
                //connect error
                aiio_log_d("connect error");
            }
            break;
        case AIIO_WIFI_EVENT_STA_GOT_IP:
            aiio_log_d("<<<<<<<<< CONNECTED GOT IP <<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_START:
            aiio_log_d("<<<<<<<<< start soft ap OK<<<<<<<<<<<");
            aiio_os_thread_create(&aiio_http_server_thread, "http_server", http_server_task, 4096, NULL, 15);
            break;
        case AIIO_WIFI_EVENT_AP_STOP:
            aiio_log_d("<<<<<<<<< stop soft ap OK<<<<<<<<<<<");
    
            aiio_wifi_config_t test_wifi_ap_config;
        
            aiio_wifi_get_config(AIIO_WIFI_IF_AP,&test_wifi_ap_config);

            aiio_log_d("ap.ssid=%s",test_wifi_ap_config.ap.ssid);
            aiio_log_d("ap.password=%s",test_wifi_ap_config.ap.password);
            aiio_log_d("ap.max_connection=%d",test_wifi_ap_config.ap.max_connection);
            aiio_log_d("ap.channel=%d",test_wifi_ap_config.ap.channel);
            aiio_log_d("ap.ssid_hidden=%d",test_wifi_ap_config.ap.ssid_hidden);
            aiio_log_d("ap.ssid_len=%d",test_wifi_ap_config.ap.ssid_len);
            aiio_log_d("ap.use_dhcp=%d",test_wifi_ap_config.ap.use_dhcp);
            aiio_log_d("ap.dhcp_params.lease_time=%d",test_wifi_ap_config.ap.dhcp_params.lease_time);
            aiio_log_d("ap.dhcp_params.start=%d",test_wifi_ap_config.ap.dhcp_params.start);
            aiio_log_d("ap.dhcp_params.end=%d",test_wifi_ap_config.ap.dhcp_params.end);

            break;
        case AIIO_WIFI_EVENT_AP_STACONNECTED:
            aiio_log_d("<<<<<<<<< station connent ap <<<<<<<<<<<");
            aiio_wifi_ap_sta_list_get(&sta_num,&sta_info_test);
            aiio_log_d("wifi ap_sta_num =  %d!!",sta_num);
            for(uint8_t i=0; i<CONFIG_MAX_STA_CONN_LIMIT;i++){
                if(sta_info_test[i].rssi != 0){
                    aiio_log_d("wifi ap_sta_idx =  %u!!",sta_info_test[i].sta_idx);
                    aiio_log_d("wifi ap_sta_mac = %02x%02x%02x%02x%02x%02x!!",sta_info_test[i].sta_mac[0],sta_info_test[i].sta_mac[1],sta_info_test[i].sta_mac[2],sta_info_test[i].sta_mac[3],sta_info_test[i].sta_mac[4],sta_info_test[i].sta_mac[5]);
                    aiio_log_d("wifi ap_sta_rssi = %d !!",sta_info_test[i].rssi);
                }
            }

            break;
        case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
            aiio_log_d("<<<<<<<<< station disconnet ap <<<<<<<<<<<");
            ap_stop_flag=1;
            break;
        default:
            break;
    }
}

static void test_ap_wifi_entry(void *pvParameters)
{
    ret = aiio_wifi_ap_ip_set(EXAMPLE_AXK_WIFI_AP_IP);
    if (ret != 0)
    {
        aiio_log_e("wifi ap_ip_set error!!");
    }

    aiio_wifi_set_mode(AIIO_WIFI_MODE_AP);

    aiio_wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = EXAMPLE_AXK_WIFI_AP_SSID,
            .password = EXAMPLE_AXK_WIFI_AP_PASS,
            .channel = EXAMPLE_AXK_WIFI_AP_CHANNEL,
            .max_connection = EXAMPLE_AXK_WIFI_MAX_STA_CONN,
            .ssid_hidden = EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE,
            .ssid_len = strlen(EXAMPLE_AXK_WIFI_AP_SSID),
            .use_dhcp = 1,
            .dhcp_params.lease_time = 120,
            .dhcp_params.start = 2,
            .dhcp_params.end = 100,
        },
    };

    aiio_wifi_set_config(AIIO_WIFI_IF_AP, &wifi_ap_config);

    aiio_wifi_start();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
        if (ap_stop_flag == 1)
        {
            aiio_httpd_stop(http_server);
            aiio_log_i("http server stop");
            ap_stop_flag = 0;
        }
    }
}

void aiio_main(void *params){

    int32_t ret=0;

    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    aiio_wifi_register_event_cb(cb_wifi_event);
    ret=aiio_wifi_init();
    if(ret!=0){
        aiio_log_e("wifi init error!!");
    }

    aiio_os_thread_create(&aiio_test_ap_wifi_thread, "test_ap_wifi_entry", test_ap_wifi_entry, 4096, NULL, 15);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

