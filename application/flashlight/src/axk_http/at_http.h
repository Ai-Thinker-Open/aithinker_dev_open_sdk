#ifndef __AT_HTTP_H__
#define __AT_HTTP_H__
#include <stdint.h>

#define HTTP_DEBUG_ENABLE	1
#if HTTP_DEBUG_ENABLE
	#define http_debug_log(fmt,...)	printf(fmt,##__VA_ARGS__)
#else
	#define http_debug_log(fmt,...)
#endif

//HTTP数据接收状态
typedef enum __http_recive_status{
	AI_HTTP_RECIVE_STATUS_RECIVEING=0,	//正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
	AI_HTTP_RECIVE_STATUS_ERROR=1,	//数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
	AI_HTTP_RECIVE_STATUS_HAVE_DATA=2,	//表示数据还没结束，后面还有数据包
	AI_HTTP_RECIVE_STATUS_OVER_OK=3,	//表示数据包已经结束，后面没有数据了，并且http请求状态码为200 OK
	AI_HTTP_RECIVE_STATUS_OVER_ERROR=4,	//表示数据包已经结束，后面没有数据了，并且http请求状态码不是200(错误)
	AI_HTTP_RECIVE_STATUS_END	//这个是用来判断枚举类型的结束位置的，必须放在最后一个位置，不要修改
}http_recive_status;

//http请求数据回调函数
//参数
//    status：表示当前请求的状态
//        -2:正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
//        -1：数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
//        0：表示数据包已经结束，后面没有数据了
//        1：表示数据还没结束，后面还有数据包
typedef void (*http_recive_callback_func)(uint8_t *data,int32_t len,http_recive_status status);

//HTTP客户端请求方法
typedef enum __http_opt{
	AI_HTTP_OPT_HEAD=1,
	AI_HTTP_GET=2,
	AI_HTTP_POST=3,
	AI_HTTP_PUT=4,
	AI_HTTP_DELETE=5,
	AI_HTTP_OPT_END	//这个是用来判断枚举类型的结束位置的，必须放在最后一个位置，不要修改
}http_opt;

//HTTP客户端请求content-type
typedef enum __http_content_type{
	AI_HTTP_APPLICATION_X_WWW_FORM_URLENCODED=0,	//application/x-www-form-urlencoded
	AI_HTTP_APPLICATION_JSON=1,	//application/json
	AI_HTTP_MULTIPART_FORM_DATA=2,	//multipart/form-data
	AI_HTTP_TEXT_XML=3,	//text/xml
	AI_HTTP_CONTENT_TYPE_NOT_SET,	//这个表示没有指定content-type(st_http_request中除了content_type成员,http_req_header也可以指定content-type,两个注意不要冲突了)
	AI_HTTP_CONTENT_TYPE_END	//这个是用来判断枚举类型的结束位置的，必须放在最后一个位置，不要修改
}http_content_type;

//HTTP客户端请求content-type
typedef enum __http_transport_type{
	AI_HTTP_TRANSPORT_OVER_TCP=1,	//使用tcp传输(http)
	AI_HTTP_TRANSPORT_OVER_SSL=2,	//使用SSL传输(https)
	AI_HTTP_TRANSPORT_TYPE_END	//这个是用来判断枚举类型的结束位置的，必须放在最后一个位置，不要修改
}http_transport_type;

//http请求参数
typedef struct __http_request{
	int32_t rxbufSize;	//接收缓冲区大小(单包最大的大小)
	int32_t timeout;	//超时时间(单位：ms)
	http_opt opt;	//请求方法
	http_content_type content_type;	//请求数据类型
	http_transport_type transport_type;	//数据传输类型
	char *host;
	char *path;	//host + [:prot] + path 拼起来就是url
	int32_t port;	//端口号
	int32_t dataLen;	//data数据长度(单位：字节)
	uint8_t *data;		//要发送的data数据
	int32_t http_req_header_num;	//http_req_header的数量
	char **http_req_header;	//请求头字符串数组
	http_recive_callback_func http_recive;	//接收到数据的回调函数
}st_http_request;

//HTTP客户端请求 axk_hal_http_create_connect() 返回值
typedef enum __http_connect_result{
	AI_HTTP_CONNECT_SUCCESS=0,	//连接成功
	AI_HTTP_CONNECT_PARAM_ERROR=1,	//参数错误
	AI_HTTP_CONNECT_FAIL=2,	//连接失败
	AI_HTTP_CONNECT_WAIT_TODO=255,	//该API还没有实现，需要适配
	AI_HTTP_CONNECT_RESULT_END	//这个是用来判断枚举类型的结束位置的，必须放在最后一个位置，不要修改
}http_connect_result;

int at_setup_http_client_cmd(int32_t argc, const char **argv);
int at_setup_http_get_size_cmd(int32_t argc, const char **argv);
int at_setup_http_cget_cmd(int32_t argc, const char **argv);
int at_setup_http_cpost_cmd(int32_t argc, const char **argv);
int at_query_http_url_cfg_cmd(int32_t argc, const char **argv);
int at_setup_http_url_cfg_cmd(int32_t argc, const char **argv);

#endif	//end of __AT_HTTP_H__
