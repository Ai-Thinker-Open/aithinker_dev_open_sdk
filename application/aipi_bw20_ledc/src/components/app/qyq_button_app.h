#ifndef __QYQ_BUTTON_APP_H_
#define __QYQ_BUTTON_APP_H_
#include "chip_include.h"
#include "app_include.h"
#include "drive_include.h"
#include "interface_include.h"
#include "algorithm_include.h"
#ifdef __QYQ_BUTTON_APP_C_
#define QYQ_BUTTON_APP_EXT
#else
#define QYQ_BUTTON_APP_EXT extern
#endif

typedef struct
{
    uint8_t button_id;
    uint16_t button_value;
} qyq_button_app_type_t;

QYQ_BUTTON_APP_EXT void qyq_button_app_task(void *para);
#endif
