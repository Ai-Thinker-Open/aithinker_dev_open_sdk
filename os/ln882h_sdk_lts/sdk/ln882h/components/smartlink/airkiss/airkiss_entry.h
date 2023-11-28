#ifndef __LN_AIRKISS_ENTRY_H__
#define __LN_AIRKISS_ENTRY_H__

#include "stdint.h"
#include "airkiss_def.h"
typedef enum
{
    CH1_MASK     = (1UL << 1 ),
    CH2_MASK     = (1UL << 2 ),
    CH3_MASK     = (1UL << 3 ),
    CH4_MASK     = (1UL << 4 ),
    CH5_MASK     = (1UL << 5 ),
    CH6_MASK     = (1UL << 6 ),
    CH7_MASK     = (1UL << 7 ),
    CH8_MASK     = (1UL << 8 ),
    CH9_MASK     = (1UL << 9 ),
    CH10_MASK    = (1UL << 10),
    CH11_MASK    = (1UL << 11),
    CH12_MASK    = (1UL << 12),
    CH13_MASK    = (1UL << 13),
    CH14_MASK    = (1UL << 14),
} airkiss_wifi_ch_mask_t;


int       airkiss_start(uint16_t channel_mask);
void      airkiss_stop(void);
int       airkiss_is_complete(void);
uint8_t * airkiss_get_ssid(void);
uint8_t * airkiss_get_pwd(void);
int       airkiss_send_ack(void);

#endif /* __LN_AIRKISS_ENTRY_H__ */


