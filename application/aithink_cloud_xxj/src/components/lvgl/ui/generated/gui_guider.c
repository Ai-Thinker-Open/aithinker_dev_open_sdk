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
#include "aiio_rtc_timer.h"


void ui_init_style(lv_style_t * style)
{
  if (style->prop_cnt > 1)
    lv_style_reset(style);
  else
    lv_style_init(style);
}

void init_scr_del_flag(lv_ui *ui)
{
  
	ui->screen_1_del = true;
	ui->screen_2_del = true;
	ui->screen_3_del = true;
	ui->screen_4_del = true;
	ui->screen_5_del = true;
	ui->screen_6_del = true;
}

void setup_ui(lv_ui *ui)
{
  init_scr_del_flag(ui);
  setup_scr_screen_1();
  setup_scr_screen_2();
  setup_scr_screen_3(ui);
  setup_scr_screen_4(ui);
  setup_scr_screen_5();
  setup_scr_screen_6();
  lv_scr_load(ui->screen_1);
}
int compare_rtc_time(aiio_rtc_time_t *time1, aiio_rtc_time_t *time2) {  
    return time1->year != time2->year ||   
           time1->mon != time2->mon ||   
           time1->day != time2->day ||   
           time1->hour != time2->hour ||
           time1->minute != time2->minute ||
           0;  
} 

void update_time(aiio_rtc_time_t **last_rtc_time, aiio_rtc_time_t **current_rtc_time) {  
    // printf("11 the Date : %02d:%02d:%02d \r\n", (*current_rtc_time)->year, (*current_rtc_time)->mon, (*current_rtc_time)->day);
    // printf("11 the time : %02d:%02d:%02d \r\n", (*current_rtc_time)->hour, (*current_rtc_time)->minute, (*current_rtc_time)->second);
    // printf("11 the week : %02d \r\n", (*current_rtc_time)->week);
    if (*last_rtc_time == NULL) {  
        *last_rtc_time = (aiio_rtc_time_t*)malloc(sizeof(aiio_rtc_time_t));    
        memcpy(*last_rtc_time, *current_rtc_time, sizeof(aiio_rtc_time_t));
        // printf("22 the Date : %02d:%02d:%02d \r\n", (*last_rtc_time)->year, (*last_rtc_time)->mon, (*last_rtc_time)->day);
        // printf("22 the time : %02d:%02d:%02d \r\n", (*last_rtc_time)->hour, (*last_rtc_time)->minute, (*last_rtc_time)->second);
        // printf("22 the week : %02d \r\n", (*last_rtc_time)->week);
    } else {  
        // printf("33 the Date : %02d:%02d:%02d \r\n", (*last_rtc_time)->year, (*last_rtc_time)->mon, (*last_rtc_time)->day);
        // printf("33 the time : %02d:%02d:%02d \r\n", (*last_rtc_time)->hour, (*last_rtc_time)->minute, (*last_rtc_time)->second);
        // printf("33 the week : %02d \r\n", (*last_rtc_time)->week);
        if (compare_rtc_time(*current_rtc_time, *last_rtc_time)) {  
            //判断在哪个界面是否需要手动更新待做
            data_auto_updata();
            // lv_obj_t * act_scr = lv_scr_act();
            // lv_disp_t * d = lv_obj_get_disp(act_scr);
            // if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr)) {
            //     if (guider_ui.screen_1_del == true) {
            //       setup_scr_screen_1();
            //     }
            //     lv_scr_load_anim(guider_ui.screen_1, LV_SCR_LOAD_ANIM_NONE, 200, 200, true);
            //     guider_ui.screen_1_del = true;
            // }
            memcpy(*last_rtc_time, *current_rtc_time, sizeof(aiio_rtc_time_t)); 
            // printf("44 the Date : %02d:%02d:%02d \r\n", (*last_rtc_time)->year, (*last_rtc_time)->mon, (*last_rtc_time)->day);
            // printf("44 the time : %02d:%02d:%02d \r\n", (*last_rtc_time)->hour, (*last_rtc_time)->minute, (*last_rtc_time)->second);
            // printf("44 the week : %02d \r\n", (*last_rtc_time)->week);
        }  
    }  
} 
