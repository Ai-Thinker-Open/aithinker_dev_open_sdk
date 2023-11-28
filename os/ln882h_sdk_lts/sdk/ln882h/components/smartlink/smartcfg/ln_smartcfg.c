#include "ln_smartcfg_port.h"
#include "ln_smartcfg_api.h"
#include "ln_smartcfg.h"
#include "ln_compiler.h"

static sc_ctl_t * sc_ctl;
static uint8_t    sc_ssid[WLAN_SSID_MAX_LEN];
static uint8_t    sc_pwd[WLAN_PWD_MAX_LEN + 1];
static uint8_t    sc_random = 0;

static uint8_t cal_crc8(const uint8_t *in, int num)
{
    #define CRC8_INIT       	(0x00)
    #define CRC8_POLY       	(0x31)

    uint8_t crc = CRC8_INIT;
    int i, j;

    for (i = 0; i < num; i++ )
    {
        crc ^= in[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x1) {
                crc = (crc >> 1) ^ CRC8_POLY;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

static int aes_ebc_decrypt(char *in, char *out, int in_len, char *key)
{
    if (ln_sc_aes_decrypt(key, (uint8_t *)in, in_len, (uint8_t *)out) != LN_TRUE) {
        return LN_FALSE;
    }
    return LN_TRUE;
}

static void dump_leadcode(lc_t *leadcode)
{
	sc_log("\t\t<sc leadcode> channel = %d\r\n", leadcode->channel);
	sc_log("\t\t<sc leadcode> pwd_len = %d\r\n", leadcode->pwd_len);
	sc_log("\t\t<sc leadcode> random  = %d\r\n", leadcode->random);
	sc_log("\t\t<sc leadcode> ssidpwd_len = %d\r\n", leadcode->ssidpwd_len);
}

static void dump_decode_result(sc_result_t *result)
{
	sc_log("\t\t<sc result> ssid     = %s\r\n", result->ssid);
	sc_log("\t\t<sc result> ssid len = %u\r\n", result->ssid_len);
	sc_log("\t\t<sc result> pwd      = %s\r\n", result->pwd);
	sc_log("\t\t<sc result> pwd len  = %u\r\n", result->pwd_len);
	sc_log("\t\t<sc result> random   = %u\r\n\r\n\r\n", result->random);
}

static void ln_sc_lead_code_proc(ieee80211_frame_t *iframe, lc_t *lead_code, uint8_t packet_num)
{
	uint8_t *data = &iframe->i_addr1[4];
	uint8_t *crc  = &iframe->i_addr1[5];
    uint8_t ssidlen = 0;
	if ( (packet_num == LEAD_CODE_SEQ_NONE) || (packet_num > LEAD_CODE_SEQ_RANDOM) ) {
		return;
	}

    if ((lead_code->lc_mark & 0x1E) == 0x1E) {
        ssidlen = lead_code->ssidpwd_len - lead_code->pwd_len;
        if ((63 >= lead_code->pwd_len) && (32 >= ssidlen)) {
            lead_code->is_lc_complete = LN_TRUE;
            dump_leadcode(lead_code);
            return;
        }

        lead_code->lc_mark &= ~(1U << LEAD_CODE_SEQ_SSIDPWD_SIZE);
        lead_code->lc_mark &= ~(1U << LEAD_CODE_SEQ_PWD_SIZE);
        sc_log("ssid/pwd len error. ssidlen=%d,pwdlen=%d\r\n", ssidlen,lead_code->pwd_len);
    }

	if (*crc != cal_crc8(data, 1)) {
		return;
	}

	switch (packet_num)
    {
		case LEAD_CODE_SEQ_CHANNEL:
			lead_code->channel = *data;
            lead_code->lc_mark |= (1U << LEAD_CODE_SEQ_CHANNEL);
			sc_log("------>CHANNEL: %d\r\n", lead_code->channel);
			return;
		case LEAD_CODE_SEQ_SSIDPWD_SIZE:
			lead_code->ssidpwd_len = *data;
            lead_code->lc_mark |= (1U << LEAD_CODE_SEQ_SSIDPWD_SIZE);
			lead_code->expect_total_pkt = (lead_code->ssidpwd_len / 2) + (lead_code->ssidpwd_len % 2) + 1;
			sc_log("------>PACKET_TOTAL: %d\r\n", lead_code->expect_total_pkt);
			return;
		case LEAD_CODE_SEQ_PWD_SIZE:
			lead_code->pwd_len = *data;
            lead_code->lc_mark |= (1U << LEAD_CODE_SEQ_PWD_SIZE);
			sc_log("------>PWD_LEN: %d\r\n", lead_code->pwd_len);
			return;
		case LEAD_CODE_SEQ_RANDOM:
			lead_code->random = *data;
            lead_code->lc_mark |= (1U << LEAD_CODE_SEQ_RANDOM);
			sc_log("------>RANDOM: %d\r\n", lead_code->random);
			return;
		default:
			return;
	}
}

static int ln_sc_decode(sc_ctl_t * ctl)
{
	uint8_t *src_pwd_data   = NULL;
	uint8_t *src_ssid_data  = NULL;
	uint8_t  src_ssid_size  = 0;
	uint8_t  src_pwd_size   = 0;
	uint8_t  i = 0, crc8 = 0, dlen = 0, temp_val = 0;
    uint8_t  temp_buf[66];

	src_ssid_size = ctl->lead_code.ssidpwd_len - ctl->lead_code.pwd_len;
	src_pwd_size  = ctl->lead_code.pwd_len;
	src_pwd_data  = ctl->recvbuff;
	src_ssid_data = ctl->recvbuff + src_pwd_size;

	if (ctl->aes_key != NULL)
    {
		if (src_pwd_size >= 66 || src_ssid_size >= 65) {
			sc_log("error: decode pwd.\r\n");
			return LN_FALSE;
		}

		if (src_pwd_size)
        {
            memset(temp_buf, 0, 66);
			if (LN_TRUE == aes_ebc_decrypt((char *)src_pwd_data, (char *)temp_buf, src_pwd_size, ctl->aes_key))
            {
				dlen = *(temp_buf + src_pwd_size - 1);
				sc_log("pwd_dlen: %d\r\n", dlen);

				for (i = 0; i < dlen; i++)
                {
                    temp_val = *(temp_buf + src_pwd_size - 1 - i);
					if (temp_val != dlen) {
						sc_log("%s: %d, aes pwd err, value:%d\r\n", __func__, __LINE__, temp_val);
						return LN_FALSE;
					}
				}

				ctl->result.pwd_len = src_pwd_size - dlen;
				memcpy(ctl->result.pwd, temp_buf, strlen((const char *)temp_buf));
			}
            else
            {
				sc_log("%s,%d, aes pwd err\r\n", __func__, __LINE__);
			}
		}

		if (src_ssid_size)
        {
            memset(temp_buf, 0, 66);
			if (LN_TRUE == aes_ebc_decrypt((char *)src_ssid_data, (char *)temp_buf, src_ssid_size, ctl->aes_key))
            {
				dlen = *(temp_buf + src_ssid_size - 1);
				sc_log("ssid_dlen: %d\r\n", dlen);

				for (i = 0; i < dlen; i++)
                {
                    temp_val = *(temp_buf + src_ssid_size - 1 - i);
					if (temp_val != dlen) {
						sc_log("%s,%d, aes ssid err, value:%d\r\n", __func__, __LINE__, temp_val);
						return LN_FALSE;
					}
				}
				ctl->result.ssid_len = src_ssid_size - dlen;
				memcpy(ctl->result.ssid, temp_buf, strlen((const char *)temp_buf));
			}
            else
            {
				sc_log("%s,%d, aes ssid err\r\n", __func__, __LINE__);
			}
		}

		*(ctl->result.pwd + ctl->result.pwd_len) = 0;
		*(ctl->result.ssid + ctl->result.ssid_len) = 0;
	}
    else if (ctl->aes_key == NULL)
    {
		ctl->result.ssid_len = src_ssid_size;
		ctl->result.pwd_len = src_pwd_size;

		memcpy(ctl->result.ssid, src_ssid_data, ctl->result.ssid_len);
		memcpy(ctl->result.pwd, src_pwd_data, ctl->result.pwd_len);

		*(ctl->result.pwd + ctl->result.pwd_len) = 0;
		*(ctl->result.ssid + ctl->result.ssid_len) = 0;
	}

	ctl->result.random = ctl->lead_code.random;

	sc_log("\t\tdecode: ssid: [%s] pwd:[%s]\r\n", ctl->result.ssid, ctl->result.pwd);

	i = (ctl->lead_code.ssidpwd_len % 2);

	crc8 = *(ctl->recvbuff + ctl->lead_code.ssidpwd_len + i);
	if (crc8 != cal_crc8(ctl->result.pwd, ctl->result.pwd_len)) {
        memset(ctl->recvbuff, 0, SC_MAX_SSIDPWD_LEN);
		sc_log("%s,%d pwd crc err,crc8 = %x\r\n", __func__, __LINE__, crc8);
		return LN_FALSE;
	}

	crc8 = *(ctl->recvbuff + ctl->lead_code.ssidpwd_len + i + 1);
	if (crc8 != cal_crc8(ctl->result.ssid, ctl->result.ssid_len)) {
        memset(ctl->recvbuff, 0, SC_MAX_SSIDPWD_LEN);
		sc_log("%s,%d ssid crc err,crc8 = %x\r\n", __func__, __LINE__,crc8);
		return LN_FALSE;
	}

    memset(ctl->recvbuff, 0, SC_MAX_SSIDPWD_LEN);
	return LN_TRUE;
	//return LN_FALSE;
}

static int ln_sc_is_packet_valid(uint8_t *pkt_mark, uint8_t data_index)
{
    uint8_t byte_mark = 0;
    uint8_t bit_mark = 0;

	if ((data_index * 2 ) > SC_MAX_SSIDPWD_LEN) {
		return LN_FALSE;
    }

    byte_mark = *(pkt_mark + data_index / 8);
    bit_mark = (1 << (8 - (data_index % 8) - 1));

	if (byte_mark & bit_mark) {
		return LN_FALSE;
    }

	*(pkt_mark + data_index / 8) |= bit_mark;

	return LN_TRUE;
}

static int ln_sc_push_data(lc_t *leadcode, uint8_t packet_num, uint16_t *recvbuff, uint16_t half_word)
{
	int16_t diff = 0;
    uint8_t data_index = packet_num - 5;

	if (LN_TRUE == ln_sc_is_packet_valid(leadcode->pkt_mark, data_index)) {
        *(recvbuff + data_index) = half_word;
		leadcode->pkt_count += 1;
        sc_log("pkt_count=%d, data_index=%d, data=0x%04x\r\n",leadcode->pkt_count, data_index, half_word);
    }

	diff = leadcode->expect_total_pkt - leadcode->pkt_count;
    //sc_log("invalid pkt:pkt_count=%d, data_index=%d, data=0x%04x\r\n",leadcode->pkt_count, data_index, half_word);

	return diff;
}

sc_state_t ln_sc_recv_frame_proc(sc_ctl_t * ctl, uint8_t *data)
{
	ieee80211_frame_t *iframe     = (ieee80211_frame_t *)data;
	uint8_t            packet_num = iframe->i_addr1[3];
	sc_state_t         state      = ctl->state;
	lc_t              *lc         = &ctl->lead_code;

	if ((packet_num > LEAD_CODE_SEQ_NONE) && (packet_num < LEAD_CODE_SEQ_COMPLETE))
    {
		if (LN_TRUE != lc->is_lc_complete)
        {
            ln_sc_lead_code_proc(iframe, lc, packet_num);

            if (LN_TRUE == lc->is_lc_complete) {
                state = SC_STATE_LOCKED_CHAN;
			}
            return state;
		}
        else
        {
            return state; //repeat received
        }
	}

	if (LN_TRUE != lc->is_lc_complete) {
		return state;
	}

	if (packet_num <= PACKET_NUM_MAX) {
        uint16_t half_word = iframe->i_addr1[4] | (iframe->i_addr1[5] << 8);
		if (0 != ln_sc_push_data(lc, packet_num, (uint16_t *)ctl->recvbuff, half_word)) {
			return state;
        }
	}

	if ((ctl->state < SC_STATE_COMPLETE) && (lc->expect_total_pkt == lc->pkt_count))
    {
		if (LN_TRUE != ln_sc_decode(ctl)) {
            lc->pkt_count = 0;
            memset(lc->pkt_mark, 0, SC_PKT_BYTE_MARK_LEN);
            memset(ctl->recvbuff, 0, SC_MAX_SSIDPWD_LEN);
            memset(&ctl->result, 0, sizeof(sc_result_t));

            memset(sc_ssid, 0, sizeof(sc_ssid));
            memset(sc_pwd, 0, sizeof(sc_pwd));
            ctl->result.ssid = sc_ssid;
            ctl->result.pwd  = sc_pwd;

			state = SC_STATE_DECODE_ERR;
            sc_log("error: decode err\r\n");
		} else {
            sc_random = ctl->result.random;
            dump_decode_result(&ctl->result);
            state = SC_STATE_COMPLETE;
		}
	}

	return state;
}

static void ln_sc_wifi_channel_hopping(void *arg)
{
    static uint8_t idx = 0;
    uint16_t map = *(uint16_t *)arg;

    while (1)
    {
        idx++;
        idx %= 14;

        if (map & (1 << idx)) {
            ln_sc_wifi_set_channel(idx);
            sc_log("set channel = %d\r\n", idx);
            break;
        }
    }
}

/**
 * @param buf
 *      mac header + data
*/
static void ln_sc_sniffer_callback(void *buf, uint16_t len, int pkt_type, int8_t rssi_db)
{
    sc_state_t curr_state;
    uint8_t *frame = (uint8_t *)buf;
    ieee80211_frame_t *iframe = (ieee80211_frame_t *)frame;

    (void)pkt_type;

    if (NULL == sc_ctl) {
        return;
    }

    if (sc_ctl->state == SC_STATE_COMPLETE) {
        return;
    }

    if (len < sizeof(ieee80211_frame_t)) {
        return;
    }

    if ((iframe->i_addr1[0] != SC_MAGIC_ADD0) || \
        (iframe->i_addr1[1] != SC_MAGIC_ADD1) || \
        (iframe->i_addr1[2] != SC_MAGIC_ADD2)) {
        if ((iframe->i_addr3[0] != SC_MAGIC_ADD0) || \
            (iframe->i_addr3[1] != SC_MAGIC_ADD1) || \
            (iframe->i_addr3[2] != SC_MAGIC_ADD2)) {
            return ;
        }
        memcpy(iframe->i_addr1, iframe->i_addr3, 6);
    }

    curr_state = ln_sc_recv_frame_proc(sc_ctl, frame);

    if ((curr_state == SC_STATE_LOCKED_CHAN) && (sc_ctl->state != curr_state))
    {
        sc_ctl->state = curr_state;
        ln_sc_wifi_channel_hop_stop();
        sc_log("channel stop hop!\r\n");
        sc_ctl->is_ch_hopping = LN_FALSE;
    }
    else if ((curr_state == SC_STATE_COMPLETE) && (sc_ctl->state < SC_STATE_COMPLETE))
    {
        sc_ctl->state = curr_state; // succeccful
    }
}

int  ln_smartconfig_start(uint16_t channel_map)
{
	if (NULL == (sc_ctl = ln_sc_malloc(sizeof(sc_ctl_t)))) {
		sc_log("%s malloc failed!\r\n", __func__);
		return LN_FALSE;
	}

    // 1.init param
	memset(sc_ctl, 0, sizeof(sc_ctl_t));
	memset(sc_ssid, 0, sizeof(sc_ssid));
	memset(sc_pwd, 0, sizeof(sc_pwd));
    sc_ctl->result.ssid = sc_ssid;
    sc_ctl->result.pwd  = sc_pwd;
    sc_ctl->is_ch_hopping = LN_FALSE;
	sc_ctl->state = SC_STATE_SEARCH_CHAN;
#if SMARTCONFIG_ENABLE_CRYPT
    sc_ctl->aes_key = SC_AES_KEY;
#endif
    sc_ctl->ch_map = channel_map;

    // 2.channel hop start.
    ln_sc_wifi_channel_hop_start(&ln_sc_wifi_channel_hopping, &sc_ctl->ch_map);
    __disable_irq();
    sc_ctl->is_ch_hopping = LN_TRUE;
    __enable_irq();
    sc_log("channel start hop!\r\n");

    // 3.sniffer enable.
    ln_sc_wifi_sniffer_enable(&ln_sc_sniffer_callback);

	return LN_TRUE;
}

void ln_smartconfig_stop(void)
{
    ln_sc_wifi_sniffer_disable();

    if (LN_TRUE == sc_ctl->is_ch_hopping) {
        ln_sc_wifi_channel_hop_stop();
    }
    ln_sc_free(sc_ctl);
}

uint8_t * ln_smartconfig_get_ssid(void)
{
    return (uint8_t *)sc_ssid;
}

uint8_t * ln_smartconfig_get_pwd(void)
{
    return (uint8_t *)sc_pwd;
}

int ln_smartconfig_is_complete(void)
{
    __disable_irq();
    if ((sc_ctl != NULL) && (sc_ctl->state == SC_STATE_COMPLETE)) {
        __enable_irq();
        return LN_TRUE;
    }
    __enable_irq();
    return LN_FALSE;
}

int ln_smartconfig_send_ack(void)
{
    if (LN_TRUE != ln_sc_udp_send_ack(sc_random)) {
        return LN_FALSE;
    }
    return LN_TRUE;
}



