/**
 * @file     ln_spi_master_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-18
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/**
        SPI主机调试说明：
                    1. 接线说明：
                                LN882H          25WQ16

                                PA5    ->       SCLK(6)
                                PA10   ->       CS(1)
                                PA6    ->       MOSI(5)
                                PA7    ->       MISO(2)
                                VCC     ->      HOLD(7)
                                VCC     ->      WP(3)
                                GND     ->      GND(4)
                                VCC     ->      VCC(8)
                                
                                测试时推荐直接焊线，不要插杜邦线
                                
                    2. SPI测试分为两部分，一部分是SPI直接读写25WQ16 Flash芯片，另一部分是通过SPI + DMA读写Flash芯片，通过 LN_DMA_EN_STATUS  这个变量来选择是否使用DMA

                    3. 测试中SPI CS引脚使用GPIO引脚控制，这样更灵活，更好用,但是要注意的是，数据发送完成后，要判断busy位，来让硬件把数据发送出去在拉高CS!!!

                    4. CS引脚也可以由SPI_CR1中的NSS和SSM控制，对应控制函数为：

                        SSOE -> hal_spi_ssoe_en();            
                        SSM  -> hal_spi_set_nss(); 

                        通过这两个函数可以设置CS引脚状态：
                        
                        （1） SSM = 1：软件管理CS引脚，从器件的选择信息由SPI_CR1寄存器中得SSI位的值驱动。外部CS引脚可供他用，这个模式只适合从模式下使用。

                        （2） SSM = 0: 硬件管理CS引脚，根据SSOE的值，可以分为两种情况：（不推荐使用a模式，自行使用标准GPIO驱动SPI会使软件更灵活）

                            a. CS输出使能（SSOE = 1），主模式下才能使用此功能，当主机开始工作时，会把CS拉低，停止工作后恢复CS。
                             
                               当一个SPI设备需要发送广播数据，它必须拉低NSS信号，以通知所有其它的设备它是主设备；如果它不能拉低NSS，这意味着总线上有另外一个主设备在通信，则这个SPI设备进入主模式失败状态：即MSTR位被自动清除，此设备进入从模式.

                            b. CS输出失能（SSOE = 0），从模式下使用该模式，会在CS为低电平时选中该从器件，CS为高电平时，取消从器件的片选
                            
                    5. 测试成功会有LOG打印，请注意观察LOG.

*/

#include "ln_spi_master_test.h"
#include "hal/hal_spi.h"
#include "hal/hal_gpio.h"
#include "hal/hal_dma.h"
#include "hal/hal_misc.h"
#include "utils/debug/log.h"

/* 配置SPI CS引脚 */
#define LN_SPI_TEST_CS_PORT_BASE    GPIOA_BASE
#define LN_SPI_TEST_CS_PIN          GPIO_PIN_10

#define LN_SPI_CS_LOW               hal_gpio_pin_reset(LN_SPI_TEST_CS_PORT_BASE,LN_SPI_TEST_CS_PIN)
#define LN_SPI_CS_HIGH              hal_gpio_pin_set(LN_SPI_TEST_CS_PORT_BASE,LN_SPI_TEST_CS_PIN)


/* 初始化变量 */
unsigned char LN_DMA_EN_STATUS  = 0;            //是否使用DMA

static unsigned char tx_data[100];
static unsigned char rx_data[100];

static unsigned char status[2];
static unsigned int err_cnt = 0;


/* 函数声明 */
void hal_25wq16_read_flash(uint32_t spi_x_base , unsigned int addr ,unsigned char *data, unsigned int length);
void hal_25wq16_write_flash(uint32_t spi_x_base , unsigned int addr ,unsigned char *data, unsigned int length);
void hal_25wq16_erase_flash(uint32_t spi_x_base ,unsigned int add);
void hal_25wq16_read_status(unsigned int spi_x_base,unsigned char *status);
void hal_25wq16_read_id(unsigned int spi_x_base,unsigned char *id);
void hal_spi_read_data_with_addr(unsigned int spi_x_base,unsigned char *addr,unsigned int addr_len,unsigned char *rec_data,unsigned int data_len);
void hal_spi_write_data_with_addr(unsigned int spi_x_base,unsigned char *addr,unsigned int addr_len,unsigned char *send_data,unsigned int data_len);
void hal_spi_write_and_read_data(unsigned int spi_x_base,unsigned char *send_data,unsigned char *rec_data,unsigned int data_len);


void ln_spi_master_init()
{
    /* 1. 配置引脚 */
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_5,SPI0_CLK);
    //hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_10,SPI0_CSN);
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_6,SPI0_MOSI);
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_7,SPI0_MISO);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_5,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_10,HAL_DISABLE);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_6,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_7,HAL_ENABLE);
    
    //配置CS引脚
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));
    gpio_init.dir = GPIO_OUTPUT;
    gpio_init.pin = LN_SPI_TEST_CS_PIN;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(LN_SPI_TEST_CS_PORT_BASE,&gpio_init);
    hal_gpio_pin_set(LN_SPI_TEST_CS_PORT_BASE,LN_SPI_TEST_CS_PIN);

    /* 2. 配置SPI */
    spi_init_type_def spi_init;
    memset(&spi_init,0,sizeof(spi_init));

    spi_init.spi_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_2;       //设置波特率
    spi_init.spi_mode      = SPI_MODE_MASTER;                           //设置主从模式
    spi_init.spi_data_size = SPI_DATASIZE_8B;                           //设置数据大小
    spi_init.spi_first_bit = SPI_FIRST_BIT_MSB;                         //设置帧格式
    spi_init.spi_cpol      = SPI_CPOL_LOW;                              //设置时钟极性
    spi_init.spi_cpha      = SPI_CPHA_1EDGE;                            //设置时钟相位
    hal_spi_init(SPI0_BASE,&spi_init);                                  //初始化SPI
    
    hal_spi_en(SPI0_BASE,HAL_ENABLE);                                   //SPI使能
    hal_spi_ssoe_en(SPI0_BASE,HAL_DISABLE);                             //关闭CS OUTPUT
}


void ln_spi_dma_init(void)
{
    /* init the spi rx dma */
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));
    
    dma_init.dma_mem_addr = (uint32_t)rx_data;      //设置DMA内存地址
    dma_init.dma_data_num = 0;                      //设置DMA传输次数
    dma_init.dma_dir = DMA_READ_FORM_P;             //设置DMA方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;       //设置DMA内存是否增长
    dma_init.dma_p_addr = SPI0_DATA_REG;            //设置DMA外设地址
    hal_dma_init(DMA_CH_3,&dma_init);               //初始化DMA
    
    //hal_dma_it_cfg(DMA_CH_3,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);        //配置DMA中断
 
    hal_dma_en(DMA_CH_3,HAL_DISABLE);                                   //DMA使能
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_RX_EN,HAL_ENABLE);                 //SPI DMA RX 使能
    
    /* init the spi tx dma */
    memset(&dma_init,0,sizeof(dma_init));
    
    dma_init.dma_mem_addr = (uint32_t)tx_data;      //设置DMA内存地址
    dma_init.dma_data_num = 0;                      //设置DMA传输次数
    dma_init.dma_dir = DMA_READ_FORM_MEM;           //设置DMA方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;       //设置DMA内存是否增长
    dma_init.dma_p_addr = SPI0_DATA_REG;            //设置DMA外设地址
    hal_dma_init(DMA_CH_4,&dma_init);               //初始化DMA
    
    //hal_dma_it_cfg(DMA_CH_4,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);        //配置DMA中断
    
    hal_dma_en(DMA_CH_4,HAL_DISABLE);                                   //DMA使能
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_ENABLE);                 //SPI DMA TX 使能
}

void hal_spi_write_and_read_data_with_dma(unsigned int spi_x_base,unsigned char *send_data,unsigned char *rec_data,unsigned int data_len)
{
    //开始传输，先拉低CS。
    LN_SPI_CS_LOW;
    
    //配置DMA传输参数。
    hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data);
    hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)send_data);
    hal_dma_set_data_num(DMA_CH_3,data_len);
    hal_dma_set_data_num(DMA_CH_4,data_len);
    
    //开始传输。
    hal_dma_en(DMA_CH_3,HAL_ENABLE);
    hal_dma_en(DMA_CH_4,HAL_ENABLE);
    
    //等待传输完成。
    while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
    hal_spi_wait_bus_idle(SPI0_BASE,10000);
   
    //传输结束，拉高CS
    LN_SPI_CS_HIGH;
    
    //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
    hal_dma_en(DMA_CH_3,HAL_DISABLE);
    hal_dma_en(DMA_CH_4,HAL_DISABLE);
}


/* 25WQ16ES FLASH 芯片调试 */
void hal_spi_write_and_read_data(unsigned int spi_x_base,unsigned char *send_data,unsigned char *rec_data,unsigned int data_len)
{
    if(!LN_DMA_EN_STATUS)
    {
        LN_SPI_CS_LOW;
        for(int i = 0; i < data_len; i++)
        {
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
            
            if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
                hal_spi_send_data(SPI0_BASE,send_data[i]);
            if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
                rec_data[i] = hal_spi_recv_data(SPI0_BASE);
            
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        }
        LN_SPI_CS_HIGH;
    }
    else
    {
        //开始传输，先拉低CS。
        LN_SPI_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data);
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)send_data);
        hal_dma_set_data_num(DMA_CH_3,data_len);
        hal_dma_set_data_num(DMA_CH_4,data_len);
        hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
        
        //开始传输。
        hal_dma_en(DMA_CH_3,HAL_ENABLE);
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
       
        //传输结束，拉高CS
        LN_SPI_CS_HIGH;
        
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_3,HAL_DISABLE);
        hal_dma_en(DMA_CH_4,HAL_DISABLE);
    }
}

void hal_spi_write_data_with_addr(unsigned int spi_x_base,unsigned char *addr,unsigned int addr_len,unsigned char *send_data,unsigned int data_len)
{
    unsigned char rec_data_buffer[255];
    
    if(!LN_DMA_EN_STATUS)
    {
        LN_SPI_CS_LOW;
    
        for(int i = 0; i < addr_len; i++)
        {
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
            
            if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
                hal_spi_send_data(SPI0_BASE,addr[i]);
            if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
                rec_data_buffer[0] = hal_spi_recv_data(SPI0_BASE);
            
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        }
        for(int i = 0; i < data_len; i++)
        {
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
            
            if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
                hal_spi_send_data(SPI0_BASE,send_data[i]);
            if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
                rec_data_buffer[0] = hal_spi_recv_data(SPI0_BASE);
            
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        }
        LN_SPI_CS_HIGH;
    }
    else
    {
        //开始传输，先拉低CS。
        LN_SPI_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data_buffer);
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)addr);
        hal_dma_set_data_num(DMA_CH_3,addr_len);
        hal_dma_set_data_num(DMA_CH_4,addr_len);
        hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
        
        //开始传输。
        hal_dma_en(DMA_CH_3,HAL_ENABLE);
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
                
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_3,HAL_DISABLE);
        hal_dma_en(DMA_CH_4,HAL_DISABLE);
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data_buffer);
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)send_data);
        hal_dma_set_data_num(DMA_CH_3,data_len);
        hal_dma_set_data_num(DMA_CH_4,data_len);
        hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
        
        //开始传输。
        hal_dma_en(DMA_CH_3,HAL_ENABLE);
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_3,HAL_DISABLE);
        hal_dma_en(DMA_CH_4,HAL_DISABLE);        
       
        //传输结束，拉高CS
        LN_SPI_CS_HIGH;
        
    }
}

void hal_spi_read_data_with_addr(unsigned int spi_x_base,unsigned char *addr,unsigned int addr_len,unsigned char *rec_data,unsigned int data_len)
{
    unsigned char buffer[255];
    LN_SPI_CS_LOW;
    if(!LN_DMA_EN_STATUS)
    {
        for(int i = 0; i < addr_len; i++)
        {
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
            
            if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
                hal_spi_send_data(SPI0_BASE,addr[i]);
            if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
                buffer[0] = hal_spi_recv_data(SPI0_BASE);
            
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        }
        for(int i = 0; i < data_len; i++)
        {
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
            
            if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
                hal_spi_send_data(SPI0_BASE,0xFF);
            if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
                rec_data[i] = hal_spi_recv_data(SPI0_BASE);
            
            hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        }
        LN_SPI_CS_HIGH;
    }
    else
    {
        //开始传输，先拉低CS。
        LN_SPI_CS_LOW;
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data);
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)addr);
        hal_dma_set_data_num(DMA_CH_3,addr_len);
        hal_dma_set_data_num(DMA_CH_4,addr_len);
        hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
        
        //开始传输。
        hal_dma_en(DMA_CH_3,HAL_ENABLE);
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
                
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_3,HAL_DISABLE);
        hal_dma_en(DMA_CH_4,HAL_DISABLE);
        
        //配置DMA传输参数。
        hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data);
        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)buffer);
        hal_dma_set_data_num(DMA_CH_3,data_len);
        hal_dma_set_data_num(DMA_CH_4,data_len);
        hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
        hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
        //开始传输。
        hal_dma_en(DMA_CH_3,HAL_ENABLE);
        hal_dma_en(DMA_CH_4,HAL_ENABLE);
        
        //等待传输完成。
        while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
        hal_spi_wait_bus_idle(SPI0_BASE,10000);
        
        //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
        hal_dma_en(DMA_CH_3,HAL_DISABLE);
        hal_dma_en(DMA_CH_4,HAL_DISABLE);        
       
        //传输结束，拉高CS
        LN_SPI_CS_HIGH;
    }
}


void hal_25wq16_read_id(unsigned int spi_x_base,unsigned char *id)
{
    unsigned char CMD_READ_ID[4] = {0x9F,0xFF,0xFF,0xFF};
    unsigned char buffer[4];
    hal_spi_write_and_read_data(spi_x_base,CMD_READ_ID,buffer,4);
    memcpy(id,buffer+1,3);
}

void hal_25wq16_read_status(unsigned int spi_x_base,unsigned char *status)
{
    unsigned char CMD_READ_STATUS_1[2] = {0x05,0xFF};
    unsigned char CMD_READ_STATUS_2[2] = {0x35,0xFF};
    unsigned char buffer[2] = {0,0};
    hal_spi_write_and_read_data(spi_x_base,CMD_READ_STATUS_1,buffer,2);
    memcpy(status,buffer+1,1);
    hal_spi_write_and_read_data(spi_x_base,CMD_READ_STATUS_2,buffer,2);
    memcpy(status+1,buffer+1,1);
}

void hal_25wq16_write_enable(unsigned int spi_x_base)
{
    unsigned char CMD_WRITE_ENABLE[1] = {0x06};
    unsigned char buffer[1];
    hal_spi_write_and_read_data(spi_x_base,CMD_WRITE_ENABLE,buffer,1);
}
void hal_25wq16_erase_flash(uint32_t spi_x_base ,unsigned int add)
{
    hal_25wq16_write_enable(spi_x_base);
    unsigned char CMD_ERASE_FLASH[4] = {0x20,0x00,0x00,0x00};
    unsigned char buffer[4];
    CMD_ERASE_FLASH[1] = (add >> 16) & 0xFF;
    CMD_ERASE_FLASH[2] = (add >> 8)  & 0xFF;
    CMD_ERASE_FLASH[3] = (add >> 0)  & 0xFF;
    hal_spi_write_and_read_data(spi_x_base,CMD_ERASE_FLASH,buffer,4);
}

void hal_25wq16_write_flash(uint32_t spi_x_base , unsigned int addr ,unsigned char *data, unsigned int length)
{   
    unsigned char buffer[4] = {0x02,0x00,0x00,0x00};
    hal_25wq16_write_enable(spi_x_base);
    buffer[1] = (addr >> 16) & 0xFF;
    buffer[2] = (addr >> 8)  & 0xFF;
    buffer[3] = (addr >> 0)  & 0xFF;
    hal_spi_write_data_with_addr(spi_x_base,buffer,4,data,length);
}

void hal_25wq16_read_flash(uint32_t spi_x_base , unsigned int addr ,unsigned char *data, unsigned int length)
{
    unsigned char buffer[4] = {0x03,0x00,0x00,0x00};
    buffer[1] = (addr >> 16) & 0xFF;
    buffer[2] = (addr >> 8)  & 0xFF;
    buffer[3] = (addr >> 0)  & 0xFF;
    
    hal_spi_read_data_with_addr(spi_x_base,buffer,4,data,length);
}

void ln_spi_master_test(void)
{
    //SPI主机模式初始化
    ln_spi_master_init();
    
    //初始化数据buffer
    for(int i = 0; i < 100 ; i++)
    {
        tx_data[i] = i + 2;
    }

    LOG(LOG_LVL_INFO,"LN882H SPI + 25WQ16 test start! \n");

    /********************************SPI直接读写Flash芯片****************************************************************/

    //读取ID
    hal_25wq16_read_id(SPI0_BASE,rx_data);
    LOG(LOG_LVL_INFO,"LN882H 25WQ16 ID: %x %x %x \n",rx_data[0],rx_data[1],rx_data[2]);
    
    //读取状态
    hal_25wq16_read_status(SPI0_BASE,rx_data);

    //写使能
    hal_25wq16_write_enable(SPI0_BASE);

    //读取状态
    hal_25wq16_read_status(SPI0_BASE,rx_data);

    //擦除芯片
    hal_25wq16_erase_flash(SPI0_BASE,0x200);

    //等待擦除完成
    while(1)
    {
        hal_25wq16_read_status(SPI0_BASE,status);
        if((status[0] & 0x01) == 0x00)
            break;
    }

    //向Flash芯片中写入数据
    hal_25wq16_write_flash(SPI0_BASE,0x200,tx_data,100);

    //等待写入完成
    while(1)
    {
        hal_25wq16_read_status(SPI0_BASE,status);
        if((status[0] & 0x01) == 0x00)
            break;
    }

    //从Flash芯片中读出数据
    hal_25wq16_read_flash(SPI0_BASE,0x200,rx_data,100);
    
    //判断写入的数据是否正确
    err_cnt = 0;
    for(int i = 0 ; i < 100; i++)
    {
        if(rx_data[i] != tx_data[i])
        {
            err_cnt++;
        }
    }

    //打印LOG
    if(err_cnt != 0)
    {
        LOG(LOG_LVL_INFO,"LN882H SPI + 25WQ16 test fail! \n");
    }
    else
    {
        LOG(LOG_LVL_INFO,"LN882H SPI + 25WQ16 test success! \n");
    }


    /********************************SPI + DMA 读写Flash芯片****************************************************************/
    LOG(LOG_LVL_INFO,"LN882H SPI + DMA + 25WQ16 test start! \n");

    LN_DMA_EN_STATUS = 1;
    ln_spi_dma_init();

    //读取ID
    hal_25wq16_read_id(SPI0_BASE,rx_data);
    LOG(LOG_LVL_INFO,"LN882H 25WQ16 ID: %x %x %x \n",rx_data[0],rx_data[1],rx_data[2]);
    
    //读取状态
    hal_25wq16_read_status(SPI0_BASE,rx_data);

    //写使能
    hal_25wq16_write_enable(SPI0_BASE);

    //读取状态
    hal_25wq16_read_status(SPI0_BASE,rx_data);

    //擦除芯片
    hal_25wq16_erase_flash(SPI0_BASE,0x200);

    //等待擦除完成
    while(1)
    {
        hal_25wq16_read_status(SPI0_BASE,status);
        if((status[0] & 0x01) == 0x00)
            break;
    }

    //向Flash芯片中写入数据
    hal_25wq16_write_flash(SPI0_BASE,0x200,tx_data,100);

    //等待写入完成
    while(1)
    {
        hal_25wq16_read_status(SPI0_BASE,status);
        if((status[0] & 0x01) == 0x00)
            break;
    }
    
    memset(rx_data,0,100);

    //从Flash芯片中读出数据
    hal_25wq16_read_flash(SPI0_BASE,0x200,rx_data,100);
    
    //判断写入的数据是否正确
    err_cnt = 0;
    for(int i = 0 ; i < 100; i++)
    {
        if(rx_data[i] != tx_data[i])
        {
            err_cnt++;
        }
    }

    //打印LOG
    if(err_cnt != 0)
    {
        LOG(LOG_LVL_INFO,"LN882H SPI + DMA + 25WQ16 test fail! \n");
    }
    else
    {
        LOG(LOG_LVL_INFO,"LN882H SPI + DMA + 25WQ16 test success! \n");
    }
    
//    while(1)
//    {
//        hal_25wq16_read_flash(SPI0_BASE,0x200,rx_data,100);
//    }

}
