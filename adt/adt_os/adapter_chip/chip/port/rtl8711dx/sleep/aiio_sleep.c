#include "aiio_sleep.h"
#include "ameba_soc.h"
#include "os_wrapper.h"
#include <stdio.h>

static bool app_pmu_init_flag = false;

extern SLEEP_ParamDef sleep_param;

static void aontimer_dslp_handler(void)
{
	printf("dslp wake from aontimer\n");

	AONTimer_ClearINT();
	RCC_PeriphClockCmd(APBPeriph_ATIM, APBPeriph_ATIM_CLOCK, DISABLE);
}

static void wakepin_dslp_handler(void)
{
	u32 pinidx;
	printf("dslp wake from wakepin\n");

	pinidx = WakePin_Get_Idx();
	WakePin_ClearINT(pinidx);
}

static void dslp_wake_handler(void)
{
	u32 BootReason;

	BootReason = SOCPS_AONWakeReason();
	aiio_log_d("DSLP WAKE REASON: %lx \n", BootReason);
	aiio_log_d("BKUP_REG1's value = 0x%08lx \n", BKUP_Read(BKUP_REG1));
	if (BootReason & AON_BIT_TIM_ISR_EVT) {
		RCC_PeriphClockCmd(APBPeriph_ATIM, APBPeriph_ATIM_CLOCK, ENABLE);
		AONTimer_INT(ENABLE);
		InterruptRegister((IRQ_FUN)aontimer_dslp_handler, AON_TIM_IRQ, NULL, 3);
		InterruptEn(AON_TIM_IRQ, 3);
	}

#if defined(CONFIG_AMEBASMART)
	if (BootReason & (AON_BIT_GPIO_PIN0_WAKDET_EVT | AON_BIT_GPIO_PIN1_WAKDET_EVT | AON_BIT_GPIO_PIN2_WAKDET_EVT | AON_BIT_GPIO_PIN3_WAKDET_EVT))
#else
	if (BootReason & (AON_BIT_GPIO_PIN0_WAKDET_EVT | AON_BIT_GPIO_PIN1_WAKDET_EVT))
#endif
	{
		InterruptRegister((IRQ_FUN)wakepin_dslp_handler, AON_WAKEPIN_IRQ, NULL, 3);
		InterruptEn(AON_WAKEPIN_IRQ, 3);
	}
}

void app_pmu_init(void)
{
	/* For reference only, users can modify the function if need */
	/* Attention, the handler is needed to clear NVIC pending int and ip int in dslp flow */
	if (BOOT_Reason() & AON_BIT_RSTF_DSLP) {
		dslp_wake_handler();
		pmu_acquire_deepwakelock(PMU_OS);
	}

	/* KM4 need do pmc init */
#if defined (ARM_CORE_CM4)
	SOCPS_sleepInit();
#endif

	/*acquire wakelock to avoid AP enter sleep mode*/
	pmu_acquire_wakelock(PMU_OS);

#if defined(CONFIG_AMEBALITE)

	if (dsp_status_on()) {
		printf("need power off DSP!!!!!!!!!!!!\n");
	}
#endif
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
	uint8_t wakeUp_Pin = 0;

	if(AIIO_LOW_POWER_NONE == wakeup_cfg->lowerPowerMode){
		return 0;	//唤醒直接返回
	}
	
    if(NULL==wakeup_cfg){
		return 3;
	}

	if(AIIO_LOW_POWER_DEEP_SLEEP != wakeup_cfg->lowerPowerMode){
		return 2;	//当前仅支持深度睡眠
	}

    aiio_at_response_ok();

	if(app_pmu_init_flag == false){

		app_pmu_init();
		app_pmu_init_flag = true;
	}

	aiio_log_d("\nAP start enter deepsleep mode ============>\n");

	BKUP_Write(BKUP_REG1, 0x12345678);
	aiio_log_d("Save 0x12345678 into BKUP_REG1\n");

	sleep_param.sleep_time = 0;

	switch(wakeup_cfg->wakeUpSourceMode){
		case AIIO_LOW_POWER_WAKE_UP_TIMER:	//使用定时器唤醒
			sleep_time = wakeup_cfg->wakeUpMs;
			aiio_log_d("set aon timer to wakeup\n");
			RCC_PeriphClockCmd(APBPeriph_ATIM, APBPeriph_ATIM_CLOCK, ENABLE);
			AONTimer_Setting(sleep_time);
			AONTimer_INT(ENABLE);

			sleep_param.dlps_enable = TRUE;
			pmu_release_deepwakelock(PMU_OS);
			pmu_release_wakelock(PMU_OS);

			aiio_log_d("lockbit:%lx \n", pmu_get_wakelock_status());
			aiio_log_d("dslp_lockbit:%lx\n", pmu_get_deepwakelock_status());

			return 0;
		case AIIO_LOW_POWER_WAKE_UP_GPIO:	//使用GPIO唤醒
			wakeUp_Pin = wakeup_cfg->wakeUpPin + 1;
            aiio_log_d("set aon wakepin to wakeup: pin=%d, level=%d\n", 
                      wakeUp_Pin, wakeup_cfg->gpioWakeUpLeve);

			// 检查引脚是否在支持范围内
#if defined(CONFIG_AMEBASMART)
            if (wakeUp_Pin > 3) {
                aiio_log_e("Error: Unsupported wakeup pin (%d), only 0-3 supported\n", wakeUp_Pin);
                return 4; // 引脚不支持唤醒
            }
#else
            if (wakeUp_Pin > 1) {
                aiio_log_e("Error: Unsupported wakeup pin (%d), only 0-1 supported\n", wakeUp_Pin);
                return 4; // 引脚不支持唤醒
            }
#endif
			// 配置唤醒引脚
            Wakepin_Debounce_Setting(100, ENABLE);  // 100ms防抖
            Wakepin_Setting(wakeUp_Pin, wakeup_cfg->gpioWakeUpLeve);

            sleep_param.dlps_enable = TRUE;
			pmu_release_deepwakelock(PMU_OS);
			pmu_release_wakelock(PMU_OS);

			aiio_log_d("lockbit:%lx \n", pmu_get_wakelock_status());
			aiio_log_d("dslp_lockbit:%lx\n", pmu_get_deepwakelock_status());

			return 0;
		default:
			return 2;
			
	}
	return 1;
	
}
