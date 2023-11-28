
#include "ln_at_drive_port.h"

//gpio
static void ln_at_drv_gpio_cfg_init(void);
static ln_at_gpio_t* ln_at_drv_get_gpio_info(at_gpio_t gpio);

//pwm
static void ln_at_drv_pwm_cfg_init(void);
static int ln_at_drv_init_pwm(uint32_t freq, uint8_t duty_res, uint8_t channel_port , ln_at_pwm_channel_t pwm_channel);
static int ln_at_drv_set_pwm_duty(uint16_t duty, uint8_t channel);

//gpio handle
//********************************************************************************************

ln_at_drv_gpio_item_t g_ln_at_drv_gpio_cfg =
{
	.gpio_base_a_mask = (uint16_t)0x0000,
	.gpio_base_b_mask = (uint16_t)0x0000,
	
	._cfg_init = ln_at_drv_gpio_cfg_init,
	._get_gpio = ln_at_drv_get_gpio_info,
};

ln_at_drv_gpio_item_t* ln_at_drv_gpio_handle(void)
{
	return &g_ln_at_drv_gpio_cfg;
}

static void ln_at_drv_gpio_cfg_init(void)
{
	ln_at_drv_gpio_item_t* hdl = ln_at_drv_gpio_handle();
	
	hdl->gpio_base_a_mask = (uint16_t)0x0000;
	hdl->gpio_base_b_mask = (uint16_t)0x0000;
	
	hdl->_get_gpio = ln_at_drv_get_gpio_info;
}

static ln_at_gpio_t* ln_at_drv_get_gpio_info(at_gpio_t gpio)
{
	ln_at_gpio_t* gpio_temp = NULL;
	
	//hal_assert(IS_AT_GPIO(gpio));
	
	if(gpio < AT_GPIO_11)
	{
		gpio_temp->gpio_base = AT_GPIO_BASE_A;
		gpio_temp->gpio_pin = GPIO_PIN_0 << gpio ;
	}
	else if(gpio >= AT_GPIO_11)
	{
		gpio_temp->gpio_base = AT_GPIO_BASE_B;
		gpio_temp->gpio_pin = GPIO_PIN_0 << (gpio - 10);
	}
	else
	{
		return NULL;
	}
	
	return gpio_temp;
}

//pwm handle
//*********************************************************************************************
ln_at_drv_pwm_item_t g_ln_at_drv_pwm_cfg =
{
	.channel_init_mask = 0x00,
	
	._cfg_init = ln_at_drv_pwm_cfg_init,
	._drv_init = ln_at_drv_init_pwm,
	._set_duty = ln_at_drv_set_pwm_duty,
};

ln_at_drv_pwm_item_t* ln_at_drv_pwm_handle(void)
{
	return &g_ln_at_drv_pwm_cfg;
}

static void ln_at_drv_pwm_cfg_init(void)
{
	ln_at_drv_pwm_item_t* hdl = ln_at_drv_pwm_handle();
	hdl->channel_init_mask = 0x00;
	memset(hdl->idx , 0 , sizeof(ln_at_pwm_channel_info_t)*PWM_CHANNEL_NUM);
	
	hdl->_drv_init = ln_at_drv_init_pwm;
	hdl->_set_duty = ln_at_drv_set_pwm_duty;
}

//freq: 100Hz ~ 8MHz   esp32: 1Hz ~ 8MHz
//duty_res: 0~16		esp32: 0~20
static int ln_at_drv_init_pwm(uint32_t freq, uint8_t duty_res, uint8_t channel_port , ln_at_pwm_channel_t pwm_channel)
{
	uint32_t reg_base = 0;
	
	ln_at_drv_pwm_item_t* pwm_hdl = ln_at_drv_pwm_handle();
	ln_at_drv_gpio_item_t* gpio_hdl = ln_at_drv_gpio_handle();
	
	ln_at_gpio_t* gpio_tmp = gpio_hdl->_get_gpio(channel_port);
	
	//para is invalid
	if(freq < PWM_FREQ_MIN || freq > PWM_FREQ_MAX || duty_res < PWM_DUTY_RES_MIN || duty_res > PWM_DUTY_RES_MAX || \
			channel_port < AT_GPIO_0 || channel_port > AT_GPIO_17 || pwm_channel < PWM_CHA_0 || pwm_channel > PWM_CHA_3)
	{
		LOG(LOG_LVL_ERROR,"param is invalid\r\n");
		return -1;
	}

	hal_gpio_pin_afio_select(gpio_tmp->gpio_base, gpio_tmp->gpio_pin ,(afio_function_t)(ADV_TIMER_PWM0 + pwm_channel));
	hal_gpio_pin_afio_en(gpio_tmp->gpio_base, gpio_tmp->gpio_pin ,HAL_ENABLE);


	switch(pwm_channel)
	{
			case PWM_CHA_0: reg_base = ADV_TIMER_0_BASE; break;
			case PWM_CHA_1: reg_base = ADV_TIMER_0_BASE; break;
			case PWM_CHA_2: reg_base = ADV_TIMER_1_BASE; break;
			case PWM_CHA_3: reg_base = ADV_TIMER_1_BASE; break;
	}

	/* PWM参数初始化 */
	adv_tim_init_t_def adv_tim_init;
	memset(&adv_tim_init,0,sizeof(adv_tim_init));

	// 设置PWM频率
	if(freq >= 10000){
			adv_tim_init.adv_tim_clk_div = 0;      
			adv_tim_init.adv_tim_load_value =  hal_clock_get_apb0_clk() * 1.0 / freq - 2;       
	}else{
			adv_tim_init.adv_tim_clk_div = (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1;      
			adv_tim_init.adv_tim_load_value =  1000000 / freq - 2;            
	}                 
	
	// 设置PWM占空比 default duty: 50%
	if((pwm_channel & 0x01) == 0)
			adv_tim_init.adv_tim_cmp_a_value = (adv_tim_init.adv_tim_load_value + 2) * 50 / 100.0f;      //设置通道a比较值
	else
			adv_tim_init.adv_tim_cmp_b_value = (adv_tim_init.adv_tim_load_value + 2) * 50 / 100.0f;      //设置通道b比较值
	
	adv_tim_init.adv_tim_dead_gap_value = 0;                                //设置死区时间
	adv_tim_init.adv_tim_dead_en        = ADV_TIMER_DEAD_DIS;               //不开启死区
	adv_tim_init.adv_tim_cnt_mode       = ADV_TIMER_CNT_MODE_INC;           //向上计数模式
	adv_tim_init.adv_tim_cha_inv_en     = ADV_TIMER_CHA_INV_EN;
	adv_tim_init.adv_tim_chb_inv_en     = ADV_TIMER_CHB_INV_EN;
	hal_adv_tim_init(reg_base,&adv_tim_init); 
	
	pwm_hdl->idx[pwm_channel].duty_res = duty_res;
	pwm_hdl->idx[pwm_channel].freq = freq;
	pwm_hdl->idx[pwm_channel].gpio.gpio_base = gpio_tmp->gpio_base;
	pwm_hdl->idx[pwm_channel].gpio.gpio_pin = gpio_tmp->gpio_pin;
	
	pwm_hdl->channel_init_mask = 1 << pwm_channel;
	
	if(gpio_tmp->gpio_base == AT_GPIO_BASE_A)
	{
		gpio_hdl->gpio_base_a_mask |= gpio_tmp->gpio_pin ;
	}
	else if(gpio_tmp->gpio_base == AT_GPIO_BASE_B)
	{
		gpio_hdl->gpio_base_b_mask |= gpio_tmp->gpio_pin ;
	}
	
	return 0;
}

static int ln_at_drv_set_pwm_duty(uint16_t duty, uint8_t channel)
{
}
//********************************************************************************************


//I2C
//********************************************************************************************

