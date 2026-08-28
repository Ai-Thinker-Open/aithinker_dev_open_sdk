/** @brief      sleep application interface.
 *
 *  @file       aiio_sleep.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       adc application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/06/14      <td>1.0.0       <td>chenc         <td>sleep Init
 *  </table>
 *
 */
#include "aiio_sleep.h"

#include "bflb_mtimer.h"
#include "bl616_glb.h"
#include "bl616_pds.h"
#include "bl616_hbn.h"
#include "bl616_aon.h"
#include "bl616_pm.h"

#define BL_LP_IO_RES_PULL_UP   1
#define BL_LP_IO_RES_PULL_DOWN 2
#define BL_LP_IO_RES_NONE      3


static void bl_lp_set_aon_io(int pin, int trigMode, int res_mode)
{
    /* set pin's GLB_GPIO_FUNC_Type as GPIO_FUN_GPIO */
    GLB_GPIO_Cfg_Type gpio_cfg;
    uint8_t pu, pd;

    gpio_cfg.drive = 0;
    gpio_cfg.smtCtrl = 0;
    gpio_cfg.outputMode = 0;
    gpio_cfg.gpioMode = GPIO_MODE_INPUT;
    gpio_cfg.pullType = GPIO_PULL_NONE;
    gpio_cfg.gpioPin = (GLB_GPIO_Type)pin;
    gpio_cfg.gpioFun = GPIO_FUN_GPIO;
    GLB_GPIO_Init(&gpio_cfg);

    if (res_mode == BL_LP_IO_RES_PULL_UP) {
        pu = 1;
        pd = 0;
    } else if (res_mode == BL_LP_IO_RES_PULL_DOWN) {
        pu = 0;
        pd = 1;
    } else {
        pu = 0;
        pd = 0;
    }

    /* set pin's aonPadCfg */
    HBN_AON_PAD_CFG_Type aonPadCfg;
    uint32_t mask = 0;

    aonPadCfg.ctrlEn = 1;
    aonPadCfg.ie = 1;
    aonPadCfg.oe = 0;
    aonPadCfg.pullUp = pu;
    aonPadCfg.pullDown = pd;
    HBN_Aon_Pad_Cfg(ENABLE, (pin - 16), &aonPadCfg);

    mask = BL_RD_REG(HBN_BASE, HBN_IRQ_MODE);
    mask = BL_GET_REG_BITS_VAL(mask, HBN_PIN_WAKEUP_MASK);
    mask = mask & ~(1 << (pin - 16));

    /* set trigMode */
    HBN_Aon_Pad_WakeUpCfg(DISABLE, (uint8_t)trigMode, mask, 0, 7);

    /* UnMask Hbn_Irq Wakeup PDS*/
    pm_pds_wakeup_src_en(PDS_WAKEUP_BY_HBN_IRQ_OUT_EN_POS);
}

static void bl_lp_set_pds_io(int pin, int trigMode, int res_mode)
{
    /* set pin's GLB_GPIO_FUNC_Type as GPIO_FUN_GPIO */
    GLB_GPIO_Cfg_Type gpio_cfg;
    uint8_t pu, pd;
    uint8_t gpio_grp;
    uint32_t tmpVal;

    if (pin > 19) {
        gpio_grp = (pin - 4) / 8;
    } else {
        gpio_grp = pin / 8;
    }

    gpio_cfg.drive = 0;
    gpio_cfg.smtCtrl = 0;
    gpio_cfg.outputMode = 0;
    gpio_cfg.gpioMode = GPIO_MODE_INPUT;
    gpio_cfg.pullType = GPIO_PULL_NONE;
    gpio_cfg.gpioPin = (GLB_GPIO_Type)pin;
    gpio_cfg.gpioFun = GPIO_FUN_GPIO;
    GLB_GPIO_Init(&gpio_cfg);

    if (res_mode == BL_LP_IO_RES_PULL_UP) {
        pu = 1;
        pd = 0;
    } else if (res_mode == BL_LP_IO_RES_PULL_DOWN) {
        pu = 0;
        pd = 1;
    } else {
        pu = 0;
        pd = 0;
    }

    PDS_Set_GPIO_Pad_Pn_Pu_Pd_Ie(gpio_grp / 2, pu, pd, 1);

    PDS_Set_GPIO_Pad_IntClr(gpio_grp);
    PDS_Set_GPIO_Pad_IntMode(gpio_grp, trigMode);

    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_PD_SET);
    if (pin > 19) {
        tmpVal &= ~(1 << (pin - 4));
    } else {
        tmpVal &= ~(1 << pin);
    }
    BL_WR_REG(PDS_BASE, PDS_GPIO_PD_SET, tmpVal);

    pm_pds_wakeup_src_en(PDS_WAKEUP_BY_PDS_GPIO_IRQ_EN_POS);
}




//设置低功耗模式
//参数
//    ch：当前通道，注意这个只是给 beforSleepCallback 回调用的，hal层不要向AT口打印数据
//    beforSleepCallback：这个表示进入低功耗之前的回调函数，需要在进入低功耗之前调用 beforSleepCallback(ch);
//返回值
//    0：成功
//    1：失败
//    2：mode错误(不支持的mode，或者当前模式错误)
//    3：参数错误
//    4：引脚不支持唤醒
//    128：不支持的操作(或者没有适配)
uint8_t aiio_set_low_power_mode(aiio_wakeup_cfg_t *wakeup_cfg)
{
	uint32_t sleep_time = 0;
    uint8_t weakup_pin = 0xFF;    

	if(AIIO_LOW_POWER_NONE == wakeup_cfg->lowerPowerMode){
		return 0;	//唤醒直接返回
	}
	
    if(NULL==wakeup_cfg){
		return 3;
	}

	if(AIIO_LOW_POWER_DEEP_SLEEP != wakeup_cfg->lowerPowerMode){
		return 2;	//当前仅支持深度睡眠
	}
    	//深度睡眠
	switch(wakeup_cfg->wakeUpSourceMode){
		case AIIO_LOW_POWER_WAKE_UP_TIMER:	//使用定时器唤醒
			sleep_time = wakeup_cfg->wakeUpMs * 32;
			// aiio_log_w("sleep_time=%d\r\n",sleep_time);
			aiio_at_response_ok();
			bflb_mtimer_delay_ms(100);
			pm_hbn_mode_enter(PM_HBN_LEVEL_0, sleep_time); 	//使用定时器作为唤醒源进入低功耗
			
			// pm_hbn_mode_enter(PM_HBN_LEVEL_0, 32768 * sleep_time); 	//使用定时器作为唤醒源进入低功耗

			return 0;
		// case AIIO_LOW_POWER_WAKE_UP_GPIO:	//使用GPIO唤醒
		// 	if(21 != wakeup_cfg->wakeUpPin ){
		// 		return 4;	//BL618仅支持IO22唤醒
		// 	}
		// 	weakup_pin = wakeup_cfg->wakeUpPin + 1;
        //     // if (wakeup_cfg->gpioWakeUpLeve){
        //     //     weakup_pin |= 0x80;
        //     // }
			
		// 	aiio_log_w("weakup_pin=0x%02x \r\n",weakup_pin);
			
		// 	uint32_t tmpVal = 0;
			
		// 	/* rf808_usb20_psw_rref output off */
		// 	tmpVal = BL_RD_REG(PDS_BASE, PDS_USB_PHY_CTRL);
		// 	tmpVal = BL_CLR_REG_BIT(tmpVal, PDS_REG_PU_USB20_PSW);
		// 	BL_WR_REG(PDS_BASE, PDS_USB_PHY_CTRL, tmpVal);

		// 	pm_pds_mask_all_wakeup_src();
		// 	AON_Output_Float_LDO15_RF();
		// 	HBN_Pin_WakeUp_Mask(0xF);

		// 	aiio_log_w("enter pds mode\r\n");
		// 	aiio_at_response_ok();
		// 	bflb_mtimer_delay_ms(100);

		// 	bl_lp_set_pds_io(22, PDS_GPIO_INT_ASYNC_FALLING_EDGE, BL_LP_IO_RES_PULL_UP);
		// 	// bl_lp_set_aon_io(19, HBN_GPIO_INT_TRIGGER_SYNC_HIGH_LEVEL, BL_LP_IO_RES_PULL_DOWN);

		// 	/* sleep time must set zero to avoid using rtc */
		// 	pm_pds_mode_enter(PM_PDS_LEVEL_15, 0);

		// 	return 0;
		default:
			return 2;
	}
	return 1;
}