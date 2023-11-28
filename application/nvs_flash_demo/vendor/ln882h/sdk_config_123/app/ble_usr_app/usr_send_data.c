#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/debug/log.h"
#include "utils/debug/ln_assert.h"
#include "rwip_config.h"
#include "llm_int.h"
#include "att.h"
#include "usr_send_data.h"
#include "ln_app_gatt.h"
#include "usr_ble_app.h"
typedef struct {

    uint16_t hdl_svc;
    int conid;
    int (*rx_callback)(int, uint8_t*, int);
    uint16_t mtu;
} data_trans_svr_t;

 data_trans_svr_t  data_trans_svr = {0};


static const struct gattm_att_desc  data_trans_atts[] = {
    [DATA_TRANS_DECL_CHAR_RX] = {
        .uuid = { 0x03, 0x28 },
        .perm = PERM_MASK_RD,
        .max_len = 0,
        .ext_perm = (0 <<  PERM_POS_UUID_LEN),
    },
    [DATA_TRANS_CHAR_VAL_RX] = {
        .uuid = {DATA_TRANS_RX_UUID },
        .perm = PERM_MASK_WRITE_REQ|PERM_MASK_WRITE_COMMAND|PERM_MASK_RD,
        .max_len = DATA_TRANS_MAX_LEN,
        .ext_perm = (2 <<PERM_POS_UUID_LEN),
    },

    [DATA_TRANS_DECL_CHAR_TX] = {
        .uuid = { 0x03, 0x28 },
        .perm = PERM_MASK_RD,
        .max_len = 0,
        .ext_perm =  (0 << PERM_POS_UUID_LEN),
    },
    [DATA_TRANS_CHAR_VAL_TX] = {
        .uuid = {DATA_TRANS_TX_UUID},
        .perm = PERM_MASK_NTF|PERM_MASK_RD,
        .max_len = DATA_TRANS_MAX_LEN,
        .ext_perm =  (2 << PERM_POS_UUID_LEN),
    },
    [DATA_TRANS_CLIENT_CHAR_CFG_TX] = {
        .uuid = { 0x02, 0x29 }, //INB_ATT_DESC_SERVER_CHAR_CFG,
        .perm = PERM_MASK_WRITE_REQ|PERM_MASK_RD,
        .max_len = 0,
        .ext_perm = 0,
    },
};


uint8_t svc_uuid[16]={0x85,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x01,0x00,0x40,0x6E};

void data_trans_svc_add(void)
{
    int nb_att = sizeof(data_trans_atts)/sizeof(data_trans_atts[0]);
    struct ln_gattm_add_svc_req p_svc_desc;
    p_svc_desc.svc_desc.start_hdl = 0;
    p_svc_desc.svc_desc.perm = (2 << 5);
    memcpy(p_svc_desc.svc_desc.uuid, svc_uuid, 16);
    p_svc_desc.svc_desc.nb_att = nb_att;
    p_svc_desc.svc_desc.atts = data_trans_atts;
    ln_app_gatt_add_svc(&p_svc_desc);
}



void in_trx_notify(int conidx, uint8_t *buf, uint32_t len)
{

}


