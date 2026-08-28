//系统头文件
#include <stdio.h>
#include <stdbool.h>
//AT框架头文件
#include "axk_http_import.h"

//系统依赖头文件(FreeRTOS)
#include "FreeRTOS.h"
#include "task.h"

//sdk依赖
#include "esp_tls.h"
#include "esp_http_client.h"
#include "../../../components/3rdparty/esp/esp_http_client/lib/include/http_header.h"
#include "esp_transport.h"
#include "../../../components/3rdparty/esp/esp_http_client/lib/include/http_auth.h"

#include "axk_at.h"

esp_http_client_event_id_t http_event;

static esp_err_t AxkHttpEventHandler(esp_http_client_event_t *evt);
static esp_err_t AxkHttpGetSizeHandler(esp_http_client_event_t *evt);

static http_recive_callback_func HttpReciveCallback=NULL;	//http请求回调函数 void (*http_recive_callback_func)(uint8_t *data,int32_t len,http_recive_status status);
static int httpSoureSize=0;	//http资源大小

//-----------------------------因为包含问题导致结构体没有定义，这里的结构体是从 bl_iot_sdk\components\3rdparty\esp\esp_http_client\esp_http_client.c 中抄过来的，必须和SDK保持一致 ------------------------------------------------------
//这个是从 bl_iot_sdk\components\3rdparty\esp\esp_http_client\esp_http_client.c 中copy来的，需要是原始文件保持一致
typedef struct {
    char *data;         /*!< The HTTP data received from the server */
    int len;            /*!< The HTTP data len received from the server */
    char *raw_data;     /*!< The HTTP data after decoding */
    char *orig_raw_data;/*!< The Original pointer to HTTP data after decoding */
    int raw_len;        /*!< The HTTP data len after decoding */
    char *output_ptr;   /*!< The destination address of the data to be copied to after decoding */
} esp_http_buffer_t;

//这个是从 bl_iot_sdk\components\3rdparty\esp\esp_http_client\esp_http_client.c 中copy来的，需要是原始文件保持一致
typedef struct {
    http_header_handle_t headers;       /*!< http header */
    esp_http_buffer_t   *buffer;        /*!< data buffer as linked list */
    int                 status_code;    /*!< status code (integer) */
    int64_t             content_length; /*!< data length */
    int                 chunk_length;   /*!< chunk length */
    int                 data_offset;    /*!< offset to http data (Skip header) */
    int64_t             data_process;   /*!< data processed */
    int                 method;         /*!< http method */
    bool                is_chunked;
} esp_http_data_t;

//这个是从 bl_iot_sdk\components\3rdparty\esp\esp_http_client\esp_http_client.c 中copy来的，需要是原始文件保持一致
typedef struct {
    char                         *url;
    char                         *scheme;
    char                         *host;
    int                          port;
    char                         *username;
    char                         *password;
    char                         *path;
    char                         *query;
    char                         *cert_pem;
    esp_http_client_method_t     method;
    esp_http_client_auth_type_t  auth_type;
    esp_http_client_transport_t  transport_type;
    int                          max_store_header_size;
} connection_info_t;

//这个是从 bl_iot_sdk\components\3rdparty\esp\esp_http_client\esp_http_client.c 中copy来的，需要是原始文件保持一致
typedef enum {
    HTTP_STATE_UNINIT = 0,
    HTTP_STATE_INIT,
    HTTP_STATE_CONNECTED,
    HTTP_STATE_REQ_COMPLETE_HEADER,
    HTTP_STATE_REQ_COMPLETE_DATA,
    HTTP_STATE_RES_COMPLETE_HEADER,
    HTTP_STATE_RES_ON_DATA_START,
    HTTP_STATE_RES_COMPLETE_DATA,
    HTTP_STATE_CLOSE
} esp_http_state_t;

//这个是从 bl_iot_sdk\components\3rdparty\esp\esp_http_client\esp_http_client.c 中copy来的，需要是原始文件保持一致
struct esp_http_client {
    int                         redirect_counter;
    int                         max_redirection_count;
    int                         max_authorization_retries;
    int                         process_again;
    struct http_parser          *parser;
    struct http_parser_settings *parser_settings;
    esp_transport_list_handle_t     transport_list;
    esp_transport_handle_t          transport;
    esp_http_data_t                 *request;
    esp_http_data_t                 *response;
    void                        *user_data;
    esp_http_auth_data_t        *auth_data;
    char                        *post_data;
    char                        *location;
    char                        *auth_header;
    char                        *current_header_key;
    char                        *current_header_value;
    int                         post_len;
    connection_info_t           connection_info;
    bool                        is_chunk_complete;
    esp_http_state_t            state;
    http_event_handle_cb        event_handler;
    int                         timeout_ms;
    int                         buffer_size_rx;
    int                         buffer_size_tx;
    bool                        disable_auto_redirect;
    esp_http_client_event_t     event;
    int                         data_written_index;
    int                         data_write_left;
    bool                        first_line_prepared;
    int                         header_index;
    bool                        is_async;
    esp_transport_keep_alive_t  keep_alive_cfg;
    struct ifreq                *if_name;
    unsigned                    cache_data_in_fetch_hdr: 1;
};
//-----------------------------------------------------------------------------------

int axk_hal_http_create_connect_init(){
	//空函数，需要调用这个函数
	//bl的SDK需要调用一下，否则当前文件中的函数不会被编译(函数会执行对应的弱定义)
	return 0;
}

http_connect_result axk_hal_http_create_connect(const st_http_request *p_requestParam){
	http_debug_log("[%s()-%d]\r\n",__func__,__LINE__);
	if(!p_requestParam){
		http_debug_log("[%s()-%d]param is error\r\n",__func__,__LINE__);
		return AI_HTTP_CONNECT_PARAM_ERROR;
	}
	HttpReciveCallback=p_requestParam->http_recive;
	esp_http_client_config_t config = {
        .host = p_requestParam->host,
        .path = p_requestParam->path,
        .port=p_requestParam->port,
        .event_handler = AxkHttpEventHandler,
        .cert_pem = NULL,	//不设置证书
        .buffer_size=p_requestParam->rxbufSize,
    };
	switch(p_requestParam->transport_type){
		case AI_HTTP_TRANSPORT_OVER_TCP:
			config.transport_type = HTTP_TRANSPORT_OVER_TCP;
			break;
		case AI_HTTP_TRANSPORT_OVER_SSL:
			config.transport_type = HTTP_TRANSPORT_OVER_SSL;
			break;
		default:
			http_debug_log("[%s()-%d]unknown transport_type %d\r\n",__func__,__LINE__,p_requestParam->transport_type);
			return AI_HTTP_CONNECT_PARAM_ERROR;
	}
	switch(p_requestParam->opt){
		case AI_HTTP_OPT_HEAD:
			config.method = HTTP_METHOD_HEAD;
			break;
		case AI_HTTP_GET:
			config.method = HTTP_METHOD_GET;
			break;
		case AI_HTTP_POST:
			config.method = HTTP_METHOD_POST;
			break;
		case AI_HTTP_PUT:
			config.method = HTTP_METHOD_PUT;
			break;
		case AI_HTTP_DELETE:
			config.method = HTTP_METHOD_DELETE;
			break;
		default:
			http_debug_log("[%s()-%d]unknown opt=%d\r\n",__func__,__LINE__,p_requestParam->opt);
			return AI_HTTP_CONNECT_PARAM_ERROR;
	}
	http_debug_log("[%s()-%d]host:%s\r\n  path:%s\r\n  port:%d\r\n  transport_type:%d\r\n  method:%d\r\n",__func__,__LINE__,config.host,config.path,config.port,config.transport_type,config.method);

	http_connect_result ret=AI_HTTP_CONNECT_FAIL;
    esp_http_client_handle_t client = esp_http_client_init(&config);
	if(HTTP_METHOD_POST==config.method){
		if(NULL==p_requestParam->data){
			http_debug_log("[%s()-%d]data is NULLr\n",__func__,__LINE__);
			ret=AI_HTTP_CONNECT_PARAM_ERROR;
			goto EXIT1;
		}
		esp_http_client_set_post_field(client,(char *)p_requestParam->data, p_requestParam->dataLen);
		http_debug_log("[%s()-%d]dataLen:%d\r\n  data:%s\r\n",__func__,__LINE__,p_requestParam->dataLen,p_requestParam->data);
	}
	//添加 content-type
	switch(p_requestParam->content_type){
		case AI_HTTP_APPLICATION_X_WWW_FORM_URLENCODED:	//application/x-www-form-urlencoded
			esp_http_client_set_header(client,"content-type","application/x-www-form-urlencoded");
			break;
		case AI_HTTP_APPLICATION_JSON:	//application/json
			esp_http_client_set_header(client,"content-type","application/json");
			break;
		case AI_HTTP_MULTIPART_FORM_DATA:	//multipart/form-data
			esp_http_client_set_header(client,"content-type","multipart/form-data");
			break;
		case AI_HTTP_TEXT_XML:	//text/xml
			esp_http_client_set_header(client,"content-type","text/xml");
			break;
		default:
			break;
	}
	//添加http头
	for(int i=0;i<p_requestParam->http_req_header_num;i++){
		if(p_requestParam->http_req_header && p_requestParam->http_req_header[i]){
			//解析头
			int len=strlen(p_requestParam->http_req_header[i]);
			char *p_key=malloc(len+1);
			if(NULL==p_key){
				http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
				continue;
			}
			memcpy(p_key,p_requestParam->http_req_header[i],len);
			p_key[len]='\0';
			char *p_val=strchr(p_key,':');
			if(NULL==p_val){
				//格式错误
				http_debug_log("[%s()-%d]param error\r\n",__func__,__LINE__);
				free(p_key);
				p_key=NULL;
				continue;
			}
			*p_val='\0';
			p_val++;
			while(' '==(*p_val)){
				p_val++;
			}
			esp_http_client_set_header(client,p_key,p_val);
			free(p_key);
			p_key=NULL;
		}else{
			http_debug_log("[%s()-%d]param error,p_requestParam->http_req_header=%p,p_requestParam->http_req_header[i]=%p\r\n",__func__,__LINE__,p_requestParam->http_req_header,p_requestParam->http_req_header[i]);
		}
	}
    // GET POST HEAD DELETE
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        http_debug_log("[%s()-%d]HTTP Status = %d, content_length = %lld\r\n",__func__,__LINE__,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));

        int status_code = esp_http_client_get_status_code(client);
		if(200 == status_code || 206 == status_code) {
			ret=AI_HTTP_CONNECT_SUCCESS;
			goto EXIT1;
		}else{
			ret=AI_HTTP_CONNECT_FAIL;
			goto EXIT1;
		}
    } else {
        http_debug_log("[%s()-%d]HTTP request failed: %s\r\n",__func__,__LINE__, esp_err_to_name(err));
		ret=AI_HTTP_CONNECT_FAIL;
		goto EXIT1;
    }
	
EXIT1:
    esp_http_client_cleanup(client);
	return ret;
}

//查询 http size
//获取http资源大小
//返回值
//    >=0：表示获取到的资源大小(单位：字节)
//    -1：参数错误
//    -2：连接错误
int32_t axk_hal_http_get_size(const st_http_request *p_requestParam){
	if(!p_requestParam){
		http_debug_log("[%s()-%d]param is error\r\n",__func__,__LINE__);
		return -1;
	}
	httpSoureSize=-2;
	esp_http_client_config_t config = {
        .host = p_requestParam->host,
        .path = p_requestParam->path,
        .port=p_requestParam->port,
        .event_handler = AxkHttpGetSizeHandler,
        .method = HTTP_METHOD_HEAD,
    };
	switch(p_requestParam->transport_type){
		case AI_HTTP_TRANSPORT_OVER_TCP:
			config.transport_type = HTTP_TRANSPORT_OVER_TCP;
			break;
		case AI_HTTP_TRANSPORT_OVER_SSL:
			config.transport_type = HTTP_TRANSPORT_OVER_SSL;
			break;
		default:
			http_debug_log("[%s()-%d]unknown transport_type %d\r\n",__func__,__LINE__,p_requestParam->transport_type);
			return -1;
	}
	http_debug_log("[%s()-%d]host:%s\r\n  path:%s\r\n  port:%d\r\n  transport_type:%d\r\n  method:%d\r\n",__func__,__LINE__,config.host,config.path,config.port,config.transport_type,config.method);

	int ret=-1;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    //HEAD
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        http_debug_log("[%s()-%d]HTTP Status = %d, content_length = %lld\r\n",__func__,__LINE__,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
		http_debug_log("[%s()-%d]\r\n",__func__,__LINE__);
		if(200==esp_http_client_get_status_code(client)){
			if(httpSoureSize<0){
				ret=-2;
			}else{
				ret=httpSoureSize;
			}
			goto EXIT1;
		}else{
			ret=-2;
			goto EXIT1;
		}
    } else {
        http_debug_log("[%s()-%d]HTTP request failed: %s\r\n",__func__,__LINE__, esp_err_to_name(err));
		ret=-2;
		goto EXIT1;
    }
	
EXIT1:
    esp_http_client_cleanup(client);
	return ret;
}

static esp_err_t AxkHttpEventHandler(esp_http_client_event_t *evt)
{
    http_event=evt->event_id;

	http_debug_log("[%s()-%d]method=%p\r\n",__func__,__LINE__,evt->client->connection_info.method);
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            http_debug_log("[%s()-%d]HTTP_EVENT_ERROR\r\n",__func__,__LINE__);
			if(HttpReciveCallback){
				HttpReciveCallback(NULL,0,AI_HTTP_RECIVE_STATUS_ERROR);
			}
            break;
        case HTTP_EVENT_ON_CONNECTED:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_CONNECTED\r\n",__func__,__LINE__);
            break;
        case HTTP_EVENT_HEADER_SENT:
            http_debug_log("[%s()-%d]HTTP_EVENT_HEADER_SENT\r\n",__func__,__LINE__);
            break;
        case HTTP_EVENT_ON_HEADER:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_HEADER, key=%s, value=%s\r\n",__func__,__LINE__, evt->header_key, evt->header_value);
            if(strcmp(evt->header_key,"Content-Range")==0)
            {
                axk_at_printf("ContentRange:%s\r\n",evt->header_value);
            }
			if(HTTP_METHOD_HEAD==evt->client->connection_info.method && HttpReciveCallback){
				//将head打印
				int tmpBufSize=strlen(evt->header_key)+strlen(evt->header_value)+3;
				uint8_t *tmpStr=malloc(tmpBufSize);
				if(NULL==tmpStr){
					 http_debug_log("[%s()-%d]malloc error\r\n",__func__,__LINE__);
					HttpReciveCallback(NULL,0,AI_HTTP_RECIVE_STATUS_ERROR);
				}else{
					memset(tmpStr,0,tmpBufSize);
					snprintf((char *)tmpStr,tmpBufSize,"%s: %s",evt->header_key, evt->header_value);
					HttpReciveCallback(tmpStr,tmpBufSize-1,AI_HTTP_RECIVE_STATUS_HAVE_DATA);
					free(tmpStr);
					tmpStr=NULL;
				}
			}
            break;
        case HTTP_EVENT_ON_DATA:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_DATA recive data_len=%d\r\n",__func__,__LINE__, evt->data_len);	//数据内容为 evt->data
            if(HttpReciveCallback){
				HttpReciveCallback(evt->data,evt->data_len,AI_HTTP_RECIVE_STATUS_HAVE_DATA);
			}
            break;
        case HTTP_EVENT_ON_FINISH:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_FINISH data_len=%d\r\n",__func__,__LINE__, evt->data_len);	//数据内容为 evt->data
            if(HttpReciveCallback){
				HttpReciveCallback(evt->data,evt->data_len,AI_HTTP_RECIVE_STATUS_OVER_OK);
			}
            break;
        case HTTP_EVENT_DISCONNECTED:
            http_debug_log("[%s()-%d]HTTP_EVENT_DISCONNECTED\r\n",__func__,__LINE__);
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                http_debug_log("[%s()-%d]Last esp error code: 0x%x\r\n",__func__,__LINE__, err);
                http_debug_log("[%s()-%d]Last mbedtls failure: 0x%x\r\n",__func__,__LINE__, mbedtls_err);
            }
			if(HttpReciveCallback){
				HttpReciveCallback(NULL,0,AI_HTTP_RECIVE_STATUS_OVER_OK);
			}
            break;
        case HTTP_EVENT_REDIRECT:
            http_debug_log("[%s()-%d]HTTP_EVENT_REDIRECT\r\n",__func__,__LINE__);
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            break;
    }
    return ESP_OK;
}

static esp_err_t AxkHttpGetSizeHandler(esp_http_client_event_t *evt)
{
       http_event=evt->event_id;

    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            http_debug_log("[%s()-%d]HTTP_EVENT_ERROR\r\n",__func__,__LINE__);
            break;
        case HTTP_EVENT_ON_CONNECTED:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_CONNECTED\r\n",__func__,__LINE__);
            break;
        case HTTP_EVENT_HEADER_SENT:
            http_debug_log("[%s()-%d]HTTP_EVENT_HEADER_SENT\r\n",__func__,__LINE__);
            break;
        case HTTP_EVENT_ON_HEADER:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_HEADER, key=%s, value=%s\r\n",__func__,__LINE__, evt->header_key, evt->header_value);
			if(0==strcmp("Content-Length",evt->header_key)){
				httpSoureSize=atoi(evt->header_value);
			}
            break;
        case HTTP_EVENT_ON_DATA:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_DATA recive data_len=%d\r\n",__func__,__LINE__, evt->data_len);	//数据内容为 evt->data
            break;
        case HTTP_EVENT_ON_FINISH:
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_FINISH\r\n",__func__,__LINE__);
            http_debug_log("[%s()-%d]HTTP_EVENT_ON_FINISH data_len=%d\r\n",__func__,__LINE__, evt->data_len);	//数据内容为 evt->data
            break;
        case HTTP_EVENT_DISCONNECTED:
            printf("[%s()-%d]HTTP_EVENT_DISCONNECTED\r\n",__func__,__LINE__);
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                http_debug_log("[%s()-%d]Last esp error code: 0x%x\r\n",__func__,__LINE__, err);
                http_debug_log("[%s()-%d]Last mbedtls failure: 0x%x\r\n",__func__,__LINE__, mbedtls_err);
            }
            break;
        case HTTP_EVENT_REDIRECT:
            http_debug_log("[%s()-%d]HTTP_EVENT_REDIRECT\r\n",__func__,__LINE__);
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            break;
    }
    return ESP_OK;
}

