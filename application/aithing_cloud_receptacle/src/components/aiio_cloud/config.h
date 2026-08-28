/**
 * @brief   This file describe the data of product dpid
 *
 * @file    config.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-03          <td>1.0.0            <td>zhuolm             <td> Product dpid data
 */
#ifndef __CONFIG_H_
#define __CONFIG_H_

/* 开发平台创建产品后生成的产品ID，用户可根据创建不同产品而获得不同的产品ID，可在该处进行修改*/
#define PRODUCT_ID "PK8pCUCD"

/* 产品标识,该标识是用户在创建不同产品时将自定义的产品标识 */
#define PRODUCT_FLAG "CZ2"

//============================ 空净物模型数据定义 ==================================================

// 开关(可下发可上报 rw)
// 备注:
#define CMD_POWERSTATE 1
// 工作状态(只上报（ro）)
// 备注:
#define CMD_STATUS 2
// 指示灯状态(可下发可上报 rw)
// 备注:
#define CMD_LEDSTATUS 3
// 总耗电量(只上报（ro）)
// 备注:
#define CMD_POWERCONSUMPTION 4
// 电流(只上报（ro）)
// 备注:
#define CMD_ELECTRICCURRENT 5
// 电压(只上报（ro）)
// 备注:
#define CMD_VOLTAGE 6
// 功率(只上报（ro）)
// 备注:
#define CMD_ELECTRICPOWER 7

#endif
