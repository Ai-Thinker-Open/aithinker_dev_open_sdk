#include "airkiss.h"
#include "airkiss_entry.h"
#include "airkiss_port.h"

static uint8_t             is_complete = AK_FALSE;
static airkiss_context_t   ak_context  = {0};
static airkiss_result_t    ak_result   = {0};

static const airkiss_config_t  ak_conf = {
    (airkiss_memset_fn) &memset,
    (airkiss_memcpy_fn) &memcpy,
    (airkiss_memcmp_fn) &memcmp,
    (airkiss_printf_fn) &ak_port_printf, //NULL
};

static void airkiss_wifi_channel_hopping(void *arg)
{
    static uint8_t idx = 0;
    uint16_t map = *(uint16_t *)arg;

    while (1)
    {
        idx++;
        idx %= 15;

        if (map & (1 << idx)) {
            ak_port_wifi_set_channel(idx);
            airkiss_change_channel(&ak_context);
            ak_port_printf("airkiss set channel = %d\r\n", idx);
            break;
        }
    }
}

static void airkiss_wifi_sniffer_callback(uint8_t *frame, uint32_t len)
{
    //TODO: frame += offset; (pointer to 802.11 mac frame header)
    frame += 1;
    len   -= 1;

    int ret = 0;

    if (is_complete == AK_TRUE) {
        return;
    }

    if(len < 24) {
        return;
    }

    ret = airkiss_recv(&ak_context, frame, len);

    if (AIRKISS_STATUS_CHANNEL_LOCKED == ret) {
        ak_port_wifi_channel_hop_stop();
        ak_port_printf("airkiss channel stop hop!\r\n");

    } else if (AIRKISS_STATUS_COMPLETE == ret) {
        is_complete = AK_TRUE;
        airkiss_get_result(&ak_context, &ak_result);
        ak_port_printf("airkiss complete!\r\n\r\n");
        ak_port_printf("\t------>ssid:[%s]\r\n",ak_result.ssid);
        ak_port_printf("\t------>pwd :[%s]\r\n",ak_result.pwd);
    }
}

int airkiss_start(uint16_t channel_mask)
{
    static uint16_t ch_mask = 0;

    // 1.init param
	memset(&ak_context, 0, sizeof(airkiss_context_t));
	memset(&ak_result, 0, sizeof(airkiss_result_t));
    if (0 > airkiss_init(&ak_context, &ak_conf)) {
        return AK_FALSE;
    }

    // 2.start channel hop.
    ch_mask = channel_mask;
    ak_port_wifi_channel_hop_timer_creat(&airkiss_wifi_channel_hopping, &ch_mask);
    ak_port_wifi_channel_hop_start();

    ak_port_printf("channel start hop!\r\n");

    // 3.sniffer enable.
    ak_port_wifi_sniffer_enable(&airkiss_wifi_sniffer_callback);

	return AK_TRUE;
}

void airkiss_stop(void)
{
    ak_port_wifi_sniffer_disable();
    ak_port_wifi_channel_hop_timer_del();
}

int airkiss_is_complete(void)
{
    return is_complete;
}

uint8_t * airkiss_get_ssid(void)
{
    return (uint8_t *)ak_result.ssid;
}

uint8_t * airkiss_get_pwd(void)
{
    return (uint8_t *)ak_result.pwd;
}

int airkiss_send_ack(void)
{
    if (AK_TRUE != ak_port_udp_send_ack(ak_result.random)) {
        return AK_FALSE;
    }
    return AK_TRUE;
}
