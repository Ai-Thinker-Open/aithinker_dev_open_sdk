/*
* Copyright 2023 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
  
	lv_obj_t *screen_1;
	bool screen_1_del;
	lv_obj_t *screen_1_label_1;
	lv_obj_t *screen_1_label_2;
	lv_obj_t *screen_1_label_3;
	lv_obj_t *screen_2;
	bool screen_2_del;
	lv_obj_t *screen_2_tileview_2;
	lv_obj_t *screen_2_tileview_2_tile_1;
	lv_obj_t *screen_2_btn_2;
	lv_obj_t *screen_2_btn_2_label;
	lv_obj_t *screen_2_btn_3;
	lv_obj_t *screen_2_btn_3_label;
	lv_obj_t *screen_2_label_1;
	lv_obj_t *screen_2_sw_1;
	lv_obj_t *screen_2_btn_4;
	lv_obj_t *screen_2_btn_4_label;
	lv_obj_t *screen_2_img_1;
	lv_obj_t *screen_3;
	bool screen_3_del;
	lv_obj_t *screen_3_img_1;
	lv_obj_t *screen_3_label_2;
	lv_obj_t *screen_4;
	bool screen_4_del;
	lv_obj_t *screen_4_img_1;
	lv_obj_t *screen_4_label_1;
	lv_obj_t *screen_5;
	bool screen_5_del;
	lv_obj_t *screen_5_img_1;
	lv_obj_t *screen_5_tileview_1;
	lv_obj_t *screen_5_tileview_1_tile_1;
	lv_obj_t *screen_5_label_2;
	lv_obj_t *screen_5_btn_1;
	lv_obj_t *screen_5_btn_1_label;
	lv_obj_t *screen_5_btn_2;
	lv_obj_t *screen_5_btn_2_label;
	lv_obj_t *screen_5_btn_3;
	lv_obj_t *screen_5_btn_3_label;
	lv_obj_t *screen_6;
	bool screen_6_del;
	lv_obj_t *screen_6_img_1;
	lv_obj_t *screen_6_tileview_1;
	lv_obj_t *screen_6_tileview_1_tile_1;
	lv_obj_t *screen_6_btn_6;
	lv_obj_t *screen_6_btn_6_label;
	lv_obj_t *screen_6_btn_5;
	lv_obj_t *screen_6_btn_5_label;
	lv_obj_t *screen_6_btn_4;
	lv_obj_t *screen_6_btn_4_label;
	lv_obj_t *screen_6_btn_7;
	lv_obj_t *screen_6_btn_7_label;
}lv_ui;

void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
void data_auto_updata(void);
extern lv_ui guider_ui;
extern char screen_time[15];
extern char screen_data[30];

void setup_scr_screen_1(void);
void setup_scr_screen_2(void);
void setup_scr_screen_3(lv_ui *ui);
void setup_scr_screen_4(lv_ui *ui);
void setup_scr_screen_5(void);
void setup_scr_screen_6(void);
LV_IMG_DECLARE(_xxj_alpha_100x85);
LV_IMG_DECLARE(_xxj_alpha_100x100);
LV_IMG_DECLARE(_xxj_alpha_100x100);
LV_IMG_DECLARE(_xxj_alpha_100x85);
LV_IMG_DECLARE(_xxj_alpha_100x85);

LV_FONT_DECLARE(lv_font_montserratMedium_100)
LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_simsun_20)
LV_FONT_DECLARE(lv_font_simsun_18)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_simsun_16)


#ifdef __cplusplus
}
#endif
#endif
