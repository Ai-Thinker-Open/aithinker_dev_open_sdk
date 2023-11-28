#ifndef __LN_AIRKISS_PORT_H__
#define __LN_AIRKISS_PORT_H__

#include "ln_types.h"
#include "airkiss_def.h"

#define AK_TRUE                (LN_TRUE)
#define AK_FALSE               (LN_FALSE)

#define CHANNEL_HOP_INTERVAL			(200)

void  ak_port_init(void);
int   ak_port_printf(const char* format, ...);
void  ak_port_wifi_set_channel(uint8_t ch);
void  ak_port_wifi_channel_hop_timer_creat(void * timer_cb, uint16_t * channel_mask);
void  ak_port_wifi_channel_hop_timer_del(void);
void  ak_port_wifi_channel_hop_start(void);
void  ak_port_wifi_channel_hop_stop(void);
void  ak_port_wifi_sniffer_enable(void * callback);
void  ak_port_wifi_sniffer_disable(void);
int   ak_port_udp_send_ack(uint8_t random);

#endif /* __LN_AIRKISS_PORT_H__ */
