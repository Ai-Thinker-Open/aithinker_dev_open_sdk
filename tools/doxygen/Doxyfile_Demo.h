/** @brief   	这里写你的文件摘要
 *  @file    	Doxyfile_Demo.h
 *  @copyright	Copyright (C) 2020 深圳市安信可科技有限公司
 *  @note    	功能实现以及注意事项
 *  @par	修改日志：
 *  <table>
 *  <tr><th>修订日期        	<th>修订版本  	<th>修订人    	<th>修订描述
 *  <tr><td>2020/07/28  	<td>1.0.0    	<td>zifeng    	<td>规范代码注释
 *  <tr><td>2020/10/28  	<td>1.0.1    	<td>zifeng    	<td>更新代码注释
 *  </table>
 *
 */
#ifndef DOXYFILE_DEMO_H
#define DOXYFILE_DEMO_H


/** 
 *  @brief 简要说明结构体目的 
 */
typedef struct DOXYFILE
{
	long DEV1, 		/*!< 简要说明文字 */
   	long DEV2, 		/*!< 简要说明文字 */
	long DEV3, 		/*!< 简要说明文字 */
}DOXYFILE_DEMO;

long KEIL_VAR01; 		/*!< 简要说明变量作用 */
long KEIL_VAR02; 		/*!< 简要说明变量作用 */

/** 这里写这个函数的概述和作用
 *  @note       注解：功能以及注意事项(首字母大写，英文句号结尾)
 *  @see        类似于请参考XXXX函数之类的(可以链接)
 */
void DOXYFILE_DEMO_01(void);
 
 
/** 
 *  @brief 				这里写这个函数的概述和作用(首字母大写，英文句号结尾)
 *  @param[in]  T1			什么作用
 *  @param[in]  T2			什么作用
 *  @param[in]  *OUT1			什么作用
 *  @return     			函数执行结果(首字母大写，英文句号结尾)
 *  @retval NB_NOTIFY_SUCCESS     	上报成功
 *  @retval NB_NOTIFY_FAIL        	上报失败
 *  @retval NB_IOT_REGIST_FAILED 	注册失败返回
 *  @retval Others  			其他错误
 *  @note       			功能以及注意事项(首字母大写，英文句号结尾)
 *  @see        			类似于请参考XXXX函数之类的(可以链接)
 */
long DOXYFILE_DEMO_02(int T1, int T2, int *OUT1);
 
#endif
