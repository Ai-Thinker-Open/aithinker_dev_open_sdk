#include "pwm_port.h"
#include "pwm_cap.h"

#include "aiio_adapter_include.h"

typedef struct
{
	uint32_t freq;
	uint32_t duty;
} ir_nec_t;

ir_nec_t nec_start = {
	.freq = 74,
	// .duty = 33,//占空比
	.duty = 66, // 占空比
};

ir_nec_t nec_one = {
	.freq = 445,
	// .duty = 75,//占空比
	.duty = 25, // 占空比
};

ir_nec_t nec_zero = {
	.freq = 892,
	.duty = 50, // 占空比
};

ir_nec_t nec_repet = {
	.freq = 89,
	// .duty = 20,//占空比
	.duty = 80, // 占空比
};

float dutylist[1024];
uint32_t freqlist[1024];
uint32_t size = 0;

void nec_send_dat(uint8_t dat)
{
	uint8_t tmp = dat;
	for (int i = 0; i < 8; i++)
	{
		if ((tmp & 0x01) == 0x01)
		{
			// pwm_cnt_set(1,nec_one.freq,nec_one.duty);
			aiio_pwm_output(AIIO_PWM_CHA_0, 1, nec_one.freq, nec_one.duty, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
		}
		else
		{
			// pwm_cnt_set(1,nec_zero.freq,nec_zero.duty);
			aiio_pwm_output(AIIO_PWM_CHA_0, 1, nec_zero.freq, nec_zero.duty, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
		}
		tmp >>= 1;
	}
}

int nec_recv(uint32_t freq, uint32_t duty)
{
	if ((freq < 76) && (freq > 72) && (duty > 64) && (duty < 68))
	{
		return 2;
	}

	if ((freq < 460) && (freq > 430) && (duty > 20) && (duty < 30))
	{
		return 1;
	}

	if ((freq < 950) && (freq > 850) && (duty > 45) && (duty < 55))
	{
		return 0;
	}

	if ((freq < 95) && (freq > 85) && (duty > 75) && (duty < 85))
	{
		return 3;
	}

	return -1;
}

int nec_anal(uint32_t *freqlist, float *dutylist, uint32_t size, uint8_t *buf)
{
	int count = 0, tmp = 0;
	if (nec_recv(freqlist[0], dutylist[0]) != 2)
	{
		return -1;
	}
	for (int i = 1; i < size; i++)
	{
		tmp >>= 1;
		if ((nec_recv(freqlist[i], dutylist[i]) != 1) && (nec_recv(freqlist[i], dutylist[i]) != 0))
		{
			return -1;
		}
		if (nec_recv(freqlist[i], dutylist[i]) == 1)
		{
			tmp |= 0x80;
		}
		if (i % 8 == 0)
		{
			buf[count] = tmp;
			tmp = 0;
			count++;
		}
	}
	return 0;
}
void aiio_liqi_test_function(void)
{
	uint8_t buf[4];
	aiio_pwmcap_pin_cfg_t aiio_pwmcap_config = {
		.aiio_gpio_pwmcap_port = AIIO_GPIO_A,
		.aiio_gpio_pwmcap_pin = AIIO_GPIO_NUM_7};
	aiio_pwmcap_capture_init(&aiio_pwmcap_config);
	aiio_pwmcap_capture_start();

	aiio_os_tick_dealy(aiio_os_ms2tick(1000));
	aiio_pwm_output(AIIO_PWM_CHA_0, 1, 1000, 0, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
	aiio_pwm_output(AIIO_PWM_CHA_0, 1, nec_start.freq, nec_start.duty, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
	nec_send_dat(0x17);
	nec_send_dat(~0x17);
	nec_send_dat(0x25);
	nec_send_dat(~0x25);

	aiio_pwmcap_capture_value(freqlist, dutylist, &size);

	nec_anal(freqlist, dutylist, size, buf);
	aiio_log_i("buf[0] = 0x%02x , buf[1] = 0x%02x ,buf[2] = 0x%02x ,buf[3] = 0x%02x", buf[0], buf[1], buf[2], buf[3]);

	for (int i = 0; i < size; i++)
	{
		aiio_log_i("PWM_CAP_DATA[%d]::duty = %01f , freq = %d", i, dutylist[i], freqlist[i]);
	}
	aiio_pwmcap_capture_close();

	aiio_pwmcap_capture_start();

	aiio_os_tick_dealy(aiio_os_ms2tick(1000));
	aiio_pwm_output(AIIO_PWM_CHA_0, 1, 1000, 0, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
	aiio_pwm_output(AIIO_PWM_CHA_0, 1, nec_start.freq, nec_start.duty, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
	nec_send_dat(0x35);
	nec_send_dat(~0x35);
	nec_send_dat(0x14);
	nec_send_dat(~0x14);

	aiio_pwmcap_capture_value(freqlist, dutylist, &size);

	nec_anal(freqlist, dutylist, size, buf);
	aiio_log_i("buf[0] = 0x%02x , buf[1] = 0x%02x ,buf[2] = 0x%02x ,buf[3] = 0x%02x", buf[0], buf[1], buf[2], buf[3]);

	for (int i = 0; i < size; i++)
	{
		aiio_log_i("PWM_CAP_DATA[%d]::duty = %01f , freq = %d", i, dutylist[i], freqlist[i]);
	}
	aiio_pwmcap_capture_close();
}
void aiio_main(void *params)
{
	aiio_log_a("Hello EasyLogger!");
	aiio_log_e("Hello EasyLogger!");
	aiio_log_w("Hello EasyLogger!");
	aiio_log_i("Hello EasyLogger!");
	aiio_log_d("Hello EasyLogger!");
	aiio_log_v("Hello EasyLogger!");

	aiio_liqi_test_function();
	while (1)
	{
		aiio_os_tick_dealy(aiio_os_ms2tick(2000));
	}
}
