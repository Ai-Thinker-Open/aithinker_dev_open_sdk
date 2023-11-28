#include "airkiss_port.h"
#include "osal/osal.h"
#include "wifi/wifi.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "utils/debug/log.h"
#include "utils/cJSON.h"
#include <stdarg.h>

static     OS_Timer_t    channel_hop_timer;

// TODO: adapt to different OS
cJSON_Hooks cjson_hooks = {
	.malloc_fn = pvPortMalloc,
	.free_fn = vPortFree
};

/**
 * @brief cJSON use OS malloc/free instead of stdanrd lib function.
 *
 */
void ak_port_init(void)
{
	cJSON_InitHooks(&cjson_hooks);
}

int ak_port_printf(const char* format, ...)
{
    #define PRINTF_BUF_LEN  (128)
    static char printf_buf[PRINTF_BUF_LEN];
    va_list args;

    va_start(args, format);
    vsnprintf(printf_buf, PRINTF_BUF_LEN, format, args);
    va_end(args);
    if(NULL != log_stdio_write){
        log_stdio_write(printf_buf, strlen(printf_buf));
    }
    return 0;
}

void ak_port_wifi_set_channel(uint8_t ch)
{
    wifi_set_channel(ch);
}

void ak_port_wifi_channel_hop_timer_creat(void * timer_cb, uint16_t * channel_mask)
{
    OS_TimerCreate(&channel_hop_timer, OS_TIMER_PERIODIC, (OS_TimerCallback_t)timer_cb, channel_mask, CHANNEL_HOP_INTERVAL);
}

void ak_port_wifi_channel_hop_start(void)
{
    OS_TimerStart(&channel_hop_timer);
}

void ak_port_wifi_channel_hop_stop(void)
{
    OS_TimerStop(&channel_hop_timer);
}

void ak_port_wifi_channel_hop_timer_del(void)
{
    OS_TimerDelete(&channel_hop_timer);
}

void ak_port_wifi_sniffer_enable(void * callback)
{
    wifi_set_promiscuous_filter(WIFI_PROMIS_FILTER_MASK_DATA);
    wifi_set_promiscuous_rx_cb((wifi_promiscuous_cb_t)callback);
    wifi_set_promiscuous(true);
}

void ak_port_wifi_sniffer_disable(void)
{
    wifi_set_promiscuous(false);
    wifi_set_promiscuous_rx_cb(NULL);
}

/**
 * @brief After connect to WiFi, this device send \em random via UDP broadcast
 * port 10000 to mobile device.
 *
 * @param random
 * @return int return 1 on success, 0 on failure.
 */
int ak_port_udp_send_ack(uint8_t random)
{
	uint8_t send_data = 0;
	int optval = 0;
	int ret = 0;

	struct sockaddr_in addr;
	int socketfd;

	send_data = random;

	if (0 > (socketfd = socket(AF_INET, SOCK_DGRAM, 0))) {
		ak_port_printf("%s() create socket fail\n", __func__);
		return AK_FALSE;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_port   = htons(AIRKISS_ACK_UDP_PORT);
	addr.sin_family = AF_INET;
	if (0 > inet_aton(AIRKISS_ACK_UDP_IP, &addr.sin_addr)) {
		ak_port_printf("%s() inet_aton error\n", __func__);
		goto err;
	}

	if (0 > setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int))) {
		ak_port_printf("%s() setsockopt error\n", __func__);
		goto err;
	}

	for (uint8_t i = 0; i < AIRKISS_ACK_REPLY_CNT; i++) {

		if (i < 1) {
			ak_port_printf("airkiss reply random: 0X%02X\r\n", random);
		}

		ret = sendto(socketfd, &send_data, sizeof(send_data), 0, (struct sockaddr *)&addr, sizeof(addr));
		if (ret == -1) {
			ak_port_printf("%s() udp send error, %d\r\n", __func__);
			goto err;
		}
		OS_MsDelay(60);
	}

	closesocket(socketfd);
	return AK_TRUE;

err:
	closesocket(socketfd);
	return AK_FALSE;
}

__WEAK__  int HAL_GetDevInfo(void *pdevInfo)
{
	return AK_FALSE;
}
