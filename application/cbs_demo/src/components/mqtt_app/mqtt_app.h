#ifndef __MQTT_APP_H_
#define __MQTT_APP_H_
#include "aiio_adapter_include.h"
#ifdef __MQTT_APP_C_
#define MQTT_APP_EXT
#else
#define MQTT_APP_EXT    extern
#endif


MQTT_APP_EXT void mqtt_app_init(void);

#endif
