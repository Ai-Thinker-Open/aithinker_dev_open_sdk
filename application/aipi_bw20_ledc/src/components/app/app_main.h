#ifndef __APP_MAIN_H_
#define __APP_MAIN_H_
#include "chip_include.h"
#include "app_include.h"
#include "drive_include.h"
#include "interface_include.h"
#include "algorithm_include.h"
#ifdef __APP_MAIN_C_
#define APP_MAIN_EXT
#else
#define APP_MAIN_EXT extern
#endif

APP_MAIN_EXT void app_main(void *para);

#endif
