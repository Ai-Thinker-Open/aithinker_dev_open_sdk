#ifndef _AXK_SLEEP_H_
#define _AXK_SLEEP_H_

#include <stdint.h>

#include "aiio_os_port.h"

typedef struct
{
    uint8_t enable;
} axk_sleep_info_t;

typedef struct
{
    int32_t(*set_info)(const axk_sleep_info_t *info);
} axk_sleep_t;

extern axk_sleep_t g_axk_sleep;
extern aiio_os_timer_handle_t sleep_timer;

int32_t axk_auto_sleep_timer_init(void);
int32_t axk_sleep_timer_reset(void);
int32_t axk_sleep_timer_stop(void);
int32_t axk_sleep_timer_change(uint32_t ms);
int32_t axk_sleep_timer_is_actic(void);

int at_setup_auto_sleep_cmd(uint32_t argc, const char **argv);

#endif
