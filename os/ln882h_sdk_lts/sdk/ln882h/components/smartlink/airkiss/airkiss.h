#ifndef __AIRKISS_H__
#define __AIRKISS_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef void* (*airkiss_memset_fn) (void* ptr, int value, unsigned int num);
typedef void* (*airkiss_memcpy_fn) (void* dst, const void* src, unsigned int num);
typedef int   (*airkiss_memcmp_fn) (const void* ptr1, const void* ptr2, unsigned int num);
typedef int   (*airkiss_printf_fn) (const char* format, ...);


typedef struct
{
	airkiss_memset_fn memset;
	airkiss_memcpy_fn memcpy;
	airkiss_memcmp_fn memcmp;
	airkiss_printf_fn printf;

} airkiss_config_t;

typedef struct
{
    int dummyap[26];
    int dummy[32];
} airkiss_context_t;

typedef struct
{
	char * pwd;						/* wifi密码，以'\0'结尾 */
	char * ssid;                    /* wifi ssid，以'\0'结尾 */
	unsigned char pwd_length;		/* wifi密码长度 */
	unsigned char ssid_length;		/* wifi ssid长度 */
	unsigned char random;			/* 随机值，根据AirKiss协议，当wifi连接成功后，需要通过udp向10000端口广播这个随机值，这样AirKiss发送端（微信客户端或者AirKissDebugger）就能知道AirKiss已配置成功 */
	unsigned char reserved;			/* 保留值 */
} airkiss_result_t;

typedef enum
{
	AIRKISS_STATUS_CONTINUE       = 0,
	AIRKISS_STATUS_CHANNEL_LOCKED = 1,
	AIRKISS_STATUS_COMPLETE       = 2
} airkiss_status_t;


/*
 * 初始化AirKiss库，如要复用context，可以多次调用
 *
 * 返回值
 * 		< 0：出错，通常是参数错误
 * 		  0：成功
 */
int airkiss_init(airkiss_context_t* context, const airkiss_config_t* config);

/*
 * 开启WiFi Promiscuous Mode后，将收到的包传给airkiss_recv以进行解码
 *
 * 参数说明
 * 		frame：802.11 frame mac header(must contain at least first 24 bytes)
 * 		length：total frame length
 *
 * 返回值
 * 		 < 0：出错，通常是参数错误
 * 		>= 0：成功，请参考airkiss_status_t
 */
int airkiss_recv(airkiss_context_t* context, const void* frame, unsigned short length);

/*
 * 当airkiss_recv()返回AIRKISS_STATUS_COMPLETE后，调用此函数来获取AirKiss解码结果
 *
 * 返回值
 * 		< 0：出错，解码状态还不是AIRKISS_STATUS_COMPLETE
 * 		  0：成
 功
 */
int airkiss_get_result(airkiss_context_t* context, airkiss_result_t* result);

/*
 * 上层切换信道以后，可以调用一下本接口清缓存，降低锁定错信道的概率，注意调用的逻辑是在airkiss_init之后
 *
 * 返回值
 * 		< 0：出错，通常是参数错误
 * 		  0：成功
 */
int airkiss_change_channel(airkiss_context_t* context);




#ifdef __cplusplus
}
#endif

#endif /* __AIRKISS_H__ */
