#ifndef __LN_SMARTCFG_H__
#define __LN_SMARTCFG_H__

#include "ln_types.h"


#define SMARTCONFIG_ENABLE_CRYPT    (0)

#define WLAN_SSID_MAX_LEN           (33)
#define WLAN_PWD_MAX_LEN            (64)
#define SC_AES_KEY                  ("1234567812345678")
#define SC_AES_KEY_LEN              (16)
#define SC_MAX_SSIDPWD_LEN          (128)
#define SC_PKT_BYTE_MARK_LEN        ((SC_MAX_SSIDPWD_LEN/2)/8)
#define PACKET_NUM_MAX 		        (127)

#define SC_MAGIC_ADD0 		        (0x01)
#define SC_MAGIC_ADD1 		        (0x00)
#define SC_MAGIC_ADD2 		        (0x5E)


typedef enum {
	SC_STATE_IDLE               = 0,   /**< decoded never started */
	SC_STATE_SEARCH_CHAN        = 1,   /**< start to search channel */
	SC_STATE_LOCKED_CHAN        = 2,   /**< decode channel info from raw frame */
    SC_STATE_DECODE_ERR         = 3,   /**< decode ssidpwd info error */
	SC_STATE_COMPLETE           = 4,   /**< all info decode OK from raw frame*/
} sc_state_t;

typedef enum
{
	LEAD_CODE_SEQ_NONE          = 0,
	LEAD_CODE_SEQ_CHANNEL       = 1,
	LEAD_CODE_SEQ_SSIDPWD_SIZE  = 2,
	LEAD_CODE_SEQ_PWD_SIZE      = 3,
	LEAD_CODE_SEQ_RANDOM        = 4,
	LEAD_CODE_SEQ_COMPLETE      = 5,
} lc_seq_t;

typedef struct {
    unsigned    frame_ctrl :16;
    unsigned    duration_id:16;
    uint8_t     i_addr1[6];            /**< receiver address */
    uint8_t     i_addr2[6];            /**< sender address */
    uint8_t     i_addr3[6];            /**< filtering address */
    unsigned    sequence_ctrl:16;
} ieee80211_frame_t;

typedef struct {
    uint8_t    channel;
    uint8_t    ssidpwd_len;
    uint8_t    pwd_len;
    uint8_t    random;    
    uint8_t    lc_mark;
    uint8_t    expect_total_pkt;       /**< how many packets should be received after leadcode. */
    uint8_t    pkt_count;              /**< how many packets are actually recieved. */
    uint8_t    is_lc_complete;
    uint8_t    pkt_mark[SC_PKT_BYTE_MARK_LEN];
} lc_t;

typedef struct {
    uint8_t  * ssid;
    uint8_t  * pwd;
    uint8_t    ssid_len;
    uint8_t    pwd_len;
    uint8_t    random;
} sc_result_t;

typedef struct {
    char        * aes_key;
    uint8_t       recvbuff[SC_MAX_SSIDPWD_LEN];
    lc_t          lead_code;
    sc_result_t   result;
    sc_state_t    state;
    uint8_t       is_ch_hopping;
    uint16_t      ch_map;
} sc_ctl_t;


#endif /* __LN_SMARTCFG_H__ */


