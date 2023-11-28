/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "guider_fonts.h"

typedef struct
{
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_label_1;
	lv_obj_t *screen_qrcode_1;
	// lv_obj_t *screen_digital_clock_1;
	lv_obj_t *screen_cpicker_1;
	lv_obj_t *screen_sw_1;
	lv_obj_t *screen_slider_1;
	lv_obj_t *screen_sw_2;
	lv_obj_t *screen_slider_2;
	lv_obj_t *screen_bar_1;
}lv_ui;

extern lv_ui guider_ui;
void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
void setup_scr_screen(lv_ui *ui);

#ifdef __cplusplus
}
#endif
#endif