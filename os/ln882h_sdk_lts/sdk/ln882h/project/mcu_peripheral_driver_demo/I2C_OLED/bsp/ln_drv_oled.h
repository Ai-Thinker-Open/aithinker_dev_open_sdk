/**
 * @file     drv_oled.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-04
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __LN_DRV_OLED_H
#define __LN_DRV_OLED_H 

#include "hal/hal_common.h"
#include "ln_drv_i2c.h"


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#define OLED_CMD  0    
#define OLED_DATA 1    

void oled_clear_point(uint8_t x,uint8_t y);
void oled_color_turn(uint8_t i);
void oled_display_turn(uint8_t i);

void oled_send_byte(uint8_t dat);
void oled_wr_byte(uint8_t dat,uint8_t mode);
void oled_display_on(void);
void oled_display_off(void);
void oled_refresh(void);
void oled_clear(void);
void oled_draw_point(uint8_t x,uint8_t y);
void oled_draw_line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);
void oled_draw_circle(uint8_t x,uint8_t y,uint8_t r);
void oled_show_char(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1);
void oled_show_string(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1);
void oled_show_string_with_len(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1,uint8_t str_len);
void oled_show_num(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1);
void oled_show_chinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1);
void oled_scroll_display(uint8_t num,uint8_t space);
void oled_wr_bp(uint8_t x,uint8_t y);
void oled_show_picture(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[]);
void oled_init(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_OLED_H
