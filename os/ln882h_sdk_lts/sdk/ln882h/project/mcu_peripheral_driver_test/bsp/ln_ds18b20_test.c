/**
 * @file     ln_ds18b20_test.c
 * @author   BSP Team 
 * @brief    This file provides ds18b20 test function.
 * @version  0.0.0.1
 * @date     2021-03-29
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/*
        DS18B20使用说明：

                    1. 接线说明：
                                    LN8210               DS18B20
                                    PB7          ->      DATA
                                    3.3V         ->      VCC
                                    GND          ->      GND
                                    
·                   2. 等待DS18B20打印Family Code 0x28，则认为初始化成功。
*/



#include "ln_ds18b20_test.h"
#include "string.h"
#include "stdbool.h"
#include "utils/debug/log.h"

ds18b20_data_str    ds18b20_data[1];

static void ln_delay_us(volatile unsigned int time)
{   
    volatile unsigned int count = 0;
    while(time--)
    {
        count = 12;
        while(count--);
    }
}

uint8_t crc_cal(void *src, uint8_t size)
{
	//crc-8/MAXIM
	//x8 + x5 + x4 + 1
	//多项式：31
	//crc初始值：0
	//计算结果异或值：0
	//当数组最后一位含有CRC值时，输出0，否则输出计算的CRC

	uint8_t ret = 0;
	uint8_t *p;
	int i = 0;
	uint8_t pBuf = 0;
	p = (uint8_t*)src;
	while(size--)
	{
		pBuf = *p ++;
		for ( i = 0; i < 8; i ++ )
		{
			if ((ret ^ (pBuf)) & 0x01){
				ret ^= 0x18;
				ret >>= 1;
				ret |= 0x80;
			}
			else{
				ret >>= 1;
			}
			pBuf >>= 1;
		}
	}
	return ret;
}

unsigned char ln_ds18b20_rst()
{
    unsigned int retry_time = 10;
    unsigned char data = 0;
 
    //rst the ds18b20.
    DATA_OUT_MODE;
    DATA_OUT(1);
    DATA_OUT(0);
    ln_delay_us(500);       //拉低总线至少480us
    
    DATA_IN_MODE;           
    ln_delay_us(10);        //释放总线然后等待最多60us
    
    //check the bus.
    while(DATA_READ && retry_time --) {ln_delay_us(50);}    //如果总线存在，总线会拉点60us-240us实现发送一个存在脉冲
    data = DATA_READ;
    if(data)
    {
        return false;
    }
    else
    {
        ln_delay_us(500);
        return true;
    }
}

unsigned char ln_ds18b20_read_one_bit()
{
    unsigned char data = 0;
    DATA_OUT(1);
    DATA_OUT_MODE;
    DATA_OUT(0);
    ln_delay_us(1);     //发出一个下降沿之后至少延时1us   然后释放总线,但是整个流程小于15us。

    DATA_IN_MODE;
    ln_delay_us(1);
    data = DATA_READ;
    
    ln_delay_us(60);    //每个读时段至少60us
    return data;
}

void ln_ds18b20_write_one_bit(unsigned char data)
{
    DATA_OUT(1);
    DATA_OUT_MODE;   
    DATA_OUT(0);
    ln_delay_us(1);     //发出一个下降沿之后至少延时1us
    if(data)
    {   
        DATA_IN_MODE;   //每个读时段至少60us
    }
    ln_delay_us(60);
}

unsigned char  ln_ds18b20_read_two_bits()
{
    unsigned char data = 0;
    for(int i = 0; i < 2; i++)
    {
        data |= (ln_ds18b20_read_one_bit()  & 0x01) << ( 1 - i);
    }
    return data;
}

unsigned char ln_ds18b20_read_one_byte()
{
    unsigned char data = 0;
    for(int i = 0; i < 8; i++)
    {
        data |= (ln_ds18b20_read_one_bit()  & 0x01) << (i);
    }
    return data;
}

void ln_ds18b20_write_one_byte(unsigned char data)
{
    for(int i = 0; i < 8; i++)
    {
        ln_ds18b20_write_one_bit((data >> i) & 0x01);    
    }
}

void ln_ds18b20_auto_search_rom_code(unsigned char *data,unsigned char *id_num)
{
    uint8_t k, l, chongtuwei, m, n, num;
	uint8_t zhan[100];
	uint8_t ss[64];
    uint8_t DS18B20_ID[100][8];
	uint8_t tempp;
    uint8_t MaxSensorNum = 100;
    uint8_t DS18B20_SensorNum;
	l = 0;
	num = 0;
    
    memset(zhan,0,100);
	do
	{
		ln_ds18b20_rst(); //注意：复位的延时不够
		ln_delay_us(480); //480、720
		ln_ds18b20_write_one_byte(0xf0);
		for (m = 0; m < 8; m++)
		{
			uint8_t s = 0;
			for (n = 0; n < 8; n++)
			{
				k = ln_ds18b20_read_two_bits();//读两位数据
 
				k = k & 0x03;
				s >>= 1;
				if (k == 0x01)//01读到的数据为0 写0 此位为0的器件响应
				{
					ln_ds18b20_write_one_bit(0);
					ss[(m * 8 + n)] = 0;
				}
				else if (k == 0x02)//读到的数据为1 写1 此位为1的器件响应
				{
					s = s | 0x80;
					ln_ds18b20_write_one_bit(1);
					ss[(m * 8 + n)] = 1;
				}
				else if (k == 0x00)//读到的数据为00 有冲突位 判断冲突位
				{
					//如果冲突位大于栈顶写0 小于栈顶写以前数据 等于栈顶写1
					chongtuwei = m * 8 + n + 1;
					if (chongtuwei > zhan[l])
					{
						ln_ds18b20_write_one_bit(0);
						ss[(m * 8 + n)] = 0;
						zhan[++l] = chongtuwei;
					}
					else if (chongtuwei < zhan[l])
					{
						s = s | ((ss[(m * 8 + n)] & 0x01) << 7);
						ln_ds18b20_write_one_bit(ss[(m * 8 + n)]);
					}
					else if (chongtuwei == zhan[l])
					{
						s = s | 0x80;
						ln_ds18b20_write_one_bit(1);
						ss[(m * 8 + n)] = 1;
						l = l - 1;
					}
				}
				else
				{
					//没有搜索到
				}
			}
			tempp = s;
			DS18B20_ID[num][m] = tempp; // 保存搜索到的ID
		}
		num = num + 1;// 保存搜索到的个数
	} while (zhan[l] != 0 && (num < MaxSensorNum));
	DS18B20_SensorNum = num;
    
    memcpy(data,DS18B20_ID,DS18B20_SensorNum * 8);
    *id_num = DS18B20_SensorNum;
	//printf("DS18B20_SensorNum=%d\r\n",DS18B20_SensorNum);
}

unsigned char ln_ds18b20_read_rom_code()
{
    unsigned char crc_value = 0;
    if(!ln_ds18b20_rst())
    {
        return false;
    }
    
    ln_ds18b20_write_one_byte(DS18B220_CMD_READ_ROM);
    
    for(int i = 0; i < 8; i++)
    {
        ds18b20_data[0].rom_code.data[i] = ln_ds18b20_read_one_byte();
    }
    
    crc_value = crc_cal(ds18b20_data[0].rom_code.data,7);
    if(crc_value == ds18b20_data[0].rom_code.data[7])
    {
       crc_value++;
    }
    return true;
}

float ln_ds18b20_data_parse(unsigned char *data)
{
    unsigned char data_resoluration = 0;
    short buffer_1 = 0;
    unsigned short buffer_2 = 0;
    float buffer_3 = 0;
    switch(data[4])
    {
        case TEMP_9_BIT:
            data_resoluration = 2;
            break;
        case TEMP_10_BIT:
            data_resoluration = 4;
            break;
        case TEMP_11_BIT:
            data_resoluration = 8;
            break;
        case TEMP_12_BIT:
            data_resoluration = 16;
            break;
        default:
            break;
    }
    buffer_2 = data[0]  + (data[1] << 8);
    buffer_1 = *(short*)(&buffer_2);
    buffer_3 = buffer_1 * 1.0 / data_resoluration;
    return buffer_3;
}

unsigned char ln_ds18b20_read_temp(float *temp)
{
    unsigned char data[9];
    memset(data,0,sizeof(data));
    short buffer1 = 0;
    if(!ln_ds18b20_rst())
    {
        return false;
    }
    ln_ds18b20_write_one_byte(DS18B220_CMD_SKIP_ROM);
    ln_ds18b20_write_one_byte(DS18B220_CMD_CONVERT_T);
    
    if(!ln_ds18b20_rst())
    {
        return false;
    }
    ln_ds18b20_write_one_byte(DS18B220_CMD_SKIP_ROM);
    ln_ds18b20_write_one_byte(DS18B220_CMD_RD_SRATCH);
    
    for(int i = 0 ; i < 9; i++)
    {
        data[i] = ln_ds18b20_read_one_byte();
    }
    
    buffer1 = crc_cal(data,8);
    if(buffer1 != data[8])
    {
        return false;
    }
    *temp = ln_ds18b20_data_parse(data);
    return true;
}

unsigned char ln_ds18b20_read_temp_1(float *temp)
{
    unsigned char data[9];
    memset(data,0,sizeof(data));
    short buffer2 = 0;
    
    if(!ln_ds18b20_rst())
    {
        return false;
    }
    ln_ds18b20_write_one_byte(DS18B220_CMD_SKIP_ROM);
    ln_ds18b20_write_one_byte(DS18B220_CMD_CONVERT_T);
    
    if(!ln_ds18b20_rst())
    {
        return false;
    }
    ln_ds18b20_write_one_byte(DS18B220_CMD_MATCH_ROM);
    
    for(int i = 0; i < 8; i++)
    {
        ln_ds18b20_write_one_byte(ds18b20_data[0].rom_code.data[i]);
    }
    
    ln_ds18b20_write_one_byte(DS18B220_CMD_RD_SRATCH);
    
    for(int i = 0 ; i < 9; i++)
    {
        data[i] = ln_ds18b20_read_one_byte();
    }
    
    buffer2 = crc_cal(data,8);
    if(buffer2 != data[8])
    {
        return false;
    }
    
    *temp = ln_ds18b20_data_parse(data);

    return true;
}


float temp_data = 0;
uint32_t cnt = 0;
unsigned char ln_ds18b20_id[100][8];
unsigned char ln_ds18b20_id_num = 0;



void ln_ds18b20_init()
{
    gpio_init_t_def gpio_init;
	memset(&gpio_init,0,sizeof(gpio_init));
    gpio_init.dir = GPIO_OUTPUT;
    gpio_init.pin = GPIO_PIN_7;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(GPIOB_BASE,&gpio_init);
    
    hal_gpio_pin_pull_set(GPIOB_BASE,GPIO_PIN_7,GPIO_PULL_UP);
}
void ln_ds18b20_test()
{
    ln_ds18b20_init();
    
    ln_ds18b20_auto_search_rom_code((uint8_t *)ln_ds18b20_id,&ln_ds18b20_id_num);
    
    ln_ds18b20_read_rom_code();
    LOG(LOG_LVL_INFO,"DS18B20 family code is    0x%X  \n",ds18b20_data[0].rom_code.bit_field.family_code);

    LOG(LOG_LVL_INFO,"DS18B20 serilal number is 0x%LX  \n",ds18b20_data[0].rom_code.bit_field.serial_code);
    while(1)
    {
        ln_ds18b20_read_temp_1(&temp_data);
        LOG(LOG_LVL_INFO,"DS18B20 value is %f  \n",temp_data);
        cnt++;
        ln_delay_us(1000000);
    }
}

