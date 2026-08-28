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


void setup_scr_screen_4(lv_ui *ui)
{
	//Write codes screen_4
	ui->screen_4 = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen_4, 320, 240);
	lv_obj_set_scrollbar_mode(ui->screen_4, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_4, lv_color_hex(0x00e0ff), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_4_img_1
	ui->screen_4_img_1 = lv_img_create(ui->screen_4);
	lv_obj_add_flag(ui->screen_4_img_1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_4_img_1, &_xxj_alpha_100x100);
	lv_img_set_pivot(ui->screen_4_img_1, 50,50);
	lv_img_set_angle(ui->screen_4_img_1, 0);
	lv_obj_set_pos(ui->screen_4_img_1, 110, 45);
	lv_obj_set_size(ui->screen_4_img_1, 100, 100);
	lv_obj_set_scrollbar_mode(ui->screen_4_img_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_4_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_4_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_4_label_1
	ui->screen_4_label_1 = lv_label_create(ui->screen_4);
	lv_label_set_text(ui->screen_4_label_1, "配网成功");
	lv_label_set_long_mode(ui->screen_4_label_1, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_4_label_1, 62, 153);
	lv_obj_set_size(ui->screen_4_label_1, 197, 46);
	lv_obj_set_scrollbar_mode(ui->screen_4_label_1, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_4_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_4_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_4_label_1, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_4_label_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_4_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_4_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->screen_4);

	
	//Init events for screen.
	events_init_screen_4(ui);
}

void paring_succ_screen(void)
{
	lv_obj_t * act_scr = lv_scr_act();
	lv_disp_t * d = lv_obj_get_disp(act_scr);
	if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr)) {
		// if (guider_ui.screen_4_del == true) {
		// 	setup_scr_screen_4(&guider_ui);
		// }
		lv_scr_load_anim(guider_ui.screen_4, LV_SCR_LOAD_ANIM_NONE, 200, 200, false);
		guider_ui.screen_4_del = true;
	}
}
