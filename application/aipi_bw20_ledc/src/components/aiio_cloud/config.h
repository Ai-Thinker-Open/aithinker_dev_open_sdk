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
#define  PRODUCT_ID                         "PKF8pfgb"

/* 产品标识,该标识是用户在创建不同产品时将自定义的产品标识 */
#define  PRODUCT_FLAG                       "CDKZ"


//============================ 空净物模型数据定义 ==================================================


//开关(可下发可上报 rw)
//备注:
#define CMD_POWERSTATE              1
//颜色(可下发可上报 rw)
//备注:
#define CMD_LIGHT_COLOR              2
//亮度(可下发可上报 rw)
//备注:
#define CMD_LIGHT_BRIGHTNESS              3
//灯珠数量(可下发可上报 rw)
//备注:
#define CMD_LIGHT_NUM              4
//模式(可下发可上报 rw)
//备注:
#define CMD_MODE              5
//颜色(可下发可上报 rw)
//备注:
#define CMD_SOUND_COLOR              6
//亮度(可下发可上报 rw)
//备注:
#define CMD_SOUND_BRIGHTNESS              7
//亮度(可下发可上报 rw)
//备注:
#define CMD_BREATHING_BRIGHTNESS              8
//速度(可下发可上报 rw)
//备注:
#define CMD_BREATHING_SPEED              9
//亮度(可下发可上报 rw)
//备注:
#define CMD_RUNNING_BRIGHTNESS              10
//速度(可下发可上报 rw)
//备注:
#define CMD_RUNNING_SPEED              11
//工作模式(可下发可上报 rw)
//备注:
#define CMD_WORK_MODE              12

#endif
