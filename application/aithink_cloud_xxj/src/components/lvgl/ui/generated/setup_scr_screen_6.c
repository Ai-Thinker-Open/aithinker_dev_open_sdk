/*
* Copyright 2023 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include "aiio_common.h"

void mode_color_screen(uint8_t mode)
{
	switch(mode)
	{
		case 0:
		{
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_6, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_5, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_4, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_7, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
		}
		break;
		case 1:
		{
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_6, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_5, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_4, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_7, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
		}
		break;
		case 2:
		{
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_6, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_5, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_4, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_7, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
		}
		break;
		case 3:
		{
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_6, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_5, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_4, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_6_btn_7, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
		}
		break;
		default:
		break;
	}
}

static void screen_mode1_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		if(Fragrance.switch_control){
			Fragrance.work_mode = 0;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
			aiio_report_int_attibute_status(NULL, NULL, 50, Fragrance.work_mode);
			mode_color_screen(Fragrance.work_mode);
		}
		break;
	default:
		break;
	}
}

static void screen_mode2_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		if(Fragrance.switch_control){
			Fragrance.work_mode = 1;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
			aiio_report_int_attibute_status(NULL, NULL, 50, Fragrance.work_mode);
			mode_color_screen(Fragrance.work_mode);
		}
		break;
	default:
		break;
	}
}

static void screen_mode3_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		if(Fragrance.switch_control){
			Fragrance.work_mode = 2;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
			aiio_report_int_attibute_status(NULL, NULL, 50, Fragrance.work_mode);
			mode_color_screen(Fragrance.work_mode);
		}
		break;
	default:
		break;
	}
}

static void screen_mode4_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		if(Fragrance.switch_control){
			Fragrance.work_mode = 3;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
			aiio_report_int_attibute_status(NULL, NULL, 50, Fragrance.work_mode);
			mode_color_screen(Fragrance.work_mode);
		}
		break;
	default:
		break;
	}
}

void setup_scr_screen_6(void)
{
	//Write codes screen_6
	guider_ui.screen_6 = lv_obj_create(NULL);
	lv_obj_set_size(guider_ui.screen_6, 320, 240);
	lv_obj_set_scrollbar_mode(guider_ui.screen_6, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_6, lv_color_hex(0x00e0ff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_6_img_1
	guider_ui.screen_6_img_1 = lv_img_create(guider_ui.screen_6);
	lv_obj_add_flag(guider_ui.screen_6_img_1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(guider_ui.screen_6_img_1, &_xxj_alpha_100x85);
	lv_img_set_pivot(guider_ui.screen_6_img_1, 50,50);
	lv_img_set_angle(guider_ui.screen_6_img_1, 0);
	lv_obj_set_pos(guider_ui.screen_6_img_1, 110, 18);
	lv_obj_set_size(guider_ui.screen_6_img_1, 100, 85);
	lv_obj_set_scrollbar_mode(guider_ui.screen_6_img_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_6_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(guider_ui.screen_6_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_6_tileview_1
	guider_ui.screen_6_tileview_1 = lv_tileview_create(guider_ui.screen_6);
	guider_ui.screen_6_tileview_1_tile_1 = lv_tileview_add_tile(guider_ui.screen_6_tileview_1, 0, 0, LV_DIR_RIGHT);
	lv_obj_set_pos(guider_ui.screen_6_tileview_1, 13, 115);
	lv_obj_set_size(guider_ui.screen_6_tileview_1, 294, 109);
	lv_obj_set_scrollbar_mode(guider_ui.screen_6_tileview_1, LV_SCROLLBAR_MODE_ON);

	//Write style for screen_6_tileview_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_6_tileview_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_6_tileview_1, lv_color_hex(0xf6f6f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_6_tileview_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_6_tileview_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_6_tileview_1, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_6_tileview_1, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_6_tileview_1, lv_color_hex(0xeaeff3), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_6_tileview_1, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);



	//Write codes screen_6_btn_6
	guider_ui.screen_6_btn_6 = lv_btn_create(guider_ui.screen_6_tileview_1_tile_1);
	guider_ui.screen_6_btn_6_label = lv_label_create(guider_ui.screen_6_btn_6);
	lv_label_set_text(guider_ui.screen_6_btn_6_label, "模式1");
	lv_label_set_long_mode(guider_ui.screen_6_btn_6_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_6_btn_6_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_6_btn_6, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_6_btn_6, 34, 10);
	lv_obj_set_size(guider_ui.screen_6_btn_6, 75, 29);
	lv_obj_set_scrollbar_mode(guider_ui.screen_6_btn_6, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_6_btn_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_6_btn_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_6_btn_6, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_6_btn_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_6_btn_6, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_6_btn_6, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_6_btn_6, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_6_btn_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_6_btn_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_6_btn_6, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_6_btn_6, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_6_btn_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_6_btn_6, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_6_btn_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_6_btn_6, screen_mode1_event_handler, LV_EVENT_ALL, NULL);
	

	//Write codes screen_6_btn_5
	guider_ui.screen_6_btn_5 = lv_btn_create(guider_ui.screen_6_tileview_1_tile_1);
	guider_ui.screen_6_btn_5_label = lv_label_create(guider_ui.screen_6_btn_5);
	lv_label_set_text(guider_ui.screen_6_btn_5_label, "模式2");
	lv_label_set_long_mode(guider_ui.screen_6_btn_5_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_6_btn_5_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_6_btn_5, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_6_btn_5, 172, 13);
	lv_obj_set_size(guider_ui.screen_6_btn_5, 75, 29);
	lv_obj_set_scrollbar_mode(guider_ui.screen_6_btn_5, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_6_btn_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_6_btn_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_6_btn_5, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_6_btn_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_6_btn_5, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_6_btn_5, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_6_btn_5, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_6_btn_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_6_btn_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_6_btn_5, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_6_btn_5, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_6_btn_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_6_btn_5, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_6_btn_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_6_btn_5, screen_mode2_event_handler, LV_EVENT_ALL, NULL);

	//Write codes screen_6_btn_4
	guider_ui.screen_6_btn_4 = lv_btn_create(guider_ui.screen_6_tileview_1_tile_1);
	guider_ui.screen_6_btn_4_label = lv_label_create(guider_ui.screen_6_btn_4);
	lv_label_set_text(guider_ui.screen_6_btn_4_label, "模式3");
	lv_label_set_long_mode(guider_ui.screen_6_btn_4_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_6_btn_4_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_6_btn_4, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_6_btn_4, 34, 66);
	lv_obj_set_size(guider_ui.screen_6_btn_4, 75, 29);
	lv_obj_set_scrollbar_mode(guider_ui.screen_6_btn_4, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_6_btn_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_6_btn_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_6_btn_4, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_6_btn_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_6_btn_4, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_6_btn_4, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_6_btn_4, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_6_btn_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_6_btn_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_6_btn_4, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_6_btn_4, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_6_btn_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_6_btn_4, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_6_btn_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_6_btn_4, screen_mode3_event_handler, LV_EVENT_ALL, NULL);

	//Write codes screen_6_btn_7
	guider_ui.screen_6_btn_7 = lv_btn_create(guider_ui.screen_6_tileview_1_tile_1);
	guider_ui.screen_6_btn_7_label = lv_label_create(guider_ui.screen_6_btn_7);
	lv_label_set_text(guider_ui.screen_6_btn_7_label, "模式4");
	lv_label_set_long_mode(guider_ui.screen_6_btn_7_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_6_btn_7_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_6_btn_7, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_6_btn_7, 171, 68);
	lv_obj_set_size(guider_ui.screen_6_btn_7, 75, 29);
	lv_obj_set_scrollbar_mode(guider_ui.screen_6_btn_7, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_6_btn_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_6_btn_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_6_btn_7, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_6_btn_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_6_btn_7, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_6_btn_7, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_6_btn_7, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_6_btn_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_6_btn_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_6_btn_7, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_6_btn_7, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_6_btn_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_6_btn_7, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_6_btn_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_6_btn_7, screen_mode4_event_handler, LV_EVENT_ALL, NULL);

	//Update current screen layout.
	lv_obj_update_layout(guider_ui.screen_6);

	
	//Init events for screen.
	events_init_screen_6(&guider_ui);
}
