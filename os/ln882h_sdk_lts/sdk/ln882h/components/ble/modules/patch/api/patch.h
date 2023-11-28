/**
****************************************************************************************
*
* @file patch.h
*
* @brief patch function
*
* Copyright (C) RivieraWaves 2009-2015
*
*
****************************************************************************************
*/
#ifndef PATCH_H_
#define PATCH_H_

/**
****************************************************************************************
* @addtogroup PATCH
* @ingroup CONTROLLER
* @brief PATCH
*
* @{
****************************************************************************************
*/


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // stack configuration

#define PATCH_START_ADDR 0x20010000
#define PATCH_MAGIC_VALUE 0xFF0055AA

#define PATCH_FUNC_MAIN 								0
#define PATCH_FUNC_RF_INIT 								1
#define PATCH_FUNC_RWIP_SLEEP 							2
#define PATCH_FUNC_UART0_IRQHandler 					3

#define PATCH_FUNC_H4TL_RX_DONE							4
#define PATCH_FUNC_H4TL_TX_EVT_Handler					5
#define PATCH_FUNC_H4TL_CMD_HDR_RX_EVT_Handler			6
#define PATCH_FUNC_H4TL_CMD_PLD_RX_EVT_Handler			7
#define PATCH_FUNC_H4TL_ACL_HDR_RX_EVT_Handler			8
#define PATCH_FUNC_HCI_SEND_2_HOST						9
#define PATCH_FUNC_HCI_TX_START							10
#define PATCH_FUNC_HCI_TX_DONE							11

#define PATCH_FUNC_RWIP_DRIVER_INIT 					12
#define PATCH_FUNC_DEEP_SLEEP_SET 						13
#define PATCH_FUNC_DEEP_SLEEP_RESUME 					14
#define PATCH_FUNC_BLE_IRQ_Handler						15
#define PATCH_FUNC_LLM_LE_FEAT_GET_Handler				16

#define PATCH_FUNC_HCI_RESET_CMD_HANDLER				17
#define PATCH_FUNC_HCI_LE_SET_ADV_EN_CMD_HANDLER		18
#define PATCH_FUNC_HCI_LE_SET_SCAN_EN_CMD_HANDLER		19
#define PATCH_FUNC_LLD_ADV_START						20
#define PATCH_FUNC_LLD_ADV_EVT_START_CBK				21
#define PATCH_FUNC_LLD_ADV_FRM_ISR						22
#define PATCH_FUNC_LLD_ADV_PKT_RX						23
#define PATCH_FUNC_LLC_START							24
#define PATCH_FUNC_LLD_CON_EVT_START_CBK				25
#define PATCH_FUNC_LLD_CON_FRM_CBK						26
#define PATCH_FUNC_LLD_TEST_START						27
#define PATCH_FUNC_LLD_TEST_EVT_START_CBK				28
#define PATCH_FUNC_LLD_TEST_FRM_CBK						29
#define PATCH_FUNC_SCH_ARB_PROG_TIMER					30
#define PATCH_FUNC_SCH_ARB_INSERT						31

#define PATCH_FUNC_RWIP_WAKEUP							32
#define PATCH_FUNC_SYSTICK_HANDLER						33
#define PATCH_FUNC_SLPTIMERWAKEUP_IRQHANDLER			34
#define PATCH_FUNC_GPIOA_IRQHANDLER						35
#define PATCH_FUNC_BLEWAKEUP_IRQHANDLER					36

#define PATCH_FUNC_MODIFY_BLE_ADDR						37
#define PATCH_FUNC_HCI_LE_SET_EXT_SCAN_EN_CMD_HANDLER	38
#define PATCH_FUNC_LLD_SCAN_PROCESS_PKT_RX_HANDLER		39

#define PATCH_FUNC_GPIOB_IRQHANDLER						40

#define PATCH_FUNC_HCI_UART_WRITE						41

#define PATCH_FUNC_MAX									42
struct patch_params
{
    uint32_t patch_magic;
    uint32_t patch_flag;
	uint32_t patch_flag2;
	uint32_t patch_func[PATCH_FUNC_MAX];
};


extern struct patch_params *Patch_params;

/*
 * FUNCTION DECLARATION
 ****************************************************************************************
 */

void patch_init(void);


///@} PATCH

#endif // PATCH_H_
