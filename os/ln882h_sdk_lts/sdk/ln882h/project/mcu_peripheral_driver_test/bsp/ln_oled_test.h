#ifndef LN_OLED_TEST_H
#define LN_OLED_TEST_H 

#include "hal/hal_common.h"

#define OLED_CMD  0    
#define OLED_DATA 1    

void ln_oled_clear_point(uint8_t x,uint8_t y);
void ln_oled_color_turn(uint8_t i);
void ln_oled_display_turn(uint8_t i);

void ln_oled_send_byte(uint8_t dat);
void ln_oled_wr_byte(uint8_t dat,uint8_t mode);
void ln_oled_display_on(void);
void ln_oled_display_off(void);
void ln_oled_refresh(void);
void ln_oled_clear(void);
void ln_oled_draw_point(uint8_t x,uint8_t y);
void ln_oled_draw_line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);
void ln_oled_draw_circle(uint8_t x,uint8_t y,uint8_t r);
void ln_oled_show_char(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1);
void ln_oled_show_string(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1);
void ln_oled_show_string_with_len(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1,uint8_t str_len);
void ln_oled_show_num(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1);
void ln_oled_show_chinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1);
void ln_oled_scroll_display(uint8_t num,uint8_t space);
void ln_oled_wr_bp(uint8_t x,uint8_t y);
void ln_oled_show_picture(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[]);
void ln_oled_init(uint32_t base);

#endif
