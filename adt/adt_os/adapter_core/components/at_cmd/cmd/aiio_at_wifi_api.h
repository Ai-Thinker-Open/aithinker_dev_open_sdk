/** @brief      AT WIFI command api.
 *
 *  @file       aiio_at_wifi_api.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_WIFI_API_H__
#define __AIIO_AT_WIFI_API_H__
#include <stdint.h>
#include "aiio_type.h"


#define AIIO_AT_CONFIG_WIFI_SSID_SIZE   (64)
#define AIIO_AT_CONFIG_WIFI_SCAN_ITEMS_MAX   (50)


typedef void (*aiio_at_callbackfunc)(void *arg);
typedef void (*aiio_at_rxdata_callbackfunc)(uint16_t len,uint8_t *data);
typedef struct {
    //wifi
    aiio_at_callbackfunc wifiScanDoneCallback;
    aiio_at_callbackfunc wifiGetIPCallback;
    aiio_at_callbackfunc wifiConnectCallback;
    aiio_at_callbackfunc wifiDisconnectCallback; 
    aiio_at_rxdata_callbackfunc apClientConnectCallback;
    aiio_at_rxdata_callbackfunc apClientDisconnectCallback;
}aiio_at_callbacks_t;

typedef enum {
    AIIO_AT_UNDEFINED=-1,
    AIIO_AT_DISABLE=0,
    AIIO_AT_ENABLE=1,
}aiio_at_enable_t;

typedef enum{
    AIIO_AT_WIFI_DISABLE=0, //WIFI has not been initialized or failed to be initialized
    AIIO_AT_WIFI_STA_MODE=1,    //station
    AIIO_AT_WIFI_AP_MODE=2, //AP
    AIIO_AT_WIFI_AP_STA_MODE=3, //AP+station
}aiio_at_wifi_mode_t;

//wifi国家码
typedef enum{
    AIIO_AT_WIFI_COUNTRY_NOT_SET=0, //No Settings, use the SDK default Settings
    AIIO_AT_WIFI_COUNTRY_JP=1,  //Japan
    AIIO_AT_WIFI_COUNTRY_AS=2,  //American Samoa
    AIIO_AT_WIFI_COUNTRY_CA=3,  //Canada
    AIIO_AT_WIFI_COUNTRY_US=4,  //American
    AIIO_AT_WIFI_COUNTRY_CN=5,  //China
    AIIO_AT_WIFI_COUNTRY_HK=6,  //Hong Kong, China
    AIIO_AT_WIFI_COUNTRY_TW=7,  //Taiwan of China
    AIIO_AT_WIFI_COUNTRY_MO=8,  //Macau, China
    AIIO_AT_WIFI_COUNTRY_IL=9,  //Israel
    AIIO_AT_WIFI_COUNTRY_SG=10, //Singapore
    AIIO_AT_WIFI_COUNTRY_KR=11, //South Korea
    AIIO_AT_WIFI_COUNTRY_TR=12, //Turkey
    AIIO_AT_WIFI_COUNTRY_AU=13, //Australia
    AIIO_AT_WIFI_COUNTRY_ZA=14, //South Africa
    AIIO_AT_WIFI_COUNTRY_BR=15, //Brazil
    AIIO_AT_WIFI_COUNTRY_EU=16, //European Union

    AIIO_AT_WIFI_COUNTRY_MAX
}aiio_at_wifi_countrycode_t;

//Connection status (Suitable for description of connection status of wifi, tcp, mqtt etc.)
typedef enum {
    AIIO_AT_CONNECT_STATUS_DISCONNECT=0,    //No connection, or disconnected
    AIIO_AT_CONNECT_STATUS_CONNECTING=1,    //Connecting or reconnecting
    AIIO_AT_WIFI_STATUS_CONNECT_NO_IP=2,    //The wifi connection is successful, but the IP is not obtained (this is the wifi exclusive state).
    AIIO_AT_CONNECT_STATUS_CONNECTED=3, //Successful connection (indicates that the wifi sta state indicates that the IP(DHCP) is successfully obtained at the same time, or static IP is used)
    AIIO_AT_CONNECT_STATUS_CONNECT_FAIL=4,  //Connection failure (the state of not being successfully connected after the number of reconnections has been exceeded)
    AIIO_AT_CONNECT_STATUS_CONNECT_TIMEOUT=5,   //Connection timeout
    AIIO_AT_CONNECT_STATUS_CONNECT_PW_ERROR=6,  //Password error
    AIIO_AT_CONNECT_STATUS_CONNECT_NOT_FOUND_AP=7,  //No AP was found.
}aiio_at_connect_status_t;

typedef enum {
    //the state is only meaningful in saveConfigInFlash (the state saved to flash)
    AIIO_AT_CONNECT_STATUS_AUTO_CON_ON=1,       //Automatically connect to wifi(sta or AP automatically open)
    AIIO_AT_CONNECT_STATUS_AUTO_CON_OFF=0,    //Disable automatic connection to wifi or AP upon startup
}aiio_at_auto_connect_status_t;

//DHCP information in STA mode
typedef struct {
    aiio_at_enable_t usedhcp;   //Whether to use DHCP
    //The following parameters are when DHCP is disabled in STA mode, static IP addresses are used
    char ip[16];    //( eg:192.168.1.100)
    char mask[16];  //( eg:255.255.255.0)
    char gateway[16];   //( eg:192.168.1.1)
}aiio_at_sta_dhcpinfo_t;

//DHCP information in AP mode
typedef struct {
    aiio_at_enable_t usedhcp;  //Whether to use DHCP
    //The following parameters are the parameters of DHCP in AP mode
    char startip[16];   //( eg:192.168.1.100)
    char endip[16]; //( eg:192.168.1.200)
    char gateway[16];   //( eg:192.168.1.1)
}aiio_at_ap_dhcpinfo_t;


typedef enum {
    AIIO_AT_SECURITY_UNKNOWN,   //Unknown encryption method(UnknownType)
    AIIO_AT_SECURITY_OPEN,  //Open network, no password(Open)
    AIIO_AT_SECURITY_WEP_PSK,                   //WEP PSK
    AIIO_AT_SECURITY_WEP_SHARED,                    //WEP SHARED
    AIIO_AT_SECURITY_WPS,                       //WPS
    AIIO_AT_SECURITY_WPA_TKIP_PSK,              //WPA TKIP
    AIIO_AT_SECURITY_WPA_TKIP_Enterprise,   //WPA TKIP Enterprise
    AIIO_AT_SECURITY_WPA_WPA2_TKIP_PSK,         //WPA/WPA2 TKIP
    AIIO_AT_SECURITY_WPA_WPA2_TKIP_Enterprise,  //WPA/WPA2 TKIP Enterprise
    AIIO_AT_SECURITY_WPA_WPA2_AES_PSK,          //WPA/WPA2 AES
    AIIO_AT_SECURITY_WPA_WPA2_AES_Enterprise,   //WPA/WPA2 AES Enterprise
    AIIO_AT_SECURITY_WPA_WPA2_Mixed_PSK,            //WPA/WPA2 Mixed
    AIIO_AT_SECURITY_WPA_WPA2_Enterprise,       //WPA/WPA2 Enterprise
    AIIO_AT_SECURITY_WPA_AES_PSK,               //WPA AES
    AIIO_AT_SECURITY_WPA_AES_Enterprise,            //WPA AES Enterprise
    AIIO_AT_SECURITY_WPA_Mixed_PSK,             //WPA Mixed
    AIIO_AT_SECURITY_WPA_Mixed_Enterprise,      //WPA Mixed Enterprise
    AIIO_AT_SECURITY_WPA2_Enterprise,           //WPA2 Enterprise
    AIIO_AT_SECURITY_WPA2_Mixed_PSK,                //WPA2 Mixed
    AIIO_AT_SECURITY_WPA2_WPA3_Mixed,           //WPA2 WPA3 Mixed
    AIIO_AT_SECURITY_WPA2_AES_PSK,              //WPA2 AES
    AIIO_AT_SECURITY_WPA2_AES_Enterprise,       //WPA2 AES Enterprise
    AIIO_AT_SECURITY_WPA2_AES_CMAC,             //WPA2 AES CMAC
    AIIO_AT_SECURITY_WPA2_TKIP_PSK,             //WPA2 TKIP
    AIIO_AT_SECURITY_WPA2_TKIP_Enterprise,      //WPA2 TKIP Enterprise
    AIIO_AT_SECURITY_WPA2_PSK_WPA3_SAE,         //WPA2 PSK/WPA3 SAE
    AIIO_AT_SECURITY_WPA3_ASE_AES_PSK,          //WPA3-ASE AES
}aiio_at_wifi_security_t;



//Get the wifi information structure
typedef struct {
    aiio_at_wifi_mode_t wifimode;   //wifi mode(This is used to record whether the wifi information or STA information is stored)
    char ssid[AIIO_AT_CONFIG_WIFI_SSID_SIZE];   /*  STA mode indicates the name of the target wifi.
                                                    AP mode Indicates the name of the hotspot that you create;
                                                */
    char passwd[AIIO_AT_CONFIG_WIFI_SSID_SIZE]; /*  STA mode represents the password of target wifi;
                                                    AP mode Indicates the password of the hotspot that you create;
                                                */
    uint8_t stabssid[6];    //Only STA mode effectively represents the mac address of the target wifi
    aiio_at_wifi_security_t security;   /*  STA mode indicates the target wifi encryption mode;
                                            AP mode Indicates the encryption mode of the hotspot created by you;
                                        */
    int32_t channel;
    uint8_t mac[6];
    char ip[16];
    char gateway[16];
    char mask[16];
}aiio_at_wifi_info_t;


//Record wifi status
typedef struct {
    aiio_at_wifi_mode_t wifimode;
    aiio_at_wifi_countrycode_t wificountry;
    //STA params
    aiio_at_connect_status_t wifistatus;
    aiio_at_auto_connect_status_t auto_conn_status;
    char ssid[AIIO_AT_CONFIG_WIFI_SSID_SIZE];
    char stapasswd[AIIO_AT_CONFIG_WIFI_SSID_SIZE];
    uint8_t stabssid[6];    //Save the mac that you want to connect to wifi. All zeros indicate no Settings
    int32_t sta_channel;
    aiio_at_sta_dhcpinfo_t stadhcpinfo;
    //AP params
    char apname[AIIO_AT_CONFIG_WIFI_SSID_SIZE];
    char appasswd[AIIO_AT_CONFIG_WIFI_SSID_SIZE];
    uint8_t apmaxconn;  //Maximum number of AP connections allowed
    int32_t aphidden;   //Whether to hide AP
    int32_t apchannel;  //AP channels (2.4G:1-14 channels; 5G:36-165 channels)
    aiio_at_ap_dhcpinfo_t apdhcpinfo;
}aiio_at_wifi_config_t;

typedef struct
{
#define WIFI_SCAN_FITER_BIT_SSID    0x01
#define WIFI_SCAN_FITER_BIT_MAC     0x02
#define WIFI_SCAN_FITER_BIT_CH      0x04
#define WIFI_SCAN_FITER_BIT_RSSI    0x08
    uint8_t fiter_button_bit;
    uint8_t ssid[33];
    uint8_t mac[6];
    uint8_t channel;
    int8_t rssi;
} aiio_at_wifiscan_fiter_t;

extern aiio_at_wifi_config_t at_wifi_config;





CORE_API uint8_t aiio_at_wifi_init(void);
CORE_API uint8_t aiio_at_wifi_country_codecheck(aiio_at_wifi_countrycode_t countrycode);
CORE_API uint8_t aiio_at_scanwifi(void);
CORE_API uint8_t aiio_at_wifi_connect(aiio_at_wifi_mode_t wifimode,aiio_at_sta_dhcpinfo_t *psta_dhcpinfo,char* ssid,char* passwd,uint8_t* pbssid);
CORE_API uint8_t aiio_at_wifi_start_ap(aiio_at_wifi_mode_t wifimode,aiio_at_ap_dhcpinfo_t *pap_dhcpinfo,char* apname,char* passwd,uint8_t apchannel,uint8_t maxconn,uint8_t hiddenssid);
CORE_API uint8_t aiio_at_wifi_get_sta_info(aiio_at_wifi_info_t* p_wifiinfo);
CORE_API uint8_t aiio_at_wifi_get_ap_info(aiio_at_wifi_info_t* p_wifiinfo);
CORE_API uint8_t aiio_at_wifi_get_apclient_list(uint8_t *p_aplistcount,uint8_t *p_aplistarray);
CORE_API uint8_t aiio_at_wifi_get_wificonfig(void);
CORE_API uint8_t aiio_at_wifi_set_wificonfig(uint8_t mode);
CORE_API uint8_t aiio_at_wifi_set_wificonfigname(uint8_t mode,char *wificonfigname);
CORE_API uint8_t aiio_at_wifi_get_wifi_rssi(int32_t *prssi);
CORE_API char*aiio_at_get_wifimac(void);
CORE_API char*aiio_at_get_wifi_security_str(aiio_at_wifi_security_t securitytype);

CORE_API uint8_t aiio_wifi_auto_connect_onboot(void);

#endif

