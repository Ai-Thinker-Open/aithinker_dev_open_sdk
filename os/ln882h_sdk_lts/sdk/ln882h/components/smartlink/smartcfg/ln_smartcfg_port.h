#ifndef __LN_SMARTCFG_PORT_H__
#define __LN_SMARTCFG_PORT_H__

#include "ln_types.h"
#include "utils/debug/log.h"

void   ln_sc_wifi_set_channel(uint8_t ch);

void   ln_sc_wifi_channel_hop_start(void * timer_cb, uint16_t * channel_map);

void   ln_sc_wifi_channel_hop_stop(void);

void   ln_sc_wifi_sniffer_enable(void * callback);

void   ln_sc_wifi_sniffer_disable(void);

int    ln_sc_aes_decrypt(char * key, uint8_t * enc_data,uint16_t enc_data_len, uint8_t * dec_buf);

int    ln_sc_udp_send_ack(uint8_t random);

void * ln_sc_malloc(size_t size);

void   ln_sc_free(void *ptr);

void   ln_sc_printf(const char *format, ...);

#define SC_LOG_ON    (1)

#define sc_log(fmt, arg...)                     \
        do {                                    \
            if(SC_LOG_ON)                       \
                log_printf("[SC] "fmt, ##arg);  \
        } while (0)



#endif /* __LN_SMARTCFG_PORT_H__ */


