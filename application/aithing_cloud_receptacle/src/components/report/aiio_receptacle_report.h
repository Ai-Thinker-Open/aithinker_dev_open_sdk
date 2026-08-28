#ifndef __AIIO_RECEPTACLE_REPORT_H_
#define __AIIO_RECEPTACLE_REPORT_H_
#include "aiio_adapter_include.h"
#include "aiio_common.h"
#include "config.h"
#include "aiio_lan_receive.h"
#include "aiio_at_receive.h"
#include "aiio_ap_config_wifi.h"
#include "aiio_ota_transmit.h"
#include "aiio_cloud_control.h"
#ifdef __AIIO_RECEPTACLE_REPORT_C_
#define AIIO_RECEPTACLE_REPORT_EXT
#else
#define AIIO_RECEPTACLE_REPORT_EXT extern
#endif

AIIO_RECEPTACLE_REPORT_EXT aiio_os_thread_handle_t *aiio_receptacle_report_thread;
AIIO_RECEPTACLE_REPORT_EXT aiio_os_thread_handle_t *aiio_receptacle_power_thread;
AIIO_RECEPTACLE_REPORT_EXT aiio_os_thread_handle_t *aiio_receptacle_voltage_sampling_thread;

AIIO_RECEPTACLE_REPORT_EXT void aiio_receptacle_report_status_set(void);
AIIO_RECEPTACLE_REPORT_EXT void aiio_receptacle_report_status_clr(void);
AIIO_RECEPTACLE_REPORT_EXT void aiio_receptacle_report_task(void *msg);
AIIO_RECEPTACLE_REPORT_EXT void aiio_receptacle_power_task(void *msg);
AIIO_RECEPTACLE_REPORT_EXT void aiio_receptacle_voltage_sampling_task(void *msg);

#endif
