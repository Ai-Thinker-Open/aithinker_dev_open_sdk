//系统头文件
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//AT框架头文件
#include "at_http.h"
#include "axk_at.h"
#include "axk_http_import.h"
// #include "axk_at_import.h"
// #include "axk_basic_import.h"

//系统依赖头文件(FreeRTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "axk_uart_init.h"
#include "esp_http_client.h"

extern esp_http_client_event_id_t http_event;

static struct
{
    char *p_Url;	//超长url的存储空间指针
    int pos;		//特殊模式已经接收的数据
    int length;		//p_longUrl的总长度
} _longUrlCfg = {0};	//AT+HTTPURLCFG 指令接收数据的变量
static struct
{
	st_http_request requestParam;	//保存http请求的参数
    int pos;			//特殊模式已经接收的数据
} _post_data = {0};	//AT+HTTPCPOST 指令接收数据的变量
static http_recive_status g_http_status=AI_HTTP_RECIVE_STATUS_RECIVEING;	//记录http回调的status参数(参考http_recive_callback_func参数定义)

static int url_parse(char *url,char **p_host,char **p_path,int32_t *p_Port,http_transport_type *p_transportType);
static void http_client_recive_callback(uint8_t *data,int32_t len,http_recive_status status);
static void http_get_recive_callback(uint8_t *data,int32_t len,http_recive_status status);
static void at_set_url_callback(char data);
static void http_post_recive_callback(uint8_t *data,int32_t len,http_recive_status status);
static void at_send_post_data_callback(char data);
static void CleanParam();


int at_setup_http_client_cmd(int32_t argc, const char **argv){
	//AT+HTTPCLIENT=<opt>,<content-type>,<"url">,[<"host">],[<"path">],<transport_type>[,<"data">][,<"http_req_header">][,<"http_req_header">][...]
	int ret=AT_ERR_FAILURE;
	if(argc<6){
		http_debug_log("[%s()-%d]argc<6 is error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}
	st_http_request requestParam={0};
	requestParam.timeout=6000;	//默认超时时间(ms)
	requestParam.rxbufSize=512;	//接收buf默认大小
	requestParam.http_recive=http_client_recive_callback;

	//opt
	int i_tmp=atoi(argv[0]);
	if(i_tmp<AI_HTTP_OPT_HEAD || i_tmp>AI_HTTP_OPT_END-1){
		http_debug_log("[%s()-%d]opt=%d error\r\n",__func__,__LINE__,i_tmp);
		return AT_INVALID_PARAMETER;
	}
	requestParam.opt=i_tmp;

	//content-type
	i_tmp=atoi(argv[1]);
	if(i_tmp<AI_HTTP_APPLICATION_X_WWW_FORM_URLENCODED || i_tmp>AI_HTTP_CONTENT_TYPE_END-1){
		http_debug_log("[%s()-%d]content-type=%d error\r\n",__func__,__LINE__,i_tmp);
		return AT_INVALID_PARAMETER;
	}
	requestParam.content_type=i_tmp;

	//url
	int urlLen=strlen(argv[2]);
	if(1==urlLen || (urlLen>1 && ('"'!=argv[2][0] || '"'!=argv[2][urlLen-1] ) ) ){
		http_debug_log("[%s()-%d]url error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}
	char *p_paramUrlMalloc=NULL;	//malloc申请的URL参数
	if(0==urlLen ||2==urlLen){
		p_paramUrlMalloc=malloc(1);
		if(NULL==p_paramUrlMalloc){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			return AT_ERR_FAILURE;
		}
		p_paramUrlMalloc[0]=0;
	}else{//这里的长度必然>2
		p_paramUrlMalloc=malloc(urlLen-1);
		if(NULL==p_paramUrlMalloc){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			return AT_ERR_FAILURE;
		}
		memset(p_paramUrlMalloc,0,urlLen-1);
		memcpy(p_paramUrlMalloc,argv[2]+1,urlLen-2);
	}
	urlLen=strlen(p_paramUrlMalloc);

	//host
	int hostLen=strlen(argv[3]);
	if(1==hostLen || (hostLen>1 && ('"'!=argv[3][0] || '"'!=argv[3][hostLen-1] ) ) ){
		http_debug_log("[%s()-%d]host error\r\n",__func__,__LINE__);
		ret=AT_INVALID_PARAMETER;
		goto EXIT1;
	}
	char *p_paramHostMalloc=NULL;	//malloc申请的host参数
	if(0==hostLen ||2==hostLen){
		p_paramHostMalloc=malloc(1);
		if(NULL==p_paramHostMalloc){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			ret=AT_ERR_FAILURE;
			goto EXIT1;
		}
		p_paramHostMalloc[0]=0;
	}else{//这里的长度必然>2
		p_paramHostMalloc=malloc(hostLen-1);
		if(NULL==p_paramHostMalloc){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			ret=AT_ERR_FAILURE;
			goto EXIT1;
		}
		memset(p_paramHostMalloc,0,hostLen-1);
		memcpy(p_paramHostMalloc,argv[3]+1,hostLen-2);
	}
	hostLen=strlen(p_paramHostMalloc);

	//path
	int pathLen=strlen(argv[4]);
	if(1==pathLen || (pathLen>1 && ('"'!=argv[4][0] || '"'!=argv[4][pathLen-1] ) ) ){
		http_debug_log("[%s()-%d]path error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}
	char *p_paramPathMalloc=NULL;	//malloc申请的path参数
	if(0==pathLen ||2==pathLen){
		p_paramPathMalloc=(char *)malloc(1);
		if(NULL==p_paramPathMalloc){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			ret=AT_ERR_FAILURE;
			goto EXIT2;
		}
		p_paramPathMalloc[0]=0;
	}else{//这里的长度必然>2
		p_paramPathMalloc=(char *)malloc(pathLen-1);
		if(NULL==p_paramPathMalloc){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			ret=AT_ERR_FAILURE;
			goto EXIT2;
		}
		memset(p_paramPathMalloc,0,pathLen-1);
		memcpy(p_paramPathMalloc,argv[4]+1,pathLen-2);
	}
	pathLen=strlen(p_paramPathMalloc);

	//transport-type
	i_tmp=atoi(argv[5]);
	if(i_tmp<AI_HTTP_TRANSPORT_OVER_TCP || i_tmp>AI_HTTP_TRANSPORT_TYPE_END-1){
		http_debug_log("[%s()-%d]transport-type=%d error\r\n",__func__,__LINE__,i_tmp);
		ret=AT_INVALID_PARAMETER;
		goto EXIT3;
	}
	requestParam.transport_type=i_tmp;

	//判断是否有data参数
	char *p_paramDataMalloc=NULL;	//存储data的数据
	if(AI_HTTP_POST==requestParam.opt){
		//POST有data参数,处理data参数
		if(argc>6){
			int dataLen=strlen(argv[6]);
			if(1==dataLen || (dataLen>1 && ('"'!=argv[6][0] || '"'!=argv[6][dataLen-1]) ) ){
				http_debug_log("[%s()-%d]data error\r\n",__func__,__LINE__);
				ret=AT_INVALID_PARAMETER;
				goto EXIT3;
			}
			if(0==dataLen ||2==dataLen){
				p_paramDataMalloc=malloc(1);
				if(NULL==p_paramDataMalloc){
					http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
					ret=AT_ERR_FAILURE;
					goto EXIT3;
				}
				p_paramDataMalloc[0]=0;
			}else{//这里的 i_tmp 必然>2
				p_paramDataMalloc=malloc(dataLen-1);
				if(NULL==p_paramDataMalloc){
					http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
					ret=AT_ERR_FAILURE;
					goto EXIT3;
				}
				memset(p_paramDataMalloc,0,dataLen-1);
				memcpy(p_paramDataMalloc,argv[6]+1,dataLen-2);
			}
			dataLen=strlen(p_paramDataMalloc);
		}else{
			//没有data则设置为空字符串，避免后面计算长度出错
			p_paramDataMalloc=malloc(1);
			if(NULL==p_paramDataMalloc){
				http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
				ret=AT_ERR_FAILURE;
				goto EXIT3;
			}
			p_paramDataMalloc[0]=0;
		}
		
	}else{
		//不是POST请求，没有data参数
		//没有data则设置为空字符串，避免后面计算长度出错
		p_paramDataMalloc=malloc(1);
		if(NULL==p_paramDataMalloc){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			ret=AT_ERR_FAILURE;
			goto EXIT3;
		}
		p_paramDataMalloc[0]=0;
	}
	requestParam.dataLen=(int32_t)strlen(p_paramDataMalloc);
	requestParam.data=(uint8_t *)p_paramDataMalloc;
	

	//head
	requestParam.http_req_header_num=0;
	int firstHeadIndex=7;	//记录第一个head的参数索引
	if(AI_HTTP_POST!=requestParam.opt){
		firstHeadIndex=6;	//非post请求没有data数据，所以索引要迁移一位
	}
	if(argc>firstHeadIndex){
		requestParam.http_req_header_num=argc-firstHeadIndex;
		requestParam.http_req_header=malloc(requestParam.http_req_header_num*sizeof(char *));
		if(NULL==requestParam.http_req_header){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			ret=AT_ERR_FAILURE;
			goto EXIT5;
		}
		memset(requestParam.http_req_header,0,requestParam.http_req_header_num*sizeof(char *));
		for(int i=0;i<requestParam.http_req_header_num;i++){
			i_tmp=strlen(argv[i+firstHeadIndex]);
			if(i_tmp<2 || '"'!=argv[i+firstHeadIndex][0] || '"'!=argv[i+firstHeadIndex][i_tmp-1]){
				http_debug_log("[%s()-%d]http req header error(index=%d)\r\n",__func__,__LINE__,i);
				ret=AT_INVALID_PARAMETER;
				goto EXIT5;
			}
			requestParam.http_req_header[i]=malloc(i_tmp-1);
			if(NULL==requestParam.http_req_header[i]){
				http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
				ret=AT_ERR_FAILURE;
				goto EXIT5;
			}
			memset(requestParam.http_req_header[i],0,i_tmp-1);
			memcpy(requestParam.http_req_header[i],argv[firstHeadIndex+i]+1,i_tmp-2);
		}
	}

	//url/host
	if(0==urlLen && NULL==_longUrlCfg.p_Url && (0==hostLen || 0==pathLen) ){
		http_debug_log("[%s()-%d]URL OR HOST/PATH error\r\n",__func__,__LINE__);
		ret=AT_INVALID_PARAMETER;
		goto EXIT5;
	}

	if(0==urlLen && NULL!=_longUrlCfg.p_Url){//使用 _longUrlCfg.p_Url 作为url
		free(p_paramPathMalloc);
		p_paramPathMalloc=NULL;
		free(p_paramHostMalloc);
		p_paramHostMalloc=NULL;
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;

		if(0!=url_parse(_longUrlCfg.p_Url,&requestParam.host,&requestParam.path,&requestParam.port,&requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			ret=AT_INVALID_PARAMETER;
			goto EXIT5;
		}
	}else if(urlLen>0){	//使用 p_paramUrlMalloc 作为url
		free(p_paramPathMalloc);
		p_paramPathMalloc=NULL;
		free(p_paramHostMalloc);
		p_paramHostMalloc=NULL;

		if(0!=url_parse(p_paramUrlMalloc,&requestParam.host,&requestParam.path,&requestParam.port,&requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			ret=AT_INVALID_PARAMETER;
			goto EXIT5;
		}
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}else{//使用host+path组成url
		requestParam.host=p_paramHostMalloc;
		requestParam.path=p_paramPathMalloc;
		p_paramPathMalloc=NULL;	//path 空间让给requestParam.path 记录了，这里不释放空间，直接将 p_paramPathMalloc 置 NULL ，避免后面重复释放
		p_paramHostMalloc=NULL;	//host 空间让给requestParam.host 记录了，这里不释放空间，直接将 p_paramHostMalloc 置 NULL ，避免后面重复释放
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
		requestParam.port=-1;	//表示需要使用默认端口号
	}
	if(-1==requestParam.port){	//使用默认端口号
		switch(requestParam.transport_type){
			case AI_HTTP_TRANSPORT_OVER_TCP:	//http默认使用80端口
				requestParam.port=80;
				break;
			case AI_HTTP_TRANSPORT_OVER_SSL:	//ssl默认使用443端口
				requestParam.port=443;
				break;
			default:
				http_debug_log("[%s()-%d]transport_type error\r\n",__func__,__LINE__);
				ret=AT_INVALID_PARAMETER;
				goto EXIT6;
		}
	}
	//创建连接
	g_http_status=AI_HTTP_RECIVE_STATUS_RECIVEING;	//初始化一下 g_http_status
	http_connect_result connectResult=axk_hal_http_create_connect(&requestParam);
	if(AI_HTTP_CONNECT_SUCCESS!=connectResult){
		http_debug_log("[%s()-%d]http connect error,ret=%d\r\n",__func__,__LINE__,connectResult);
		ret=AT_ERR_FAILURE;
		goto EXIT6;
	}
	int timeoutCount=0;
	while(1){
		switch(g_http_status){//这个在接收数据回调 http_client_recive_callback() 中修改，SDK调用 requestParam.http_recive() 即可
			case AI_HTTP_RECIVE_STATUS_RECIVEING:	//正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
				if(timeoutCount++>10*60){
					http_debug_log("[%s()-%d]http recive timeout break\r\n",__func__,__LINE__);
					ret=AT_ERR_FAILURE;
					goto EXIT6;
				}
				break;
			case AI_HTTP_RECIVE_STATUS_ERROR:	//数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
				http_debug_log("[%s()-%d]http recive error\r\n",__func__,__LINE__);
				ret=AT_ERR_FAILURE;
				goto EXIT6;
			case AI_HTTP_RECIVE_STATUS_OVER_OK:	//表示数据包已经结束，后面没有数据了，并且http请求状态码为200 OK
				axk_at_printf("\r\nOK\r\n");
				ret=AT_ERR_SUCCESS;
				goto EXIT6;
			case AI_HTTP_RECIVE_STATUS_OVER_ERROR:	//表示数据包已经结束，后面没有数据了，并且http请求状态码不是200(错误)
				http_debug_log("[%s()-%d]http status is not 200\r\n",__func__,__LINE__);
				ret=AT_ERR_FAILURE;
				goto EXIT6;
			case AI_HTTP_RECIVE_STATUS_HAVE_DATA: //表示数据还没结束，后面还有数据包
				break;
			default:
				http_debug_log("[%s()-%d]unknown http status:%d\r\n",__func__,__LINE__,(int)g_http_status);
				ret=AT_ERR_FAILURE;
				goto EXIT6;
		}
		vTaskDelay(100/portTICK_RATE_MS);
	}

EXIT6:
	if(NULL!=requestParam.host){
		free(requestParam.host);
		requestParam.host=NULL;
	}
	if(NULL!=requestParam.path){
		free(requestParam.path);
		requestParam.path=NULL;
	}
EXIT5:
	if(NULL!=requestParam.http_req_header){
		for(int i=0;i<requestParam.http_req_header_num;i++){
			if(NULL!=requestParam.http_req_header[i]){
				free(requestParam.http_req_header[i]);
				requestParam.http_req_header[i]=NULL;
			}
		}
		free(requestParam.http_req_header);
		requestParam.http_req_header=NULL;
	}
//EXIT4:
	if(NULL!=p_paramDataMalloc){
		free(p_paramDataMalloc);
		p_paramDataMalloc=NULL;
	}
EXIT3:
	if(NULL!=p_paramPathMalloc){
		free(p_paramPathMalloc);
		p_paramPathMalloc=NULL;
	}
EXIT2:
	if(NULL!=p_paramHostMalloc){
		free(p_paramHostMalloc);
		p_paramHostMalloc=NULL;
	}
EXIT1:
	if(NULL!=p_paramUrlMalloc){
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}
	CleanParam();	//清空之前的数据
	return ret;
}

//AT+HTTPGETSIZE=<"url">
int at_setup_http_get_size_cmd(int32_t argc, const char **argv){
	if(1!=argc){
		http_debug_log("[%s()-%d]argc=%d is error\r\n",__func__,__LINE__,(int)argc);
		return AT_INVALID_PARAMETER;
	}
	int urlLen=strlen(argv[0]);
	if( urlLen<11 && 2!=urlLen ){
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}
	if( '"'!=argv[0][0] || '"'!=argv[0][urlLen-1]){
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}
	char *p_paramUrlMalloc=NULL;	//malloc申请的URL参数
	p_paramUrlMalloc=malloc(urlLen-1);
	if(NULL==p_paramUrlMalloc){
		http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
		return AT_ERR_FAILURE;
	}
	memset(p_paramUrlMalloc,0,urlLen-1);
	memcpy(p_paramUrlMalloc,argv[0]+1,urlLen-2);
	urlLen=strlen(p_paramUrlMalloc);

	int i_ret=-255;
	st_http_request requestParam={0};
	requestParam.timeout=6000;	//默认超时时间(ms)
	requestParam.rxbufSize=512;	//接收buf默认大小
	requestParam.opt=AI_HTTP_GET;
	if(0==urlLen && NULL!=_longUrlCfg.p_Url){//使用 _longUrlCfg.p_Url 作为url
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;

		if(0!=url_parse(_longUrlCfg.p_Url,&requestParam.host,&requestParam.path,&requestParam.port,&requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			i_ret=AT_INVALID_PARAMETER;
			goto EXIT2;
		}
	}else if(urlLen>0){	//使用 p_paramUrlMalloc 作为url
		if(0!=url_parse(p_paramUrlMalloc,&requestParam.host,&requestParam.path,&requestParam.port,&requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			i_ret=AT_INVALID_PARAMETER;
			goto EXIT2;
		}
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}else{
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		i_ret=AT_INVALID_PARAMETER;
		goto EXIT2;
	}
	int32_t httpSize=axk_hal_http_get_size(&requestParam);
	if(httpSize<0){
		http_debug_log("[%s()-%d]get http size error,ret=%d\r\n",__func__,__LINE__,(int)httpSize);
		i_ret=AT_ERR_FAILURE;
		goto EXIT3;
	}
	axk_at_printf("+HTTPGETSIZE:%d\r\n\r\nOK\r\n",httpSize);	//获取成功，打印
	i_ret = AT_ERR_SUCCESS;
	
EXIT3:
	if(NULL!=requestParam.host){
		free(requestParam.host);
		requestParam.host=NULL;
	}
	if(NULL!=requestParam.path){
		free(requestParam.path);
		requestParam.path=NULL;
	}
EXIT2:
	if(NULL!=p_paramUrlMalloc){
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}
//EXIT1:
	return i_ret;
}

int at_setup_http_cget_cmd(int32_t argc, const char **argv){
	if(argc<1){
		http_debug_log("[%s()-%d]argc=%d is error\r\n",__func__,__LINE__,(int)argc);
		return AT_INVALID_PARAMETER;
	}
	int urlLen=strlen(argv[0]);
	if( urlLen<11 && 2!=urlLen ){
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}
	if( '"'!=argv[0][0] || '"'!=argv[0][urlLen-1]){
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}

	char *p_paramUrlMalloc=NULL;	//malloc申请的URL参数
	p_paramUrlMalloc=malloc(urlLen-1);
	if(NULL==p_paramUrlMalloc){
		http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
		return AT_ERR_FAILURE;
	}
	memset(p_paramUrlMalloc,0,urlLen-1);
	memcpy(p_paramUrlMalloc,argv[0]+1,urlLen-2);
	urlLen=strlen(p_paramUrlMalloc);

	int i_ret=-255;
	st_http_request requestParam={0};
	requestParam.content_type=AI_HTTP_CONTENT_TYPE_NOT_SET;	//默认不指定content-type
	requestParam.opt=AI_HTTP_GET;
	requestParam.http_recive=http_get_recive_callback;
	if(argc>2){
		requestParam.rxbufSize=atoi(argv[2]);
	}else{
		requestParam.rxbufSize=2048;	//接收buf默认大小
	}
	if(argc>3){
		requestParam.timeout=atoi(argv[3]);
	}else{
		requestParam.timeout=5000;	//默认超时时间(ms)
	}
	if(0==urlLen && NULL!=_longUrlCfg.p_Url){//使用 _longUrlCfg.p_Url 作为url
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;

		if(0!=url_parse(_longUrlCfg.p_Url,&requestParam.host,&requestParam.path,&requestParam.port,&requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			i_ret=AT_INVALID_PARAMETER;
			goto EXIT2;
		}
	}else if(urlLen>0){	//使用 p_paramUrlMalloc 作为url
		if(0!=url_parse(p_paramUrlMalloc,&requestParam.host,&requestParam.path,&requestParam.port,&requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			i_ret=AT_INVALID_PARAMETER;
			goto EXIT2;
		}
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}else{
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		i_ret=AT_INVALID_PARAMETER;
		goto EXIT2;
	}
	requestParam.dataLen=0;

	//发起连接
	g_http_status=AI_HTTP_RECIVE_STATUS_RECIVEING;	//初始化一下 g_http_status
	http_connect_result connectResult=axk_hal_http_create_connect(&requestParam);
	if(AI_HTTP_CONNECT_SUCCESS!=connectResult){
		http_debug_log("[%s()-%d]http get error,ret=%d\r\n",__func__,__LINE__,connectResult);
		i_ret=AT_ERR_FAILURE;
		goto EXIT3;
	}
	int timeoutCount=0;
	while(1){
		switch(g_http_status){//这个在接收数据回调 http_get_recive_callback() 中修改，SDK调用 requestParam.http_recive() 即可
			case AI_HTTP_RECIVE_STATUS_RECIVEING:	//正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
				if(timeoutCount++>(requestParam.timeout/100)){
					http_debug_log("[%s()-%d]http recive timeout break\r\n",__func__,__LINE__);
					i_ret=AT_ERR_FAILURE;
					goto EXIT3;
				}
				break;
			case AI_HTTP_RECIVE_STATUS_ERROR:	//数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
				http_debug_log("[%s()-%d]http recive error\r\n",__func__,__LINE__);
				i_ret=AT_ERR_FAILURE;
				goto EXIT3;
			case AI_HTTP_RECIVE_STATUS_OVER_OK:	//表示数据包已经结束，后面没有数据了，并且http请求状态码为200 OK
				axk_at_printf("OK\r\n");
				i_ret=AT_ERR_SUCCESS;
				goto EXIT3;
			case AI_HTTP_RECIVE_STATUS_OVER_ERROR:	//表示数据包已经结束，后面没有数据了，并且http请求状态码不是200(错误)
				http_debug_log("[%s()-%d]http status not 200\r\n",__func__,__LINE__);
				i_ret=AT_ERR_FAILURE;
				goto EXIT3;
			case AI_HTTP_RECIVE_STATUS_HAVE_DATA: //表示数据还没结束，后面还有数据包
				break;
			default:
				http_debug_log("[%s()-%d]unknown http status:%d\r\n",__func__,__LINE__,(int)g_http_status);
				i_ret=AT_ERR_FAILURE;
				goto EXIT3;
		}
		vTaskDelay(100/portTICK_RATE_MS);
	}

EXIT3:
	if(NULL!=requestParam.host){
		free(requestParam.host);
		requestParam.host=NULL;
	}
	if(NULL!=requestParam.path){
		free(requestParam.path);
		requestParam.path=NULL;
	}
EXIT2:
	if(NULL!=p_paramUrlMalloc){
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}
//EXIT1:
	return i_ret;
}

//AT+HTTPCPOST=<"url">,<length>[,<http_req_header_cnt>][,<http_req_header>..<http_req_header>]
int at_setup_http_cpost_cmd(int32_t argc, const char **argv){
	int i_ret=AT_ERR_FAILURE;

	CleanParam();	//清空之前的数据
	
	if(argc<2){
		http_debug_log("[%s()-%d]argc=%d is error\r\n",__func__,__LINE__,(int)argc);
		return AT_INVALID_PARAMETER;
	}
	int urlLen=strlen(argv[0]);
	if(urlLen<9 && 2!=urlLen){
		http_debug_log("[%s()-%d]len=%d is error\r\n",__func__,__LINE__,urlLen);
		return AT_INVALID_PARAMETER;
	}
	if( '"'!=argv[0][0] || '"'!=argv[0][urlLen-1]){
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		return AT_INVALID_PARAMETER;
	}
	_post_data.requestParam.dataLen=atoi(argv[1]);
	if(_post_data.requestParam.dataLen<1){
		http_debug_log("[%s()-%d]data len=%d is error\r\n",__func__,__LINE__,_post_data.requestParam.dataLen);
		return AT_INVALID_PARAMETER;
	}
	int httpReqHeaderCnt=0;
	if(argc>2){
		httpReqHeaderCnt=atoi(argv[2]);
	}
	if(httpReqHeaderCnt<0){
		http_debug_log("[%s()-%d]http_req_header_cnt=%d is error\r\n",__func__,__LINE__,httpReqHeaderCnt);
		return AT_INVALID_PARAMETER;
	}
	if( (argc!=httpReqHeaderCnt+2) && (argc!=httpReqHeaderCnt+3) ){
		http_debug_log("[%s()-%d]argc=%d httpReqHeaderCnt=%d is error\r\n",__func__,__LINE__,(int)argc,httpReqHeaderCnt);
		return AT_INVALID_PARAMETER;
	}
	char *p_paramUrlMalloc=NULL;	//malloc申请的URL参数
	p_paramUrlMalloc=malloc(urlLen-1);
	if(NULL==p_paramUrlMalloc){
		http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
		return AT_ERR_FAILURE;
	}
	memset(p_paramUrlMalloc,0,urlLen-1);
	memcpy(p_paramUrlMalloc,argv[0]+1,urlLen-2);
	urlLen=strlen(p_paramUrlMalloc);
	
	
	_post_data.requestParam.opt=AI_HTTP_POST;
	_post_data.requestParam.http_recive=http_post_recive_callback;
	_post_data.requestParam.rxbufSize=512;
	if(0==urlLen && NULL!=_longUrlCfg.p_Url){//使用 _longUrlCfg.p_Url 作为url
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;

		if(0!=url_parse(_longUrlCfg.p_Url,&_post_data.requestParam.host,&_post_data.requestParam.path,&_post_data.requestParam.port,&_post_data.requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			i_ret=AT_INVALID_PARAMETER;
			goto EXIT3;
		}
	}else if(urlLen>0){	//使用 p_paramUrlMalloc 作为url
		if(0!=url_parse(p_paramUrlMalloc,&_post_data.requestParam.host,&_post_data.requestParam.path,&_post_data.requestParam.port,&_post_data.requestParam.transport_type) ){
			//url解析失败
			http_debug_log("[%s()-%d]URL parse error\r\n",__func__,__LINE__);
			i_ret=AT_INVALID_PARAMETER;
			goto EXIT3;
		}
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}else{
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		i_ret=AT_INVALID_PARAMETER;
		goto EXIT2;
	}
	//指令没有指定content-type(不影响head中的设置)
	_post_data.requestParam.content_type=AI_HTTP_CONTENT_TYPE_NOT_SET;
	//http_req_header
	if(httpReqHeaderCnt>0){
		_post_data.requestParam.http_req_header=malloc(httpReqHeaderCnt*sizeof(char *));
		if(NULL==_post_data.requestParam.http_req_header){
			http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
			i_ret=AT_ERR_FAILURE;
			goto EXIT4;
		}
		memset(_post_data.requestParam.http_req_header,0,httpReqHeaderCnt*sizeof(char *));
		_post_data.requestParam.http_req_header_num=httpReqHeaderCnt;
		for(int i=0;i<_post_data.requestParam.http_req_header_num;i++){
			int i_tmp=strlen(argv[i+3]);
			if(i_tmp<2 || '"'!=argv[i+3][0] || '"'!=argv[i+3][i_tmp-1]){
				http_debug_log("[%s()-%d]http req header error(index=%d)\r\n",__func__,__LINE__,i);
				i_ret=AT_INVALID_PARAMETER;
				goto EXIT4;
			}
			_post_data.requestParam.http_req_header[i]=malloc(i_tmp-1);
			if(NULL==_post_data.requestParam.http_req_header[i]){
				http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
				i_ret=AT_ERR_FAILURE;
				goto EXIT4;
			}
			memset(_post_data.requestParam.http_req_header[i],0,i_tmp-1);
			memcpy(_post_data.requestParam.http_req_header[i],argv[3+i]+1,i_tmp-2);
		}
	}

	_post_data.pos=0;
	_post_data.requestParam.data=malloc(_post_data.requestParam.dataLen+1);
	if(NULL==_post_data.requestParam.data){
		http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
		i_ret=AT_ERR_FAILURE;
		goto EXIT5;
	}
	//进入特殊模式接收数据
	if (0 != at_port_enter_specific(at_send_post_data_callback)) {
		http_debug_log("[%s()-%d]enter specific error\r\n",__func__,__LINE__);
        i_ret=AT_ERR_FAILURE;
		goto EXIT5;
    }
	i_ret=AT_ERR_SUCCESS;
	axk_at_printf("\r\nOK\r\n\r\n>");
	goto EXIT2;

EXIT5:
	if(NULL!=_post_data.requestParam.data){
		free(_post_data.requestParam.data);
		_post_data.requestParam.data=NULL;
	}
EXIT4:
	if(NULL!=_post_data.requestParam.http_req_header){
		for(int i=0;i<_post_data.requestParam.http_req_header_num;i++){
			if(NULL!=_post_data.requestParam.http_req_header[i]){
				free(_post_data.requestParam.http_req_header[i]);
				_post_data.requestParam.http_req_header[i]=NULL;
			}
		}
		free(_post_data.requestParam.http_req_header);
		_post_data.requestParam.http_req_header=NULL;
		_post_data.requestParam.http_req_header_num=0;
	}
EXIT3:
	if(NULL!=_post_data.requestParam.host){
		free(_post_data.requestParam.host);
		_post_data.requestParam.host=NULL;
	}
	if(NULL!=_post_data.requestParam.path){
		free(_post_data.requestParam.path);
		_post_data.requestParam.path=NULL;
	}
EXIT2:
	if(NULL!=p_paramUrlMalloc){
		free(p_paramUrlMalloc);
		p_paramUrlMalloc=NULL;
	}
//EXIT1:
	return i_ret;
}

//AT+HTTPURLCFG?
int at_query_http_url_cfg_cmd(int32_t argc, const char **argv){
	if(NULL==_longUrlCfg.p_Url){
		http_debug_log("[%s()-%d]log url not set\r\n",__func__,__LINE__);
	}else{
		axk_at_printf("+HTTPURLCFG:%d,%s\r\n",strlen(_longUrlCfg.p_Url),_longUrlCfg.p_Url);
	}
	axk_at_printf("OK\r\n");
	return AT_ERR_SUCCESS;
}

//AT+HTTPURLCFG=<url length>
int at_setup_http_url_cfg_cmd(int32_t argc, const char **argv){
	if(1!=argc){
		http_debug_log("[%s()-%d]argc=%d is error\r\n",__func__,__LINE__,(int)argc);
		return AT_INVALID_PARAMETER;
	}
	int urlLen=atoi(argv[0]);
	if(0==urlLen){
		if(NULL!=_longUrlCfg.p_Url){
			free(_longUrlCfg.p_Url);
		}
		_longUrlCfg.p_Url=NULL;
		_longUrlCfg.length=0;
		_longUrlCfg.pos=0;
		axk_at_printf("OK\r\n");
		return AT_ERR_SUCCESS;
	}
	if(urlLen<9){
		http_debug_log("[%s()-%d]len=%d is error\r\n",__func__,__LINE__,urlLen);
		return AT_INVALID_PARAMETER;
	}
	if(NULL!=_longUrlCfg.p_Url){
		free(_longUrlCfg.p_Url);
		_longUrlCfg.p_Url=NULL;
		_longUrlCfg.length=0;
	}
	_longUrlCfg.p_Url=malloc(urlLen+1);
	if(NULL==_longUrlCfg.p_Url){
		http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
		return AT_ERR_FAILURE;
	}
	memset(_longUrlCfg.p_Url,0,urlLen+1);
	_longUrlCfg.length=urlLen;
	_longUrlCfg.pos=0;
	if (0 != at_port_enter_specific(at_set_url_callback)) {
		http_debug_log("[%s()-%d]enter specific error\r\n",__func__,__LINE__);
        return AT_ERR_FAILURE;
    }
	axk_at_printf("\r\nOK\r\n\r\n>");
	return AT_ERR_SUCCESS;
}

//----------------------------------------- 内部调用函数 ----------------------------
//清空 _post_data 数据
static void CleanParam(){
	if(NULL!=_post_data.requestParam.host){
		free(_post_data.requestParam.host);
		_post_data.requestParam.host=NULL;
	}
	if(NULL!=_post_data.requestParam.path){
		free(_post_data.requestParam.path);
		_post_data.requestParam.path=NULL;
	}
	if(NULL!=_post_data.requestParam.http_req_header){
		for(int i=0;i<_post_data.requestParam.http_req_header_num;i++){
			if(NULL!=_post_data.requestParam.http_req_header[i]){
				free(_post_data.requestParam.http_req_header[i]);
				_post_data.requestParam.http_req_header[i]=NULL;
			}
		}
		free(_post_data.requestParam.http_req_header);
		_post_data.requestParam.http_req_header=NULL;
		_post_data.requestParam.http_req_header_num=0;
	}
	if(NULL!=_post_data.requestParam.data){
		free(_post_data.requestParam.data);
		_post_data.requestParam.data=NULL;
		_post_data.requestParam.dataLen=0;
	}
	memset(&_post_data.requestParam,0,sizeof(_post_data.requestParam));
}

//将url解析为hot+path+port，通过参数传出
//参数传入的是指针的地址，函数内部分配空间，使用完成后需要自己free
//    *p_host,*p_path 成功为对应空间指针(使用完成后需要自己free)，失败返回NULL
//    *p_Port：url中指定了port则为对应值，否则为-1
//返回值
//    0：解析成功
//    -1:参数错误
//    -2：格式错误
//    -3：malloc失败
static int url_parse(char *url,char **p_host,char **p_path,int32_t *p_Port,http_transport_type *p_transportType){
	if(NULL==url || NULL==p_host || NULL==p_path || NULL==p_Port){
		http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
		return -1;
	}
	if(strlen(url)<9){
		http_debug_log("[%s()-%d]url len error,url:%s\r\n",__func__,__LINE__,url);
		return -2;
	}

	char *p_start=NULL;
	char *p_url=NULL;
	char tmpBuf[9]={0};
	memcpy(tmpBuf,url,7);
	if(0==strcasecmp(tmpBuf,"http://")){
		*p_transportType=AI_HTTP_TRANSPORT_OVER_TCP;
		*p_Port=80;	//http默认端口为80
		p_start=url+7;
		p_url=url+7;
	}else{
		memcpy(tmpBuf,url,8);
		if(0==strcasecmp(tmpBuf,"https://")){
			*p_transportType=AI_HTTP_TRANSPORT_OVER_SSL;
			*p_Port=443;	//https默认端口为443
			p_start=url+8;
			p_url=url+8;
		}else{
			http_debug_log("[%s()-%d]url head error\r\n",__func__,__LINE__);
			return -2;
		}
	}
	
	*p_host=NULL;
	*p_path=NULL;
	int ret=-255;
	enum{
		PARSE_HTTP_HOST,	//解析host
		PARSE_HTTP_PORT,	//解析port
		PARSE_HTTP_PATH,	//解析path
	}parseStep=PARSE_HTTP_HOST;
	for(;0!=(*p_url);p_url++){
		//http_debug_log("[%s()-%d]*p_url=%c *p_start=%c parseStep=%d\r\n",__func__,__LINE__,*p_url,*p_start,parseStep);
		switch(parseStep){
			case PARSE_HTTP_HOST:
				if(':'==(*p_url) ){
					*p_host=malloc(p_url-p_start+1);
					if(NULL==(*p_host) ){
						http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
						return -3;
					}
					memcpy(*p_host,p_start,p_url-p_start);
					(*p_host)[p_url-p_start]='\0';
					p_start=p_start+(p_url-p_start+1);
					parseStep=PARSE_HTTP_PORT;
					break;
				}
				if('/'==(*p_url) ){
					*p_host=malloc(p_url-p_start+1);
					if(NULL==(*p_host) ){
						http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
						return -3;
					}
					memcpy(*p_host,p_start,p_url-p_start);
					(*p_host)[p_url-p_start]='\0';
					p_start=p_start+(p_url-p_start);
					if('\0'==p_url[1]){//如果已经结束则直接设置path
						*p_path=malloc(2);
						if(NULL==(*p_path) ){
							http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
							ret=-3;
							goto EXIT1;
						}
						(*p_path)[0]='/';
						(*p_path)[1]='\0';
						goto PARSE_OVER;
					}
					parseStep=PARSE_HTTP_PATH;
					break;
				}
				break;
			case PARSE_HTTP_PORT:
				if('/'==(*p_url) ){
					if(p_url-p_start>5){
						http_debug_log("[%s()-%d]port len error\r\n",__func__,__LINE__);
						ret=-2;
						goto EXIT1;
					}
					char portStr[6]={0};
					memcpy(portStr,p_start,p_url-p_start);
					*p_Port=atoi(portStr);
					if(*p_Port<0 || *p_Port>65535){
						http_debug_log("[%s()-%d]port=%d is error\r\n",__func__,__LINE__,(int)(*p_Port));
						ret=-2;
						goto EXIT1;
					}
					p_start=p_start+(p_url-p_start);
					if('\0'==p_url[1]){//如果已经结束则直接设置path
						*p_path=malloc(2);
						if(NULL==(*p_path) ){
							http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
							ret=-3;
							goto EXIT1;
						}
						(*p_path)[0]='/';
						(*p_path)[1]='\0';
						goto PARSE_OVER;
					}
					parseStep=PARSE_HTTP_PATH;
					break;
				}
				break;
			case PARSE_HTTP_PATH:
				*p_path=malloc(strlen(p_start)+1);
				if(NULL==(*p_path) ){
					http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
					ret=-3;
					goto EXIT1;
				}
				memcpy(*p_path,p_start,strlen(p_start));
				(*p_path)[strlen(p_start)]='\0';
				goto PARSE_OVER;
		}
	}
PARSE_OVER:
	if(NULL==(*p_host) || NULL==(*p_path) ){
		http_debug_log("[%s()-%d]parse error\r\n",__func__,__LINE__);
		ret=-3;
		goto EXIT2;
	}
	return 0;	//成功返回

EXIT2:
	if(NULL!=(*p_path) ){
		free(*p_path);
		*p_path=NULL;
	}
EXIT1:
	if(NULL!=(*p_host) ){
		free(*p_host);
		*p_host=NULL;
	}
	return ret;
}

//AT+HTTPCLIENT指令创建http连接后收到数据后的回调函数
static void http_client_recive_callback(uint8_t *data,int32_t len,http_recive_status status){
	static uint8_t recv_data[1024]={0};
	static int recv_len=0;
	g_http_status=status;

	switch(status){
		case AI_HTTP_RECIVE_STATUS_RECIVEING:	//正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
			http_debug_log("[%s()-%d]status=%d is error\r\n",__func__,__LINE__,(int)status);
			break;
		case AI_HTTP_RECIVE_STATUS_ERROR:	//数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
			http_debug_log("[%s()-%d]http recive error\r\n",__func__,__LINE__);
			break;
		case AI_HTTP_RECIVE_STATUS_OVER_OK:	//表示数据包已经结束，后面没有数据了，并且http请求状态码为200 OK
		case AI_HTTP_RECIVE_STATUS_OVER_ERROR:	//表示数据包已经结束，后面没有数据了，并且http请求状态码不是200(错误)
		case AI_HTTP_RECIVE_STATUS_HAVE_DATA:	//表示数据还没结束，后面还有数据包
			if(len>0){
				// axk_at_printf("+HTTPCLIENT:%d,",(int)len);
				// g_uart.send_data((uint8_t *)data,len);
				// axk_at_printf("\r\n");
				// axk_at_send_lock();
				// axk_at_printf_without_lock("+HTTPCLIENT:%d,",(int)len);
				// axk_at_send_data_without_lock((char *)data, len);
				// axk_at_printf_without_lock("\r\n");
				// axk_at_send_unlock();
			if(recv_len+len<=sizeof(recv_data))
			{
				memcpy(recv_data+recv_len,data,len);
				recv_len+=len;
			}
			else
			{
				memcpy(recv_data+ recv_len,data,sizeof(recv_data)-recv_len);
				axk_at_printf("+HTTPCLIENT:%d,",sizeof(recv_data));
				g_uart.send_data((uint8_t *)recv_data,sizeof(recv_data));
				axk_at_printf("\r\n");
				memset(recv_data,0,sizeof(recv_data));
				memcpy(recv_data,data+(len-(len+recv_len-sizeof(recv_data))),len+recv_len-sizeof(recv_data));
				recv_len=len+recv_len-sizeof(recv_data);
			}

			}else{
				http_debug_log("[%s()-%d]len=0\r\n",__func__,__LINE__);
			}

			if(status==AI_HTTP_RECIVE_STATUS_OVER_OK&& http_event!=HTTP_EVENT_DISCONNECTED)
			{
				if(recv_len<=0) break;
				axk_at_printf("+HTTPCLIENT:%d,",(int)recv_len);
				g_uart.send_data((uint8_t *)recv_data,recv_len);
				axk_at_printf("\r\n");
				memset(recv_data,0,sizeof(recv_data));
				recv_len=0;
				printf("http recive over ok\r\n");				
			}

			break;
		default:
			http_debug_log("[%s()-%d]unknown http status:%d\r\n",__func__,__LINE__,(int)status);
			break;
	}
}

//AT+HTTPCGET 指令数据回调函数
static void http_get_recive_callback(uint8_t *data,int32_t len,http_recive_status status){
	g_http_status=status;

	switch(status){
		case AI_HTTP_RECIVE_STATUS_RECIVEING:	//正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
			http_debug_log("[%s()-%d]status=%d is error\r\n",__func__,__LINE__,(int)status);
			break;
		case AI_HTTP_RECIVE_STATUS_ERROR:	//数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
			http_debug_log("[%s()-%d]http recive error\r\n",__func__,__LINE__);
			break;
		case AI_HTTP_RECIVE_STATUS_OVER_OK:	//表示数据包已经结束，后面没有数据了，并且http请求状态码为200 OK
		case AI_HTTP_RECIVE_STATUS_OVER_ERROR:	//表示数据包已经结束，后面没有数据了，并且http请求状态码不是200(错误)
		case AI_HTTP_RECIVE_STATUS_HAVE_DATA:	//表示数据还没结束，后面还有数据包
			if(len>0){
				axk_at_printf("+HTTPCGET:%d,",(int)len);
				g_uart.send_data((uint8_t *)data,len);
				axk_at_printf("\r\n");
			}else{
				http_debug_log("[%s()-%d]len=0\r\n",__func__,__LINE__);
			}
			break;
		default:
			http_debug_log("[%s()-%d]unknown http status:%d\r\n",__func__,__LINE__,(int)status);
			break;
	}
}

//AT+HTTPURLCFG=<url length>指令进入特殊模式的时候串口数据的回调函数
static void at_set_url_callback(char data){
	if (_longUrlCfg.pos < _longUrlCfg.length) {
		_longUrlCfg.p_Url[_longUrlCfg.pos] = data;
		_longUrlCfg.pos++;
		if (_longUrlCfg.pos < _longUrlCfg.length) {
			return;
		}
	}

	_longUrlCfg.p_Url[_longUrlCfg.length]='\0';
	axk_at_printf("\r\nSET OK\r\n");
	at_port_exit_specific();
}

//AT+HTTPCPOST 指令数据回调函数
static void http_post_recive_callback(uint8_t *data,int32_t len,http_recive_status status){
	g_http_status=status;

	switch(status){
		case AI_HTTP_RECIVE_STATUS_RECIVEING:	//正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
			http_debug_log("[%s()-%d]status=%d is error\r\n",__func__,__LINE__,(int)status);
			break;
		case AI_HTTP_RECIVE_STATUS_ERROR:	//数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
			http_debug_log("[%s()-%d]http recive error\r\n",__func__,__LINE__);
			break;
		case AI_HTTP_RECIVE_STATUS_OVER_OK:	//表示数据包已经结束，后面没有数据了，并且http请求状态码为200 OK
		case AI_HTTP_RECIVE_STATUS_OVER_ERROR:	//表示数据包已经结束，后面没有数据了，并且http请求状态码不是200(错误)
		case AI_HTTP_RECIVE_STATUS_HAVE_DATA:	//表示数据还没结束，后面还有数据包
			if(len>0){
				axk_at_printf("+HTTPCPOST:%d,",(int)len);
				g_uart.send_data((uint8_t *)data,len);
				axk_at_printf("\r\n");
			}else{
				http_debug_log("[%s()-%d]len=0\r\n",__func__,__LINE__);
			}
			break;
		default:
			http_debug_log("[%s()-%d]unknown http status:%d\r\n",__func__,__LINE__,(int)status);
			break;
	}
}

//AT+HTTPCPOST指令进入特殊模式的时候串口数据的回调函数
static void at_send_post_data_callback(char data){
	if (_post_data.pos < _post_data.requestParam.dataLen) {
		_post_data.requestParam.data[_post_data.pos] = data;
		_post_data.pos++;
		if (_post_data.pos < _post_data.requestParam.dataLen) {
			return;
		}
	}
	//数据接收完毕
	_post_data.requestParam.data[_post_data.requestParam.dataLen]='\0';

	//发起连接
	g_http_status=AI_HTTP_RECIVE_STATUS_RECIVEING;	//初始化一下 g_http_status
	http_connect_result connectResult=axk_hal_http_create_connect(&_post_data.requestParam);
	if(AI_HTTP_CONNECT_SUCCESS!=connectResult){
		http_debug_log("[%s()-%d]http post error,ret=%d\r\n",__func__,__LINE__,connectResult);
		axk_at_printf("SEND FAIL\r\n");
		goto EXIT1;
	}
	//数据发送完成了，等待接收完成
	int timeoutCount=0;
	while(1){
		switch(g_http_status){//这个在接收数据回调 http_post_recive_callback() 中修改，SDK调用 requestParam.http_recive() 即可
			case AI_HTTP_RECIVE_STATUS_RECIVEING:	//正在接收中，需要等待接收结束(这个状态是给AT框架使用的，http调用时不要传入该状态)
				if(timeoutCount++>10*60){
					http_debug_log("[%s()-%d]http recive timeout break\r\n",__func__,__LINE__);
					axk_at_printf("SEND FAIL\r\n");
					goto EXIT1;
				}
				break;
			case AI_HTTP_RECIVE_STATUS_ERROR:	//数据接收错误，连接断开(错误后中断本次接收，不继续接收了)
				http_debug_log("[%s()-%d]http recive error\r\n",__func__,__LINE__);
				axk_at_printf("SEND FAIL\r\n");
				goto EXIT1;
			case AI_HTTP_RECIVE_STATUS_OVER_OK: //表示数据包已经结束，后面没有数据了，并且http请求状态码为200 OK
				axk_at_printf("SEND OK\r\n");
				at_port_exit_specific();
				return;
			case AI_HTTP_RECIVE_STATUS_OVER_ERROR: //表示数据包已经结束，后面没有数据了，并且http请求状态码不是200(错误)
				axk_at_printf("SEND FAIL\r\n");
				goto EXIT1;
			case AI_HTTP_RECIVE_STATUS_HAVE_DATA: //表示数据还没结束，后面还有数据包
				break;
			default:
				http_debug_log("[%s()-%d]unknown http status:%d\r\n",__func__,__LINE__,(int)g_http_status);
				axk_at_printf("SEND FAIL\r\n");
				goto EXIT1;
		}
		vTaskDelay(100/portTICK_RATE_MS);
	}

EXIT1:
	at_port_exit_specific();
	//清空之前的数据
	if(NULL!=_post_data.requestParam.host){
		free(_post_data.requestParam.host);
		_post_data.requestParam.host=NULL;
	}
	if(NULL!=_post_data.requestParam.path){
		free(_post_data.requestParam.path);
		_post_data.requestParam.path=NULL;
	}
	if(NULL!=_post_data.requestParam.http_req_header){
		for(int i=0;i<_post_data.requestParam.http_req_header_num;i++){
			if(NULL!=_post_data.requestParam.http_req_header[i]){
				free(_post_data.requestParam.http_req_header[i]);
				_post_data.requestParam.http_req_header[i]=NULL;
			}
		}
		free(_post_data.requestParam.http_req_header);
		_post_data.requestParam.http_req_header=NULL;
		_post_data.requestParam.http_req_header_num=0;
	}
	if(NULL!=_post_data.requestParam.data){
		free(_post_data.requestParam.data);
		_post_data.requestParam.data=NULL;
		_post_data.requestParam.dataLen=0;
	}
	memset(&_post_data.requestParam,0,sizeof(_post_data.requestParam));
}

