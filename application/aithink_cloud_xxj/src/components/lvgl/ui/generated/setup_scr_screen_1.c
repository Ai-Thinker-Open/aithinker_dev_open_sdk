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

char screen_time[15] = {0};
char screen_data[30] = {0};
char screen_week[9] = {0};

void week_turn_string(uint8_t week)
{
	switch(week)
	{
		case 0:
			strcpy(screen_week, "星期日");
		break;
		case 1:
			strcpy(screen_week, "星期一");
		break;
		case 2:
			strcpy(screen_week, "星期二");
		break;
		case 3:
			strcpy(screen_week, "星期三");
		break;
		case 4:
			strcpy(screen_week, "星期四");
		break;
		case 5:
			strcpy(screen_week, "星期五");
		break;
		case 6:
			strcpy(screen_week, "星期六");
		break;
		default:
		break;
	}
	// screen_week[sizeof(screen_week) - 1] = '\0';
}

void tnum_to_tstring(void){
	sprintf(screen_time,"%d:%02d",current_rtc_time->hour,current_rtc_time->minute);
	sprintf(screen_data,"%d年%d月%d日",current_rtc_time->year,current_rtc_time->mon,current_rtc_time->day);
	if(current_rtc_time->week >= 0 && current_rtc_time->week <= 6){
		week_turn_string(current_rtc_time->week);
	}
}

void data_auto_updata(void){
	tnum_to_tstring();
	lv_label_set_text(guider_ui.screen_1_label_1, screen_time);
	lv_label_set_text(guider_ui.screen_1_label_2, screen_data);
	lv_label_set_text(guider_ui.screen_1_label_3, screen_week);
}

void setup_scr_screen_1(void)
{
	tnum_to_tstring();
	//Write codes screen_1
	guider_ui.screen_1 = lv_obj_create(NULL);
	lv_obj_set_size(guider_ui.screen_1, 320, 240);
	lv_obj_set_scrollbar_mode(guider_ui.screen_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(guider_ui.screen_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(guider_ui.screen_1, lv_color_hex(0x00e0ff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_1
	guider_ui.screen_1_label_1 = lv_label_create(guider_ui.screen_1);
	// lv_label_set_text(ui->screen_1_label_1, "11:52");
	lv_label_set_text(guider_ui.screen_1_label_1, screen_time);
	lv_label_set_long_mode(guider_ui.screen_1_label_1, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(guider_ui.screen_1_label_1, 16, 18);
	lv_obj_set_size(guider_ui.screen_1_label_1, 286, 111);
	lv_obj_set_scrollbar_mode(guider_ui.screen_1_label_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_1_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_1_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_1_label_1, &lv_font_montserratMedium_100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(guider_ui.screen_1_label_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_1_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_1_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_2
	guider_ui.screen_1_label_2 = lv_label_create(guider_ui.screen_1);
	// lv_label_set_text(ui->screen_1_label_2, "2023年9月21日");
	lv_label_set_text(guider_ui.screen_1_label_2, screen_data);
	lv_label_set_long_mode(guider_ui.screen_1_label_2, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(guider_ui.screen_1_label_2, 16, 141);
	lv_obj_set_size(guider_ui.screen_1_label_2, 176, 30);
	lv_obj_set_scrollbar_mode(guider_ui.screen_1_label_2, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_1_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_1_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_1_label_2, &lv_font_simsun_20, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(guider_ui.screen_1_label_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_1_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_1_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_3
	guider_ui.screen_1_label_3 = lv_label_create(guider_ui.screen_1);
	// lv_label_set_text(ui->screen_1_label_3, "星期四");
	lv_label_set_text(guider_ui.screen_1_label_3, screen_week);
	lv_label_set_long_mode(guider_ui.screen_1_label_3, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(guider_ui.screen_1_label_3, 192, 138);
	lv_obj_set_size(guider_ui.screen_1_label_3, 100, 33);
	lv_obj_set_scrollbar_mode(guider_ui.screen_1_label_3, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_1_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(guider_ui.screen_1_label_3, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(guider_ui.screen_1_label_3, &lv_font_simsun_20, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(guider_ui.screen_1_label_3, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(guider_ui.screen_1_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(guider_ui.screen_1_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(guider_ui.screen_1);

	
	//Init events for screen.
	events_init_screen_1(&guider_ui);
}
