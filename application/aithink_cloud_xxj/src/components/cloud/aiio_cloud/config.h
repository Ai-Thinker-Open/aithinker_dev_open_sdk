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



// /* 开发平台创建产品后生成的产品ID，用户可根据创建不同产品而获得不同的产品ID，可在该处进行修改*/
// #define  PRODUCT_ID                         "PKEVvgUr"

// /* 产品标识,该标识是用户在创建不同产品时将自定义的产品标识 */
// #define  PRODUCT_FLAG                       "kxx"

/* 开发平台创建产品后生成的产品ID，用户可根据创建不同产品而获得不同的产品ID，可在该处进行修改*/
#define  PRODUCT_ID                         "PKQGRGf0"

/* 产品标识,该标识是用户在创建不同产品时将自定义的产品标识 */
#define  PRODUCT_FLAG                       "ZM"


//============================ 空净物模型数据定义 ==================================================

//开关(可下发可上报 rw)
//备注:
#define CMD_POWERSTATE              1
//喷雾档位设定(可下发可上报 rw)
//备注:
#define CMD_SPRAY_MODE              2
//喷雾模式(可下发可上报 rw)
//备注:
#define CMD_BOTANY_MODE              50

#endif
