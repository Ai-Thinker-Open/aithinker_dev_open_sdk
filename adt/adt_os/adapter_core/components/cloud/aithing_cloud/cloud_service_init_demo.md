### cloud service初始化配置(当WiFi已连接后，便可开始调用处理service的初始化)

```c
static void aiio_user_service_init(void)
{
    char clientid[65] = "y1WVSovrEcw6fH";				
    char productid[33] = "PK8Y2yRG";
    char user_name[65] = "MeS0LG";
    char user_passwd[65] = "Lijwa37r2QHLct";
    char fwVer[10] = "1.2.0";
    char mcuVer[10] = "1.0.0";
    char devType[10] = "bl602";
    char mac[20] = {0};
    char uhost[33] = "120.77.96.118";
    char ussid[10] = {0};
    char ubssid[10] = {0};
    char uip[10] = {0};
    char umask[10] = {0};
    char ugw[10] = {0};
    aiio_service_config_t  service_config = {0};
    aiio_wifi_info_t wifi_info = {0};

    aiio_wifi_sta_mac_get((uint8_t *)mac);
    aiio_log_i("wifi mac = %02x%02x%02x%02x%02x%02x!!",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    snprintf(mac, 20, "%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    aiio_log_i("mac: %s\r\n", mac);

    service_config.service_data.deviceid = clientid;
    service_config.service_data.productid = productid;
    service_config.service_data.user_name = user_name;
    service_config.service_data.passwd = user_passwd;
    service_config.dev_info.fwVer = fwVer;
    service_config.dev_info.mcuVer = mcuVer;
    service_config.dev_info.devType = devType;
    service_config.dev_info.mac = mac;

    wifi_info.ssid = ussid;
    wifi_info.bssid = ubssid;
    wifi_info.ip = uip;
    wifi_info.mask = umask;
    wifi_info.gw = ugw;
    memcpy(&service_config.wifi_info, &wifi_info, sizeof(aiio_wifi_info_t));
    service_config.service_netif.host = uhost;
    service_config.service_netif.port = 1883;
    service_config.service_netif.keepalive = 50;
    service_config.service_netif.transport = MQTT_TRANSPORT_OVER_SSL;
    service_config.activity = true;
    service_config.rev_service_data_cb = aiio_service_receive_data;

    aiio_service_init(&service_config);
}

```



#### 回调接口定义

```c
static void aiio_service_receive_data(aiio_service_rev_event_t *rev_event)
{
    if(rev_event == NULL)
    {
        aiio_log_i("receive event is NULL \r\n");
        return ;
    }

    if(rev_event->data)
    {
        aiio_log_i("receive data: %s \r\n", rev_event->data);
    }

    switch (rev_event->service_event)
    {
        case AIIO_SERVICE_LANCH_FAIL_EVNET:
        {
            aiio_log_i("AIIO_SERVICE_LANCH_FAIL_EVNET \r\n");
        }
        break;

        case AIIO_SERVICE_SUBSCRIBE_TIMEOUT:
        {
            aiio_log_i("AIIO_SERVICE_SUBSCRIBE_TIMEOUT \r\n");
        }
        break;

        case AIIO_SERVICE_MQTT_DISCONNECT:
        {
            aiio_log_i("AIIO_SERVICE_MQTT_DISCONNECT \r\n");
        }
        break;

        case AIIO_SERVICE_ACTIVITY_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_ACTIVITY_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_RESTORE_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_RESTORE_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_REBOOT_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_REBOOT_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_INFO_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_INFO_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_CONFIG_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_CONFIG_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_UPGRADE_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_UPGRADE_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_QUERY_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_QUERY_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_CONTROL_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_CONTROL_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_UTC_EVENT:
        {
            aiio_log_i("AIIO_SERVICE_UTC_EVENT \r\n");
            if(rev_event->data)
            {
                aiio_log_i("receive data: %s \r\n", rev_event->data);
                aiio_online_update_local_time(rev_event->data, aiio_strlen(rev_event->data));
            }
        }
        break;
        
        default:
            aiio_log_i("event id err \r\n");
            break;
    }
}
```

