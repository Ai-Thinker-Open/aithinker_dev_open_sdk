#ifndef __AXK_HTTP_IMPORT_H__
#define __AXK_HTTP_IMPORT_H__
//导出给SDK回调AT指令框架的API

#include "at_http.h"

//发起一次http连接
//从云端接收到的数据通过 p_requestParam->http_recive() 回调传递给AT框架
http_connect_result axk_hal_http_create_connect(const st_http_request *p_requestParam);

//获取http资源大小
//返回值
//    >=0：表示获取到的资源大小(单位：字节)
//    -1：参数错误
//    -2：连接错误
//    -255:该API还没有实现，需要适配
int32_t axk_hal_http_get_size(const st_http_request *p_requestParam);

#endif	//end of __AXK_HTTP_IMPORT_H__

