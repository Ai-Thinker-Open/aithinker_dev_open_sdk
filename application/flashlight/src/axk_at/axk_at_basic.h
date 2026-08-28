#ifndef _AXK_AT_BASIC_H_
#define _AXK_AT_BASIC_H_

#include <stdint.h>

void axk_enter_sleep(void);
int at_exe_sleep_cmd(uint32_t argc, const char **argv);
int at_query_rssi_cmd(uint32_t argc, const char **argv);

#endif //_AXK_AT_BASIC_H_
