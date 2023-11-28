/**
 * @file     ln_drv_oled.c
 * @author   BSP Team 
 * @brief    OLED驱动文件
 * @version  0.0.0.1
 * @date     2021-11-04
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#include "ln_drv_oled.h"
#include "ln_oled_font.h"

/**
 * @brief OLED显存
 * 
 */
uint8_t OLED_GRAM[144][8];

/**
 * @brief OLED I2C地址
 * 
 */
#define OLED_ADDR 0x78


/**
 * @brief OLED写命令
 * 
 * @param cmd 要写入的命令
 */
void oled_write_command_byte(uint8_t cmd)
{
    unsigned char cmd_arr[2];
    cmd_arr[0] = 0x00;
    cmd_arr[1] = cmd;
    i2c_master_7bit_write(OLED_ADDR, cmd_arr, 2);
};

/**
 * @brief OLED写数据
 * 
 * @param data 要写入的数据
 */
void oled_write_data_byte(uint8_t data)
{
    unsigned char data_arr[2];
    data_arr[0] = 0x40;
    data_arr[1] = data;
    i2c_master_7bit_write(OLED_ADDR, data_arr, 2);
};


/**
 * @brief OLED反显
 * 
 * @param i 0：正常显示，1：反向显示 
 */
void oled_color_turn(uint8_t i)
{
    if (i == 0)
    {
        oled_wr_byte(0xA6, OLED_CMD); 
    }
    if (i == 1)
    {
        oled_wr_byte(0xA7, OLED_CMD); 
    }
}


/**
 * @brief OLED屏幕旋转180°
 * 
 * @param i 0：正常显示，1：旋转180°
 */
void oled_display_turn(uint8_t i)
{
    if (i == 0)
    {
        oled_wr_byte(0xC8, OLED_CMD); 
        oled_wr_byte(0xA1, OLED_CMD);
    }
    if (i == 1)
    {
        oled_wr_byte(0xC0, OLED_CMD); 
        oled_wr_byte(0xA0, OLED_CMD);
    }
}

/**
 * @brief 发送一个字节
 * 
 * @param dat  数据
 * @param mode 0：表示命令，1：表示数据
 */
void oled_wr_byte(uint8_t dat, uint8_t mode)
{
    if (mode)
    {
        oled_write_data_byte(dat);
    }
    else
    {
        oled_write_command_byte(dat);
    }
}

/**
 * @brief 开始显示
 * 
 */
void oled_display_on(void)
{
    oled_wr_byte(0x8D, OLED_CMD); 
    oled_wr_byte(0x14, OLED_CMD); 
    oled_wr_byte(0xAF, OLED_CMD); 
}

/**
 * @brief 关闭显示
 * 
 */
void oled_display_off(void)
{
    oled_wr_byte(0x8D, OLED_CMD); 
    oled_wr_byte(0x10, OLED_CMD); 
    oled_wr_byte(0xAF, OLED_CMD); 
}

/**
 * @brief 刷屏
 * 
 */
void oled_refresh(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        oled_wr_byte(0xb0 + i, OLED_CMD); 
        oled_wr_byte(0x00, OLED_CMD);     
        oled_wr_byte(0x10, OLED_CMD);     
        for (n = 0; n < 128; n++)
            oled_wr_byte(OLED_GRAM[n][i], OLED_DATA);
    }
}

/**
 * @brief 清屏
 * 
 */
void oled_clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        for (n = 0; n < 128; n++)
        {
            OLED_GRAM[n][i] = 0; 
        }
    }
    oled_refresh(); 
}

/**
 * @brief 在OLED上画点
 * 
 * @param x 横轴坐标
 * @param y 纵轴坐标
 */
void oled_draw_point(uint8_t x, uint8_t y)
{
    uint8_t i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    OLED_GRAM[x][i] |= n;
}

/**
 * @brief 清除OLED点
 * 
 * @param x 横轴坐标
 * @param y 纵轴坐标
 */
void oled_clear_point(uint8_t x, uint8_t y)
{
    uint8_t i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
    OLED_GRAM[x][i] |= n;
    OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
}

//画线
//x:0~128
//y:0~64
/**
 * @brief 画线
 * 
 * @param x1 坐标x1
 * @param y1 坐标y1
 * @param x2 坐标x2
 * @param y2 坐标y2
 */
void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    uint8_t i, k, k1, k2;
    if ((x2 > 128) || (y2 > 64) || (x1 > x2) || (y1 > y2))
        return;
    if (x1 == x2) 
    {
        for (i = 0; i < (y2 - y1); i++)
        {
            oled_draw_point(x1, y1 + i);
        }
    }
    else if (y1 == y2)
    {
        for (i = 0; i < (x2 - x1); i++)
        {
            oled_draw_point(x1 + i, y1);
        }
    }
    else 
    {
        k1 = y2 - y1;
        k2 = x2 - x1;
        k = k1 * 10 / k2;
        for (i = 0; i < (x2 - x1); i++)
        {
            oled_draw_point(x1 + i, y1 + i * k / 10);
        }
    }
}


/**
 * @brief  画圆
 * 
 * @param x 圆心x坐标
 * @param y 圆心y坐标
 * @param r 半径
 */
void oled_draw_circle(uint8_t x, uint8_t y, uint8_t r)
{
    int a, b, num;
    a = 0;
    b = r;
    while (2 * b * b >= r * r)
    {
        oled_draw_point(x + a, y - b);
        oled_draw_point(x - a, y - b);
        oled_draw_point(x - a, y + b);
        oled_draw_point(x + a, y + b);

        oled_draw_point(x + b, y + a);
        oled_draw_point(x + b, y - a);
        oled_draw_point(x - b, y - a);
        oled_draw_point(x - b, y + a);

        a++;
        num = (a * a + b * b) - r * r; 
        if (num > 0)
        {
            b--;
            a--;
        }
    }
}

/**
 * @brief 显示字符
 * 
 * @param x x:0~127
 * @param y y:0~63
 * @param chr 要显示的字符
 * @param size1 size:12/16/24
 */
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1)
{
    uint8_t i, m, temp, size2, chr1;
    uint8_t y0 = y;
    size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2); 
    chr1 = chr - ' ';                                           
    for (i = 0; i < size2; i++)
    {
        if (size1 == 12)
        {
            temp = asc2_1206[chr1][i];
        } 
        else if (size1 == 16)
        {
            temp = asc2_1608[chr1][i];
        } 
        else if (size1 == 24)
        {
            temp = asc2_2412[chr1][i];
        } 
        else
            return;
        for (m = 0; m < 8; m++) 
        {
            if (temp & 0x80)
                oled_draw_point(x, y);
            else
                oled_clear_point(x, y);
            temp <<= 1;
            y++;
            if ((y - y0) == size1)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief 显示字符串
 * 
 * @param x x:0~127
 * @param y y:0~63
 * @param chr 要显示的字符串
 * @param size1 size:12/16/24
 */
void oled_show_string(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1)
{
    while ((*chr >= ' ') && (*chr <= '~')) 
    {
        oled_show_char(x, y, *chr, size1);
        x += size1 / 2;
        if (x > 128 - size1) 
        {
            x = 0;
            y += 2;
        }
        chr++;
    }
}

/**
 * @brief 显示字符串
 * 
 * @param x x:0~127
 * @param y y y:0~63
 * @param chr 要显示的字符串
 * @param size1 size:12/16/24
 * @param str_len 字符串长度
 */
void oled_show_string_with_len(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1,uint8_t str_len)
{
    while (str_len--) 
    {
        if(*chr == 0)
            *chr = ' ';
        oled_show_char(x, y, *chr, size1);
        x += size1 / 2;
        if (x > 128 - size1) 
        {
            x = 0;
            y += 2;
        }
        chr++;
    }
}


uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--)
    {
        result *= m;
    }
    return result;
}

/**
 * @brief 显示数字
 * 
 * @param x 坐标x
 * @param y 坐标y
 * @param num 要显示的数字
 * @param len 数字位数
 * @param size1 数字大小（高度和宽度）
 */
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1)
{
    uint8_t t, temp;
    for (t = 0; t < len; t++)
    {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;
        if (temp == 0)
        {
            oled_show_char(x + (size1 / 2) * t, y, '0', size1);
        }
        else
        {
            oled_show_char(x + (size1 / 2) * t, y, temp + '0', size1);
        }
    }
}

//显示汉字
void oled_show_chinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1)
{
    uint8_t i, m, n = 0, temp, chr1;
    uint8_t x0 = x, y0 = y;
    uint8_t size3 = size1 / 8;
    while (size3--)
    {
        chr1 = num * size1 / 8 + n;
        n++;
        for (i = 0; i < size1; i++)
        {
            if (size1 == 16)
            {
                temp = Hzk1[chr1][i];
            } //
            else if (size1 == 24)
            {
                temp = Hzk2[chr1][i];
            } //
            else if (size1 == 32)
            {
                temp = Hzk3[chr1][i];
            } //
            else if (size1 == 64)
            {
                temp = Hzk4[chr1][i];
            } //
            else
                return;

            for (m = 0; m < 8; m++)
            {
                if (temp & 0x01)
                    oled_draw_point(x, y);
                else
                    oled_clear_point(x, y);
                temp >>= 1;
                y++;
            }
            x++;
            if ((x - x0) == size1)
            {
                x = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
}

//num 
//space 
void oled_scroll_display(uint8_t num, uint8_t space)
{
    uint8_t i, n, t = 0, m = 0;
    while (1)
    {
        if (m == 0)
        {
            oled_show_chinese(128, 24, t, 16); 
            t++;
        }
        if (t == num)
        {
            {
                for (i = 0; i < 144; i++)
                {
                    for (n = 0; n < 8; n++)
                    {
                        OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
                    }
                }
                oled_refresh();
            }
            t = 0;
        }
        m++;
        if (m == 16)
        {
            m = 0;
        }
        for (i = 0; i < 144; i++) //
        {
            for (n = 0; n < 8; n++)
            {
                OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
            }
        }
        oled_refresh();
    }
}

void oled_wr_bp(uint8_t x, uint8_t y)
{
    oled_wr_byte(0xb0 + y, OLED_CMD); 
    oled_wr_byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    oled_wr_byte((x & 0x0f) | 0x01, OLED_CMD);
}

/**
 * @brief OLED显示图片
 * 
 * @param x0 坐标x0
 * @param y0 坐标y0
 * @param x1 坐标x1
 * @param y1 坐标y1
 * @param BMP 图片指针
 */
void oled_show_picture(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[])
{
    uint32_t j = 0;
    uint8_t x = 0, y = 0;
    if (y % 8 == 0)
        y = 0;
    else
        y += 1;
    for (y = y0; y < y1; y++)
    {
        oled_wr_bp(x0, y);
        for (x = x0; x < x1; x++)
        {
            oled_wr_byte(BMP[j], OLED_DATA);
            j++;
        }
    }
}

/**
 * @brief 延时
 * 
 */
void delay(void)
{
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 1000; j++)
            ;
}

/**
 * @brief OLED初始化
 * 
 */
void oled_init(void)
{
    oled_wr_byte(0xAE, OLED_CMD); //--turn off oled panel
    oled_wr_byte(0x00, OLED_CMD); //---set low column address
    oled_wr_byte(0x10, OLED_CMD); //---set high column address
    oled_wr_byte(0x40, OLED_CMD); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    oled_wr_byte(0x81, OLED_CMD); //--set contrast control register
    oled_wr_byte(0xCF, OLED_CMD); // Set SEG Output Current Brightness
    oled_wr_byte(0xA1, OLED_CMD); //--Set SEG/Column Mapping     0xa0×óÓÒ·´ÖÃ 0xa1Õý³£
    oled_wr_byte(0xC8, OLED_CMD); //Set COM/Row Scan Direction   0xc0ÉÏÏÂ·´ÖÃ 0xc8Õý³£
    oled_wr_byte(0xA6, OLED_CMD); //--set normal display
    oled_wr_byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
    oled_wr_byte(0x3f, OLED_CMD); //--1/64 duty
    oled_wr_byte(0xD3, OLED_CMD); //-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
    oled_wr_byte(0x00, OLED_CMD); //-not offset
    oled_wr_byte(0xd5, OLED_CMD); //--set display clock divide ratio/oscillator frequency
    oled_wr_byte(0x80, OLED_CMD); //--set divide ratio, Set Clock as 100 Frames/Sec
    oled_wr_byte(0xD9, OLED_CMD); //--set pre-charge period
    oled_wr_byte(0xF1, OLED_CMD); //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    oled_wr_byte(0xDA, OLED_CMD); //--set com pins hardware configuration
    oled_wr_byte(0x12, OLED_CMD);
    oled_wr_byte(0xDB, OLED_CMD); //--set vcomh
    oled_wr_byte(0x40, OLED_CMD); //Set VCOM Deselect Level
    oled_wr_byte(0x20, OLED_CMD); //-Set Page Addressing Mode (0x00/0x01/0x02)
    oled_wr_byte(0x02, OLED_CMD); //
    oled_wr_byte(0x8D, OLED_CMD); //--set Charge Pump enable/disable
    oled_wr_byte(0x14, OLED_CMD); //--set(0x10) disable
    oled_wr_byte(0xA4, OLED_CMD); // Disable Entire Display On (0xa4/0xa5)
    oled_wr_byte(0xA6, OLED_CMD); // Disable Inverse Display On (0xa6/a7)
    oled_wr_byte(0xAF, OLED_CMD);
    oled_clear();
}
