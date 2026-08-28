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

static void screen_paring_event_handler(lv_event_t *e)
{
	aiio_rev_queue_t  rev_queue = {0};
	lv_event_code_t code = lv_event_get_code(e);
    switch (code)
	{
	case LV_EVENT_CLICKED:
		rev_queue.common_event = REV_CONFIG_START_EVENT;
		if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
		{
			printf("queue send failed\r\n");
		}
		reparing_flag = true;
		paring_succ_flag = false;
		aiio_TimerStart(paring_led_timer_handle, 250);
		memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
		break;
	default:
		break;
	}
}

static void screen_switch_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * target = lv_event_get_target(e);
	if(code == LV_EVENT_VALUE_CHANGED &&  lv_obj_has_state(target, LV_STATE_CHECKED)){
		Fragrance.switch_control = 1;
		aiio_pwm_control(Fragrance.spray_size);
    	aiio_flash_save_device_params(&Fragrance);
		aiio_report_bool_attibute_status(NULL, NULL, 1, Fragrance.switch_control);
	}else if(code == LV_EVENT_VALUE_CHANGED &&  !lv_obj_has_state(target, LV_STATE_CHECKED)){
		Fragrance.switch_control = 0;
		aiio_pwm_control(XXJ_SIZE_OFF);
    	aiio_flash_save_device_params(&Fragrance);
		aiio_report_bool_attibute_status(NULL, NULL, 1, Fragrance.switch_control);
	}
}

void setup_scr_screen_2(void)
{
	//Write codes screen_2
	guider_ui.screen_2 = lv_obj_create(NULL);
	lv_obj_set_size(guider_ui.screen_2, 320, 240);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2, lv_color_hex(0x00e0ff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_tileview_2
	guider_ui.screen_2_tileview_2 = lv_tileview_create(guider_ui.screen_2);
	guider_ui.screen_2_tileview_2_tile_1 = lv_tileview_add_tile(guider_ui.screen_2_tileview_2, 0, 0, LV_DIR_RIGHT);
	lv_obj_set_pos(guider_ui.screen_2_tileview_2, 14, 100);
	lv_obj_set_size(guider_ui.screen_2_tileview_2, 292, 119);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2_tileview_2, LV_SCROLLBAR_MODE_ON);

	//Write style for screen_2_tileview_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_tileview_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2_tileview_2, lv_color_hex(0xf6f6f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_tileview_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_2_tileview_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_2_tileview_2, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_tileview_2, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2_tileview_2, lv_color_hex(0xeaeff3), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_tileview_2, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);



	//Write codes screen_2_btn_2
	guider_ui.screen_2_btn_2 = lv_btn_create(guider_ui.screen_2_tileview_2_tile_1);
	guider_ui.screen_2_btn_2_label = lv_label_create(guider_ui.screen_2_btn_2);
	lv_label_set_text(guider_ui.screen_2_btn_2_label, "档位");
	lv_label_set_long_mode(guider_ui.screen_2_btn_2_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_2_btn_2_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_2_btn_2, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_2_btn_2, 105, 69);
	lv_obj_set_size(guider_ui.screen_2_btn_2, 80, 30);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2_btn_2, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_2_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2_btn_2, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_2_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_btn_2, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_2_btn_2, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_2_btn_2, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_2_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_2_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_2_btn_2, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_2_btn_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_2_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_2_btn_2, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_2_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_btn_3
	guider_ui.screen_2_btn_3 = lv_btn_create(guider_ui.screen_2_tileview_2_tile_1);
	guider_ui.screen_2_btn_3_label = lv_label_create(guider_ui.screen_2_btn_3);
	lv_label_set_text(guider_ui.screen_2_btn_3_label, "配网");
	lv_label_set_long_mode(guider_ui.screen_2_btn_3_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_2_btn_3_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_2_btn_3, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_2_btn_3, 202, 69);
	lv_obj_set_size(guider_ui.screen_2_btn_3, 80, 30);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2_btn_3, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_2_btn_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2_btn_3, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_2_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_btn_3, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_2_btn_3, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_2_btn_3, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_2_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_2_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_2_btn_3, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_2_btn_3, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_2_btn_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_2_btn_3, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_2_btn_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_2_btn_3, screen_paring_event_handler, LV_EVENT_ALL, NULL);

	//Write codes screen_2_label_1
	guider_ui.screen_2_label_1 = lv_label_create(guider_ui.screen_2_tileview_2_tile_1);
	lv_label_set_text(guider_ui.screen_2_label_1, "喷雾开关");
	lv_label_set_long_mode(guider_ui.screen_2_label_1, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(guider_ui.screen_2_label_1, 0, 10);
	lv_obj_set_size(guider_ui.screen_2_label_1, 100, 32);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2_label_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_2_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_2_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_2_label_1, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(guider_ui.screen_2_label_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_2_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_sw_1
	guider_ui.screen_2_sw_1 = lv_switch_create(guider_ui.screen_2_tileview_2_tile_1);
	lv_obj_set_pos(guider_ui.screen_2_sw_1, 217, 10);
	lv_obj_set_size(guider_ui.screen_2_sw_1, 55, 20);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2_sw_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_2_sw_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_sw_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2_sw_1, lv_color_hex(0x2FCADA), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_2_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_sw_1, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_2_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_2_sw_1, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_sw_1, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(guider_ui.screen_2_sw_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
	lv_obj_set_style_border_width(guider_ui.screen_2_sw_1, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

	//Write style for screen_2_sw_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_sw_1, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2_sw_1, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_2_sw_1, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_sw_1, 100, LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_obj_add_event_cb(guider_ui.screen_2_sw_1, screen_switch_event_handler, LV_EVENT_ALL, NULL);

	//Write codes screen_2_btn_4
	guider_ui.screen_2_btn_4 = lv_btn_create(guider_ui.screen_2_tileview_2_tile_1);
	guider_ui.screen_2_btn_4_label = lv_label_create(guider_ui.screen_2_btn_4);
	lv_label_set_text(guider_ui.screen_2_btn_4_label, "模式");
	lv_label_set_long_mode(guider_ui.screen_2_btn_4_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(guider_ui.screen_2_btn_4_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(guider_ui.screen_2_btn_4, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(guider_ui.screen_2_btn_4, 7, 69);
	lv_obj_set_size(guider_ui.screen_2_btn_4, 82, 30);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2_btn_4, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_2_btn_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_2_btn_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_2_btn_4, lv_color_hex(0x009ea9), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(guider_ui.screen_2_btn_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_2_btn_4, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_2_btn_4, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(guider_ui.screen_2_btn_4, lv_color_hex(0x0d4b3b), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(guider_ui.screen_2_btn_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(guider_ui.screen_2_btn_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(guider_ui.screen_2_btn_4, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(guider_ui.screen_2_btn_4, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_2_btn_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_2_btn_4, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_2_btn_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_img_1
	guider_ui.screen_2_img_1 = lv_img_create(guider_ui.screen_2);
	lv_obj_add_flag(guider_ui.screen_2_img_1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(guider_ui.screen_2_img_1, &_xxj_alpha_100x85);
	lv_img_set_pivot(guider_ui.screen_2_img_1, 50,50);
	lv_img_set_angle(guider_ui.screen_2_img_1, 0);
	lv_obj_set_pos(guider_ui.screen_2_img_1, 110, 8);
	lv_obj_set_size(guider_ui.screen_2_img_1, 100, 85);
	lv_obj_set_scrollbar_mode(guider_ui.screen_2_img_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_2_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(guider_ui.screen_2_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(guider_ui.screen_2);

	
	//Init events for screen.
	events_init_screen_2(&guider_ui);
}
