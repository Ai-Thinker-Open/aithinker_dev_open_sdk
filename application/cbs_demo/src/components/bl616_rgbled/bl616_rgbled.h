#ifndef __BL616_RGBLED_H_
#define __BL616_RGBLED_H_
#include "chip_include.h"
#ifdef __BL616_RGBLED_C_
#define BL616_RGBLED_EXT
#else
#define BL616_RGBLED_EXT extern
#endif

BL616_RGBLED_EXT void bl616_rgbled_init(void);
#endif
