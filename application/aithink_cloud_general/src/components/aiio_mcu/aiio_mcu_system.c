#include "aiio_adapter_include.h"
#include "aiio_mcu_system.h"
#include "aiio_mcu_protocol.h"
#include "aiio_uart.h"
#include "aiio_log.h"
#include "aiio_wan_service.h"
#include "aiio_common.h"
#include "aiio_os_port.h"
#include "aiio_cloud_control.h"

#define     MCU_DATA_QUEUE_NUM_MAX                       (255)            /**< 最大的发送数据队列 */

#define     MCU_UPDATE_INFO_LEN                          300

#define             STR_PID                 "pid"
#define             STR_VER                 "ver"
#define             STR_FLAG                "flag"
#define             STR_MD5                 "md5"

#define portMAX_DELAY (TickType_t)0xffffffffUL

typedef struct
{
    uint8_t cmd_id;
    uint8_t cmd_type;
    uint16_t data_len;
    uint8_t *data;
}cmd_data_structure_t;


typedef struct
{
    uint8_t data_seq;
    bool    is_multiple_data;
    cmd_data_structure_t cmd_data;
}data_list_node_t;


/* 串口发送数据缓存 */
static uint8_t sedBuff[CONFIG_UART1_RECEIVE_BUFFER_MAX_LEN] = {0};
/* 串口接收数据缓存 */
static uint8_t revBuff[CONFIG_UART1_RECEIVE_BUFFER_MAX_LEN] = {0};
static uint8_t processBuff[CONFIG_UART1_RECEIVE_BUFFER_MAX_LEN] = {0};  
static bool     mcuConnectFlag = false;
static uint8_t  WifiNetworkState = WIFI_NOT_CONNECTED;
static char   productid[65] = {0};
static char   wifiFlag[65] = {0};
static char   mcuVer[10] = {0};
static char   wifiSsid[33] = {0};
static char   wifiPasswd[65] = {0};
static bool  TestWifiIsStart = false;
static bool  DeviceIsStart = false;
static bool  McuDeviceIsStart = false;
static uint8_t  SendHeatBeatCount = 0;
uint8_t  *revBuffPt_in = NULL;
static uint8_t  *revBuffPt_out = NULL;
static aiioList_t CmdDataList = {0};
static aiio_os_mutex_handle_t metex = NULL;
static profile_info_t profile_info = {0};
static uint8_t  McuOtaStatus = OTA_STATUS_INVAIL;


static int32_t MutexLock(void)
{
    int ret = 0;
    // return 0;
    if(!metex)
    {
        ret = aiio_os_mutex_create(&metex);
        
        if(ret != 0)
        {
            aiio_log_e("mutex create failed \r\n");
            return -1;
        }
    }
    return aiio_os_mutex_get(metex,portMAX_DELAY);
}


static int32_t MutexUnLock(void)
{
    int ret = 0;
    // return 0;
    if(!metex)
    {
        aiio_os_mutex_create(&metex);
        if(ret != 0)
        {
            aiio_log_e("mutex create failed \r\n");
            return -1;
        }
    }

    return aiio_os_mutex_put(metex);
}

bool McuCmdDataListInit(void)
{
    if (!aiio_ListInit(&CmdDataList, MCU_DATA_QUEUE_NUM_MAX))
    {
        aiio_log_e("cmd data list init err");
        return false;
    }
    else
    {
        return true;
    }
}


uint8_t UartCheckValue(uint8_t *dataBuff, uint16_t dataLen)
{
    uint8_t sum = 0;
    uint16_t n = 0;

    for(n = 0; n < dataLen; n++)
    {
        sum += dataBuff[n];
    }
    return sum;
}


static uint16_t UartProFramePacket(uint8_t *packetBuff, uint8_t cmdType, uint8_t *data, uint16_t dataLen)
{
    uint16_t packetLen = 0;

    if(packetBuff == NULL)
    {
        aiio_log_e("param err \r\n");
        return 0;
    }
    packetBuff[HEAD_H] = FRAME_H;
    packetBuff[HEAD_L] = FRAME_L;
    packetBuff[PRO_VER] = SED_VER;
    packetBuff[CMD_TYPE] = cmdType;

    packetLen = dataLen + FRAME_MINI_LEN;  
    packetBuff[LEN_H] = ((dataLen & 0xff00) >> 8);
    packetBuff[LEN_L] =  (dataLen & 0xff);

    if(packetLen > CONFIG_UART1_RECEIVE_BUFFER_MAX_LEN)
    {
        aiio_log_e("data len is too big\r\n");
        return 0;
    }

    if(data != NULL)
    {
        memcpy(&packetBuff[DATA_LOCAL], data, dataLen);
    }
    packetBuff[packetLen - 1] = UartCheckValue(packetBuff, packetLen - 1);

    return packetLen;
}


int8_t aiio_send_mcu_cmd(uint8_t cmdType, uint8_t *data, uint16_t dataLen)
{
    uint16_t len = 0;
    int8_t   ret = 0;

    len = UartProFramePacket(sedBuff, cmdType, data, dataLen);
    if(len == 0)
    {
        aiio_log_e("data packet err \r\n");
        return -1;
    }

    MutexLock();
    ret = aiio_uart_send_data(AIIO_UART1, sedBuff, len);
    MutexUnLock();

    memset(sedBuff, 0, CONFIG_UART1_RECEIVE_BUFFER_MAX_LEN);

    return ret;
}


bool GetMCUConnectState(void)
{
    return mcuConnectFlag;
}


int8_t aiio_control_send_mcu(uint8_t *data, uint16_t dataLen)
{
    return aiio_send_mcu_cmd(DATA_QUERT_CMD, data, dataLen);
}


int8_t UartProtocolInit(void)
{
    memset(revBuff, 0, sizeof(revBuff));

    revBuffPt_in = (uint8_t *)revBuff;
    revBuffPt_out = (uint8_t *)revBuff;

    return 0;
}

int8_t ProcessReportDataInfo(uint8_t *data, uint16_t data_len)
{
    data_list_node_t  data_list_node = {0};
    cmd_data_structure_t cmd_data = {0};
    ListNode_t new_node = {0};
    uint16_t node_num = 0;
    
    if(data == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }

    MutexLock();
    node_num = aiio_ListGetNodeNum(&CmdDataList);
    // MCU_InfoDebug("node_num = %d \r\n", node_num);

    cmd_data.cmd_id = data[0];
    cmd_data.cmd_type = data[1];
    cmd_data.data_len = ((data[2] << 8) | data[3]);

    // MCU_InfoDebug("cmd_id = %d \r\n", cmd_data.cmd_id);
    // MCU_InfoDebug("cmd_type = %d \r\n", cmd_data.cmd_type);
    // MCU_InfoDebug("cmd_len = %d \r\n", cmd_data.data_len);
    cmd_data.data = malloc(cmd_data.data_len);
    if(cmd_data.data == NULL)
    {
        aiio_log_e("malloc err \r\n");
        MutexUnLock();
        return -2;
    }
    memset((char *)cmd_data.data, 0, cmd_data.data_len);
    memcpy((char *)cmd_data.data, (char *)&data[4], cmd_data.data_len);

    memcpy(&data_list_node.cmd_data, &cmd_data, sizeof(cmd_data_structure_t));

    new_node.size = sizeof(data_list_node_t);
    new_node.pbase = (uint8_t *)&data_list_node;

    if (!aiio_ListAddNode(&CmdDataList, &new_node))
    {
        aiio_log_e("add timer to list err");
        MutexUnLock();
        return -3;
    }
    MutexUnLock();
    return 0;
}

static bool CheckDpidIsSame(uint8_t num, uint8_t pulse, char *value, uint8_t value_len)
{
    uint8_t len = 0;

    if(value == NULL)
    {
        aiio_log_e("param err \r\n");
        return 0;
    }

    aiio_log_d("num = %d \r\n", num);
    aiio_log_d("value[%d] = %d \r\n", num, value[num]);
    for(uint8_t n = 0; n < value_len; n += pulse)
    {
        if(value[num] != 0 && value[num] == value[n] && num != n)
        {
            return true;
        }
    }

    return false;
}


static int32_t McuGetBoolValue(uint8_t *data,bool *value)
{
    if(data == NULL)
    {
        return -1;
    }

    if(data[0])
    {
        *value = true;
    }
    else
    {
        *value = false;
    }

    return 0;
}

static int32_t McuGetEnumValue(uint8_t *data, uint8_t *value)
{
    if(data == NULL)
    {
        return -1;
    }

    *value = *data;

    return 0;
}

static int32_t McuGetIntValue(uint8_t *data, uint32_t *value)
{
    if(data == NULL)
    {
        return -1;
    }

    *value = (*data << 24) | (*(data + 1) << 16) | (*(data + 2) << 8) | (*(data + 3));

    return 0;
}


void McuMultipleCmdIDCheck(void)
{
    ListNode_t *node_A = NULL;
    ListNode_t *node_B = NULL;
    data_list_node_t *data_list_node_A = NULL;
    data_list_node_t *data_list_node_B = NULL;
    bool  ClearMultipleStatus = false;
    uint16_t node_num = 0;

    node_num = aiio_ListGetNodeNum(&CmdDataList);
    aiio_log_d("node_num = %d \r\n", node_num);
    for(uint8_t n = 0; n < node_num; n++)
    {
        node_A = aiio_ListGetNodeFromIndex(&CmdDataList, n);
        if(!node_A)                 continue;

        data_list_node_A = (data_list_node_t *)node_A->pbase;
        if(!data_list_node_A)         continue;

        if(data_list_node_A->is_multiple_data)
        {
            ClearMultipleStatus = true;
        }

        for(uint8_t m = 0; m < node_num; m++)
        {
            node_B = aiio_ListGetNodeFromIndex(&CmdDataList, m);
            if(!node_B)             continue;

            data_list_node_B = (data_list_node_t *)node_B->pbase;
            if(!data_list_node_B)         continue;

            if(n != m && data_list_node_A->cmd_data.cmd_id == data_list_node_B->cmd_data.cmd_id)
            {
                data_list_node_A->is_multiple_data = false;
                data_list_node_B->is_multiple_data = true;
                ClearMultipleStatus = false;
                aiio_log_e("cmd id[%d] set multiple flag \r\n", data_list_node_B->cmd_data.cmd_id);
            }
        }

        if(data_list_node_A->is_multiple_data && ClearMultipleStatus)
        {
            data_list_node_A->is_multiple_data = false;
            ClearMultipleStatus = false;
            aiio_log_d("cmd id[%d] clear multiple flag \r\n", data_list_node_A->cmd_data.cmd_id);
        }
        
    }
}


bool CheckMcuDeviceStart(void)
{
    return McuDeviceIsStart;
}


void UartSendHeatBeat(void)
{
    aiio_send_mcu_cmd(HEART_BEAT_CMD, NULL, 0);
    if(mcuConnectFlag)
    {
        SendHeatBeatCount++;
        aiio_log_d("SendHeatBeatCount = %d \r\n", SendHeatBeatCount);
    }
}


void McuPubliReportAttributes(void)
{
    cJSON *json_control = NULL;
    char numBuff[10] = {0};
    int32_t ret = 1;
    uint8_t cmd_id = 0;
    uint16_t node_num = 0;
    ListNode_t *node = NULL;
    data_list_node_t *data_list_node = NULL;
    bool bool_attribute = false;
    uint32_t int_attribute = 0;
    uint8_t  enum_attribute = 0;
    char *string_attribute = NULL;
    char  *json_str = NULL;

    // if(AilinkOtaIsStart())
    // {
    //     MCU_ErrorLog("ota starting, other function invaild \r\n");
    //     goto exit;
        
    // }
    node_num = aiio_ListGetNodeNum(&CmdDataList);
    aiio_log_i("node_num = %d \r\n", node_num);
    while(aiio_ListGetNodeNum(&CmdDataList) > 0)
    {

        json_control = cJSON_CreateObject();
        if (!json_control)
        {
            aiio_log_e("json create object fail \r\n");
            goto exit;
        }

        McuMultipleCmdIDCheck();
        for(uint8_t j = 0; j < node_num; )
        {
            node = aiio_ListGetNodeFromIndex(&CmdDataList, j);
            if(!node)                   continue;
            if(j == node_num)           break;

            data_list_node = (data_list_node_t *)node->pbase;
            if(data_list_node != NULL)
            {
                aiio_log_i("cmd_id = %d \r\n", data_list_node->cmd_data.cmd_id);

                if(data_list_node->is_multiple_data)
                {
                    j++;
                    continue;
                }

                if(data_list_node->cmd_data.cmd_type == DATA_TYPE_BOOL)
                {
                    aiio_log_i("bool_attribute = %d \r\n", bool_attribute);
                    McuGetBoolValue(data_list_node->cmd_data.data, (bool *)&bool_attribute);

                    snprintf(numBuff, sizeof(numBuff), "%d", data_list_node->cmd_data.cmd_id);
                    
                    if(bool_attribute)
                    {
                        cJSON_AddTrueToObject(json_control, numBuff);
                    }
                    else
                    {
                        cJSON_AddFalseToObject(json_control, numBuff);
                    }

                }     
                else if(data_list_node->cmd_data.cmd_type == DATA_TYPE_VALUE)
                {
                    McuGetIntValue( data_list_node->cmd_data.data, (uint32_t *)&int_attribute);
                    aiio_log_i("int_attribute = %ld \r\n", int_attribute);
                    snprintf(numBuff, sizeof(numBuff), "%d", data_list_node->cmd_data.cmd_id);
                    cJSON_AddNumberToObject(json_control, numBuff, int_attribute);
                }     
                else if(data_list_node->cmd_data.cmd_type == DATA_TYPE_STRING)
                {
                    string_attribute = malloc(data_list_node->cmd_data.data_len + 1);
                    if(string_attribute == NULL)
                    {
                        aiio_log_e("malloc fail \r\n");
                        goto exit;
                    }
                    memset(string_attribute, 0, data_list_node->cmd_data.data_len + 1);
                    memcpy(string_attribute, data_list_node->cmd_data.data, data_list_node->cmd_data.data_len);
                    aiio_log_i("string_attribute = %s \r\n", string_attribute);
                    snprintf(numBuff, sizeof(numBuff), "%d", data_list_node->cmd_data.cmd_id);
                    cJSON_AddStringToObject(json_control, numBuff, string_attribute);
                }     
                else if(data_list_node->cmd_data.cmd_type == DATA_TYPE_ENUM)
                {
                    McuGetEnumValue(data_list_node->cmd_data.data, (uint8_t *)&enum_attribute);
                    aiio_log_i("enum_attribute = %d \r\n", enum_attribute);
                    snprintf(numBuff, sizeof(numBuff), "%d", data_list_node->cmd_data.cmd_id);
                    cJSON_AddNumberToObject(json_control, numBuff, enum_attribute);
                }     


                if(data_list_node->cmd_data.data != NULL)
                {
                    free(data_list_node->cmd_data.data);
                }
            }

            if (!aiio_ListDelNode(&CmdDataList, node))
            {
                aiio_log_e("delete timer from list err");
            }

            node_num = aiio_ListGetNodeNum(&CmdDataList);
            aiio_log_i("node_num = %d \r\n", node_num);

            if(string_attribute)
            {
                free(string_attribute);
                string_attribute = NULL;
            }
            bool_attribute = false;
            int_attribute = 0;
            enum_attribute = 0;
        }
        json_str = cJSON_PrintUnformatted(json_control);
        if(json_str == NULL)
        {
            aiio_log_e("json create str fail \r\n");
            cJSON_Delete(json_control);
            goto exit;
            //return ;
        }
        //aiio_service_report_control_ack(AilinkGetProMid(), AilinkGetProfrom(), 0);
        ret = aiio_service_report_attribute(AilinkGetProMid(), AilinkGetProfrom(), json_str);
        // aiio_service_report_attribute(NULL, NULL, json_str);
        cJSON_free(json_str);
        //printf("6666666 ret is %d\r\n",ret);
        if(ret == 0)
        {
            cJSON_Delete(json_control);
        }
        json_control = NULL;

    }

    MutexUnLock();

exit:
    while (aiio_ListGetNodeNum(&CmdDataList) > 0)
    {
        node = aiio_ListGetNodeFromIndex(&CmdDataList, 0);

        if(!node)                   continue;

        if (!aiio_ListDelNode(&CmdDataList, node))
        {
            aiio_log_e("delete timer from list err");
        }

    }
    if(string_attribute)
    {
        free(string_attribute);
        string_attribute = NULL;
    }
    if(json_control)
    {
        cJSON_Delete(json_control);
    }
}


bool HeartBeatTimeout(void)
{
    if(SendHeatBeatCount > 20)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static void UartSendMac(void)
{
    uint8_t dataBuf[7] = {0};

    dataBuf[0] = 0;

    aiio_wifi_sta_mac_get(dataBuf);
    printf("mac :\r\n");
    for(uint8_t n = 0; n < 7; n++)
    {
        printf("%02x", dataBuf[n]);
    }
    printf("\r\n");
    aiio_send_mcu_cmd(GET_MAC_CMD, dataBuf, sizeof(dataBuf));
}


static void UartSendLocalTime(void)
{
    aiio_rtc_time_t *timer = NULL;
    uint8_t dataBuf[8] = {0};

    timer = aiio_get_rtc_time();
    if(timer == NULL)
    {
        aiio_log_e("timer is NULL \r\n");
        return ;
    }

    dataBuf[0] = 1;
    dataBuf[1] = (timer->year - 2000); 
    dataBuf[2] = timer->mon;
    dataBuf[3] = timer->day;
    dataBuf[4] = timer->hour;
    dataBuf[5] = timer->minute;
    dataBuf[6] = timer->second;
    dataBuf[7] = timer->week;

    aiio_send_mcu_cmd(GET_LOCAL_TIME_CMD, dataBuf, sizeof(dataBuf));

}


static bool ProductInfoIsEmpty(void)
{
    if(strlen(productid) == 0 || strlen(wifiFlag) == 0 || strlen(mcuVer) == 0)
    {
        return true;
    } 

    return false;
}


void SetWifiNetWorkState(uint8_t State)
{
    WifiNetworkState = State;

    aiio_send_mcu_cmd(WIFI_STATE_CMD, &WifiNetworkState, sizeof(WifiNetworkState));

}


char *GetProductID(void)
{
    return productid;
}


char *GetProductFlag(void)
{
    return wifiFlag;
}


char *GetMcuVersion(void)
{
    return mcuVer;
}


profile_info_t *AilinkGetProfileInfo(void)
{
    return (&profile_info);
}

uint8_t GetMcuOTAStatus(void)
{
    return McuOtaStatus;
}

void ResetMcuOTAStatus(void)
{
    McuOtaStatus = OTA_STATUS_INVAIL;
}

static void ParseProductInfo(uint8_t *info, uint16_t data_len)
{
    char *buff = NULL;
    cJSON  *cjson_root = NULL;
    cJSON  *cjson_value = NULL;
    char  *str_value = NULL;
    profile_info_t *proinfo = NULL;
    char  *json_str = NULL;

    buff = malloc(data_len + 1);
    if(buff == NULL)
    {
        aiio_log_e("malloc fail \r\n");
        return ;
    }
    memset(buff, 0, data_len + 1);
    memcpy(buff , info, data_len);
    aiio_log_i("buff = %s \r\n", buff);

    proinfo = AilinkGetProfileInfo();

    cjson_root = cJSON_Parse(buff);
    if(cjson_root == NULL)
    {
        aiio_log_e("json parse err \r\n");
        return ;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, STR_PID);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(productid, str_value, strlen(str_value));
            aiio_log_d("str_value = %s \r\n", str_value);
        }
        str_value = NULL;
    }
    cjson_value = cJSON_GetObjectItem(cjson_root, STR_VER);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(mcuVer, str_value, strlen(str_value));
            aiio_log_d("str_value = %s \r\n", str_value);
        }
        str_value = NULL;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, STR_FLAG);
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(wifiFlag, str_value, strlen(str_value));
            aiio_log_d("str_value = %s \r\n", str_value);
        }
        str_value = NULL;
    }

    memcpy(proinfo->productid, productid, strlen(productid));
    memcpy(proinfo->mcu_version, mcuVer, strlen(mcuVer));
    json_str = cJSON_PrintUnformatted(cjson_value);
    aiio_service_report_attribute(NULL ,NULL, json_str);

    free(buff);
    cJSON_Delete(cjson_root);
}


static void ResponseWifiTestState(bool state)
{
    bool buff = state;

    aiio_send_mcu_cmd(WIFI_CONNECT_TEST_CMD, (uint8_t *)&buff, sizeof(buff));
    TestWifiIsStart = false;
}

static void ParseWifiInfo(uint8_t *wifi_info, uint16_t info_len)
{
    char *buff = NULL;
    cJSON  *cjson_root = NULL;
    cJSON  *cjson_value = NULL;
    char  *str_value = NULL;
    static uint8_t status = 0;

    buff = malloc(info_len + 1);
    if(buff == NULL)
    {
        aiio_log_e("malloc fail \r\n");
        ResponseWifiTestState(false);
        return ;
    }
    memset(buff, 0, info_len + 1);
    memcpy(buff , wifi_info, info_len);
    aiio_log_i("buff = %s \r\n", buff);

    cjson_root = cJSON_Parse(buff);
    if(cjson_root == NULL)
    {
        aiio_log_e("json parse err \r\n");
        ResponseWifiTestState(false);
        return ;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, "ssid");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(wifiSsid, str_value, strlen(str_value));
            status++;
            aiio_log_i("ssid = %s \r\n", wifiSsid);
        }
        str_value = NULL;
    }
    cjson_value = cJSON_GetObjectItem(cjson_root, "password");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        if(str_value)
        {
            memcpy(wifiPasswd, str_value, strlen(str_value));
            status++;
            aiio_log_i("passwd = %s \r\n", wifiPasswd);
        }
        str_value = NULL;
    }

    if(status == 0)
    {
        ResponseWifiTestState(false);
    }

    if(status >= 2)
    {
        iot_connect_wifi(wifiSsid, wifiPasswd);
        TestWifiIsStart = true;
        ResponseWifiTestState(true);
        status = 0;
    }


    free(buff);
    cJSON_Delete(cjson_root);
}

void NotifyMcuDeviceUpdate(char *ver, char *md5)
{
    char *info = NULL;

    if(ver == NULL || md5 == NULL)
    {
        aiio_log_e("Param fail\r\n");
        return ;
    }

    info = (char *)malloc(MCU_UPDATE_INFO_LEN+1);
    if(info == NULL)
    {
        aiio_log_e("mallo fail\r\n");
        return ;
    }
    memset(info, 0, MCU_UPDATE_INFO_LEN+1);

    snprintf(info, MCU_UPDATE_INFO_LEN, "{\"%s\":\"%s\", \"%s\":\"%s\"}", STR_VER, ver, STR_MD5, md5);

    aiio_send_mcu_cmd(UPDATE_START_CMD, (uint8_t *)info, strlen(info));

    free(info);
}

void TransferMcuDeviceUOtaData(char *data, uint16_t data_len)
{
    if(data == NULL)
    {
        aiio_log_e("Param fail\r\n");
        return ;
    }

    aiio_send_mcu_cmd(UPDATE_TRANS_CMD, (uint8_t *)data, data_len);
}

uint8_t ProcessUartCmd(uint8_t *DataCmd, uint16_t data_len)
{
    uint8_t cmdType = 0;
    uint16_t len = 0;
    uint16_t count = 0;
    uint16_t dp_len = 0;

    if(DataCmd == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }

    cmdType = DataCmd[CMD_TYPE];

    aiio_log_d("###################### receive ######################\r\n");
    aiio_log_d("cmdType = %d \r\n", cmdType);
    for(uint16_t n = 0; n < data_len; n++)
    {
        aiio_log_d("0x%02x", DataCmd[n]);
    }
    aiio_log_d("###################### end ######################\r\n");

    switch (cmdType)
    {

        case HEART_BEAT_CMD:
        {
            aiio_log_d("HEART_BEAT_CMD \r\n");
            if((DataCmd[DATA_LOCAL] == 0) || !DeviceIsStart)
            {
                aiio_log_d("send PRODUCT_INFO_CMD \r\n");
                aiio_send_mcu_cmd(PRODUCT_INFO_CMD, NULL, 0);
                DeviceIsStart = true;
                
                if(DataCmd[DATA_LOCAL] == 0)
                {
                    memset(productid, 0, sizeof(productid));
                    memset(wifiFlag, 0, sizeof(wifiFlag));
                    memset(mcuVer, 0, sizeof(mcuVer));
                    McuDeviceIsStart = true;
                }
                if(!mcuConnectFlag)
                {
                    aiio_log_i("change heart beat time \r\n");
                    SetHeartBeatTimeout(15 * 1000);
                    mcuConnectFlag = true;
                }
            }
            SendHeatBeatCount = 0;
            aiio_log_d("###################### end ######################\r\n");
        }
        break;

        case WORK_MODE_CMD:
        {
            aiio_log_d("WORK_MODE_CMD \r\n");
            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);

            aiio_log_d("len = %d \r\n", len);
            if(len > 0)
            {
                aiio_log_d("send STATE_QUERY_CMD \r\n");
                aiio_send_mcu_cmd(STATE_QUERY_CMD, NULL, 0);
            }
            else
            {
                aiio_log_d("send WIFI_STATE_CMD \r\n");
                aiio_send_mcu_cmd(WIFI_STATE_CMD, &WifiNetworkState, sizeof(WifiNetworkState));
            }
        }
        break;

        case WIFI_STATE_CMD:
        {
            aiio_log_d("WIFI_STATE_CMD \r\n");
            aiio_log_d("send STATE_QUERY_CMD \r\n");
            aiio_send_mcu_cmd(STATE_QUERY_CMD, NULL, 0);
        }
        break;

        case STATE_UPLOAD_CMD:
        {
            aiio_log_d("STATE_UPLOAD_CMD \r\n");
            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);
            aiio_log_d("len = %d \r\n", len);

            ProcessReportDataInfo(&DataCmd[DATA_LOCAL], len);
            SetPublicDataTimeout(40);

        }
        break;

        case WIFI_RESET_CMD:
        {
            aiio_log_d("WIFI_RESET_CMD \r\n");
            if(mcuConnectFlag)
            {
                aiio_send_mcu_cmd(WIFI_RESET_CMD, NULL, 0);
                aiio_repair();
                aiio_log_i("Enter Pair \r\n");
                aiio_os_tick_dealy(aiio_os_ms2tick(20));
                aiio_restart();
            }
        }
        break;
        
        case WIFI_MODE_CMD:
        {
            aiio_log_d("WIFI_MODE_CMD \r\n");
            if(mcuConnectFlag)
            {
                aiio_send_mcu_cmd(WIFI_MODE_CMD, NULL, 0);
                aiio_repair();
                aiio_log_i("reset wifi module, and enter pair\r\n");
                aiio_os_tick_dealy(aiio_os_ms2tick(20));
                aiio_restart();
            }
        }
        break;

        case GET_MAC_CMD:
        {
            aiio_log_d("GET_MAC_CMD \r\n");
            UartSendMac();
        }
        break;

        case GET_WIFI_STATUS_CMD:
        {
            aiio_log_d("GET_WIFI_STATUS_CMD \r\n");
            aiio_send_mcu_cmd(WIFI_STATE_CMD, &WifiNetworkState, sizeof(WifiNetworkState));
        }
        break;

        case UPDATE_START_CMD:
        {
            aiio_log_d("UPDATE_START_CMD \r\n");
            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);
            aiio_log_d("len = %d \r\n", len);
            McuOtaStatus = DataCmd[DATA_LOCAL];
            aiio_log_d("McuOtaStatus = %d \r\n", McuOtaStatus);
        }
        break;

        case UPDATE_TRANS_CMD:
        {
            aiio_log_d("UPDATE_TRANS_CMD \r\n");
            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);
            aiio_log_d("len = %d \r\n", len);
            McuOtaStatus = DataCmd[DATA_LOCAL];
            aiio_log_d("McuOtaStatus = %d \r\n", McuOtaStatus);
        }
        break;

        case PRODUCT_INFO_CMD:
        {
            aiio_log_d("PRODUCT_INFO_CMD \r\n");

            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);

            aiio_log_d("len = %d \r\n", len);
            
            ParseProductInfo(&DataCmd[DATA_LOCAL], len);

            if(!ProductInfoIsEmpty())
            {
                aiio_log_d("send WORK_MODE_CMD \r\n");
                aiio_send_mcu_cmd(WORK_MODE_CMD, NULL, 0);
                McuDeviceIsStart = false;
            }
        }
        break;

        case GET_LOCAL_TIME_CMD:
        {
            aiio_log_d("GET_LOCAL_TIME_CMD \r\n");
            UartSendLocalTime();
        }
        break;

        case WIFI_TEST_CMD:
        {
            aiio_log_d("WIFI_TEST_CMD \r\n");
        }
        break;

        case WIFI_CONNECT_TEST_CMD:
        {
            aiio_log_d("WIFI_CONNECT_TEST_CMD \r\n");

            len = ((DataCmd[LEN_H] << 8) | DataCmd[LEN_L]);
            aiio_log_d("len = %d \r\n", len);
            if(mcuConnectFlag)
            {
                ParseWifiInfo(&DataCmd[DATA_LOCAL], len);
            }
            
        }
        break;

        default:
            break;
    }


    return 0;
}


int8_t UartRevOneByte(uint8_t data)
{
    if((revBuffPt_in > revBuffPt_out) && ((revBuffPt_in - revBuffPt_out) >= sizeof(revBuff)))
    {
        aiio_log_e("buff is full \r\n");
    }
    else
    {
        if(revBuffPt_in >= (uint8_t *)(revBuff + sizeof(revBuff)))
        {
            revBuffPt_in = (uint8_t *)(revBuff);
        }

        *revBuffPt_in++ = data;
        
    }

    return 0;
}


int8_t UartRevStream(uint8_t *data, uint16_t data_len)
{
    uint16_t n = 0;

    if(data == NULL)
    {
        aiio_log_e("param err \r\n");
        return -1;
    }
    for(n = 0; n < data_len; n++)
    {
        UartRevOneByte(data[n]);
    }

    return 0;
}


bool revBuffIsEmpty(void)
{
    if(revBuffPt_in != revBuffPt_out)
    {
        return false;
    }
    else
    {
        return true;
    }
}


uint8_t TakeDataFromBuff(void)
{
    uint8_t value = 0;

    if(revBuffPt_in != revBuffPt_out)
    {
        //有数据
        if(revBuffPt_out >= (uint8_t *)(revBuff + sizeof(revBuff))) {
            //数据已经到末尾
            revBuffPt_out = (uint8_t *)(revBuff);
        }
        
        value = *revBuffPt_out ++;   
    }

    return value;
}


int8_t UartProcessPro(void)
{
    uint16_t dataLen = 0;
    static uint16_t revLen = 0;
    uint16_t offset = 0;

    while ((revLen < sizeof(revBuff)) && (!revBuffIsEmpty()))
    {
        processBuff[revLen++] = TakeDataFromBuff();
    }
    
    if(revLen < FRAME_MINI_LEN)
    {
        return -1;
    }

    while ((revLen - offset) >= FRAME_MINI_LEN)
    {
        if(processBuff[offset + HEAD_H] != FRAME_H)
        {
            offset++;
            aiio_log_e("frame head err \r\n");
            continue;
        }

        if(processBuff[offset + HEAD_L] != FRAME_L)
        {
            offset++;
            aiio_log_e("frame head err \r\n");
            continue;
        }


        if(processBuff[offset + PRO_VER] != REV_VER )
        {
            offset += 2;
            aiio_log_e("frame ver err \r\n");
            continue;
        }

        dataLen = ((processBuff[offset + LEN_H] << 8) | processBuff[offset + LEN_L]);
        dataLen += FRAME_MINI_LEN;
        if(dataLen > (sizeof(processBuff) + FRAME_MINI_LEN))
        {
            offset += 3;
            aiio_log_e("frame len err \r\n");
            continue;
        }

        if((revLen - offset) < dataLen)
        {
            break;
        }

        if(processBuff[offset + dataLen - 1] != UartCheckValue(&processBuff[offset], dataLen-1))
        {
            offset += 3;
            aiio_log_e("frame check err \r\n");
            continue;
        }
        // printf("complate date :\r\n");
        // for(uint8_t n = 0; n < dataLen; n++)
        // {
        //     printf("0x%02x ", processBuff[offset+n]);
        // }
        // printf("\r\n");
        ProcessUartCmd(&processBuff[offset], dataLen);
        offset += dataLen;
    }
    
    revLen -= offset;
    if(revLen > 0)
    {
        aiio_log_d("revLen = %d \r\n", revLen);
        aiio_log_d("offset = %d \r\n", offset);
        memcpy(processBuff, &processBuff[offset], revLen);
    }
    return 0;
}
