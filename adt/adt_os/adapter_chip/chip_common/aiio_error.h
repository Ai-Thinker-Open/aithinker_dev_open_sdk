#ifndef __AIIO_ERROR_H__
#define __AIIO_ERROR_H__


#define AIIO_OK                 (0)
#define AIIO_ERROR              (-1)    //通用错误码
#define AIIO_FUNC_NO_DEFINE     (-2)    //函数未定义
#define AIIO_PARAM_ERROR        (-3)    //参数错误
#define AIIO_PARAM_NOT_SUPPORT  (-4)    //参数设置值不支持
#define AIIO_NOT_INIT           (-5)    //功能未初始化
//串口相关
#define AIIO_UART_BAUD_RATE_NOT_SUPPORT     (-50)    //串口配置错误，不支持的波特率
#define AIIO_UART_DATA_BITS_NOT_SUPPORT     (-51)    //串口配置错误，不支持的数据位
#define AIIO_UART_STOP_BITS_NOT_SUPPORT     (-52)    //串口配置错误，不支持的停止位
#define AIIO_UART_PARITY_NOT_SUPPORT        (-53)    //串口配置错误，不支持的校验位
#define AIIO_UART_FLOW_CONTROL_NOT_SUPPORT  (-54)    //串口配置错误，不支持的流控
#define AIIO_UART_NUMBER_NOT_SUPPORT        (-55)    //串口配置错误，不支持的串口号
#define AIIO_UART_PIN_NOT_SUPPORT           (-56)    //串口配置错误，不支持的pin脚
//GPIO/PWM相关
#define AIIO_PWM_NO_FREE_PWM               (-125)    //没有空闲的PWM线路

#endif
