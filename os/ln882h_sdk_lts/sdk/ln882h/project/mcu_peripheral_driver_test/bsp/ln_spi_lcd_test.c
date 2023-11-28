/**
 * @file     ln_spi_lcd_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-09-24
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

 
/**
        SPI LCD测试说明：
        
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
 
 

#include "hal/hal_spi.h"
#include "ln_test_common.h"
#include "ln_spi_lcd_test.h"
#include "hal/hal_gpio.h"
#include "hal/hal_dma.h"
#include "ln_spi_lcd_font.h"
#include "ln_spi_lcd_pic.h"


/* 配置SPI CS引脚 */
#define LN_SPI_LCD_TEST_CS_PORT_BASE    GPIOA_BASE
#define LN_SPI_LCD_TEST_CS_PIN          GPIO_PIN_10

#define LN_SPI_LCD_CS_LOW               hal_gpio_pin_reset(LN_SPI_LCD_TEST_CS_PORT_BASE,LN_SPI_LCD_TEST_CS_PIN)
#define LN_SPI_LCD_CS_HIGH              hal_gpio_pin_set(LN_SPI_LCD_TEST_CS_PORT_BASE,LN_SPI_LCD_TEST_CS_PIN)

#define LN_SPI_LCD_DC_LOW               hal_gpio_pin_reset(GPIOA_BASE,GPIO_PIN_7)
#define LN_SPI_LCD_DC_HIGH              hal_gpio_pin_set(GPIOA_BASE,GPIO_PIN_7)

#define LN_SPI_LCD_RST_LOW              hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_4)
#define LN_SPI_LCD_RST_HIGH             hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_4)


void ln_spi_lcd_spi_init()
{
    /*SPI INIT*/

    /* 1. 配置引脚*/
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_5,SPI0_CLK);
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_6,SPI0_MOSI);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_5,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_6,HAL_ENABLE);
    
    //配置CS引脚
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));
    gpio_init.dir = GPIO_OUTPUT;
    gpio_init.pin = LN_SPI_LCD_TEST_CS_PIN;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(LN_SPI_LCD_TEST_CS_PORT_BASE,&gpio_init);
    hal_gpio_pin_set(LN_SPI_LCD_TEST_CS_PORT_BASE,LN_SPI_LCD_TEST_CS_PIN);

    //配置DC引脚
    gpio_init.pin = GPIO_PIN_7;
    hal_gpio_init(GPIOA_BASE,&gpio_init);
    hal_gpio_pin_set(GPIOA_BASE,GPIO_PIN_7);

    //配置RST引脚
    gpio_init.pin = GPIO_PIN_4;
    hal_gpio_init(GPIOB_BASE,&gpio_init);
    hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_4);

    /* 2. 配置SPI */
    spi_init_type_def spi_init;
    memset(&spi_init,0,sizeof(spi_init));

    spi_init.spi_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_2;         //设置波特率
    spi_init.spi_mode      = SPI_MODE_MASTER;                           //设置主从模式
    spi_init.spi_data_size = SPI_DATASIZE_8B;                           //设置数据大小
    spi_init.spi_first_bit = SPI_FIRST_BIT_MSB;                         //设置帧格式
    spi_init.spi_cpol      = SPI_CPOL_LOW;                              //设置时钟极性
    spi_init.spi_cpha      = SPI_CPHA_1EDGE;                            //设置时钟相位
    hal_spi_init(SPI0_BASE,&spi_init);                                  //初始化SPI
    
    hal_spi_en(SPI0_BASE,HAL_ENABLE);                                   //SPI使能
    hal_spi_ssoe_en(SPI0_BASE,HAL_DISABLE);                             //关闭CS OUTPUT


    /* init the spi rx dma */
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));
    
    dma_init.dma_mem_addr = (uint32_t)0;            //设置DMA内存地址
    dma_init.dma_data_num = 0;                      //设置DMA传输次数
    dma_init.dma_dir = DMA_READ_FORM_P;             //设置DMA方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;       //设置DMA内存是否增长
    dma_init.dma_p_addr = SPI0_DATA_REG;            //设置DMA外设地址
    dma_init.dma_p_size = DMA_P_SIZE_16_BIT;
    dma_init.dma_mem_size = DMA_MEM_SIZE_16_BIT;
    hal_dma_init(DMA_CH_3,&dma_init);               //初始化DMA
    
    /* init the spi tx dma */
    memset(&dma_init,0,sizeof(dma_init));
    
    dma_init.dma_mem_addr = (uint32_t)0;            //设置DMA内存地址
    dma_init.dma_data_num = 0;                      //设置DMA传输次数
    dma_init.dma_dir = DMA_READ_FORM_MEM;           //设置DMA方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;       //设置DMA内存是否增长
    dma_init.dma_p_addr = SPI0_DATA_REG;            //设置DMA外设地址
    dma_init.dma_p_size = DMA_P_SIZE_16_BIT;
    dma_init.dma_mem_size = DMA_MEM_SIZE_16_BIT;
    hal_dma_init(DMA_CH_4,&dma_init);               //初始化DMA

    hal_dma_en(DMA_CH_4,HAL_DISABLE);                                   //DMA使能
    hal_dma_en(DMA_CH_3,HAL_DISABLE);                                   //DMA使能
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_RX_EN,HAL_DISABLE);                //SPI DMA RX 使能
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_DISABLE);                //SPI DMA TX 使能
}

void ln_spi_lcd_write_u8_data(uint8_t data) 
{	
    LN_SPI_LCD_CS_LOW;
    hal_spi_wait_bus_idle(SPI0_BASE,1000000);   
    if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
        hal_spi_send_data(SPI0_BASE,data);
    if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
        data = hal_spi_recv_data(SPI0_BASE);
    hal_spi_wait_bus_idle(SPI0_BASE,1000000);
    LN_SPI_LCD_CS_HIGH;
}

void ln_spi_lcd_write_u16_data(uint16_t data) 
{
    ln_spi_lcd_write_u8_data(data >> 8);
    ln_spi_lcd_write_u8_data(data);
}

void ln_spi_lcd_write_cmd(uint8_t cmd)
{
    LN_SPI_LCD_DC_LOW;
    ln_spi_lcd_write_u8_data(cmd);
    LN_SPI_LCD_DC_HIGH;
}

/**
 * @brief 设置起始和结束地址
 *        x1,x2 设置列的起始和结束地址
 *        y1,y2 设置行的起始和结束地址
 */
void ln_spi_lcd_set_addr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    ln_spi_lcd_write_cmd(0x2a);    //列地址设置
    ln_spi_lcd_write_u16_data(x1);
    ln_spi_lcd_write_u16_data(x2);
    ln_spi_lcd_write_cmd(0x2b);    //行地址设置
    ln_spi_lcd_write_u16_data(y1);
    ln_spi_lcd_write_u16_data(y2);
    ln_spi_lcd_write_cmd(0x2c);    //储存器写
}


void ln_spi_lcd_init(void)
{
	ln_spi_lcd_spi_init(); //初始化GPIO
	
	LN_SPI_LCD_RST_LOW;    //复位
	ln_delay_ms(100);
	LN_SPI_LCD_RST_HIGH;
	ln_delay_ms(100);
	
	//************* Start Initial Sequence **********//
	ln_spi_lcd_write_cmd(0x11);    //Sleep out 
	ln_delay_ms(120);              //Delay 120ms 
	//************* Start Initial Sequence **********// 
//------------------------------display and color format setting--------------------------------//
	
		ln_spi_lcd_write_cmd(0X36);// Memory Access Control
		if(USE_HORIZONTAL==0)ln_spi_lcd_write_u8_data(0x00);
		else if(USE_HORIZONTAL==1)ln_spi_lcd_write_u8_data(0xC0);
		else if(USE_HORIZONTAL==2)ln_spi_lcd_write_u8_data(0x70);
		else ln_spi_lcd_write_u8_data(0xA0);
		ln_spi_lcd_write_cmd(0X3A);
		ln_spi_lcd_write_u8_data(0X05);
	//--------------------------------ST7789S Frame rate setting-------------------------

		ln_spi_lcd_write_cmd(0xb2);
		ln_spi_lcd_write_u8_data(0x0c);
		ln_spi_lcd_write_u8_data(0x0c);
		ln_spi_lcd_write_u8_data(0x00);
		ln_spi_lcd_write_u8_data(0x33);
		ln_spi_lcd_write_u8_data(0x33);
		ln_spi_lcd_write_cmd(0xb7);
		ln_spi_lcd_write_u8_data(0x35);
		//---------------------------------ST7789S Power setting-----------------------------

		ln_spi_lcd_write_cmd(0xbb);
		ln_spi_lcd_write_u8_data(0x35);
		ln_spi_lcd_write_cmd(0xc0);
		ln_spi_lcd_write_u8_data(0x2c);
		ln_spi_lcd_write_cmd(0xc2);
		ln_spi_lcd_write_u8_data(0x01);
		ln_spi_lcd_write_cmd(0xc3);
		ln_spi_lcd_write_u8_data(0x13);
		ln_spi_lcd_write_cmd(0xc4);
		ln_spi_lcd_write_u8_data(0x20);
		ln_spi_lcd_write_cmd(0xc6);
		ln_spi_lcd_write_u8_data(0x0f);
		ln_spi_lcd_write_cmd(0xca);
		ln_spi_lcd_write_u8_data(0x0f);
		ln_spi_lcd_write_cmd(0xc8);
		ln_spi_lcd_write_u8_data(0x08);
		ln_spi_lcd_write_cmd(0x55);
		ln_spi_lcd_write_u8_data(0x90);
		ln_spi_lcd_write_cmd(0xd0);
		ln_spi_lcd_write_u8_data(0xa4);
		ln_spi_lcd_write_u8_data(0xa1);
		//--------------------------------ST7789S gamma setting------------------------------
		ln_spi_lcd_write_cmd(0xe0);
		ln_spi_lcd_write_u8_data(0xd0);
		ln_spi_lcd_write_u8_data(0x00);
		ln_spi_lcd_write_u8_data(0x06);
		ln_spi_lcd_write_u8_data(0x09);
		ln_spi_lcd_write_u8_data(0x0b);
		ln_spi_lcd_write_u8_data(0x2a);
		ln_spi_lcd_write_u8_data(0x3c);
		ln_spi_lcd_write_u8_data(0x55);
		ln_spi_lcd_write_u8_data(0x4b);
		ln_spi_lcd_write_u8_data(0x08);
		ln_spi_lcd_write_u8_data(0x16);
		ln_spi_lcd_write_u8_data(0x14);
		ln_spi_lcd_write_u8_data(0x19);
		ln_spi_lcd_write_u8_data(0x20);
		ln_spi_lcd_write_cmd(0xe1);
		ln_spi_lcd_write_u8_data(0xd0);
		ln_spi_lcd_write_u8_data(0x00);
		ln_spi_lcd_write_u8_data(0x06);
		ln_spi_lcd_write_u8_data(0x09);
		ln_spi_lcd_write_u8_data(0x0b);
		ln_spi_lcd_write_u8_data(0x29);
		ln_spi_lcd_write_u8_data(0x36);
		ln_spi_lcd_write_u8_data(0x54);
		ln_spi_lcd_write_u8_data(0x4b);
		ln_spi_lcd_write_u8_data(0x0d);
		ln_spi_lcd_write_u8_data(0x16);
		ln_spi_lcd_write_u8_data(0x14);
		ln_spi_lcd_write_u8_data(0x21);
		ln_spi_lcd_write_u8_data(0x20);
		ln_spi_lcd_write_cmd(0x29);
} 


void ln_spi_lcd_fill(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{          
    ln_spi_lcd_set_addr(x1,y1,x2-1,y2-1);

    hal_spi_set_data_size(SPI0_BASE,SPI_DATASIZE_16B);
    hal_dma_set_mem_size(DMA_CH_4,DMA_MEM_SIZE_16_BIT);
    hal_dma_set_p_size(DMA_CH_4,DMA_P_SIZE_16_BIT);
    
    if((x2-x1) * (y2-y1) > 65535)
    {
        //开始第一次传输，先拉低CS。
        LN_SPI_LCD_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)&color);
        hal_dma_set_data_num(DMA_CH_4,65535);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_DIS);

        
        //开始传输。
        hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_ENABLE);        
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
        LN_SPI_LCD_CS_HIGH;
                
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_4,HAL_DISABLE);	
        
        
        
        //开始第二次传输，先拉低CS。
        LN_SPI_LCD_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)&color);
        hal_dma_set_data_num(DMA_CH_4,(x2-x1) * (y2-y1) - 65535);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_DIS);
        
        //开始传输。
        hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_ENABLE);        
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
        LN_SPI_LCD_CS_HIGH;
                
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_4,HAL_DISABLE);	
    }
    else
    {
        //开始传输，先拉低CS。
        LN_SPI_LCD_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)&color);
        hal_dma_set_data_num(DMA_CH_4,(x2-x1) * (y2-y1));
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_DIS);
        
        //开始传输。
        hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_ENABLE);        
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
        LN_SPI_LCD_CS_HIGH;
                
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_4,HAL_DISABLE);	
    }
    
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_DISABLE);   	
    hal_spi_set_data_size(SPI0_BASE,SPI_DATASIZE_8B);    
}

void ln_spi_lcd_draw_point(uint16_t x,uint16_t y,uint16_t color)
{
	ln_spi_lcd_set_addr(x,y,x,y);//设置光标位置 
	ln_spi_lcd_write_u16_data(color);
} 


void ln_spi_lcd_draw_line(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		ln_spi_lcd_draw_point(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

void ln_spi_lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	ln_spi_lcd_draw_line(x1,y1,x2,y1,color);
	ln_spi_lcd_draw_line(x1,y1,x1,y2,color);
	ln_spi_lcd_draw_line(x1,y2,x2,y2,color);
	ln_spi_lcd_draw_line(x2,y1,x2,y2,color);
}

void ln_spi_lcd_draw_circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		ln_spi_lcd_draw_point(x0-b,y0-a,color);             //3           
		ln_spi_lcd_draw_point(x0+b,y0-a,color);             //0           
		ln_spi_lcd_draw_point(x0-a,y0+b,color);             //1                
		ln_spi_lcd_draw_point(x0-a,y0-b,color);             //2             
		ln_spi_lcd_draw_point(x0+b,y0+a,color);             //4               
		ln_spi_lcd_draw_point(x0+a,y0-b,color);             //5
		ln_spi_lcd_draw_point(x0+a,y0+b,color);             //6 
		ln_spi_lcd_draw_point(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}

///******************************************************************************
//      函数说明：显示汉字串
//      入口数据：x,y显示坐标
//                *s 要显示的汉字串
//                fc 字的颜色
//                bc 字的背景色
//                sizey 字号 可选 16 24 32
//                mode:  0非叠加模式  1叠加模式
//      返回值：  无
//******************************************************************************/
//void LCD_ShowChinese(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
//{
//	while(*s!=0)
//	{
//		if(sizey==12) ln_spi_lcd_show_chinese_12x12(x,y,s,fc,bc,sizey,mode);
//		else if(sizey==16) ln_spi_lcd_show_chinese_16x16(x,y,s,fc,bc,sizey,mode);
//		else if(sizey==24) ln_spi_lcd_show_chinese_24x24(x,y,s,fc,bc,sizey,mode);
//		else if(sizey==32) ln_spi_lcd_show_chinese_32x32(x,y,s,fc,bc,sizey,mode);
//		else return;
//		s+=2;
//		x+=sizey;
//	}
//}

///******************************************************************************
//      函数说明：显示单个12x12汉字
//      入口数据：x,y显示坐标
//                *s 要显示的汉字
//                fc 字的颜色
//                bc 字的背景色
//                sizey 字号
//                mode:  0非叠加模式  1叠加模式
//      返回值：  无
//******************************************************************************/
//void ln_spi_lcd_show_chinese_12x12(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
//{
//	uint8_t i,j,m=0;
//	uint16_t k;
//	uint16_t HZnum;//汉字数目
//	uint16_t TypefaceNum;//一个字符所占字节大小
//	uint16_t x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	                         
//	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//统计汉字数目
//	for(k=0;k<HZnum;k++) 
//	{
//		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
//		{ 	
//			ln_spi_lcd_set_addr(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{	
//					if(!mode)//非叠加方式
//					{
//						if(tfont12[k].Msk[i]&(0x01<<j))ln_spi_lcd_write_u16_data(fc);
//						else ln_spi_lcd_write_u16_data(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//叠加方式
//					{
//						if(tfont12[k].Msk[i]&(0x01<<j))	ln_spi_lcd_draw_point(x,y,fc);//画一个点
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}				  	
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
//} 

///******************************************************************************
//      函数说明：显示单个16x16汉字
//      入口数据：x,y显示坐标
//                *s 要显示的汉字
//                fc 字的颜色
//                bc 字的背景色
//                sizey 字号
//                mode:  0非叠加模式  1叠加模式
//      返回值：  无
//******************************************************************************/
//void ln_spi_lcd_show_chinese_16x16(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
//{
//	uint8_t i,j,m=0;
//	uint16_t k;
//	uint16_t HZnum;//汉字数目
//	uint16_t TypefaceNum;//一个字符所占字节大小
//	uint16_t x0=x;
//  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//统计汉字数目
//	for(k=0;k<HZnum;k++) 
//	{
//		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
//		{ 	
//			ln_spi_lcd_set_addr(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{	
//					if(!mode)//非叠加方式
//					{
//						if(tfont16[k].Msk[i]&(0x01<<j))ln_spi_lcd_write_u16_data(fc);
//						else ln_spi_lcd_write_u16_data(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//叠加方式
//					{
//						if(tfont16[k].Msk[i]&(0x01<<j))	ln_spi_lcd_draw_point(x,y,fc);//画一个点
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}				  	
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
//} 


///******************************************************************************
//      函数说明：显示单个24x24汉字
//      入口数据：x,y显示坐标
//                *s 要显示的汉字
//                fc 字的颜色
//                bc 字的背景色
//                sizey 字号
//                mode:  0非叠加模式  1叠加模式
//      返回值：  无
//******************************************************************************/
//void ln_spi_lcd_show_chinese_24x24(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
//{
//	uint8_t i,j,m=0;
//	uint16_t k;
//	uint16_t HZnum;//汉字数目
//	uint16_t TypefaceNum;//一个字符所占字节大小
//	uint16_t x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//统计汉字数目
//	for(k=0;k<HZnum;k++) 
//	{
//		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
//		{ 	
//			ln_spi_lcd_set_addr(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{	
//					if(!mode)//非叠加方式
//					{
//						if(tfont24[k].Msk[i]&(0x01<<j))ln_spi_lcd_write_u16_data(fc);
//						else ln_spi_lcd_write_u16_data(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//叠加方式
//					{
//						if(tfont24[k].Msk[i]&(0x01<<j))	ln_spi_lcd_draw_point(x,y,fc);//画一个点
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}				  	
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
//} 

///******************************************************************************
//      函数说明：显示单个32x32汉字
//      入口数据：x,y显示坐标
//                *s 要显示的汉字
//                fc 字的颜色
//                bc 字的背景色
//                sizey 字号
//                mode:  0非叠加模式  1叠加模式
//      返回值：  无
//******************************************************************************/
//void ln_spi_lcd_show_chinese_32x32(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
//{
//	uint8_t i,j,m=0;
//	uint16_t k;
//	uint16_t HZnum;//汉字数目
//	uint16_t TypefaceNum;//一个字符所占字节大小
//	uint16_t x0=x;
//	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
//	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//统计汉字数目
//	for(k=0;k<HZnum;k++) 
//	{
//		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
//		{ 	
//			ln_spi_lcd_set_addr(x,y,x+sizey-1,y+sizey-1);
//			for(i=0;i<TypefaceNum;i++)
//			{
//				for(j=0;j<8;j++)
//				{	
//					if(!mode)//非叠加方式
//					{
//						if(tfont32[k].Msk[i]&(0x01<<j))ln_spi_lcd_write_u16_data(fc);
//						else ln_spi_lcd_write_u16_data(bc);
//						m++;
//						if(m%sizey==0)
//						{
//							m=0;
//							break;
//						}
//					}
//					else//叠加方式
//					{
//						if(tfont32[k].Msk[i]&(0x01<<j))	ln_spi_lcd_draw_point(x,y,fc);//画一个点
//						x++;
//						if((x-x0)==sizey)
//						{
//							x=x0;
//							y++;
//							break;
//						}
//					}
//				}
//			}
//		}				  	
//		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
//	}
//}


/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ln_spi_lcd_show_char(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	ln_spi_lcd_set_addr(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))ln_spi_lcd_write_u16_data(fc);
				else ln_spi_lcd_write_u16_data(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))ln_spi_lcd_draw_point(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}


/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ln_spi_lcd_show_string(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{         
	while(*p!='\0')
	{       
		ln_spi_lcd_show_char(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}


/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void ln_spi_lcd_show_intnum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	uint8_t sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				ln_spi_lcd_show_char(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	ln_spi_lcd_show_char(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/******************************************************************************
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void ln_spi_lcd_show_float(uint16_t x,uint16_t y,float num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp,sizex;
	uint16_t num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			ln_spi_lcd_show_char(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	ln_spi_lcd_show_char(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}


/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void ln_spi_lcd_show_picture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[])
{
    ln_spi_lcd_set_addr(x,y,x+length-1,y+width-1);

    hal_dma_set_mem_size(DMA_CH_4,DMA_MEM_SIZE_8_BIT);
    hal_dma_set_p_size(DMA_CH_4,DMA_P_SIZE_8_BIT);
    hal_spi_set_data_size(SPI0_BASE,SPI_DATASIZE_8B);    
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_ENABLE);  
    
    if( length * width * 2 > 65535)
    {
        //开始第一次传输，先拉低CS。
        LN_SPI_LCD_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)pic);
        hal_dma_set_data_num(DMA_CH_4,65535);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);

        
        //开始传输。
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
        LN_SPI_LCD_CS_HIGH;
                
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_4,HAL_DISABLE);	
        
        if((length * width) * 2 - 65535 > 65535)
        {
            //开始第二次传输，先拉低CS。
            LN_SPI_LCD_CS_LOW;
            
            //配置DMA传输参数。
            hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)(pic + 65535));
            hal_dma_set_data_num(DMA_CH_4,65535);
            hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);

            
            //开始传输。
            hal_dma_en(DMA_CH_4,HAL_ENABLE);
            
            //等待传输完成。
            while(hal_dma_get_data_num(DMA_CH_4) != 0);
            hal_spi_wait_bus_idle(SPI0_BASE,10000);
            
            LN_SPI_LCD_CS_HIGH;
                    
            //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
            hal_dma_en(DMA_CH_4,HAL_DISABLE);	
            
            
            //开始第三次传输，先拉低CS。
            LN_SPI_LCD_CS_LOW;
            
            //配置DMA传输参数。
            hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)(pic + 65535 * 2));
            hal_dma_set_data_num(DMA_CH_4,length * width * 2 - 65535 * 2);
            hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);

            
            //开始传输。
            hal_dma_en(DMA_CH_4,HAL_ENABLE);
            
            //等待传输完成。
            while(hal_dma_get_data_num(DMA_CH_4) != 0);
            hal_spi_wait_bus_idle(SPI0_BASE,10000);
            
            LN_SPI_LCD_CS_HIGH;
                    
            //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
            hal_dma_en(DMA_CH_4,HAL_DISABLE);	
        }
        else
        {
            //开始第二次传输，先拉低CS。
            LN_SPI_LCD_CS_LOW;
            
            //配置DMA传输参数。
            hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)(pic + 65535));
            hal_dma_set_data_num(DMA_CH_4,length * width * 2 - 65535);
            hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);

            
            //开始传输。
            hal_dma_en(DMA_CH_4,HAL_ENABLE);
            
            //等待传输完成。
            while(hal_dma_get_data_num(DMA_CH_4) != 0);
            hal_spi_wait_bus_idle(SPI0_BASE,10000);
            
            LN_SPI_LCD_CS_HIGH;
                    
            //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
            hal_dma_en(DMA_CH_4,HAL_DISABLE);	
        } 
    }
    else
    {
        //开始传输，先拉低CS。
        LN_SPI_LCD_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)pic);
        hal_dma_set_data_num(DMA_CH_4,length * width * 2);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);

        
        //开始传输。
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
        LN_SPI_LCD_CS_HIGH;
                
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_4,HAL_DISABLE);	
    }
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_DISABLE);   	
}





void ln_spi_lcd_test()
{
    ln_spi_lcd_init();
    
    ln_spi_lcd_fill(0,0,240,320,WHITE);
    
    ln_spi_lcd_show_picture(0,0,240,233,gImage_pic);
    
    ln_spi_lcd_show_string(0,240,(const uint8_t *)"Lightning Semi",BLACK,WHITE,32,0);
    
    ln_spi_lcd_show_string(0,280,(const uint8_t *)"LN882H DEMO",BLACK,WHITE,32,0);
    
    while(1)
    {
        ln_delay_ms(1000);
    }
}

