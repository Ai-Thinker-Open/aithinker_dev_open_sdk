/** @brief      blufi api.
 *
 *  @file       aiio_blufi.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/18      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#include <stdlib.h>
#include <string.h>

#include "blufi_int.h"
#include "blufi_prf.h"
#include "aiio_blufi_export_import.h"

static char BleConfigName[64] = "BLUFI_DEVICE";

//Set the broadcast name of the Bluetooth distribution network
//The Settings are valid before starting Bluetooth network distribution, and will not be saved after power failure
void aiio_blufi_set_ble_name(char *bleName){
	memset(BleConfigName,0,sizeof(BleConfigName));
	int len=strlen(bleName);
	if(len>sizeof(BleConfigName)-1){
		len=sizeof(BleConfigName)-1;
	}
	memcpy(BleConfigName,bleName,len);
}

char * aiio_blufi_get_ble_name(){
	return BleConfigName;
}

void aiio_blufi_adv_start(void)
{
    uint16_t offect = 0;
    uint8_t adv_data[37] = {0};
    at_ble_adv_data_t data = {0};

    adv_data[offect++] = 2;
    adv_data[offect++] = 0x01;
    adv_data[offect++] = 0x06;

    // char name[] = "BLUFI_DEVICE";
    adv_data[offect++] = strlen(BleConfigName) + 1;
    adv_data[offect++] = 0x09;
    memcpy(&adv_data[offect], BleConfigName, strlen(BleConfigName));
    offect += strlen(BleConfigName);
    // axk_hal_ble_set_gap_name(name);

    adv_data[offect++] = 2;
    adv_data[offect++] = 0x0A;
    adv_data[offect++] = 0x03;

    adv_data[offect++] = 3;
    adv_data[offect++] = 0x03;
    adv_data[offect++] = 0xFF;
    adv_data[offect++] = 0xFF;

    memcpy(data.data, adv_data, offect);
    data.data_len = offect;

    aiio_hal_ble_adv_start(&data);
}

uint8_t aiio_blufi_init(void)
{
    blufi_env.enabled = true;
    aiio_blufi_cb_param_t param;
    param.init_finish.state = AIIO_BLUFI_INIT_OK;
    btc_blufi_cb_to_app(AIIO_BLUFI_EVENT_INIT_FINISH, &param);
    return AIIO_BLUFI_SUCCESS;
}

void aiio_blufi_deinit(void)
{
    blufi_env.enabled = false;
    btc_msg_t msg;
    aiio_blufi_cb_param_t param;
    msg.act = AIIO_BLUFI_EVENT_DEINIT_FINISH;
    param.deinit_finish.state = AIIO_BLUFI_DEINIT_OK;
    btc_transfer_context(&msg, &param, sizeof(aiio_blufi_cb_param_t), NULL);
}

void aiio_blufi_adv_stop(void)
{
    aiio_hal_ble_adv_stop();
}




void  aiio_blufi_disconnect()
{
    aiio_hal_blufi_gap_disconnect();
}

void aiio_blufi_send_notify(void *arg)
{
    struct pkt_info *pkts = (struct pkt_info *) arg;
    aiio_hal_ble_gatt_blufi_notify_send(pkts->pkt,pkts->pkt_len);

}

void aiio_blufi_send_encap(void *arg)
{
    struct blufi_hdr *hdr = (struct blufi_hdr *)arg;
    if (blufi_env.is_connected == false)
    {
        aiio_log_d("[BLUFI] %s ble connection is broken\r\n", __func__);
        free(hdr);
        hdr = NULL;
        return;
    }
    btc_blufi_send_notify((uint8_t *)hdr,
                          ((hdr->fc & BLUFI_FC_CHECK) ? hdr->data_len + sizeof(struct blufi_hdr) + 2 : hdr->data_len + sizeof(struct blufi_hdr)));
}




