/**
 * @file        chsc6540_i2c.c
 * @brief
 * @author      Jimmy (ccolacat@163.com)
 * @version     v1.0.0
 * @date        2023-06-06
 * 
 * @copyright   Copyright (c) 2020-2023 Shenzhen Ai-Thinker Technology Co., Ltd. All Rights Reserved.
 * 
 * Disclaimer: This program is for informational purposes only, please keep author information and source code license.
 * 
 */
#include "touch_conf_user.h"
#ifdef TOUCH_I2C_CHSC6540

#include "bflb_mtimer.h"
#include "bflb_gpio.h"
#include "bflb_i2c.h"
#include "chsc6540_i2c.h"

static struct bflb_device_s *touch_chsc6540_i2c = NULL;

static void chsc6540_i2c_gpio_init(void)
{
    struct bflb_device_s *ft63x6_i2c_gpio = NULL;
    ft63x6_i2c_gpio = bflb_device_get_by_name("gpio");

    /* I2C0_SCL */
    bflb_gpio_init(ft63x6_i2c_gpio, TOUCH_I2C_SCL_PIN, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    /* I2C0_SDA */
    bflb_gpio_init(ft63x6_i2c_gpio, TOUCH_I2C_SDA_PIN, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
}

static int chsc6540_i2c_peripheral_init(void)
{
    touch_chsc6540_i2c = bflb_device_get_by_name("i2c0");

    if (touch_chsc6540_i2c) {
        // printf("chsc6540 i2c gpio init\r\n");
        /* init i2c gpio */
        chsc6540_i2c_gpio_init();
        /* init i2c 200k */
        bflb_i2c_init(touch_chsc6540_i2c, 200000);
    } else {
        printf("i2c device get fail\r\n");
        return -1;
    }

    return 0;
}

static int chsc6540_i2c_read_byte(uint8_t register_addr, uint8_t *data_buf, uint16_t len)
{
    static struct bflb_i2c_msg_s msg[2];

    msg[0].addr = CHSC6540_I2C_SLAVE_ADDR;
    msg[0].flags = I2C_M_NOSTOP;
    msg[0].buffer = &register_addr;
    msg[0].length = 1;

    msg[1].flags = I2C_M_READ;
    msg[1].buffer = data_buf;
    msg[1].length = len;
    return bflb_i2c_transfer(touch_chsc6540_i2c, msg, 2);
}


int chsc6540_i2c_init(void)
{
    printf("chsc6540 i2c init\r\n");

    chsc6540_i2c_peripheral_init();
    return 0;
}

int chsc6540_i2c_read(uint8_t *point_num, touch_coord_t *touch_coord, uint8_t max_num)
{
    uint8_t data_buf[CHSC6540_TD_DATA_NUM+1];
    *point_num = 0;

    if (point_num == NULL || touch_coord == NULL || max_num == 0) {
        return -1;
    }

    if (chsc6540_i2c_read_byte(CHSC6540_TD_STAT_REG, &data_buf[0], CHSC6540_TD_DATA_NUM)) {
        return -1;
    }
    data_buf[CHSC6540_TD_POINT_NUM] &= CHSC6540_TD_STAT_MASK;
    /* no touch or err */
    if (data_buf[2] == 0 || data_buf[2] > 2) {
        return -2;
    }

    /* Get the first point */
    
    touch_coord[0].coord_x = (uint16_t)(data_buf[CHSC6540_P1_XH_REG] & CHSC6540_MSB_MASK) << 8 | (data_buf[CHSC6540_P1_XL_REG] & CHSC6540_LSB_MASK);
    touch_coord[0].coord_y = (uint16_t)(data_buf[CHSC6540_P1_YH_REG] & CHSC6540_MSB_MASK) << 8 | (data_buf[CHSC6540_P1_YL_REG] & CHSC6540_LSB_MASK);
    *point_num = 1;

    /* Get the second point */
    if (data_buf[CHSC6540_TD_POINT_NUM] > 1 && max_num > 1) {
        touch_coord[1].coord_x = (data_buf[CHSC6540_P2_XH_REG] & CHSC6540_MSB_MASK) << 8 | (data_buf[CHSC6540_P2_XL_REG] & CHSC6540_LSB_MASK);
        touch_coord[1].coord_y = (data_buf[CHSC6540_P2_YH_REG] & CHSC6540_MSB_MASK) << 8 | (data_buf[CHSC6540_P2_YL_REG] & CHSC6540_LSB_MASK);
        *point_num = 2;
    }
    return 0;
}

#endif




/************************ Copyright (c) 2023 Shenzhen Ai-Thinker Technology Co., Ltd. ************************/
