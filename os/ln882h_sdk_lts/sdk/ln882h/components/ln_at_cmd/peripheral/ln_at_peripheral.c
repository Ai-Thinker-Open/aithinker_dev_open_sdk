
#include "ln_at_peripheral.h"


static ln_at_err_t ln_at_set_pwm_init(uint8_t para_num, const char *name)
{
	bool is_default = false;
	uint8_t para_index = 1;
	
	int duty_res = 0;
	int gpio = 0;
	int freq = 0;
	
	ln_at_drv_pwm_item_t* phandle = ln_at_drv_pwm_handle();
	
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &freq))
	{
		goto __exit;
	}

	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &duty_res))
	{
		goto __exit;
	}

	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &gpio))
	{
		goto __exit;
	}
	
	if(-1==phandle->_drv_init((uint32_t)freq, (uint8_t)duty_res, (uint8_t)gpio, PWM_CHA_0))
	{
		goto __exit;
	}

  ln_at_printf(LN_AT_RET_OK_STR);
  return LN_AT_ERR_NONE;

__exit:
	ln_at_printf(LN_AT_RET_ERR_STR);
	return LN_AT_ERR_COMMON;
}

LN_AT_CMD_REG(DRVPWMINIT, NULL, ln_at_set_pwm_init, NULL, NULL);