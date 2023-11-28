#include "ln_softapcfg_port.h"
#include "ln_softapcfg_api.h"
#include "ln_softapcfg.h"

#define SOFTAPCFG_TASK_STACK_SIZE   (3 * 512)
static  OS_Thread_t      g_softapcfg_thread;
static  softapcfg_ctl_t  apcfg_ctl = {0};
static  product_param_t  product = {0};

static uint8_t     target_ssid[WLAN_SSID_MAX_LEN+1];
static uint8_t     target_pwd[WLAN_PWD_MAX_LEN];
static uint8_t     apcfg_token[TOKEN_MAX_LEN+1];
static uint8_t     aes_flag = LN_FALSE;

static int recv_proc(uint8_t * buf, uint16_t buf_len)
{
    int16_t len = 0;
    char * pos_start = (char *)buf;

    if ((buf_len > RECV_JSON_DATA_LEN_MIN) && \
        (buf_len < RECV_JSON_DATA_LEN_MAX))
    {
        /* tempelate {"cmdType":1,"ssid":"Home-WiFi","password":"12345678","token":"6ab82618a9d529a2ee777bf6e528a0fd","aes":"0"} */
        pos_start = strstr(pos_start,"\"ssid\"");
        if (pos_start > 0) {
            pos_start = strchr(pos_start,':');
            if (pos_start <= 0) {
                goto error_end;
            }
            pos_start += sizeof(char);

            pos_start = strchr(pos_start,'"');
            if (pos_start <= 0) {
                goto error_end;
            }
            pos_start += sizeof(char);

            len = strcspn(pos_start, "\"");
            if ((len < 1) || (len > WLAN_SSID_MAX_LEN)){
                goto error_end;
            }

            memset(target_ssid, '\0', sizeof(target_ssid));
            memcpy(target_ssid, pos_start, len);
            pos_start += len;
        } else {
            goto error_end;
        }

        pos_start = strstr(pos_start,"\"password\"");
        if (pos_start > 0) {
            pos_start = strchr(pos_start,':');
            if (pos_start <= 0) {
                goto error_end;
            }
            pos_start += sizeof(char);

            pos_start = strchr(pos_start,'"');
            if (pos_start <= 0) {
                goto error_end;
            }
            pos_start += sizeof(char);

            len = strcspn(pos_start, "\"");
            if (len >= WLAN_PWD_MAX_LEN){
                goto error_end;
            }

            memset(target_pwd, '\0', sizeof(target_pwd));
            memcpy(target_pwd, pos_start, len);
            pos_start += len;
        } else {
            goto error_end;
        }

        pos_start = strstr(pos_start,"\"token\"");
        if (pos_start > 0) {
            pos_start = strchr(pos_start,':');
            if (pos_start <= 0) {
                goto error_end;
            }
            pos_start += sizeof(char);

            pos_start = strchr(pos_start,'"');
            if (pos_start <= 0) {
                goto error_end;
            }
            pos_start += sizeof(char);

            len = strcspn(pos_start, "\"");
            if ((len < 1) || (len > TOKEN_MAX_LEN)){
                goto error_end;
            }

            memset(apcfg_token, '\0', sizeof(apcfg_token));
            memcpy(apcfg_token, pos_start, len);
            pos_start += len;
        } else {
            goto error_end;
        }

        pos_start = strstr(pos_start,"\"aes\"");
        if (pos_start > 0) {
            pos_start = strchr(pos_start,':');
            if (pos_start <= 0) {
                aes_flag = LN_FALSE;
            }
            pos_start += sizeof(char);

            pos_start = strchr(pos_start,'"');
            if (pos_start <= 0) {
                aes_flag = LN_FALSE;
            }
            pos_start += sizeof(char);

            if (*pos_start == '0') {
                aes_flag = LN_FALSE;
            } else if (*pos_start == '1') {
                aes_flag = LN_TRUE;
            } else {
                aes_flag = LN_FALSE;
            }
        } else {
            aes_flag = LN_FALSE;
        }
    }
    else
    {
        return LN_FALSE;
    }

    return LN_TRUE;

error_end:
    return LN_FALSE;
}

static void softapcfg_task_entry(void * param)
{
    int rcv_size = 0;
    uint8_t * rxtx_buf;

    struct sockaddr_in server_address;
    ip4_addr_t bind_addr;

    int socket_fd;
    fd_set read_set;
    fd_set write_set;
    struct timeval timeout;

    struct sockaddr from = {0};
    socklen_t fromlen = sizeof(struct sockaddr);

    if (NULL == (rxtx_buf = OS_Malloc(RXTX_BUF_SIZE))) {
        apcfg_log("malloc failed!\r\n");
        OS_ThreadDelete(&g_softapcfg_thread);
    }

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        OS_ThreadDelete(&g_softapcfg_thread);
    }

    ip4addr_aton((char *)param, &bind_addr);
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(SOFTAP_LOCAL_PORT);
    server_address.sin_addr.s_addr = bind_addr.addr;

    if (bind(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        close(socket_fd);
        OS_ThreadDelete(&g_softapcfg_thread);
    }

    while (1)
    {
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_SET(socket_fd, &read_set);
        FD_SET(socket_fd, &write_set);

        timeout.tv_sec  = 0;
        timeout.tv_usec = 50;

        if (0 < select(socket_fd+1, &read_set, &write_set, 0, &timeout))
        {
            if (FD_ISSET(socket_fd, &read_set))
            {
                if (LN_TRUE == apcfg_ctl.stop) {
                    break;
                }

                memset(rxtx_buf, 0, RXTX_BUF_SIZE);
                rcv_size = recvfrom(socket_fd, rxtx_buf, RXTX_BUF_SIZE, 0, &from, &fromlen);

                if (rcv_size < 0)
                {
                    apcfg_log("receive data error!\r\n");
                    continue;
                }
                else if (rcv_size > 0)
                {
                    apcfg_log("recv data: [len=%d,data:%s]\r\n", rcv_size, rxtx_buf);

                    if (LN_TRUE == recv_proc(rxtx_buf, rcv_size))
                    {
                        apcfg_log("\r\n\t------>target_ssid:[%s]\r\n\t------>target_pwd :[%s]\r\n\t------>apcfg_token:[%s]\r\n\t------>aes=%d\r\n", \
                                  target_ssid, target_pwd, apcfg_token, aes_flag);

                        memset(rxtx_buf, 0, RXTX_BUF_SIZE);

                        if (product.product_id && product.device_name && product.proto_ver) {
                            snprintf((char *)rxtx_buf, RXTX_BUF_SIZE, "{\"cmdType\":2,\"productId\":\"%s\",\"deviceName\":\"%s\",\"protoVersion\":\"%s\"}", \
                                                                         product.product_id, product.device_name, product.proto_ver);
                        } else {
                            rxtx_buf = (uint8_t *)"{\"cmdType\":2,\"productId\":\"id_xxx\",\"deviceName\":\"name_xxx\",\"protoVersion\":\"ver_xxx\"}";
                        }
                        uint16_t ack_len = strlen((char *)rxtx_buf);

                        apcfg_log("send ack: [len=%d,ack:%s]\r\n", ack_len, rxtx_buf);

                        for (uint8_t i = 0; i < 3; i++) {
                            sendto(socket_fd, rxtx_buf, ack_len, 0, &from, sizeof(struct sockaddr));
                            OS_MsDelay(60);
                        }

                        OS_Free(rxtx_buf);
                        apcfg_ctl.is_complete = LN_TRUE;
                        apcfg_log("softapcfg complete!\r\n");
                        break;
                    }
                }
            }
        }
        OS_MsDelay(150);
    }

    close(socket_fd);
    OS_ThreadDelete(&g_softapcfg_thread);
    return;
}

static int wifi_softap_startup(const char * ssid, const char * pwd, const char * local_ip)
{
    uint8_t macaddr[6] = {0};
    wifi_config_t wifi_config = {
        .ap = {
            .ssid            = {0},
            .ssid_len        = 0,
            .password        = {0},
            .channel         = 6,
            .authmode        = WIFI_AUTH_OPEN,
            .ssid_hidden     = 0,
            .max_connection  = 4,
            .beacon_interval = 100,
            .reserved        = 0,
        },
    };

    if ((ssid == NULL) || ((ssid != NULL) && (strlen(ssid) > WLAN_SSID_MAX_LEN))) {
        apcfg_log("wifi softap ssid invalid!\r\n");
        return LN_FALSE;
    }
    strcpy((char *)wifi_config.ap.ssid, ssid);
    wifi_config.ap.ssid_len = strlen(ssid);

    if (pwd == NULL) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    else {
        uint16_t pwdlen = strlen(pwd);
        if (pwdlen < 8) {
            apcfg_log("wifi softap pwd len < 8!\r\n");
            return LN_FALSE;
        }
        else if (pwdlen >= WLAN_PWD_MAX_LEN) {
            apcfg_log("wifi softap pwd len >= %d!\r\n", WLAN_PWD_MAX_LEN);
            return LN_FALSE;
        }
        else{
            strcpy((char *)wifi_config.ap.password, pwd);
            wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        }
    }

    wifi_init_type_t wifistart_init_param = {0};
    memset(&wifistart_init_param, 0, sizeof(wifi_init_type_t));
    wifistart_init_param.wifi_mode = WIFI_MODE_AP;
    wifistart_init_param.sta_ps_mode = WIFI_NO_POWERSAVE;
    wifistart_init_param.dhcp_mode = WLAN_DHCP_SERVER;

    strcpy(wifistart_init_param.local_ip_addr, local_ip);
    strcpy(wifistart_init_param.net_mask,"255.255.255.0");
    strcpy(wifistart_init_param.gateway_ip_addr, local_ip);

    ln_generate_random_mac(macaddr);
    wifi_set_macaddr_current(SOFT_AP_IF, macaddr);

    wifi_set_mode(WIFI_MODE_AP);

    wifi_set_config(SOFT_AP_IF, &wifi_config);

    if (0 != wifi_start(&wifistart_init_param, true)) {
        apcfg_log("wifi softap start failed!\r\n");
        return LN_FALSE;
    }

    return LN_TRUE;
}

int ln_softapcfg_set_product_param(const char * product_id, const char * device_name, const char * proto_ver)
{
    if (!product_id || !device_name || !proto_ver) {
        return LN_FALSE;
    }

    apcfg_log("product_id:[%s],device_name:[%s],proto_ver:[%s]\r\n",product_id, device_name, proto_ver);

    product.product_id  = product_id;
    product.device_name = device_name;
    product.proto_ver   = proto_ver;

    return LN_TRUE;
}

int ln_softapcfg_start(const char * ssid, const char * pwd, const char * local_ip)
{
    static const char * ip = NULL;

    apcfg_log("ssid:[%s],pwd:[%s],local ip:[%s]\r\n",ssid, pwd, local_ip);

    memset(target_ssid, 0, sizeof(target_ssid));
    memset(target_pwd,  0, sizeof(target_pwd));
    memset(apcfg_token, 0, sizeof(apcfg_token));

    apcfg_ctl.ssid  = target_ssid;
    apcfg_ctl.pwd   = target_pwd;
    apcfg_ctl.token = apcfg_token;
    apcfg_ctl.is_complete = LN_FALSE;
    apcfg_ctl.stop  = LN_FALSE;

    if (LN_TRUE != wifi_softap_startup(ssid, pwd, local_ip)) {
        return LN_FALSE;
    }

    ip = local_ip;
    if(OS_OK != OS_ThreadCreate(&g_softapcfg_thread, "softapcfg", softapcfg_task_entry, (void *)ip, OS_PRIORITY_BELOW_NORMAL,SOFTAPCFG_TASK_STACK_SIZE)) {
        return LN_FALSE;
    }

    return LN_TRUE;
}

void ln_softapcfg_stop(void)
{
    apcfg_ctl.stop = LN_TRUE;
    OS_MsDelay(250);
    wifi_stop();
}

int ln_softapcfg_is_complete(void)
{
    return apcfg_ctl.is_complete;
}

uint8_t * ln_softapcfg_get_ssid(void)
{
    return (uint8_t *)target_ssid;
}

uint8_t * ln_softapcfg_get_pwd(void)
{
    return (uint8_t *)target_pwd;
}

uint8_t * ln_softapcfg_get_token(void)
{
    return (uint8_t *)apcfg_token;
}
