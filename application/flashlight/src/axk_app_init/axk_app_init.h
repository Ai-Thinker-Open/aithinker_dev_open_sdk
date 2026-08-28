#ifndef _AXK_APP_INIT_H_
#define _AXK_APP_INIT_H_

#include <stdint.h>

typedef struct axk_app_init
{
    void (*init)(void);
} axk_app_init_t;

extern axk_app_init_t g_app_init;

#endif //_AXK_APP_INIT_H_
