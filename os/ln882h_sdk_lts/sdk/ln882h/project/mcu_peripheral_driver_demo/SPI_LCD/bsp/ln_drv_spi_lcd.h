/**
 * @file     ln_drv_spi_lcd.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_SPI_LCD_H
#define __LN_DRV_SPI_LCD_H


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#define TEM_HUM_ADDR        0x88


#include "hal/hal_common.h"
#include "hal/hal_spi.h"
#include "hal/hal_gpio.h"

#include "ln_test_common.h"



#define USE_HORIZONTAL 0  


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 240
#define LCD_H 320

#else
#define LCD_W 320
#define LCD_H 240
#endif 

/* R     G      B      */
/* 00000 000000 00000  */

#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 
#define BRRED 			 0XFC07
#define GRAY  			 0X8430 
#define DARKBLUE      	 0X01CF
#define LIGHTBLUE      	 0X7D7C	
#define GRAYBLUE       	 0X5458 
#define LIGHTGREEN     	 0X841F 
#define LGRAY 			 0XC618 
#define LGRAYBLUE        0XA651 
#define LBBLUE           0X2B12 




void spi_lcd_init(void);
void spi_lcd_set_addr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void spi_lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void spi_lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void spi_lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void spi_lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void spi_lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
void spi_lcd_show_char(uint16_t x, uint16_t y, uint8_t c, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void spi_lcd_show_string(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void spi_lcd_show_intnum(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey);
void spi_lcd_show_float(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey);
void spi_lcd_show_picture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[]);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_SPI_LCD_H


