#ifndef __DEVICE_INIT_CONFIG_H__
#define __DEVICE_INIT_CONFIG_H__
#ifdef __cplusplus
extern "C" {
#endif

#define  PAIR_TIMEOUT_KEY "pair_timeout"

void Device_Initialize();
void Device_Restore();
#ifdef __cplusplus
}
#endif
#endif