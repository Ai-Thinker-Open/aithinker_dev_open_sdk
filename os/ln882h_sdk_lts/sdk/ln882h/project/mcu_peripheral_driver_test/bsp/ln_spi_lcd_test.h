/**
 * @file     ln_spi_lcd_test.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-09-24
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
 
#ifndef LN_SPI_LCD_TEST_H
#define LN_SPI_LCD_TEST_H

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

void ln_spi_lcd_test(void);

#endif
