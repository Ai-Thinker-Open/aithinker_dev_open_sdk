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


void setup_scr_screen_3(lv_ui *ui)
{
	//Write codes screen_3
	ui->screen_3 = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen_3, 320, 240);
	lv_obj_set_scrollbar_mode(ui->screen_3, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_3, lv_color_hex(0x00e0ff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_3_img_1
	ui->screen_3_img_1 = lv_img_create(ui->screen_3);
	lv_obj_add_flag(ui->screen_3_img_1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_3_img_1, &_xxj_alpha_100x100);
	lv_img_set_pivot(ui->screen_3_img_1, 50,50);
	lv_img_set_angle(ui->screen_3_img_1, 0);
	lv_obj_set_pos(ui->screen_3_img_1, 110, 48);
	lv_obj_set_size(ui->screen_3_img_1, 100, 100);
	lv_obj_set_scrollbar_mode(ui->screen_3_img_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_3_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_3_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_3_label_2
	ui->screen_3_label_2 = lv_label_create(ui->screen_3);
	lv_label_set_text(ui->screen_3_label_2, "配网中。。。");
	lv_label_set_long_mode(ui->screen_3_label_2, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_3_label_2, 62, 161);
	lv_obj_set_size(ui->screen_3_label_2, 197, 46);
	lv_obj_set_scrollbar_mode(ui->screen_3_label_2, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_3_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_3_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_3_label_2, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_3_label_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_3_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_3_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->screen_3);

	
	//Init events for screen.
	events_init_screen_3(ui);
}
