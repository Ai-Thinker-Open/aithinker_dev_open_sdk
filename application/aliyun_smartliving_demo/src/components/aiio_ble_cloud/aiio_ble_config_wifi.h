#ifndef __AIIO_BLE_CONFIG_WIFI_H_
#define __AIIO_BLE_CONFIG_WIFI_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"

#define MSG_REQ_ID_LEN                       (16)
#define TOPIC_LEN_MAX                        (128)
#define AWSS_REPORT_LEN_MAX       (256)

#define ILOP_VER                             "1.0"
#define TOPIC_MATCH_REPORT                   "/sys/%s/%s/thing/awss/enrollee/match"

#define METHOD_DEV_INFO_NOTIFY               "device.info.notify"
#define METHOD_AWSS_DEV_INFO_NOTIFY          "awss.device.info.notify"
#define METHOD_AWSS_CONNECTAP_NOTIFY         "awss.event.connectap.notify"
#define METHOD_AWSS_DEV_AP_SWITCHAP          "awss.device.softap.switchap"
#define METHOD_EVENT_ZC_SWITCHAP             "thing.awss.device.switchap"
#define METHOD_EVENT_ZC_ENROLLEE             "thing.awss.enrollee.found"
#define METHOD_EVENT_ZC_CHECKIN              "thing.awss.enrollee.checkin"
#define METHOD_EVENT_ZC_CIPHER               "thing.cipher.get"
#define METHOD_MATCH_REPORT                  "thing.awss.enrollee.match"
#define METHOD_LOG_POST                      "things.log.post"

#define AWSS_ACK_FMT                         "{\"id\":%s,\"code\":%d,\"data\":%s}"
#define AWSS_REQ_FMT                         "{\"id\":%s,\"version\":\"%s\",\"method\":\"%s\",\"params\":%s}"
#define AWSS_JSON_PARAM                      "params"
#define AWSS_JSON_CODE                       "code"
#define AWSS_JSON_ID                         "id"
#define AWSS_JSON_EXT                        "ext"
#define AWSS_JSON_TOKEN                      "token"
#define AWSS_JSON_TOKEN_TYPE                 "tokenType"
#define AWSS_STATIS_FMT                      "{\"template\":\"timestamp logLevel module traceContext logContent\",\"contents\":[\"%u %s %s %u %s\"]}"


enum {
    AWSS_CMP_PKT_TYPE_REQ = 1,
    AWSS_CMP_PKT_TYPE_RSP,
};

int combo_net_init();
void combo_ap_conn_notify(void);
void combo_token_report_notify(void);
int awss_report_token_to_cloud(void);

#endif