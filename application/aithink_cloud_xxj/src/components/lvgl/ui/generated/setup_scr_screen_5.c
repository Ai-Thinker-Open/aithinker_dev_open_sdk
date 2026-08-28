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

void level_color_screen(uint8_t level)
{
	switch(level)
	{
		case 0:
		{
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_1, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_3, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
		}
		break;
		case 1:
		{
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_1, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_2, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_3, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
		}
		break;
		case 2:
		{
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_2, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
			lv_obj_set_style_bg_color(guider_ui.screen_5_btn_3, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
		}
		break;
		default:
		break;
	}
}

static void screen_switch1_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		if(Fragrance.switch_control){
			Fragrance.spray_size = 2;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
			aiio_report_int_attibute_status(NULL, NULL, 2, Fragrance.spray_size);
			level_color_screen(Fragrance.spray_size);
		}
		break;
	default:
		break;
	}
}

static void screen_switch2_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		if(Fragrance.switch_control){
			Fragrance.spray_size = 0;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
			aiio_report_int_attibute_status(NULL, NULL, 2, Fragrance.spray_size);
			level_color_screen(Fragrance.spray_size);
		}
		break;
	default:
		break;
	}
}

static void screen_switch3_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		if(Fragrance.switch_control){
			Fragrance.spray_size = 1;
			aiio_pwm_control(Fragrance.spray_size);
    		aiio_flash_save_device_params(&Fragrance);
			aiio_report_int_attibute_status(NULL, NULL, 2, Fragrance.spray_size);
			level_color_screen(Fragrance.spray_size);
		}
		break;
	default:
		break;
	}
}

void setup_scr_screen_5()
{
	//Write codes screen_5
	guider_ui.screen_5 = lv_obj_create(NULL);
	lv_obj_set_size(guider_ui.screen_5, 320, 240);
	lv_obj_set_scrollbar_mode(guider_ui.screen_5, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_5, lv_color_hex(0x00e0ff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_5_img_1
	guider_ui.screen_5_img_1 = lv_img_create(guider_ui.screen_5);
	lv_obj_add_flag(guider_ui.screen_5_img_1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(guider_ui.screen_5_img_1, &_xxj_alpha_100x85);
	lv_img_set_pivot(guider_ui.screen_5_img_1, 50,50);
	lv_img_set_angle(guider_ui.screen_5_img_1, 0);
	lv_obj_set_pos(guider_ui.screen_5_img_1, 110, 18);
	lv_obj_set_size(guider_ui.screen_5_img_1, 100, 85);
	lv_obj_set_scrollbar_mode(guider_ui.screen_5_img_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_5_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(guider_ui.screen_5_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_5_tileview_1
	guider_ui.screen_5_tileview_1 = lv_tileview_create(guider_ui.screen_5);
	guider_ui.screen_5_tileview_1_tile_1 = lv_tileview_add_tile(guider_ui.screen_5_tileview_1, 0, 0, LV_DIR_RIGHT);
	lv_obj_set_pos(guider_ui.screen_5_tileview_1, 13, 127);
	lv_obj_set_size(guider_ui.screen_5_tileview_1, 294, 102);
	lv_obj_set_scrollbar_mode(guider_ui.screen_5_tileview_1, LV_SCROLLBAR_MODE_ON);

	//Write style for screen_5_tileview_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_5_tileview_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_5_tileview_1, lv_color_hex(0xf6f6f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_5_tileview_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_5_tileview_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_5_tileview_1, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_5_tileview_1, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_5_tileview_1, lv_color_hex(0xeaeff3), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_5_tileview_1, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);



	//Write codes screen_5_label_2
	guider_ui.screen_5_label_2 = lv_label_create(guider_ui.screen_5_tileview_1_tile_1);
	lv_label_set_text(guider_ui.screen_5_label_2, "");
	lv_label_set_long_mode(guider_ui.screen_5_label_2, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(guider_ui.screen_5_label_2, 8, 10);
	lv_obj_set_size(guider_ui.screen_5_label_2, 43, 17);
	lv_obj_set_scrollbar_mode(guider_ui.screen_5_label_2, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_5_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_5_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_5_label_2, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(guider_ui.screen_5_label_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_5_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_5_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_5_btn_1
	guider_ui.screen_5_btn_1 = lv_btn_create(guider_ui.screen_5_tileview_1_tile_1);
	guider_ui.screen_5_btn_1_label = lv_label_create(guider_ui.screen_5_btn_1);
	lv_label_set_text(guider_ui.screen_5_btn_1_label, "大雾");
	lv_label_set_long_mode(guider_ui.screen_5_btn_1_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_5_btn_1_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_5_btn_1, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_5_btn_1, 12, 38);
	lv_obj_set_size(guider_ui.screen_5_btn_1, 75, 29);
	lv_obj_set_scrollbar_mode(guider_ui.screen_5_btn_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_5_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_5_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_5_btn_1, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_5_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_5_btn_1, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_5_btn_1, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_5_btn_1, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_5_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_5_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_5_btn_1, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_5_btn_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_5_btn_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_5_btn_1, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_5_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_5_btn_1, screen_switch1_event_handler, LV_EVENT_ALL, NULL);

	//Write codes screen_5_btn_2
	guider_ui.screen_5_btn_2 = lv_btn_create(guider_ui.screen_5_tileview_1_tile_1);
	guider_ui.screen_5_btn_2_label = lv_label_create(guider_ui.screen_5_btn_2);
	lv_label_set_text(guider_ui.screen_5_btn_2_label, "小雾");
	lv_label_set_long_mode(guider_ui.screen_5_btn_2_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_5_btn_2_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_5_btn_2, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_5_btn_2, 209, 38);
	lv_obj_set_size(guider_ui.screen_5_btn_2, 75, 29);
	lv_obj_set_scrollbar_mode(guider_ui.screen_5_btn_2, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_5_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_5_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_5_btn_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_5_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_5_btn_2, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_5_btn_2, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_5_btn_2, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_5_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_5_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_5_btn_2, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_5_btn_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_5_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_5_btn_2, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_5_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_5_btn_2, screen_switch2_event_handler, LV_EVENT_ALL, NULL);

	//Write codes screen_5_btn_3
	guider_ui.screen_5_btn_3 = lv_btn_create(guider_ui.screen_5_tileview_1_tile_1);
	guider_ui.screen_5_btn_3_label = lv_label_create(guider_ui.screen_5_btn_3);
	lv_label_set_text(guider_ui.screen_5_btn_3_label, "中雾");
	lv_label_set_long_mode(guider_ui.screen_5_btn_3_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_5_btn_3_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_5_btn_3, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_5_btn_3, 113, 38);
	lv_obj_set_size(guider_ui.screen_5_btn_3, 75, 29);
	lv_obj_set_scrollbar_mode(guider_ui.screen_5_btn_3, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_5_btn_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_5_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_5_btn_3, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_5_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_5_btn_3, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_5_btn_3, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_5_btn_3, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_5_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_5_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_5_btn_3, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_5_btn_3, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_5_btn_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_5_btn_3, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_5_btn_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_5_btn_3, screen_switch3_event_handler, LV_EVENT_ALL, NULL);

	//Update current screen layout.
	lv_obj_update_layout(guider_ui.screen_5);

	
	//Init events for screen.
	events_init_screen_5(&guider_ui);
}
