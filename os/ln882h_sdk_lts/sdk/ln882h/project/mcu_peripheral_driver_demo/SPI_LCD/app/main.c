/**
 * @file     main.c
 * @author   BSP Team 
 * @brief    
 * @version  0.0.0.1
 * @date     2021-08-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "hal/hal_common.h"
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"


#include "ln_drv_spi_lcd.h"
#include "ln_spi_lcd_pic.h"


/*
        SPI LCD驱动说明：
        
        1. 接线说明：
                        LN8210              LCD
                        PA10        ->      CS
                        PA7         ->      DC
                        PB4         ->      RES
                        PA6         ->      SDA
                        PA5         ->      SCL
                        3.3v        ->      VCC
                        GND         ->      GND
*/
int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. spi flash 测试***********************/
    spi_lcd_init();

    spi_lcd_fill(0, 0, 240, 320, WHITE);

    spi_lcd_show_picture(0, 0, 240, 233, gImage_pic);

    spi_lcd_show_string(0, 240, (const uint8_t *)"Lightning Semi", BLACK, WHITE, 32, 0);

    spi_lcd_show_string(0, 280, (const uint8_t *)"LN882H DEMO", BLACK, WHITE, 32, 0);
    
    while(1)
    {
        LOG(LOG_LVL_INFO,"LN882H is running! \n");
        ln_delay_ms(1000);
    }
}
