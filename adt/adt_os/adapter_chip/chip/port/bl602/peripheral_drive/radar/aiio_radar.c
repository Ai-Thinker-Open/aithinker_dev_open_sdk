#include "aiio_autoconf.h"

#if CONFIG_AIIO_RADAR

#include "aiio_radar.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"

#include "bl602.h"
#include "hosal_uart.h"
#include "hosal_i2c.h"
#include "hosal_gpio.h"
#include "hosal_spi.h"
#include "bl602_dma.h"
#include <bl_dma.h>
#include "hosal_dma.h"
#include "bl602_spi.h"
#include "bl_irq.h"
#include "nmsis_gcc.h"
#include "easyflash.h"
#include "bl602_uart.h"
#include "bl_uart.h"

#define USER_UART_ENABLE                (0)
#define UART_TRAN_NUM                   (1)
#define USER_UART_BAUD                  (256000)
#define CMD_PROCESS_TASK_PRIORITY       (13)

#define CMD_PROCESS_BIT     (1 << 1)

#define RADAR_I2C_SCL_PIN   (14)
#define RADAR_I2C_SDA_PIN   (3)

#define RADAR_EN_PIN        (12)
#define RADAR_POWER_PIN     (20)
#define RADAR_REXT_PIN      (21)

#define RADAR_CS_PIN        (22)

#define RADAR_POWER_TIMEOUT (60)

#define EVT_GROUP_SPI_FLAG  (1 << 1)
#define SPI_INII_STACK_SIZE (2048)

#define SPI_DMA_BUFF_LEN                    (48)

#define EVT_GROUP_SPI_TX    (1<<0)
#define EVT_GROUP_SPI_RX    (1<<1)
#define EVT_GROUP_SPI_TR    (EVT_GROUP_SPI_TX | EVT_GROUP_SPI_RX)

#define MAX_REG_NUM         (100)

#define FUNC_QUEUE_SIZE     (34)
#define SPI_FRAME_DLEN_MAX      (40)
#define SPI_FRAME_HLEN          (4)
#define SPI_FRAME_TLEN          (4)
#define SPI_FRAME_LEN_MAX       (SPI_FRAME_DLEN_MAX + SPI_FRAME_HLEN + SPI_FRAME_TLEN)
#define DATA_RECV_BUF_SIZE      (SPI_FRAME_LEN_MAX * 2) /*ping-pong buffer*/

#define DATA_HEAD		          (0xAA)
#define DATA_TAIL		          (0x55)
#define ID_MASK                   (0xF0)

#define FFT0_ID                   (0x30)
#define FFT1_ID                   (0x70)

#define DSRAW0_ID                   (0x20)
#define DSRAW1_ID                   (0x60)

#define CHIRP_INDEX_POS0          (5)
#define CHIRP_INDEX_POS1          (3)
#define CHIRP_INDEX_MASK          (0x0F)
#define SKIP_NUM                  (4)

#define DFFT0_ID                  (0xB0)
#define DFFT1_ID                  (0xF0)
#define FRAME_CNT_POS             (8)

#define DFFT_PEAK_ID              (0x40)

#define DATA_PROC_STACK_SIZE      (512)
#define RADAR_DATA_QUEUE_SIZE     (2)


#define I2C_ADDR_BanYan_Chip0		    (0x40)
#define I2C_ADDR_BanYan_Chip1		    (0x44)
#define MAX_REG_NUM                 (100)
#define REG_FIX_NUM                 (5)
#define RAW_MAP_NUM                 (5)

#define BANYAN_DIG_FUN_SWITCH       (0x01)
#define BANYAN_DIG_RAW_PEAK_NUM     (0x04)
#define BANYAN_DIG_FFT_NUM          (0x05)
#define BANYAN_DIG_FFT_SETTING      (0x0A)
#define BANYAN_DIG_DFFT_DATA_NUM    (0x0D)
#define BANYAN_DIG_DFFT_CHIRP_NUM   (0x0E)
#define BANYAN_PAT_CHIRP_NUM        (0x44)

#define BANYAN_DFFT_DATA_NUM_POS    (8)
#define BANYAN_DFFT_CHIRP_NUM_POS   (8)
#define BANYAN_RAW_POS              (8)

#define BANYAN_PEAK_MASK            (0x1F)
#define BANYAN_RAW_MASK             (0x07)

#define BIT(x)                      ((uint32_t)((uint32_t)0x01U<<(x)))
#define BANYAN_FFT_DATA             BIT(2)
#define BANYAN_DFFT_DATA            BIT(12)
#define BANYAN_DFFT_PEAK_DATA       BIT(4)
#define BANYAN_DSRAW_DATA           BIT(1)

#define INDICATOR_RECV_THRESHOLD            (1000)
#define INDICATOR_RECV_THD_DFFT_SHIFT       (3)
#define INDICATOR_RECV_THD_DPEAK_SHIFT      (6)
#define INDICATOR_SEND_OF_THRESHOLD         (10)

#define LE16_TO_BIG(x) \
            ((uint16_t)( \
                (((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
                (((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))
#define BIG16_TO_LE LE16_TO_BIG

#define I2C_ADDR_BanYan_Chip0   (0x40)

#define     ONEFFT_POINT        (10)
#define     FRAME_DEPTH         (34)
#define     ACC_DEPTH           ((FRAME_DEPTH-2) >> 2)

#define     ROI_MIN             (2)
#define     NOCOHERENTLEN       (40)

#define     RADAR_RANGE_RES     (75)/* ����ֱ��ʣ�cm�� */

#define PROTOCOL_HEAD                    (0XF1F2F3F4)
#define PROTOCOL_TAIL                    (0XF5F6F7F8)
#define REPORT_DEBUG_TYPE                (0x01)
#define REPORT_TARGET_TYPE               (0x02)

#define DATA_HEAD                        (0xAA)
#define DATA_TAIL                        (0x55)

#define REPORT_HEAD_LENGTH               (4)
#define DATA_LEN_LENGTH                  (2)
#define MAX_MOTION_BINS_LENGTH           (1)
#define MAX_MOTIONLESS_BINS_LENGTH       (1)
#define MOTION_DATA_RANGE_BIN_LENGTH     (ONEFFT_POINT)
#define MOTIONLESS_DATA_RANGE_BIN_LENGTH (ONEFFT_POINT)
#define REPORT_TAIL_LENGTH               (4)
#define TARGET_DATA_LENGTH               (9)
#define DATA_TYPE_LENGTH                 (1)
#define DATA_HEAD_LENGTH                 (1)
#define DATA_TAIL_LENGTH                 (1)
#define DATA_CHECK_LENGTH                (1)

#define CONFIDENCECNT					 (7)

#define POWER_TYPE_10HZ      (1)
#define POWER_TYPE_1HZ       (0)

#define ARRAY_SIZE(x)       (sizeof(x) / sizeof(*x))

#define APP_NFFT          (ONEFFT_POINT-1)
#define REPORT_LENGTH    (REPORT_HEAD_LENGTH +  \
                          DATA_LEN_LENGTH + \
                          DATA_TYPE_LENGTH + \
                          DATA_HEAD_LENGTH + \
                          TARGET_DATA_LENGTH + \
                          MAX_MOTION_BINS_LENGTH + \
                          MAX_MOTIONLESS_BINS_LENGTH + \
                          MOTION_DATA_RANGE_BIN_LENGTH + \
                          MOTIONLESS_DATA_RANGE_BIN_LENGTH + \
                          DATA_TAIL_LENGTH + \
                          DATA_CHECK_LENGTH+REPORT_TAIL_LENGTH)

#define UPLOAD_SAMPLE_RATE          (16)
#define RADAR_DATA_MAX_LEN          (40)
#define DEBUG_MODE_DEFAULT          (0)

#define FLASH_PAGE_SIZE             (4096)
#define FLASH_MAGIC_NUM             (0x12345678)
#define FLASH_WORD_LEN              (4)

/*	========================== 雷达参数 =============================	*/
#define PARAM_RANGE_RES                 75            //距离分辨率：75厘米

/*	=================================================================	*/


/*  ========================== 算法参数 =============================	*/
#define PARAM_MAX_MOTION_RANGE          600
#define PARAM_MAX_MOTIONLESS_RANGE      600
#define PARAM_MOTION_MAX                (PARAM_MAX_MOTION_RANGE/PARAM_RANGE_RES)
#define PARAM_MOTIONLESS_MAX            (PARAM_MAX_MOTIONLESS_RANGE/PARAM_RANGE_RES)


/*裸板*/
#define PARAM_MOTION_SENSITIBITY_RANG0  (50)
#define PARAM_MOTION_SENSITIBITY_RANG1  (50)
#define PARAM_MOTION_SENSITIBITY_RANG2  (40)
#define PARAM_MOTION_SENSITIBITY_RANG3  (40)
#define PARAM_MOTION_SENSITIBITY_RANG4  (40)
#define PARAM_MOTION_SENSITIBITY_RANG5  (40)
#define PARAM_MOTION_SENSITIBITY_RANG6  (30)
#define PARAM_MOTION_SENSITIBITY_RANG7  (30)
#define PARAM_MOTION_SENSITIBITY_RANG8  (30)


#define PARAM_MOTIONLESS_SENSITIBITY_RANG0    (5500000)//不参与判断
#define PARAM_MOTIONLESS_SENSITIBITY_RANG1    (5500000)//不参与判断
#define PARAM_MOTIONLESS_SENSITIBITY_RANG2 (40)
#define PARAM_MOTIONLESS_SENSITIBITY_RANG3 (40)
#define PARAM_MOTIONLESS_SENSITIBITY_RANG4 (40)
#define PARAM_MOTIONLESS_SENSITIBITY_RANG5 (40)
#define PARAM_MOTIONLESS_SENSITIBITY_RANG6 (15)
#define PARAM_MOTIONLESS_SENSITIBITY_RANG7 (15)
#define PARAM_MOTIONLESS_SENSITIBITY_RANG8 (15)

#define PARAM_OFF_TIME                  5       // s
/*	=================================================================	*/

//========================== CMD相关 =================================//
typedef enum  
{
	CMD_HEAD_0 = 0,
    CMD_HEAD_1,
    CMD_HEAD_2,
    CMD_HEAD_3,
    CMD_HEAD_MAX
}cmdHeadEnum;

typedef enum  
{
	CMD_TAIL_0 = 0,
    CMD_TAIL_1,
    CMD_TAIL_2,
    CMD_TAIL_3,
    CMD_TAIL_MAX
}cmdTailEnum;

#define CMD_LEN_MAX         (64)
#define CMD_REG_MAX         (25)
#define CMD_PARA_MAX        (13)
#define CMD_DATA_QUEUE_SIZE     (2)

#define CMD_LENGTH_LEN      (2)
#define CMD_TYPE_LEN        (2)
#define CMD_STATUS_LEN      (2)
#define CMD_STR_LEN         (2)
#define CMD_OVERHEAD_LEN    (CMD_HEAD_MAX + CMD_LENGTH_LEN + CMD_TYPE_LEN + CMD_TAIL_MAX)
#define CMD_BUF_LEN         (CMD_LEN_MAX - CMD_HEAD_MAX - CMD_TAIL_MAX - CMD_LENGTH_LEN)
#define CMD_LEN_HIGH_POS    (8)
#define CMD_DATA_POS_OLD    (CMD_HEAD_MAX + CMD_LENGTH_LEN + CMD_TYPE_LEN)
#define CMD_DATA_POS        (CMD_HEAD_MAX + CMD_LENGTH_LEN + CMD_TYPE_LEN + CMD_STATUS_LEN)

#define CMD_DEV_ADDR_LEN    (2)
#define CMD_REG_ADDR_LEN    (2)
#define CMD_PARA_NAME_LEN   (2)

#define START_CFG_CMD       (0x00FF)
#define FINISH_CFG_CMD      (0x00FE)
#define READ_VER_CMD	    (0x0000)
#define WRITE_REG_CMD       (0x0001)
#define READ_REG_CMD        (0x0002)
#define WRITE_MTT_CMD       (0x0003)
#define READ_MTT_CMD        (0x0004)
#define ADC_RESET_CMD       (0x0010)
#define READ_SN_CMD	        (0x0011)
#define WRITE_SYS_CMD       (0x0012)
#define READ_SYS_CMD        (0x0013)
#define CASCADING_MODE_CMD  (0x0014)
#define FFT_ZEROFILL_CMD  	(0x0015)
#define START_I2C_TEST_CMD  (0x0020)
#define STOP_I2C_TEST_CMD   (0x0021)
#define GET_I2C_TEST_RESULT_CMD  (0x0022)
#define NOP_CONFIG_CMD   		(0x0023)
#define ENTER_FACTORYMODE_CMD   (0x0024)
#define EXIT_FACTORYMODE_CMD   	(0x0025)


#define PARAM_CFG_CMD		(0x0060)    //cmd: config algorithm param to flash
#define PARAM_READ_CMD	    (0x0061)    //cmd: read algorithm param from flash or ram
#define ENABLE_DEBUG_CMD	(0x0062)
#define DISABLE_DEBUG_CMD	(0x0063)
#define THRESHOLD_SET_CMD   (0x0064)

// 增加AMPS读取雷达测试结果指令
#define AMPS_READ_RESULT   (0x00A1)

//BLE CMD
#define BLE_REBOOT_CMD      (0x00B0)    //reboot
#define BLE_RESTORE_CMD     (0x00B1)    //restore


#define CMD_ACK_TYPE        (0x0100)
#define ACK_OK              (0)
#define ACK_FAIL            (1)

#define ACK_OK_OLD              (1)
#define ACK_FAIL_OLD            (0)

#define RESET_DELAY_TIME    (2)
#define SN_LEN              (3)

#define CMD_PROC_STACK_SIZE      (512)

#define _TO_STR(__str) #__str 
#define TO_STR(__str) _TO_STR(__str)// convert to string

#define XEND103_TYPE_VALUE              (0x8005)
#define RADAR_DEV_MODEL_TYPE_SN	  (0x8802)
#define RADAR_DEV_TYPE (0x8002)
//========================== END CMD相关 =================================//

#define RADAR_PROTOCOL_VERSION  (2)
#define RADAR_DEV_VER_MAJOR                 1
#define RADAR_DEV_VER_MINOR                 0
#define RADAR_DEV_VER_PATCH                 0


extern COMPLEX16_T chirp_data_clean[ONEFFT_POINT];
extern COMPLEX16_T Yone[ONEFFT_POINT];
extern COMPLEX32_T Wone[ONEFFT_POINT];
void cluster_remove(COMPLEX16_T *range_chirp);
void OneRecursiveCanceller(COMPLEX16_T *range_chirp, uint8_t start_pos, uint8_t shift);

Result BodyPresence(uint8_t* dataBuf, uint16_t index, uint8_t motion_max, uint8_t motionless_max, 
                    uint32_t *pMotion_sensitivity,uint32_t* pMotionless_sensitivity,
                    uint32_t off_time);
void EnableDebugMode(void);
void DisableDebugMode(void);
uint8_t IsInDebugMode(void);
uint8_t GetLowPowerType(void);
extern uint8_t NormalDopplerMaxVal[ONEFFT_POINT];
extern uint8_t NormalAccSum[ONEFFT_POINT];

uint32_t abs_32(int32_t src);
uint32_t Euclidean_32(uint32_t x, uint32_t y);
uint16_t abs_16(int16_t src);
uint16_t Euclidean_16(uint16_t x, uint16_t y);

static int rd01_iic_write(uint16_t dev_addr, uint8_t reg_addr, uint16_t data);
void create_spi_init_task(void (*spi_slave_init_type)(void));
static void bsp_spi_slave_init(void);
static void Algo_SaveParameter(uint32_t*data,uint16_t len);

typedef struct {
    int8_t tx_dma_ch;
    int8_t rx_dma_ch;
    uint8_t rx_enable;
    EventGroupHandle_t spi_event_group;
} spi_dma_priv_t;

#pragma pack(4)
typedef struct ALGORITHM_PARAM 
{
    uint32_t nMaxMotionRangeBin;
    uint32_t nMaxMotionLessRangeBin;
    uint32_t nThresholdValOfMotion[ONEFFT_POINT-1 ];
    uint32_t nThresholdValOfMotionLess[ONEFFT_POINT-1 ];
    uint32_t nOffTime;
}ALGORITHM_PARAM_T;
#pragma pack()

typedef enum  
{
	FLASH_ELEM_RADAR = 0,
    FLASH_ELEM_SYS,
    FLASH_ELEM_MAX
}flashElemEnum;

typedef struct FLASH_ELEMENT
{
    uint32_t elemLen;
    void *elemAddr; 
}FLASH_ELEMENT_T;

#pragma pack(4)
typedef struct FLASH_DATA
{
    uint32_t magic;
    FLASH_ELEMENT_T elem[FLASH_ELEM_MAX];
}FLASH_DATA_T;
#pragma pack()

typedef enum  
{
	SYS_SYSTEM_MODE = 0,
    SYS_UPLOAD_SP_RATE,
    SYS_DEBUG_MODE
}sysParaEnum;

typedef struct SYS_PARA
{
    int16_t systemMode;
    int16_t uploadSampleRate;
    int16_t debugMode;
    int16_t pad;
}SYS_PARA_T;

typedef struct 
{
    char *magic;
    char *elem_len[FLASH_ELEM_MAX];
    char *elem_member[FLASH_ELEM_MAX];
    char *bodysensing;
    char *parameter;
} elem_key_t;

typedef struct CMD_PARSE
{
    uint8_t buf[CMD_BUF_LEN];
    uint8_t state;
    uint16_t curIndex;
    uint16_t len;
}CMD_PARSE_T;

typedef enum  
{
	CMD_STATE_HEAD0 = 0,
    CMD_STATE_HEAD1,
    CMD_STATE_HEAD2,
    CMD_STATE_HEAD3,
    CMD_STATE_LEN0,
    CMD_STATE_LEN1,
    CMD_STATE_DATA,
    CMD_STATE_TAIL0,
    CMD_STATE_TAIL1,
    CMD_STATE_TAIL2,
    CMD_STATE_TAIL3
}cmdStateEnum;

typedef enum  
{
	NOPCONFIG_ENABLE = 0,
    NOPCONFIG_STARTTIME,
    NOPCONFIG_STOPTIME,
	NOPCONFIG_STARTREGISTER,
	NOPCONFIG_STOPREGISTER,
}nopCofigParaEnum;

typedef struct CMD 
{
	uint16_t cmdType;
	uint16_t cmdData[0];
}CMD_T;

typedef struct CMD_REG 
{
	uint16_t addr;
	uint16_t val;
}CMD_REG_T;

#pragma pack(1)
typedef struct CMD_PARA 
{
	uint16_t type;
	int32_t val;
}CMD_PARA_T;
#pragma pack()

static hosal_i2c_dev_t i2c0;
static hosal_gpio_dev_t en_gpio;
static hosal_gpio_dev_t power_gpio;
static hosal_gpio_dev_t rext_gpio;
static hosal_gpio_dev_t cs_io;
static hosal_spi_dev_t spi;

TimerHandle_t timer_60ms_handle;
static EventGroupHandle_t SPIEventGroup;
QueueHandle_t SpiDataQueue;
TaskHandle_t QueueTaskHandle = NULL;

StackType_t xInitStack[SPI_INII_STACK_SIZE];
StaticTask_t xInitTaskBuffer;

static uint8_t sg_spi_enable = 0;

DMA_LLI_PP_Struct rx_dmaLliPPStruct = {0};
volatile static uint8_t dma_irq_cnt = 0;
volatile uint8_t g_dataRecvFlag[1][2];
uint8_t *pbuff;
uint8_t g_dataRecvBuf[1][DATA_RECV_BUF_SIZE];
static uint8_t is_yone_buffer_clear = 0;

static FLASH_DATA_T flashData;
static uint8_t needFlashWrite = 0;
static SYS_PARA_T sysPara __attribute__((aligned (4)));
static uint8_t CmdModeFlag = 0;
static uint8_t DebugModeFlag = 0;
static uint16_t masterProtocolVer = 0;
static uint8_t enableNopConfig = 0;
static uint8_t nopConfigStartRegisterIndex = 0;
static uint8_t nopConfigStopRegisterIndex = 0;
static uint16_t nopConfigStartMs = RESET_DELAY_TIME;
static uint16_t nopConfigStopMs = RESET_DELAY_TIME + 2;
int32_t nopConfigStartRegister[16] = {0};
int32_t nopConfigStopRegister[16] = {0};
static uint8_t g_FtcmdFlag = 0;
static uint8_t g_TxCount = (1 + 1) / 2;

static uint16_t start_cfg_ack[2] = {0};
static const uint16_t read_ver_ack[5] = {XEND103_TYPE_VALUE, 0x00, RADAR_DEV_VER_MAJOR,
                                                                 RADAR_DEV_VER_MINOR,
                                                                 RADAR_DEV_VER_PATCH};

uint8_t rx_ping_buf[SPI_DMA_BUFF_LEN] __attribute__((at(0x42020000)));   // __attribute__((section(".ram_data")));
uint8_t rx_pong_buf[SPI_DMA_BUFF_LEN] __attribute__((at(0x42021000)));   //__attribute__((section(".ram_data")));
uint8_t rx_buf_max[SPI_DMA_BUFF_LEN*2] __attribute__((at(0x42022000)));  // __attribute__((section(".ram_data")));

RADAR_DATA_PARSE_T RadarDataParse[1];
RADAR_PARA_T RadarPara;
/* 低功耗类型，用于两种低功耗切换   POWER_TYPE_1HZ：1s检测一次   POWER_TYPE_10HZ：100ms检测一次 */
static uint8_t lowPowerType = POWER_TYPE_10HZ;
ALGORITHM_PARAM_T gAlgorithmParam;
/* 上报数据缓冲区 */
static uint8_t ReportBuffer[REPORT_LENGTH] __ALIGNED(4) = {0};

static volatile uint8_t g_uartDmaSendingFlag = 0;
static CMD_RECV_T g_cmdRecv;
static CMD_PARSE_T CmdDataParse;
static const uint8_t CmdHead[CMD_HEAD_MAX] = {0xFD, 0xFC, 0xFB, 0xFA};
static const uint8_t CmdTail[CMD_TAIL_MAX] = {0x04, 0x03, 0x02, 0x01};
static uint8_t CmdAck[CMD_LEN_MAX] __attribute__((aligned (4))) = {0};

static rd01_data_output_t data_output_cb = NULL;

HOSAL_UART_DEV_DECL(uart_dev_int, UART_TRAN_NUM, 16, 7, 256000);
EventGroupHandle_t CmdProcessEventGroup;

static elem_key_t elem_key = 
{
    .magic = "magic",
    .elem_len[0] = "elemLen1",
    .elem_len[1] = "elemLen2",
    .elem_member[0] = "elem_mb1",
    .elem_member[1] = "elem_mb2",
    .bodysensing = "bodyss",
    .parameter = "param",
};

static uint16_t rawPointMap[RAW_MAP_NUM] = 
{
	RAW_POINT_64,
    RAW_POINT_128,
    RAW_POINT_256,
    RAW_POINT_512,
    RAW_POINT_1024
};

static RADAR_REG_T InitRegList[MAX_REG_NUM] __attribute__((aligned (4))) =
{
    {0x40, 0x4207},
    {0x41, 0x0004},
    {0x01, 0x0000},
    {0x67, 0x0000},
    {0x72, 0x0650},
    {0x42, 0x0001},
    {0x43, 0xD4C0},
    {0x44, 0x0022},
    {0x45, 0x0000},
    {0x46, 0x0FA0},
    {0x47, 0x1001},
    {0x48, 0x57C0},
    {0x49, 0x2000},
    {0x4A, 0x55F0},
    {0x4B, 0x0000},
    {0x4C, 0x1770},
    {0x4D, 0x0000},
    {0x4E, 0x0001},
    {0x4F, 0x0001},
    {0x50, 0x86A0},
    {0x51, 0x0003},
    {0x52, 0x0D40},
    {0x53, 0x0A02},
    {0x54, 0xAAAB},
    {0x55, 0x0000},
    {0x56, 0x0011},
    {0x57, 0xFFFF},
    {0x58, 0xFFBC},
    {0x59, 0x0000},
    {0x5A, 0x0000},
    {0x5B, 0x0022},
    {0x5C, 0x0022},
    {0x5D, 0x1919},
    {0x5E, 0xFF00},
    {0x61, 0x02B5},
    {0x62, 0x02B5},
    {0x63, 0x02B5},
    {0x64, 0x02B5},
    {0x6E, 0x83FC},
    {0x66, 0x0A00},
    {0x6C, 0x9990},
    {0x6D, 0x9580},
    {0x70, 0x2EA0},
    {0x76, 0x0021},
    {0x02, 0x2064},
    {0x04, 0x020C},
    {0x09, 0x6901},
    {0x0A, 0xC200},
    {0x0B, 0xC064},
    {0x05, 0x000A},
    // {0x06, 0x11FE}, // 1.538Mhz  可以收
    // {0x06, 0x11B7}, // 1.987MHz  可以收
    // {0x06, 0x1192}, // 2.510MHz  收不完整
    // {0x06, 0x017F}, // 2.980MHz  收不完整
    // {0x06, 0x015B}, // 3.974MHz  收不完整
    // {0x06, 0x0137}, // 5.960MHz  收不完整
    // {0x06, 0x0125}, // 7.947MHz  收不到
    {0x06, 0x0123}, // 12MHz     收不到
    {0x0D, 0x1E00},
    {0x0E, 0x2200},
    {0x14, 0x5A03},
    {0x15, 0x1708},
    {0x17, 0x0210},
    {0x20, 0x0000},
    {0x21, 0x0000},
    {0x22, 0x0000},
    {0x23, 0x1DDC},
    {0x24, 0x1D5E},
    {0x25, 0x1CE2},
    {0x26, 0x1C64},
    {0x27, 0x17D0},
    {0x28, 0x16D4},
    {0x29, 0x15DC},
    {0x2A, 0x15DC},
    {0x2B, 0x15DC},
    {0x2C, 0x15DC},
    {0x2D, 0x15DC},
    {0x2E, 0x15DC},
    {0x2F, 0x15DC},
    {0x72, 0x0793},
    {0x67, 0x1840},
    {0x01, 0x8E24},
    {0x41, 0xC844},
    {0x40, 0x0207},
    {0x00, 0x0000} /*must be last, do not delete!!!*/
};

int32_t aiio_rd01_data_output_cb_register(rd01_data_output_t cb)
{
    data_output_cb = cb;
    return 0;
}

static void rd01_en_gpio_init(uint8_t status)
{
    en_gpio.port = RADAR_EN_PIN;
    en_gpio.config = OUTPUT_PUSH_PULL;
    hosal_gpio_init(&en_gpio);
    hosal_gpio_output_set(&en_gpio, status);
}

static void rd01_power_gpio_init(uint8_t status)
{
    power_gpio.port = RADAR_POWER_PIN;
    power_gpio.config = OUTPUT_OPEN_DRAIN_PULL_UP;
    hosal_gpio_init(&power_gpio);
    hosal_gpio_output_set(&power_gpio, status);
}

static void rd01_power_on(void)
{
    hosal_gpio_output_set(&power_gpio, 0);
}

void rd01_power_off(void)
{
    hosal_gpio_output_set(&power_gpio, 1);
}

void rd01_rext_gpio_init(uint8_t status)
{
    rext_gpio.port = RADAR_REXT_PIN;
    rext_gpio.config = OUTPUT_PUSH_PULL;
    hosal_gpio_init(&rext_gpio);
    hosal_gpio_output_set(&rext_gpio, status);
}

void rd01_rext_on(void)
{
    hosal_gpio_output_set(&rext_gpio, 1);
}

static void rd01_i2c_init(int scl_pin, int sda_pin)
{
    int ret     = -1;

    i2c0.port = 0;
    i2c0.config.freq = 100000;                                    /* only support 305Hz~100000Hz */
    i2c0.config.address_width = HOSAL_I2C_ADDRESS_WIDTH_7BIT;     /* only support 7bit */
    i2c0.config.mode = HOSAL_I2C_MODE_MASTER;                     /* only support master */
    i2c0.config.scl = scl_pin;
    i2c0.config.sda = sda_pin;

    /* init i2c with the given settings */
    ret = hosal_i2c_init(&i2c0);
    if (ret != 0) 
    {
        hosal_i2c_finalize(&i2c0);
        printf("hosal i2c init failed!\r\n");
        return;
    }
}

void WriteSocAllReg(void)
{
    uint16_t loop = 0;
    
    while(InitRegList[loop].addr) 
    {
        rd01_iic_write(I2C_ADDR_BanYan_Chip0, (uint8_t)(InitRegList[loop].addr), InitRegList[loop].val);
        loop++;
    }
}

void setSOCEnterNormalMode(void)
{
    rd01_power_on();
    vTaskDelay(10);
    vTaskEnterCritical();      // 不进入临界区可能会导致雷达无数据出现
    WriteSocAllReg();
    vTaskExitCritical();
}

static void timer_timeout_func(TimerHandle_t xTimer)
{
    setSOCEnterNormalMode();

    create_spi_init_task(bsp_spi_slave_init);
}

/**
 * 串口DMA发送完成回调
 */
static int __uart_tx_dma_callback(void *p_arg)
{
    /* If TX transmission is completed */
    g_uartDmaSendingFlag = 0;
    return 0;
}

static void uart_dma_send(uint8_t *buff, uint16_t len)
{
#if 1
    hosal_uart_dma_cfg_t txdam_cfg = {
        .dma_buf = buff,
        .dma_buf_size = len,
    };

    /* Start a UART TX DMA transfer */
    hosal_uart_ioctl(&uart_dev_int, HOSAL_UART_DMA_TX_START, &txdam_cfg);
#else
    hosal_uart_send(&uart_dev_int, buff, len);
    g_uartDmaSendingFlag = 0;
#endif
}

/**
 * 中断回调
 */
static void __uart_rx_callback(void *p_arg)
{
	int cnt;
    hosal_uart_dev_t *p_dev = (hosal_uart_dev_t *)p_arg;

    cnt = hosal_uart_receive(p_dev, &g_cmdRecv.buf[g_cmdRecv.bufRecv][g_cmdRecv.curIndex], RADAR_CMD_RECV_BUF_SIZE);

    g_cmdRecv.curIndex += cnt;
    g_cmdRecv.idleCnt = 0;
    if (g_cmdRecv.curIndex >= RADAR_CMD_RECV_BUF_SIZE)
    {
        g_cmdRecv.cmdReady = 1;
        g_cmdRecv.bufProc = g_cmdRecv.bufRecv;
        g_cmdRecv.bufRecv = (++g_cmdRecv.bufRecv) % BUF_MAX;
        g_cmdRecv.bufLen = g_cmdRecv.curIndex;
        g_cmdRecv.curIndex = 0;
		// setUartRecv_Flag();

        BaseType_t xHigherPriorityTaskWoken, xResult;

        xHigherPriorityTaskWoken = pdFALSE;
        xResult = xEventGroupSetBitsFromISR(CmdProcessEventGroup, CMD_PROCESS_BIT, &xHigherPriorityTaskWoken );

        /* Was the message posted successfully? */
        if( xResult != pdFAIL )
        {
            /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
            switch should be requested.  The macro used is port specific and will
            be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
            the documentation page for the port being used. */
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}

static void bsp_uart_init(int uart_id, int baudrate)
{
    uart_dev_int.config.uart_id = uart_id;
    uart_dev_int.config.baud_rate = baudrate;
    /* Uart init device */
    hosal_uart_init(&uart_dev_int);

    /* Set DMA TX RX transmission complete interrupt callback */
    hosal_uart_callback_set(&uart_dev_int, HOSAL_UART_TX_DMA_CALLBACK,
                          __uart_tx_dma_callback, &uart_dev_int);

    bl_uart_int_rx_notify_register(uart_dev_int.port, __uart_rx_callback, &uart_dev_int);

    // hosal_uart_ioctl(&uart_dev_int, HOSAL_UART_DMA_TX_START, &txdma_cfg);

    bl_uart_int_rx_enable(uart_dev_int.port);
#if (UART_TRAN_NUM==1)
    bl_irq_register(UART1_IRQn, UART1_IRQHandler);
    bl_irq_enable(UART1_IRQn);
#elif (UART_TRAN_NUM==0)
    bl_irq_register(UART0_IRQn, UART0_IRQHandler);
    bl_irq_enable(UART0_IRQn);
#endif
}

static void rd01_platform_init(void)
{
#if USER_UART_ENABLE
    // bsp_uart_init(UART_TRAN_NUM, USER_UART_BAUD);
    uart_dma_send((uint8_t *)"Rd-01 version: 1.0.0\r\n", strlen("Rd-01 version: 1.0.0\r\n"));
#endif
    rd01_i2c_init(RADAR_I2C_SCL_PIN, RADAR_I2C_SDA_PIN);

    rd01_en_gpio_init(1);

    // soc断电
    rd01_power_gpio_init(1); //pmos控制，高电平不导通
    // 上电后，REXT先不接地
    rd01_rext_gpio_init(0);  //nmos控制，低电平不导通

    //REXT 断开后延迟3毫秒，来自AN10014C
    vTaskDelay(3);

    // Soc上电   
    rd01_power_on();
    vTaskDelay(7);  

    // 配置定时器60ms中断
    timer_60ms_handle = xTimerCreate("timer_ms", pdMS_TO_TICKS(RADAR_POWER_TIMEOUT), pdFALSE, (void *)0, timer_timeout_func);

    printf("Platform_Init done\r\n");
}

static int rd01_iic_write(uint16_t dev_addr, uint8_t reg_addr, uint16_t data)
{
    // 需要连续写
    uint8_t databuff[3];
    databuff[0] = reg_addr;
    databuff[1] = (uint8_t)(data >> 8);
    databuff[2] = (uint8_t)(data);

    dev_addr >>= 1;

    hosal_i2c_master_send(&i2c0, dev_addr, (const uint8_t *)databuff, sizeof(databuff)/sizeof(databuff[0]), HOSAL_WAIT_FOREVER);

	return 0;
}

static int rd01_iic_read(uint16_t devAddr, uint8_t regAddr, uint16_t *regVal)
{  
    uint16_t val = 0;
    uint8_t *pval = (uint8_t *)&val; 

    devAddr >>= 1;

    hosal_i2c_mem_read(&i2c0, devAddr, regAddr, HOSAL_I2C_MEM_ADDR_SIZE_8BIT, pval, sizeof(val), HOSAL_WAIT_FOREVER);
    *regVal = BIG16_TO_LE(val);

	return 0;
}

static void rd01_reg_init(void)
{
    uint16_t loop = 0;
    uint16_t data;

    while(InitRegList[loop].addr)
    {
        rd01_iic_write(I2C_ADDR_BanYan_Chip0, (uint8_t)(InitRegList[loop].addr), InitRegList[loop].val);
        loop++;
    }

    loop = 0; 
    while(InitRegList[loop].addr) 
    {
        data = 0;
        rd01_iic_read(I2C_ADDR_BanYan_Chip0, (uint8_t)(InitRegList[loop].addr), &data);
        printf("InitRegList[%d].addr=0x%02x, data=0x%04x\r\n", loop, InitRegList[loop].addr, data);
        loop++;
    }

    vTaskDelay(22);
    rd01_rext_on();  //REXT接地
    vTaskDelay(23);
}

uint8_t GetLowPowerType(void)
{
	return lowPowerType;
}

static uint8_t Radar_GetDataType(void)
{
    uint8_t dataType = DATA_TYPE_FFT;
    uint16_t val;
    
    rd01_iic_read(I2C_ADDR_BanYan_Chip0, BANYAN_DIG_FUN_SWITCH, &val);

    if (val & BANYAN_DFFT_DATA)
    {
        dataType = DATA_TYPE_DFFT;
    }
    else if (val & BANYAN_FFT_DATA)
    {
        dataType = DATA_TYPE_FFT;
    }
    else if (val & BANYAN_DFFT_PEAK_DATA)
    {
        dataType = DATA_TYPE_DFFT_PEAK;
    }
	else if (val & BANYAN_DSRAW_DATA)
    {
        dataType = DATA_TYPE_DSRAW;
    }
    else
    {
        dataType = DATA_TYPE_MAX;
    } 

    return dataType;
}

static uint16_t Radar_GetRawPoint(void)
{
    uint16_t val = 256;
    uint16_t rawVal = 0;
    
    rd01_iic_read(I2C_ADDR_BanYan_Chip0, BANYAN_DIG_RAW_PEAK_NUM, &val);
    rawVal = (val >> BANYAN_RAW_POS) & BANYAN_RAW_MASK;

    if (rawVal < RAW_MAP_NUM)
    {
        return rawPointMap[rawVal];
    }
    else
    {
        return RAW_POINT_64;
    }
}

uint16_t Radar_GetFftPoint(void)
{
	uint16_t regVal = 64;
    rd01_iic_read(I2C_ADDR_BanYan_Chip0, BANYAN_DIG_FFT_NUM, &regVal);
	return regVal;
}

uint16_t Radar_GetDfftPeakSize(void)
{
    uint16_t val = 32;
    
    rd01_iic_read(I2C_ADDR_BanYan_Chip0, BANYAN_DIG_RAW_PEAK_NUM, &val);

    return ((val & BANYAN_PEAK_MASK) * 4); /*4--word length*/
}

uint16_t Radar_GetOneFrameChirpNum(void)
{
	uint16_t val = 32;
    rd01_iic_read(I2C_ADDR_BanYan_Chip0, BANYAN_PAT_CHIRP_NUM, &val);
	return val;
}

static int8_t GetRadarPara(RADAR_PARA_T *radarPara)
{
    if (NULL == radarPara)
    {
        return -1;
    }
    
    radarPara->dataType = Radar_GetDataType();

    printf("dataType=%d\r\n", radarPara->dataType);

    switch (radarPara->dataType)
    {
		case DATA_TYPE_DSRAW:
            radarPara->dataLen = Radar_GetRawPoint() * 2 * 2 + SPI_FRAME_HLEN + SPI_FRAME_TLEN;  /*16 bit, IQ*/
            break;
        
        case DATA_TYPE_FFT:
            radarPara->dataLen = Radar_GetFftPoint() * 2 * 2 + SPI_FRAME_HLEN + SPI_FRAME_TLEN;  /*16 bit, IQ*/
            break;
        
        case DATA_TYPE_DFFT_PEAK:
            radarPara->dataLen = Radar_GetDfftPeakSize() + SPI_FRAME_HLEN + SPI_FRAME_TLEN;
            break;
        
        default:
            printf("Error: unsupport dataType\r\n");
            return -1;
    }

    printf("dataLen=%d\r\n", radarPara->dataLen);

    if (radarPara->dataLen > SPI_FRAME_LEN_MAX)
    {
        printf("Error: dataLen is too long\r\n");
        return -1;
    }

    radarPara->chirpNum = Radar_GetOneFrameChirpNum();

    printf("chirpNum=%d\r\n", radarPara->chirpNum);
    
    return 0;
}

static void rd01_data_proc_init(void)
{
    uint8_t channel = 0;
    
    memset(&RadarDataParse, 0 ,sizeof(RadarDataParse));
    memset(&RadarPara, 0 ,sizeof(RadarPara));

    if (GetRadarPara(&RadarPara) != 0)  // IIC读取雷达参数
    {
        printf("get radar param failed!\r\n");
    }

    for (channel = 0; channel < 1; channel++)
    {
        RadarDataParse[channel].needCopyLen = RadarPara.dataLen - SPI_FRAME_HLEN - SPI_FRAME_TLEN;
    }
}

static void DataStateIdParse(uint8_t data, uint8_t channel)
{
    uint8_t flag = 0;
    
    switch (RadarPara.dataType)
    {
        case DATA_TYPE_FFT: // ID由4bit组成，FFT_ID: 0 – FFT0, 1 – FFT1，所以数据为3或7
            if ((data & ID_MASK) == FFT0_ID || (data & ID_MASK) == FFT1_ID)
            {
				if(channel > 1)
				{
					data += 0x20;
				}
                RadarDataParse[channel].chirpIndex = (data & CHIRP_INDEX_MASK) << CHIRP_INDEX_POS0;
                flag = 1;
            }
            break;
        case DATA_TYPE_DSRAW:
            if ((data & ID_MASK) == DSRAW0_ID || (data & ID_MASK) == DSRAW1_ID)
            {
				if(channel > 1)
				{
					data += 0x20;
				}
                RadarDataParse[channel].chirpIndex = (data & CHIRP_INDEX_MASK) << CHIRP_INDEX_POS0;
                flag = 1;
            }
            break;
        case DATA_TYPE_DFFT:
			if ((data & ID_MASK) == DFFT0_ID || (data & ID_MASK) == DFFT1_ID)
            {
				if(channel > 1)
				{
					data -= 0x80;
				}
                flag = 1;
			}
            break;
        
        case DATA_TYPE_DFFT_PEAK:
            if ((data & ID_MASK) == DFFT_PEAK_ID)
            {			
                flag = 1;
			}
            break;
        
        default:
            break;
    }

    if (flag)
    {
        RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = data;
        RadarDataParse[channel].state = DATA_STATE_INDEX1;
    }
    else
    {
        RadarDataParse[channel].state = DATA_STATE_HEAD;
    }
}

static void DataStateIndex1Parse(uint8_t data, uint8_t channel)
{
    switch (RadarPara.dataType)
		case DATA_TYPE_DSRAW:
    {
        case DATA_TYPE_FFT:
            RadarDataParse[channel].chirpIndex += data >> CHIRP_INDEX_POS1;
            break;
        
        case DATA_TYPE_DFFT:
            RadarDataParse[channel].frameCnt = data << FRAME_CNT_POS;
            break;
        
        case DATA_TYPE_DFFT_PEAK:
            break;
        
        default:
            break;
    }

    RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = data;
    RadarDataParse[channel].state = DATA_STATE_INDEX2;
}

static void DataStateIndex2Parse(uint8_t data, uint8_t channel)
{
    switch (RadarPara.dataType)
    {
        case DATA_TYPE_FFT:
            break;
        
        case DATA_TYPE_DFFT:
            RadarDataParse[channel].frameCnt += data;
            break;
        
        case DATA_TYPE_DFFT_PEAK:
            break;
        
        default:
            break;
    }
    
    RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = data;
    RadarDataParse[channel].state = DATA_STATE_DATA;
}

static void DataCopy(uint8_t* buf, uint16_t len, uint8_t channel, uint16_t *i)
{
    uint16_t copyLen = 0;

    if (NULL == buf || NULL == i)
    {
        return;
    }
    
	copyLen = (RadarDataParse[channel].needCopyLen > (len - *i))?
            (len - *i) : RadarDataParse[channel].needCopyLen;
    memcpy(&RadarDataParse[channel].buf[RadarDataParse[channel].curIndex], &buf[*i], copyLen);

    RadarDataParse[channel].curIndex += copyLen;
    *i += (copyLen - 1);
    RadarDataParse[channel].needCopyLen -= copyLen;

    if (!RadarDataParse[channel].needCopyLen)
    {
    	RadarDataParse[channel].state = DATA_STATE_TAIL0;
        RadarDataParse[channel].needCopyLen = RadarPara.dataLen - SPI_FRAME_HLEN - SPI_FRAME_TLEN;
    }
}

static uint8_t DataStateTail2Parse(uint8_t data, uint8_t channel)
{
    switch (RadarPara.dataType)
    {
        case DATA_TYPE_FFT:
			if(channel > 1)
			{
				data += 0x20;
			}
            break;
				
        case DATA_TYPE_DSRAW:
			if(channel > 1)
			{
				data += 0x20;
			}
            
            break;
        
        default:
            break;
    }

	RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = data;
	RadarDataParse[channel].state = DATA_STATE_TAIL3;
    return 1;
}

static uint8_t DataStateTail3Parse(uint8_t data, uint8_t channel)
{
    RadarDataParse[channel].state = DATA_STATE_HEAD;
    if (data != DATA_TAIL) 
    {
        return 0;
    }

    RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = data;
    
    return 1;
}

static uint8_t DataParse(uint8_t* buf, uint16_t len, uint8_t channel, uint16_t* left)
{
    uint16_t i = 0;
    uint8_t parseFinish = 0;
	*left = len;
	
    if (NULL == buf || 0 == len || channel >= 1)
    {
        return 0;
    }

	for (i = 0; (i < len) && (parseFinish == 0); i++) 
    {		
        switch(RadarDataParse[channel].state)   // 数据解析状态
		{                    
			case DATA_STATE_HEAD:               // 包头原封不动存入buffer
				if (buf[i] == DATA_HEAD) 
                {
                    RadarDataParse[channel].curIndex = 0;
                    RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = buf[i];
					RadarDataParse[channel].state = DATA_STATE_ID;
				}
				break;
                
			case DATA_STATE_ID:
                DataStateIdParse(buf[i], channel);
				break;
                
			case DATA_STATE_INDEX1:
                DataStateIndex1Parse(buf[i], channel);
				break;
                
			case DATA_STATE_INDEX2:
                DataStateIndex2Parse(buf[i], channel);
				break;
            
			case DATA_STATE_DATA:	
                DataCopy(buf, len, channel, &i);
				break;

            case DATA_STATE_TAIL0:
                RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = buf[i];
				RadarDataParse[channel].state = DATA_STATE_TAIL1;
				break;

            case DATA_STATE_TAIL1:
                RadarDataParse[channel].buf[RadarDataParse[channel].curIndex++] = buf[i];
				RadarDataParse[channel].state = DATA_STATE_TAIL2;
				break;

            case DATA_STATE_TAIL2:
				DataStateTail2Parse(buf[i], channel);
				break;
                
   		    case DATA_STATE_TAIL3:
                parseFinish = DataStateTail3Parse(buf[i], channel);
				break;
			
			default:
				RadarDataParse[channel].state = DATA_STATE_HEAD;
				break;
		}
	}

	*left -= i;
	
    return parseFinish;
}

static void CheckChirpIndex(uint8_t channel, uint8_t chirpIndex)
{
    static uint8_t curIndex[1] = {0};
    static uint8_t oldCurIndex[1] = {0};
    static uint8_t skipNum = SKIP_NUM;

    if (channel >= 1)
    {
        return;
    }
        
	if (skipNum) 
    {
		skipNum--;
		curIndex[channel] = oldCurIndex[channel] = chirpIndex % RadarPara.chirpNum;
	} 
    else 
    {
		curIndex[channel] = chirpIndex % RadarPara.chirpNum;
		if (curIndex[channel] != ((oldCurIndex[channel] + 1) % RadarPara.chirpNum))
        {
			// Indicator_RadarDataIndexError();
		}
		oldCurIndex[channel] = curIndex[channel];
	}
}

static void CheckFrameCnt(uint8_t channel, uint16_t frameCnt)
{
    static uint16_t oldFrameCnt[1] = {0};
    static uint8_t skipNum = SKIP_NUM;

    if (channel >= 1)
    {
        return;
    }
    
	if (skipNum) 
    {
		skipNum--;
		oldFrameCnt[channel] = frameCnt;
	} 
    else if (frameCnt != oldFrameCnt[channel] + 1)
    {
        // Indicator_RadarDataIndexError();
    }
    oldFrameCnt[channel] = frameCnt;
}

/********************************************
 @名称；CheckFullFrame
 @功能：判断MCU是否收到完整帧
 @参数：chirpIndex， 待检查chirp索引
 @返回：-1，错误的帧索引; 0，帧索引正确，不是完整帧; 1，接收完整帧
 @作者：AE TEAM
*********************************************/
static int8_t CheckFullFrame(uint16_t index)
{
    static uint8_t nExpIndex = 0;
    if (index == nExpIndex) 
    {
        nExpIndex = (nExpIndex + 1) % FRAME_DEPTH;
    }
    else
    {
        nExpIndex = 0;
		/*解决烧录后上电第一次数据异常的问题*/
		// if(isFirstRun() == 0)
			// NVIC_SystemReset();  //TODO 暂时注释
        return -1;
    }

    if (index == (FRAME_DEPTH-1))
    {
        return 1;
    }

    return 0;
}

void bsp_spi_deinit(void)
{
    if (sg_spi_enable != 0)
    {
       hosal_spi_finalize(&spi);
       sg_spi_enable = 0;
    }
}

static uint16_t FillReportBuf(Result * result)
{
    uint8_t  pos = 0;
    uint16_t ret = REPORT_LENGTH - MAX_MOTION_BINS_LENGTH - MAX_MOTIONLESS_BINS_LENGTH - MOTION_DATA_RANGE_BIN_LENGTH - MOTIONLESS_DATA_RANGE_BIN_LENGTH;
    uint16_t report_length = TARGET_DATA_LENGTH + DATA_TYPE_LENGTH + DATA_HEAD_LENGTH + DATA_TAIL_LENGTH + DATA_CHECK_LENGTH;
    uint8_t  report_type   = REPORT_TARGET_TYPE;
    memset(ReportBuffer, 0, sizeof(ReportBuffer));

    /* 1、填充协议头部 */
    *(uint32_t*)&ReportBuffer[pos] = (uint32_t)PROTOCOL_HEAD;
    pos += REPORT_HEAD_LENGTH;
    
    /* 2、填充数据类型，长度。长度包括数据类型，数据头，数据，数据尾，校验 */
    if(IsInDebugMode())
    {
        report_length += (MOTION_DATA_RANGE_BIN_LENGTH + MOTIONLESS_DATA_RANGE_BIN_LENGTH + MAX_MOTION_BINS_LENGTH + MAX_MOTIONLESS_BINS_LENGTH);
        report_type =  REPORT_DEBUG_TYPE;
    }
    *(uint16_t*)&ReportBuffer[pos] =  report_length;
    pos += DATA_LEN_LENGTH;
    ReportBuffer[pos] = report_type;
    pos += DATA_TYPE_LENGTH;
    
    /* 4、填充数据头部 */
    ReportBuffer[pos] = DATA_HEAD;
    pos += DATA_HEAD_LENGTH;

    /* 5、填充目标信息 */
    ReportBuffer[pos++] = result->status;

   *(uint16_t*)&ReportBuffer[pos] = result->Motion_Distance;
    pos += 2;

    ReportBuffer[pos++] = result->Motion_MaxVal;
    
    *(uint16_t*)&ReportBuffer[pos] = result->MotionLess_Distance;
    pos += 2;

    ReportBuffer[pos++] = result->MotionLess_MaxVal;
    
    *(uint16_t*)&ReportBuffer[pos] = result->Distance;
    pos += 2;
   
    /* 6、如果是DEBUG类型，则填充距离门信息，最后一个距离门的信息上报，但作为保留信息，上位机不解析 */
    if(IsInDebugMode())
    {
        /* 最远运动距离门 */
        ReportBuffer[pos] = (uint16_t)(APP_NFFT - 1);
        pos += MAX_MOTION_BINS_LENGTH;

        /* 最远微动距离门 */
        ReportBuffer[pos] = (uint16_t)(APP_NFFT - 1);
        pos += MAX_MOTIONLESS_BINS_LENGTH;

        /* 运动距离门能量值 */
        for(uint8_t k = 0; k < APP_NFFT; k++)
        {
            ReportBuffer[pos++] = NormalDopplerMaxVal[k];
        }
    
        /* 微动距离门能量值 */
        NormalAccSum[0] = 0;
        NormalAccSum[1] = 0;
        for(uint8_t k = 0; k < APP_NFFT; k++)
        {
           ReportBuffer[pos++] = NormalAccSum[k];
        }
        ReportBuffer[pos++] = NormalDopplerMaxVal[APP_NFFT];
        ReportBuffer[pos++] = NormalAccSum[APP_NFFT];
    } 
     /* 7、填充协议尾部 */
    ReportBuffer[pos] = DATA_TAIL;
    pos += DATA_TAIL_LENGTH;

    ReportBuffer[pos] = 0x00;
    pos += DATA_CHECK_LENGTH;

    *(uint32_t*)&ReportBuffer[pos] = (uint32_t)PROTOCOL_TAIL;

    if(IsInDebugMode())
    {
        ret = REPORT_LENGTH;
    }
    return ret;
}

static void AlgoResultTransfer(Result * result)
{
    uint8_t len = 0;
    /* 填充并发送数据 */
    len = FillReportBuf(result);

    if(data_output_cb != NULL)
    {
        data_output_cb(ReportBuffer, len);
    }

    /* Uart DMA 发送数据 */
#if USER_UART_ENABLE
    if (g_uartDmaSendingFlag == 0)
    {
        g_uartDmaSendingFlag = 1;
        uart_dma_send(ReportBuffer, len);
    }

    while(1 == g_uartDmaSendingFlag){}
#endif

    // for (size_t i = 0; i < len; i++)
    // {
    //     printf("%02X ", ReportBuffer[i]);
    // }
    // printf("\r\n");
}

static uint8_t CmdProc_InCmdMode(void)
{
    return CmdModeFlag;
}

static uint8_t CmdProc_IsInDebugMode(void)
{
    return DebugModeFlag;
}

static void DataDispatch(uint8_t* frameBuf, uint16_t bufLen, uint8_t channel, uint16_t index)
{
    uint8_t* dataBuf = NULL;
    uint16_t dataLen = 0;
	Result mResult;
	
    if (NULL == frameBuf || 0 == bufLen)
    {
        return;
    }

    dataBuf = frameBuf + SPI_FRAME_HLEN;
    dataLen = bufLen - SPI_FRAME_HLEN - SPI_FRAME_TLEN;

#if 1
	if (CmdProc_InCmdMode()/* || (g_axk_ble_radar_cfg.get_cmd_mode_flag()) || (g_axk_ble_ota_ctrl.get_update_flag()) */)
    {
    #if RADAR_WORK_MODE_LOG_ENABLE
        printf("Cmd Mode!!!\r\n");
    #endif
        return;
    }
#endif

	if(CmdProc_IsInDebugMode()) 
    {
        EnableDebugMode();
    }
    else
    {
        DisableDebugMode();
    }
    
	if(is_yone_buffer_clear == 0)
    {
        is_yone_buffer_clear = 1;
        memset(Yone, 0, sizeof(Yone));
        memset(Wone, 0, sizeof(Wone));
    }

    if(1 == CheckFullFrame(index))
    {
    #if COMPLETE_DATA_LOG_ENABLE
        printf("\r\nfull frame\r\n");
    #endif

    #if 1
        bsp_spi_deinit();

        /* SoC进入低功耗 */
        rd01_power_off();

        if (SpiDataQueue != NULL)
            xQueueReset(SpiDataQueue);
    #endif

        xTimerReset(timer_60ms_handle, pdMS_TO_TICKS(10));
    }

    mResult = BodyPresence(frameBuf, index,
                           gAlgorithmParam.nMaxMotionRangeBin,
                           gAlgorithmParam.nMaxMotionLessRangeBin,
                           gAlgorithmParam.nThresholdValOfMotion,
                           gAlgorithmParam.nThresholdValOfMotionLess,
                           gAlgorithmParam.nOffTime);

    if(NOBODY != mResult.status)
    {
		lowPowerType = POWER_TYPE_10HZ;
    }
    else
    {
		lowPowerType = POWER_TYPE_1HZ;
    }

	if(1 == mResult.bNeedReport)
    {
    #if COMPLETE_DATA_LOG_ENABLE
        printf("report result\r\n");
    #endif

        /* 填充并发送数据 */
        AlgoResultTransfer(&mResult);

        /* 等待数据完成 */
        // while(1 == g_uartDmaSendingFlag){}

        /* 置数据处理完成标志位 */
        // setFrameLsatChirpDataFinished();
    }
}

static void CmdProc_NopConfig(void)
{
    if (enableNopConfig 
		&& (nopConfigStopMs > nopConfigStartMs)
		&& CmdModeFlag == 0)
    {     
		uint16_t loop = 0;

		vTaskDelay(nopConfigStartMs);

		for(loop = 0; loop < nopConfigStartRegisterIndex; loop++)
		{
			int32_t val = nopConfigStartRegister[loop];
			CMD_REG_T *cmdReg = (CMD_REG_T *)(&val);
            rd01_iic_write(I2C_ADDR_BanYan_Chip0, cmdReg->addr, cmdReg->val);
			if(g_TxCount > 1)
			{
                rd01_iic_write(I2C_ADDR_BanYan_Chip1, cmdReg->addr, cmdReg->val);
			}
		}

		vTaskDelay(nopConfigStartMs);

		for(loop = 0; loop < nopConfigStopRegisterIndex; loop++)
		{
			int32_t val = nopConfigStopRegister[loop];
			CMD_REG_T *cmdReg = (CMD_REG_T *)(&val);

            rd01_iic_write(I2C_ADDR_BanYan_Chip0, cmdReg->addr, cmdReg->val);

			if(g_TxCount > 1)
			{
                rd01_iic_write(I2C_ADDR_BanYan_Chip1, cmdReg->addr, cmdReg->val);
			}
		}
    }
}

static void DoNopConfig(uint8_t channel)
{
    switch (RadarPara.dataType)
    {
		case DATA_TYPE_DSRAW:
        case DATA_TYPE_FFT:
            if ((RadarDataParse[channel].chirpIndex == RadarPara.chirpNum - 1) && (channel == 1 - 1))
            {
				CmdProc_NopConfig();
            }

            break;
        
        case DATA_TYPE_DFFT:             
            if (channel == 1 - 1)
            {
				CmdProc_NopConfig();
            }
            break;
        
        case DATA_TYPE_DFFT_PEAK:
            break;
        
        default:
            break;
    }	
}

static void DataProcess(uint8_t channel, uint8_t dmaFlag, uint8_t *recvBuf, uint16_t bufLen)
{
    uint8_t parseFinish = 0;
    uint16_t bufLeftLen = bufLen;
    uint16_t index = 0;
    uint16_t threshold = 1000;
    
    if (channel >= 1 || dmaFlag >= 2 || NULL == recvBuf)
    {
        printf("Error para!\r\n");
        return;
    }

	while (bufLeftLen > 0)
	{
		parseFinish = DataParse(recvBuf + bufLen - bufLeftLen, bufLeftLen, channel, &bufLeftLen);

		g_dataRecvFlag[channel][dmaFlag] = 0;

		if (!parseFinish)
		{
			continue;
		}
		
		switch (RadarPara.dataType)
		{
			case DATA_TYPE_DSRAW:
			case DATA_TYPE_FFT:
				CheckChirpIndex(channel, RadarDataParse[channel].chirpIndex);
				index = RadarDataParse[channel].chirpIndex;
				break;
			
			case DATA_TYPE_DFFT:
				CheckFrameCnt(channel, RadarDataParse[channel].frameCnt);	
				index = RadarDataParse[channel].frameCnt;
				threshold >>= INDICATOR_RECV_THD_DFFT_SHIFT;
				break;
			
			case DATA_TYPE_DFFT_PEAK:
				threshold >>= INDICATOR_RECV_THD_DPEAK_SHIFT;
				break;
			
			default:
				break;
		}
		
        DataDispatch(RadarDataParse[channel].buf, RadarPara.dataLen, channel, index);	
	}

    DoNopConfig(channel);
}

static void DataProc_Recv(void)
{
    uint8_t channel = 0;
    uint8_t dmaFlag = 0;
    uint16_t dataPos = 0;

    for (dmaFlag = 0; dmaFlag < 2; dmaFlag++)   // 2个buffer
    {
        if (!g_dataRecvFlag[channel][dmaFlag])  // 未接收完成
        {
            continue;
        }
        
        if (0 == dmaFlag)                       // buffer0
        {
            dataPos = 0;
        }
        else                                    // buffer1
        {
            dataPos = RadarPara.dataLen;        // 第二个buffer的起始位置
        }
        
        DataProcess(channel, dmaFlag, &g_dataRecvBuf[channel][dataPos], RadarPara.dataLen);
    }
}

static void queue_data_process_task(void *param)
{
    uint8_t spidatabuff[48];
    uint8_t index = 0;

    for (;;)
    {
        xQueueReceive(SpiDataQueue, spidatabuff, portMAX_DELAY);

    #if RAW_DATA_LOG_ENABLE
        for (size_t i = 0; i < sizeof(spidatabuff); i++)
        {
            printf("0x%02X ", spidatabuff[i]);
        }
        printf("\r\n");
    #endif

        memcpy(&g_dataRecvBuf[0][index], spidatabuff, RadarPara.dataLen);

        if (index == 0)
        {
            g_dataRecvFlag[0][0] = 1;
            index = RadarPara.dataLen;
        }
        else
        {
            g_dataRecvFlag[0][1] = 1;
            index = 0;
        }

        DataProc_Recv();
    }
}

void cs_io_irq(void *arg)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (SPIEventGroup != NULL)
    {
        xResult = xEventGroupSetBitsFromISR(SPIEventGroup, EVT_GROUP_SPI_FLAG, &xHigherPriorityTaskWoken);
        if(xResult != pdFAIL) 
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void cs_interrupt_init(void)
{
    cs_io.port = RADAR_CS_PIN;
    cs_io.config = INPUT_PULL_UP;
    hosal_gpio_init(&cs_io);
    hosal_gpio_irq_set(&cs_io, HOSAL_IRQ_TRIG_NEG_PULSE, cs_io_irq, NULL);
}

void spi_init_task(void *param)
{
    EventBits_t uxBits;

    void (*func_init)(void) = param;

    for(;;)
    {
        uxBits = xEventGroupWaitBits(SPIEventGroup, EVT_GROUP_SPI_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        if (uxBits & EVT_GROUP_SPI_FLAG)
        {
            // hosal_gpio_finalize(&cs_io);
            // hosal_gpio_irq_mask(&cs_io, 1); 
            func_init();
            break;
        }
    }

    vTaskDelete(NULL);
}

void create_spi_init_task(void (*spi_slave_init_type)(void))
{
    TaskHandle_t handle;
    handle = xTaskCreateStatic(
                            spi_init_task,       /* Function that implements the task. */
                            "spi_init_t",          /* Text name for the task. */
                            SPI_INII_STACK_SIZE,      /* Number of indexes in the xStack array. */
                            spi_slave_init_type,    /* Parameter passed into the task. */
                            14,                     /* Priority at which the task is created. */
                            xInitStack,          /* Array to use as the task's stack. */
                            &xInitTaskBuffer );  /* Variable to hold the task's data structure. */

    if (handle == NULL)
    {
        printf("create spi_init_task failed\r\n");
    }
}

static void spi_slave_cfg(void)
{
    /* spi port set */
    spi.port = 0;
    /* spi master mode */
    spi.config.mode = HOSAL_SPI_MODE_SLAVE;

    spi.config.dma_enable = 1; // 使能DMA

    spi.config.polar_phase = 0;
    spi.config.freq = 40000000; // 雷达spi主机clk freq为12.5MHz，需要比它大
    spi.config.pin_clk = 11;
    /* pin cs now must be pin 2 */
    spi.config.pin_mosi = 0;
    spi.config.pin_miso = 0xff;
    /* init spi device */
    hosal_spi_init(&spi);

    sg_spi_enable = 1;
}

int hosal_spi_dma_pp_recv(hosal_spi_dev_t *arg, uint16_t len, uint8_t *ping_buf, uint8_t *pong_buf, void (*dma_rx_cb)(void *arg, uint32_t flag))
{
    EventBits_t uxBits;
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    DMA_LLI_Cfg_Type rxllicfg;
    DMA_LLI_Ctrl_Type *ptxlli = NULL;
    spi_dma_priv_t *dma_arg = (spi_dma_priv_t*)arg->priv;

    struct DMA_Control_Reg rx_dmaCtrlRegVal = {
        .TransferSize = len/4,
        .SBSize = DMA_BURST_SIZE_1,
        .DBSize = DMA_BURST_SIZE_1,
        .SWidth = DMA_TRNS_WIDTH_32BITS,
        .DWidth = DMA_TRNS_WIDTH_32BITS,
        .SI = DMA_MINC_DISABLE,
        .DI = DMA_MINC_ENABLE,
        .I = 1,
    };

    if (!arg) {
        printf("arg err.\r\n");
        return -1;
    }

    if (dma_arg->rx_dma_ch == -1) {
        dma_arg->rx_dma_ch = hosal_dma_chan_request(0);
        if (dma_arg->rx_dma_ch < 0) {
            printf("SPI TX DMA CHANNEL get failed!\r\n");
            return -1;
        }
    }
    
    rxllicfg.dir = DMA_TRNS_P2M;
    rxllicfg.srcPeriph = DMA_REQ_SPI_RX; 
    rxllicfg.dstPeriph = DMA_REQ_NONE;

    xEventGroupClearBits(dma_arg->spi_event_group, EVT_GROUP_SPI_TR);

    SPI_Enable(arg->port, SPI_WORK_MODE_SLAVE);

    rx_dmaLliPPStruct.dmaChan = dma_arg->rx_dma_ch;
    rx_dmaLliPPStruct.dmaCtrlRegVal = rx_dmaCtrlRegVal;
    rx_dmaLliPPStruct.DMA_LLI_Cfg = &rxllicfg;
    rx_dmaLliPPStruct.operatePeriphAddr = SPI_BASE + SPI_FIFO_RDATA_OFFSET;
    rx_dmaLliPPStruct.is_single_mode = DISABLE;
    rx_dmaLliPPStruct.chache_buf_addr[0] = (uint32_t)ping_buf;
    rx_dmaLliPPStruct.chache_buf_addr[1] = (uint32_t)pong_buf;
    
    memset(ping_buf, 0x00, len);
    memset(pong_buf, 0x00, len);

    /* dma lli pp struct steup */
    DMA_LLI_PpStruct_Init(&rx_dmaLliPPStruct);

    DMA_IntMask(rx_dmaLliPPStruct.dmaChan, DMA_INT_ALL, MASK);
    DMA_IntMask(rx_dmaLliPPStruct.dmaChan, DMA_INT_TCOMPLETED, UNMASK);
    DMA_IntMask(rx_dmaLliPPStruct.dmaChan, DMA_INT_ERR, UNMASK);

    /* Install the interrupt callback function */
    hosal_dma_irq_callback_set(dma_arg->rx_dma_ch, dma_rx_cb, arg);
    rx_dmaLliPPStruct.trans_index = 0; 
    pbuff = rx_buf_max;
    bl_irq_enable(DMA_ALL_IRQn);
    DMA_Enable();
    DMA_LLI_PpStruct_Start(&rx_dmaLliPPStruct);
	return 0;
}

static void spi_dma_handler_rx(void *arg, uint32_t flag)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    hosal_spi_dev_t *dev = (hosal_spi_dev_t *)arg;
    if (NULL != dev)
    {
        if (dev->config.dma_enable)
        {
            spi_dma_priv_t *priv=  (spi_dma_priv_t *)dev->priv;
            bl_dma_int_clear(priv->rx_dma_ch);

            if (SpiDataQueue != NULL)
            {
                uint8_t *buff;

                if (dma_irq_cnt == 0)
                {
                    buff = rx_ping_buf;
                    dma_irq_cnt = 1;
                }
                else
                {
                    buff = rx_pong_buf;
                    dma_irq_cnt = 0;
                }

                xResult = xQueueSendFromISR( SpiDataQueue, buff, &xHigherPriorityTaskWoken );
                if (xResult != pdFAIL)
                {
                    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                }
            }
            else
            {
                printf("\r\nSpiDataQueue==null\r\n");
            }
        } 
    } 
    else 
    {
        printf("hosal_spi_int_handler_rx no clear isr.\r\n");
    }
}

static void bsp_spi_slave_init(void)
{
    if (sg_spi_enable != 0)
    {
        printf("spi already init\r\n");
        return;
    }

    spi_slave_cfg();

    memset(g_dataRecvBuf[0], 0, sizeof(g_dataRecvBuf[0]));

    int err = hosal_spi_dma_pp_recv(&spi, SPI_DMA_BUFF_LEN, rx_ping_buf, rx_pong_buf, spi_dma_handler_rx);
    if (err != 0)
    {
        printf("spi DMA init failed\r\n");
    }
}

static void *Radar_GetRadarParaAddr(void)
{
    // printf("InitRegList addr=%p\r\n", &InitRegList);
    return (void *)&InitRegList;
}

static uint32_t Radar_GetRadarParaLen(void)
{
    return sizeof(InitRegList);
}

static void System_ParaInit(void)
{
    sysPara.uploadSampleRate = UPLOAD_SAMPLE_RATE;
    sysPara.debugMode = DEBUG_MODE_DEFAULT;
}

static void *System_GetSysParaAddr(void)
{
    return (void *)&sysPara;
}

static uint32_t System_GetSysParaLen(void)
{
    return sizeof(sysPara);
}

static int bsp_nv_write(char *key, char *value, uint16_t val_len)
{
    int err;

    err = ef_set_env_blob(key, (void *)value, val_len);
    if (err != EF_NO_ERR)
    {
        printf("[NV] write key[%s] err\r\n", key);
    }

    return err;
}

static int bsp_nv_read(char *key, char *value, uint16_t val_len)
{
    size_t len = 0;

    ef_get_env_blob(key, (void *)value, val_len, &len);
    if (len < val_len)
    {
        printf("[NV] get fail:%d \r\n", len);
        return -1;
    }

    return 0;
}

static void Config_WriteData2Flash(void)
{
    uint16_t idx = 0;
    int err;

    flashData.magic = FLASH_MAGIC_NUM;

    err = bsp_nv_write(elem_key.magic, (char *)&flashData.magic, sizeof(flashData.magic));
    if (err != EF_NO_ERR)
    {
        printf("save magic failed\r\n");
    }

    for (idx = 0; idx < FLASH_ELEM_MAX; idx++)
    {
        bsp_nv_write(elem_key.elem_len[idx], (char *)&flashData.elem[idx].elemLen, sizeof(flashData.elem[idx].elemLen));
        bsp_nv_write(elem_key.elem_member[idx], (char *)flashData.elem[idx].elemAddr, flashData.elem[idx].elemLen/FLASH_WORD_LEN);
    }
}

void Config_RetrieveFlashData(void)
{
    uint16_t idx = 0;

    bsp_nv_read(elem_key.magic, (char *)&flashData.magic, sizeof(flashData.magic));
    printf("flashData.magic=%#x\r\n", flashData.magic);
    if (FLASH_MAGIC_NUM != flashData.magic)
    {
        printf("flash setting is empty!\r\n");
        Config_WriteData2Flash();               // 往flash写入数据
    }

    flashData.magic = 0;
    bsp_nv_read(elem_key.magic, (char *)&flashData.magic, sizeof(flashData.magic));
    if (FLASH_MAGIC_NUM != flashData.magic) // 写入失败擦除参数，死循环
    {
        printf("Error: flash work abnormal!\r\n");
    }
    
    for (idx = 0; idx < FLASH_ELEM_MAX; idx++)
    {
        bsp_nv_read(elem_key.elem_len[idx], (char *)&flashData.elem[idx].elemLen, sizeof(flashData.elem[idx].elemLen));
        printf("flashData.elem[%d].elemLen=%d\r\n", idx, flashData.elem[idx].elemLen);
        printf("flashData.elem[%d].elemAddr=%p\r\n", idx, flashData.elem[idx].elemAddr);
        bsp_nv_read(elem_key.elem_member[idx], (char *)flashData.elem[idx].elemAddr, flashData.elem[idx].elemLen/FLASH_WORD_LEN);
    }
	// Config_ReloadAlgoRegister(System_GetSysMode());             
}

static void Config_Init(void)
{
    uint16_t totalLen = 0;

    totalLen += sizeof(flashData.magic);
    
    flashData.elem[FLASH_ELEM_RADAR].elemAddr = Radar_GetRadarParaAddr();   // 获取雷达参数结构体数组地址
    flashData.elem[FLASH_ELEM_RADAR].elemLen = Radar_GetRadarParaLen();     // 获取结构体数组大小
    totalLen += flashData.elem[FLASH_ELEM_RADAR].elemLen + sizeof(flashData.elem[FLASH_ELEM_RADAR].elemLen);
    printf("flashData.elem[FLASH_ELEM_RADAR].elemAddr=%p\r\n", flashData.elem[FLASH_ELEM_RADAR].elemAddr);
    printf("flashData.elem[FLASH_ELEM_RADAR].elemLen=%d\r\n", flashData.elem[FLASH_ELEM_RADAR].elemLen);

    System_ParaInit();
    flashData.elem[FLASH_ELEM_SYS].elemAddr = System_GetSysParaAddr(); 
    flashData.elem[FLASH_ELEM_SYS].elemLen = System_GetSysParaLen();
    totalLen += flashData.elem[FLASH_ELEM_SYS].elemLen + sizeof(flashData.elem[FLASH_ELEM_SYS].elemLen);
    printf("flashData.elem[FLASH_ELEM_SYS].elemAddr=%p\r\n", flashData.elem[FLASH_ELEM_SYS].elemAddr);
    printf("flashData.elem[FLASH_ELEM_SYS].elemLen=%d\r\n", flashData.elem[FLASH_ELEM_SYS].elemLen);

    if (totalLen > FLASH_PAGE_SIZE)     // 1K
    {
        printf("Error: flashDataLen is more than FLASH_PAGE_SIZE!\r\n");
    }
	Config_RetrieveFlashData();         // flash有参数则读取参数，无参数则写入参数
}

static uint16_t FillCmdAck(const uint16_t *data, uint16_t dataLen, uint16_t cmdType, uint16_t status)
{
    uint16_t index = 0;

    if (dataLen * sizeof(uint16_t) > (CMD_LEN_MAX - CMD_OVERHEAD_LEN))
    {
        return 0;
    }
    
	memcpy(&CmdAck[index], CmdHead, sizeof(CmdHead));
    index += sizeof(CmdHead);

    if (data == NULL)
    {
        *((uint16_t*)(&CmdAck[index])) = CMD_TYPE_LEN + CMD_STATUS_LEN;
    }
	else if(masterProtocolVer != 0)
    {
        #if (RADAR_PROTOCOL_VERSION == 1)
        /* 协议版本1, 协议长度 = command length + ack length + data length */
        *((uint16_t*)(&CmdAck[index])) = CMD_TYPE_LEN + CMD_STATUS_LEN + (dataLen * sizeof(uint16_t));
        #elif (RADAR_PROTOCOL_VERSION == 2)
        /* 协议版本2 */
        /* Read version command */
        if(READ_VER_CMD == cmdType)
        {
            /* 协议长度 = command length + ack length + command string length + data length word legnth */
            *((uint16_t*)(&CmdAck[index])) = CMD_TYPE_LEN + CMD_STATUS_LEN + CMD_STR_LEN + dataLen;
        }
        else
        {
            /* 协议长度 = command length + ack length + data length */
            *((uint16_t*)(&CmdAck[index])) = CMD_TYPE_LEN + CMD_STATUS_LEN + (dataLen * sizeof(uint16_t));
        }
        #endif
    }
    else
    {
        *((uint16_t*)(&CmdAck[index])) = CMD_TYPE_LEN + (dataLen * sizeof(uint16_t));
    }
    index += sizeof(uint16_t);
    
    *((uint16_t*)(&CmdAck[index])) = cmdType | CMD_ACK_TYPE;
    index += sizeof(uint16_t);
    
	if(masterProtocolVer != 0 || data == NULL)
	{
		if(masterProtocolVer == 0)
		{
			status = (status == ACK_OK) ? ACK_OK_OLD : ACK_FAIL_OLD;
		}
		*((uint16_t*)(&CmdAck[index])) = status;
		index += sizeof(uint16_t);
	}
	
    if (data != NULL)
    {
        if((RADAR_PROTOCOL_VERSION == 2) && (READ_VER_CMD == cmdType))
        {
            *((uint16_t*)(&CmdAck[index])) = dataLen;
            index += sizeof(uint16_t);
            memcpy(&CmdAck[index], data, dataLen);
            index += dataLen;
        }
        else
        {
            memcpy(&CmdAck[index], data, dataLen*sizeof(uint16_t));
            index += dataLen * sizeof(uint16_t);
        } 
    }

	memcpy(&CmdAck[index], CmdTail, sizeof(CmdTail));
    index += sizeof(CmdTail);
    
	return index;
}

static void Config_SavePara2Flash(void)
{    
    if (!needFlashWrite)
    {
        return;
    }

    Config_WriteData2Flash();
    needFlashWrite = 0;
}

static uint8_t DataProc_NeedReconfig(void)
{
    uint8_t channel = 0;
    uint8_t needReconfig = 0;
    RADAR_PARA_T radarParaTmp = {0};
    
    if (0 != GetRadarPara(&radarParaTmp))
    {
        printf("\r\nGetRadarPara failed!\r\n");
    }

    RadarPara.chirpNum = radarParaTmp.chirpNum;
    if ((radarParaTmp.dataType != RadarPara.dataType) || (radarParaTmp.dataLen != RadarPara.dataLen))
    {
        needReconfig = 1;
        RadarPara.dataType = radarParaTmp.dataType;
        RadarPara.dataLen = radarParaTmp.dataLen;
    }
	
	memset(&RadarDataParse, 0 ,sizeof(RadarDataParse));
	for (channel = 0; channel < 1; channel++)
	{
		RadarDataParse[channel].needCopyLen = RadarPara.dataLen - SPI_FRAME_HLEN - SPI_FRAME_TLEN;
	}
	
    return needReconfig;
}

static void System_Reconfig(void)
{
    // printf("befoer deinit spi\r\n");
    bsp_spi_deinit();
    // printf("after deinit spi\r\n");
    // vTaskDelay(pdMS_TO_TICKS(1));

	DataProc_NeedReconfig();
    // printf("befoer init spi\r\n");
    bsp_spi_slave_init();
    // printf("after init spi\r\n");
}

static void Config_NeedFlashWrite(void)
{
    needFlashWrite = 1;
}

static void Radar_UpdateReg(uint16_t addr, uint16_t val)/*currently only update existing reg*/
{
    uint16_t loop = 0;

    while(InitRegList[loop].addr) 
    {   
        if (loop < REG_FIX_NUM)
        {
            loop++;
            continue;
        }
        
        if (InitRegList[loop].addr == addr)
        {
            InitRegList[loop].val = val;
            Config_NeedFlashWrite();
            return;
        }
        loop++;
    }
}

static uint16_t DoWriteReg(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint16_t status = ACK_OK;
    uint16_t regNum = 0;
    uint16_t loop = 0;
    uint16_t regVal = 0;
	uint16_t devAddress = I2C_ADDR_BanYan_Chip0;
    CMD_REG_T *cmdReg = NULL;

    if (NULL == cmd || 0 == cmdLen)
    {
        return 0;
    }
        
	if (CmdModeFlag) 
    {
		devAddress = cmd->cmdData[0] << 1;
		regNum = (cmdLen - CMD_TYPE_LEN - CMD_DEV_ADDR_LEN) / sizeof(CMD_REG_T);
		for (loop = 0; loop < regNum; loop++) 
        {
            cmdReg = (CMD_REG_T*)((uint8_t*)(cmd->cmdData) + CMD_DEV_ADDR_LEN + (loop * sizeof(CMD_REG_T)));
            rd01_iic_write(devAddress, (uint8_t)(cmdReg->addr), cmdReg->val);
			if(devAddress == I2C_ADDR_BanYan_Chip0)
			{
                //update master register
				Radar_UpdateReg(cmdReg->addr, cmdReg->val);
			}

            rd01_iic_read(devAddress, (uint8_t)(cmdReg->addr), &regVal);
			if (regVal != cmdReg->val) 
            {
				status = ACK_FAIL;
			}
		}
	} 
    else 
    {
		status = ACK_FAIL;
	}
    
    ackLen = FillCmdAck(NULL, 0, cmd->cmdType, status);
    
    return ackLen;
}

static uint16_t DoReadReg(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint16_t status = ACK_OK;
    uint16_t regNum = 0;
    uint16_t loop = 0;
    uint16_t *readBuf = NULL;
	uint16_t devAddress = I2C_ADDR_BanYan_Chip0;
	
    if (NULL == cmd || 0 == cmdLen)
    {
        return 0;
    }
        
    if (CmdModeFlag) 
    {
		devAddress = cmd->cmdData[0] << 1;
        regNum = (cmdLen - CMD_TYPE_LEN - CMD_DEV_ADDR_LEN) / CMD_REG_ADDR_LEN;
        
        if (regNum > CMD_REG_MAX)
        {
			regNum = 0;
            status = ACK_FAIL;
        }
        else
        {
			uint16_t dataIndex = (masterProtocolVer != 0) ? CMD_DATA_POS : CMD_DATA_POS_OLD;
            readBuf = (uint16_t*)(&CmdAck[dataIndex]);
            for (loop = 0; loop < regNum; loop++) 
            {
				uint16_t regVal = 0;
                rd01_iic_read(devAddress, (uint8_t)(cmd->cmdData[loop + 1]), &regVal);
				readBuf[loop] = regVal;
            }
        }   
    } 
    else 
    {
        status = ACK_FAIL;
    }
    
    ackLen = FillCmdAck(readBuf, regNum, cmd->cmdType, status);

    return ackLen;
}

int8_t System_ParaUpdate(uint16_t type, int32_t val)
{
    switch (type)  
    {
       case SYS_DEBUG_MODE:
            sysPara.debugMode = (int16_t)val;
            break;
        
        default:
            return -1;
    }

    return 0;
}

int8_t NopCofig_ParaUpdate(uint16_t type, int32_t val)
{
    switch (type)  
    {
		case NOPCONFIG_ENABLE:
			enableNopConfig = (int8_t)val;
			nopConfigStartRegisterIndex = 0;
			nopConfigStopRegisterIndex = 0;
			//printf("enablenopreg:%d\r\n", enableNopConfig);
			break;
		case NOPCONFIG_STARTTIME:
			nopConfigStartMs = (uint16_t)val;
			//printf("starttime:%d\r\n", nopConfigStartMs);
			break;
		case NOPCONFIG_STOPTIME:
			nopConfigStopMs = (uint16_t)val;
			//printf("stoptime:%d\r\n", nopConfigStopMs);
			break;
		case NOPCONFIG_STARTREGISTER:
			nopConfigStartRegister[nopConfigStartRegisterIndex++] = val;
			//printf("NOPCONFIG_STARTREGISTER:%x\r\n", val);
			break;
		case NOPCONFIG_STOPREGISTER:
			nopConfigStopRegister[nopConfigStopRegisterIndex++] = val;
			//printf("NOPCONFIG_STOPREGISTER:%x\r\n", val);
			break;        
        default:
            return -1;
    }
    return 0;
}

static uint16_t DoWritePara(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint16_t status = ACK_OK;
    uint16_t paraNum = 0;
    uint16_t loop = 0;
    CMD_PARA_T *cmdPara = NULL;
    int8_t ret = 0;

    if (NULL == cmd || 0 == cmdLen)
    {
        return 0;
    }
        
	if (CmdModeFlag) 
    {
		paraNum = (cmdLen - CMD_TYPE_LEN) / sizeof(CMD_PARA_T);
		for (loop = 0; loop < paraNum; loop++) 
        {
            cmdPara = (CMD_PARA_T*)((uint8_t*)(cmd->cmdData) + (loop * sizeof(CMD_PARA_T)));
            switch (cmd->cmdType)
            {
                case WRITE_MTT_CMD: 
                    // ret = MTT_ParaUpdate(cmdPara->type, cmdPara->val);
                    ret = -1;
                    break;

                case WRITE_SYS_CMD: 				
                    ret = System_ParaUpdate(cmdPara->type, cmdPara->val);
                    break;
                case FFT_ZEROFILL_CMD:
					// ret = fftzerofill_ParaUpdate(cmdPara->type, cmdPara->val);
                    ret = -1;
					break;
				case NOP_CONFIG_CMD:
					ret = NopCofig_ParaUpdate(cmdPara->type, cmdPara->val);
					break;
                default:
                    ret = -1;
                    break;
            }
            if (ret)
            {
                status = ACK_FAIL;
            }
			else
			{
				Config_NeedFlashWrite();
			}
		}
	} 
    else 
    {
		status = ACK_FAIL;
	}
    
    ackLen = FillCmdAck(NULL, 0, cmd->cmdType, status);
    
    return ackLen;
}

int32_t MTT_ParaRead(uint16_t type)
{
    return 0x7fffffff; /*invalid value*/
}

int32_t System_ParaRead(uint16_t type)
{
    switch (type)  
    {
        case SYS_SYSTEM_MODE:
            return sysPara.systemMode;

        case SYS_UPLOAD_SP_RATE:
            return sysPara.uploadSampleRate;

       case SYS_DEBUG_MODE:
            return sysPara.debugMode;

        default:
            return 0x7fffffff; /*invalid value*/
    }
}

static uint16_t DoReadPara(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint16_t status = ACK_OK;
    uint16_t paraNum = 0;
    uint16_t loop = 0;
    uint32_t *readBuf = NULL;

    if (NULL == cmd || 0 == cmdLen)
    {
        return 0;
    }
        
    if (CmdModeFlag) 
    {
        paraNum = (cmdLen - CMD_TYPE_LEN) / CMD_PARA_NAME_LEN;
        
        if (paraNum > CMD_PARA_MAX)
        {
			paraNum = 0;
            status = ACK_FAIL;
        }
        else
        {
			uint16_t dataIndex = (masterProtocolVer != 0) ? CMD_DATA_POS : CMD_DATA_POS_OLD;
            readBuf = (uint32_t*)(&CmdAck[dataIndex]);
            for (loop = 0; loop < paraNum; loop++) 
            {
                switch (cmd->cmdType)
                {
                    case READ_MTT_CMD:
                        readBuf[loop] = MTT_ParaRead(cmd->cmdData[loop]);
                        break;
                    
                    case READ_SYS_CMD:
                        readBuf[loop] = System_ParaRead(cmd->cmdData[loop]);
                        break;
                    
                    default:
                        break;
                }
            }
        }   
    } 
    else 
    {
        status = ACK_FAIL;
    }
    
    ackLen = FillCmdAck((uint16_t*)readBuf, paraNum*2, cmd->cmdType, status);
    
    return ackLen;
}

uint32_t Config_GetSN(void)
{
    return 0xffffffff;
}

static uint16_t DoReadSn(CMD_T *cmd)
{
    uint16_t ackLen = 0;
    uint16_t status = ACK_OK;
    uint16_t sn[SN_LEN] = {0};

    if (NULL == cmd)
    {
        return 0;
    }
        
    if (CmdModeFlag) 
    {   
        sn[0] = RADAR_DEV_MODEL_TYPE_SN;
        *(uint32_t*)(&sn[1]) = Config_GetSN();      
    } 
    else 
    {
        status = ACK_FAIL;
    }
    
    ackLen = FillCmdAck(sn, sizeof(sn), cmd->cmdType, status);
    return ackLen;
}

uint16_t DoConfigurePara(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint16_t range_bin;
    uint8_t i;
    ALGORITHM_PARAM_T tempParam = gAlgorithmParam;
    
    /* Note: the transfer data is merged by ID and DATA. The ID is 6bits and DATA is 32bits,
             PARAM_CFG_CMD(0x0060) protocol: cmd->cmdData[0] is MaxMotinRangeBinID(16bits)
                                             cmd->cmdData[1] is MaxMotinRangeBin(32bits) 
                                             cmd->cmdData[3] is MaxMotinRangeLessBinID(16bits)
                                             cmd->cmdData[4] is MaxMotinRangeLessBin(32bits)
                                             cmd->cmdData[6] is OFF_TimeID(16bits)
                                             cmd->cmdData[7] is OFF_Time(32bits)
                     
             THRESHOLD_SET_CMD(0x0064) proyocol: cmd->cmdData[0] is RangeBinID(16bits)
                                            cmd->cmdData[1] is RangeBin(32bits) 
                                            cmd->cmdData[3] is MotionThresholdID(16bits)
                                            cmd->cmdData[4] is MotionThreshold(32bits) 
                                            cmd->cmdData[6] is MotionLessThresholdID(16bits) 
                                            cmd->cmdData[7] is MotionLessThreshold(32bits),
                                            if RangeBin value is 0xFFFF, Then the threshold is for all Bins. 
     */
    
    /* Radar must be in cmd mode */
    if(0 == CmdModeFlag)
    {
        // clearCmdHeadFlag();
        goto ACK_FAIL_RETURN; 
    }
    /* config RangeBin threshold cmd */
    if(THRESHOLD_SET_CMD == cmd->cmdType)
    {
        /* check paras ID */
        if(0x0000 != cmd->cmdData[0] || 0x0001 != cmd->cmdData[3] || 0x0002 != cmd->cmdData[6])
        {
            goto ACK_FAIL_RETURN;
        }

         range_bin =  cmd->cmdData[1];
         /* range bin is out of range */
        if((0xFFFF != range_bin) && (range_bin > ONEFFT_POINT-2))
        {
            goto ACK_FAIL_RETURN;
        } 

        /* config all range bins threshold */
        if(0xFFFF == range_bin)
        {
            for(i = 0; i < ONEFFT_POINT-1 ; i++)
            {
                 tempParam.nThresholdValOfMotion[i] = (uint32_t)(cmd->cmdData[4]);
                 tempParam.nThresholdValOfMotionLess[i] = (uint32_t)(cmd->cmdData[7]);
            }
        }
        /* config sigle bin threshold */
        else
        {
            tempParam.nThresholdValOfMotion[range_bin] = (uint32_t)(cmd->cmdData[4]);
            tempParam.nThresholdValOfMotionLess[range_bin] = (uint32_t)(cmd->cmdData[7]);
        }
    }
    /* config max range bin and off time cmd */
    else if(PARAM_CFG_CMD == cmd->cmdType)
    {
         /* check para ID */
        if(0x0000 != cmd->cmdData[0] || 0x0001 != cmd->cmdData[3] || 0x0002 != cmd->cmdData[6])
        {
            goto ACK_FAIL_RETURN;
        }

        tempParam.nMaxMotionRangeBin = cmd->cmdData[1] ;
        tempParam.nMaxMotionLessRangeBin = cmd->cmdData[4];
        tempParam.nOffTime = cmd->cmdData[7];
    }

    /* is some differences between new para with old para */
    if (memcmp((void*)(&gAlgorithmParam), (void*)(&tempParam), sizeof(ALGORITHM_PARAM_T)) != 0)
    {
        memcpy((void*)(&gAlgorithmParam), (void*)(&tempParam), sizeof(ALGORITHM_PARAM_T));
        Algo_SaveParameter((uint32_t *)&gAlgorithmParam, sizeof(ALGORITHM_PARAM_T));
    }

    ackLen = FillCmdAck(NULL, 2, cmd->cmdType, ACK_OK); 
    return ackLen;

ACK_FAIL_RETURN:
    ackLen = FillCmdAck(NULL, 2, cmd->cmdType, ACK_FAIL);
    return ackLen;
}

static uint16_t DoReadConfigureParam(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint8_t i;
    uint8_t pos = 0;

    union
    {
       uint8_t  tx_buf_u8[6 + (ONEFFT_POINT-1)  * 2];
       uint16_t tx_buf_u16[3 +(ONEFFT_POINT-1) ];
		
    }Buf;

    /* Radar must be in cmd mode */
    if((0 == CmdModeFlag)/* && (0 == g_axk_ble_radar_cfg.get_cmd_mode_flag()) */)
    {
        // clearCmdHeadFlag();
        ackLen = FillCmdAck(Buf.tx_buf_u16, sizeof(Buf.tx_buf_u16)/sizeof(Buf.tx_buf_u16[0]), cmd->cmdType, ACK_FAIL);
        return ackLen;
    }
    /* fill ack data */
    Buf.tx_buf_u8[pos++] = 0xAA;
    Buf.tx_buf_u8[pos++] = ONEFFT_POINT - 1-1;
    Buf.tx_buf_u8[pos++] = (uint8_t)gAlgorithmParam.nMaxMotionRangeBin;
    Buf.tx_buf_u8[pos++] = (uint8_t)gAlgorithmParam.nMaxMotionLessRangeBin;
    for(i = 0; i < ONEFFT_POINT-1 ; i++)
    {
        Buf.tx_buf_u8[pos++] = gAlgorithmParam.nThresholdValOfMotion[i];
    }

    for(i = 0; i < ONEFFT_POINT-1 ; i++)
    {
        Buf.tx_buf_u8[pos++] = gAlgorithmParam.nThresholdValOfMotionLess[i];
    }

    *(uint16_t*)&Buf.tx_buf_u8[pos] = gAlgorithmParam.nOffTime ;

    ackLen = FillCmdAck(Buf.tx_buf_u16, sizeof(Buf.tx_buf_u16)/sizeof(Buf.tx_buf_u16[0]), cmd->cmdType, ACK_OK); 
    return ackLen;
}

static uint16_t DoCascadingMode(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint16_t status = ACK_OK;

    if (NULL == cmd || 0 == cmdLen)
    {
        return 0;
    }

    ackLen = FillCmdAck(NULL, 0, cmd->cmdType, status);

    return ackLen;
}

static uint16_t StartI2CTest(CMD_T *cmd, uint32_t cmdLen)
{
    uint16_t ackLen = 0;
    uint16_t status = ACK_OK;
	
    if (NULL == cmd || 0 == cmdLen)
    {
        return 0;
    }

    ackLen = FillCmdAck(NULL, 0, cmd->cmdType, status);
    return ackLen;
}

static uint16_t StopI2CTest(CMD_T *cmd, uint32_t cmdLen)
{
	uint16_t ackLen = 0;
    uint16_t status = ACK_OK;

    ackLen = FillCmdAck(NULL, 0, cmd->cmdType, status);
	return ackLen;	
}

static uint16_t GetI2CTestResult(CMD_T *cmd, uint32_t cmdLen)
{
	uint16_t ackLen = 0;
	uint16_t status = ACK_OK;
	uint8_t buf[10] = {0};

	ackLen = FillCmdAck((uint16_t*)buf, 5, cmd->cmdType, status);
	return ackLen;
}

static void CmdExec(CMD_T *cmd, uint32_t cmdLen)
{
    // CMD_REG_T *cmdReg = NULL;
    uint16_t  ackLen = 0;
    uint16_t status = ACK_OK;
    uint16_t temp_result = 0;
    int ret;

    char *str = "v1.0.0";

    if (NULL == cmd || 0 == cmdLen)
    {
        return;
    }

    printf("cmd->cmdType=0X%04X\r\n", cmd->cmdType);

	switch(cmd->cmdType)
    {
		 /* Enable debug mode, Send all rangebin mag to PC */
        case ENABLE_DEBUG_CMD:
            if(1 == CmdModeFlag)
            {
                DebugModeFlag = 1;
                ackLen = FillCmdAck(NULL, 0, cmd->cmdType, ACK_OK);
            }
            break;

            /* Disable debug mode, only send target information to PC */
        case DISABLE_DEBUG_CMD:
            if(1 == CmdModeFlag)
            {
                ackLen = FillCmdAck(NULL, 0, cmd->cmdType, ACK_OK);
                DebugModeFlag = 0;
            }
            break;

		case START_CFG_CMD:
            printf("start cfg\r\n");
			CmdModeFlag = 1;

			if(cmdLen > CMD_TYPE_LEN)
			{
				start_cfg_ack[0] = RADAR_PROTOCOL_VERSION;;
				start_cfg_ack[1] = CMD_LEN_MAX;
				
				masterProtocolVer = cmd->cmdData[0];
				ackLen = FillCmdAck(start_cfg_ack, ARRAY_SIZE(start_cfg_ack), cmd->cmdType, ACK_OK);
			}
			else
			{
				masterProtocolVer = 0;
				ackLen = FillCmdAck(NULL, 0, cmd->cmdType, ACK_OK);
			}
		    break;
        
		case FINISH_CFG_CMD:
            printf("finish cfg\r\n");
			ackLen = FillCmdAck(NULL, 0, cmd->cmdType, ACK_OK);

            if (g_FtcmdFlag == 0)   //非产测模式
            {
                Config_SavePara2Flash();
                System_Reconfig();
            }
		    break;
            
		case WRITE_REG_CMD:
            ackLen = DoWriteReg(cmd, cmdLen);
		    break;
            
		case READ_REG_CMD:
            ackLen = DoReadReg(cmd, cmdLen);
		    break;
            
		case READ_VER_CMD:
            printf("read version\r\n");
			if(masterProtocolVer != 0)
			{
                #if (RADAR_PROTOCOL_VERSION == 1)
                ackLen = FillCmdAck(read_ver_ack, ARRAY_SIZE(read_ver_ack), cmd->cmdType, ACK_OK);
                #elif (RADAR_PROTOCOL_VERSION == 2)
                ackLen = FillCmdAck((uint16_t *)str, strlen(str), cmd->cmdType, ACK_OK);
                #endif
			}
			else
			{
				ackLen = FillCmdAck(&read_ver_ack[2], ARRAY_SIZE(read_ver_ack) - 2 , cmd->cmdType, ACK_OK);
			}
            
            break;
        
		case WRITE_MTT_CMD:
        case WRITE_SYS_CMD:
		case FFT_ZEROFILL_CMD:
		case NOP_CONFIG_CMD:
            ackLen = DoWritePara(cmd, cmdLen);
		    break;

        case READ_MTT_CMD:
        case READ_SYS_CMD:
            ackLen = DoReadPara(cmd, cmdLen);
		    break;
        
        case READ_SN_CMD:
            ackLen = DoReadSn(cmd);
		    break;
		
		 /* config para */
        case PARAM_CFG_CMD: 
        case THRESHOLD_SET_CMD:

            ackLen = DoConfigurePara(cmd, cmdLen);
            break;

        /* Read all para */
        case PARAM_READ_CMD: 

            ackLen = DoReadConfigureParam(cmd, cmdLen);
           break;
		
		case CASCADING_MODE_CMD:
			ackLen = DoCascadingMode(cmd, cmdLen);
            break;
		
		case START_I2C_TEST_CMD:
			ackLen = StartI2CTest(cmd, cmdLen);
            break;
		
		case STOP_I2C_TEST_CMD:
			ackLen = StopI2CTest(cmd, cmdLen);
            break;
        
		case GET_I2C_TEST_RESULT_CMD:
			ackLen = GetI2CTestResult(cmd, cmdLen);
            break;
		
        #if 0
		case ENTER_FACTORYMODE_CMD:     // 进产测
            printf("\r\nenter factory!\r\n\r\n");
            if(1 == CmdModeFlag)
            {
                status = ACK_OK;
            }
            else
            {
                status = ACK_FAIL;
            }

            //TODO写进入产测标志？？？
            //TODO此处配置iic？
            extern int ft_create_queue_and_task(void);
            ft_create_queue_and_task();
            Ft_System_Reconfig();

            g_FtcmdFlag =  1;
            /* Factory test command ack buffer */
            uint16_t  FT_ack[7] = {0x00, 0x01, 0x01, 0x00, 0x08, 0x08, 0x08};
			ackLen = FillCmdAck(FT_ack, ARRAY_SIZE(FT_ack), cmd->cmdType, status);
			break;
		
		case EXIT_FACTORYMODE_CMD:      // 退出产测
            if(1 == CmdModeFlag)
            {
                status = ACK_OK;
            }
            else
            {
                status = ACK_FAIL;
            }

            g_FtcmdFlag = 0;

            //写产测结果,后续AMPS通过检测结果判断雷达有无进行过产测,1为通过，0为失败
            printf("\r\nexit factory\r\n\r\n");
            printf("result: %d, len(%d)\r\n", cmd->cmdData[0], cmdLen);
            rd01_nv_write(FACTORY_RESULT_KEY, (char *)&cmd->cmdData[0], 2);

			ackLen = DoExitFactoryMode(cmd, cmdLen);
			break;
        
        case AMPS_READ_RESULT: //获取产测结果指令
            // FD FC FB FA 02 00 A1 00 04 03 02 01
            CmdModeFlag = 1;        // 不让串口输出信息
            printf("AMPS read result!\r\n");
            ret = bsp_nv_read(FACTORY_RESULT_KEY, (char *)&temp_result, sizeof(temp_result));
            if (ret != 0)
            {
                printf("AMPS read result failed!\r\n");
                uart_dma_send((uint8_t *)"\r\nRESULT FAILED\r\n", strlen("\r\nRESULT FAILED\r\n"));
            }
            else
            {
                printf("AMPS read result:%d\r\n", temp_result);

                while(1 == g_uartDmaSendingFlag){}
                g_uartDmaSendingFlag = 1;

                if (temp_result == 1)
                {
                    uart_dma_send((uint8_t *)"\r\nRESULT OK\r\n", strlen("\r\nRESULT OK\r\n"));
                }
                else
                {
                    uart_dma_send((uint8_t *)"\r\nRESULT FAILED\r\n", strlen("\r\nRESULT FAILED\r\n"));
                }
            }
            CmdModeFlag = 0;

            break;
        #endif

		default:
            printf("unknown cmd\r\n");
		    break;
	}
	
    if (ackLen > 0)
    {
        while(1 == g_uartDmaSendingFlag){}
        g_uartDmaSendingFlag = 1;
        uart_dma_send(CmdAck, ackLen);
        // printf("ack:");
        // for (size_t i = 0; i < ackLen; i++)
        // {
        //     printf("%02x ",CmdAck[i]);
        // }
        // printf("\r\n");

		if(cmd->cmdType == FINISH_CFG_CMD)
		{
			CmdModeFlag = 0;
		}
    }
}

static void CmdProcess(uint8_t* buf, uint16_t len)
{
    uint16_t loop = 0;

    if (NULL == buf || 0 == len)
    {
        return;
    }

    // printf("recvLen:%d\r\n", len);
    // for (int i = 0; i < len; i++)
    // {
    //     printf("0x%02x ", buf[i]);
    // }
    // printf("\r\n");

	for (loop = 0; loop < len; loop++) 
    {
		switch(CmdDataParse.state)
		{                    
			case CMD_STATE_HEAD0:
				if (buf[loop] == CmdHead[CMD_HEAD_0]) 
                {
                    CmdDataParse.curIndex = 0;
					CmdDataParse.state = CMD_STATE_HEAD1;
				}
				break;
                
			case CMD_STATE_HEAD1:
				if (buf[loop] == CmdHead[CMD_HEAD_1])
                {
                    CmdDataParse.state = CMD_STATE_HEAD2;
				}
                else
                {
                    CmdDataParse.state = CMD_STATE_HEAD0;
                }
				break;

            case CMD_STATE_HEAD2:
				if (buf[loop] == CmdHead[CMD_HEAD_2])
                {
                    CmdDataParse.state = CMD_STATE_HEAD3;
				}
                else
                {
                    CmdDataParse.state = CMD_STATE_HEAD0;
                }
				break;

            case CMD_STATE_HEAD3:
				if (buf[loop] == CmdHead[CMD_HEAD_3])
                {
                    CmdDataParse.state = CMD_STATE_LEN0;
				}
                else
                {
                    CmdDataParse.state = CMD_STATE_HEAD0;
                }
				break;

            case CMD_STATE_LEN0:
                CmdDataParse.len = buf[loop];
                CmdDataParse.state = CMD_STATE_LEN1;
				break;

            case CMD_STATE_LEN1:
                CmdDataParse.len += buf[loop] << CMD_LEN_HIGH_POS;
                if (CmdDataParse.len <= CMD_BUF_LEN)
                {
                    CmdDataParse.state = CMD_STATE_DATA;
                }
                else
                {
                    CmdDataParse.state = CMD_STATE_HEAD0;
                }
				break;
                
            case CMD_STATE_DATA:
                CmdDataParse.buf[CmdDataParse.curIndex++] = buf[loop];
				if (CmdDataParse.curIndex == CmdDataParse.len)
                {
					CmdDataParse.state = CMD_STATE_TAIL0;
				}
				break;
                
            case CMD_STATE_TAIL0:
				if (buf[loop] == CmdTail[CMD_TAIL_0])
                {
                    CmdDataParse.state = CMD_STATE_TAIL1;
				}
                else
                {
                    CmdDataParse.state = CMD_STATE_HEAD0;
                }
				break;

            case CMD_STATE_TAIL1:
				if (buf[loop] == CmdTail[CMD_TAIL_1])
                {
                    CmdDataParse.state = CMD_STATE_TAIL2;
				}
                else
                {
                    CmdDataParse.state = CMD_STATE_HEAD0;
                }
				break;
                
            case CMD_STATE_TAIL2:
				if (buf[loop] == CmdTail[CMD_TAIL_2])
                {
                    CmdDataParse.state = CMD_STATE_TAIL3;
				}
                else
                {
                    CmdDataParse.state = CMD_STATE_HEAD0;
                }
				break;

            case CMD_STATE_TAIL3:
				if (buf[loop] == CmdTail[CMD_TAIL_3])
                {
                    CmdExec((CMD_T*)(CmdDataParse.buf), CmdDataParse.len);  
				}
                CmdDataParse.state = CMD_STATE_HEAD0;
				break; 
			
			default:
				CmdDataParse.state = CMD_STATE_HEAD0;
				break;

		}
	}
}

static void CmdProc_Recv(void)
{
    if (!g_cmdRecv.cmdReady)
    {
        return;
    }

    CmdProcess(g_cmdRecv.buf[g_cmdRecv.bufProc], g_cmdRecv.bufLen);

    g_cmdRecv.cmdReady = 0;
}

static void cmd_process_task(void *param)
{
    EventBits_t uxBits;

    for (;;)
    {
        uxBits = xEventGroupWaitBits(CmdProcessEventGroup, CMD_PROCESS_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        
        if( ( uxBits & CMD_PROCESS_BIT ) != 0 )
        {
            CmdProc_Recv();
        }
    }
}

static uint32_t Config_ReadAlgorithmParamLen(void)
{
    int ret;
    uint32_t dataLen = 0xffffffff;

    ret = bsp_nv_read(elem_key.bodysensing, (char *)&dataLen, sizeof(dataLen));
    if (ret != 0)
    {
        dataLen = 0xffffffff;
        printf("read dataLen failed\r\n");
    }
    
    return dataLen;   
}

static uint32_t Algo_ReadParameterExistFlag(void)
{
    uint32_t bParamExistFlag = Config_ReadAlgorithmParamLen();
    return bParamExistFlag;
}

static void Algo_ReadParameter(uint32_t *buf, uint32_t bufLen)
{
    bsp_nv_read(elem_key.parameter, (char *)buf, (uint16_t)bufLen);
}

static void Config_WritebodysensingFlash(uint32_t* addr, uint16_t len)
{
    uint32_t dataLen = len;

    bsp_nv_write(elem_key.bodysensing, (char *)&dataLen, sizeof(dataLen));
    bsp_nv_write(elem_key.parameter, (char *)addr, len);
}

static void Algo_SaveParameter(uint32_t*data,uint16_t len)
{
    Config_WritebodysensingFlash((uint32_t*)data, len);
}

static void InitAlgorithmParam(void)
{

    /* Read old paras from flash, if there no paras, then set them default */
    if (Algo_ReadParameterExistFlag() != 0xffffffff)    // 读取flash判断是否有保存值
    {
        // 读取数据
        printf("read nv parameter\r\n");
        Algo_ReadParameter((uint32_t *)&gAlgorithmParam, sizeof(ALGORITHM_PARAM_T));
        printf("gAlgorithmParam.nMaxMotionLessRangeBin=%d\r\n", gAlgorithmParam.nMaxMotionLessRangeBin);
        printf("ggAlgorithmParam.nMaxMotionRangeBin=%d\r\n", gAlgorithmParam.nMaxMotionRangeBin);
        printf("gAlgorithmParam.nThresholdValOfMotion[0]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[0]);
        printf("gAlgorithmParam.nThresholdValOfMotion[1]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[1]);
        printf("gAlgorithmParam.nThresholdValOfMotion[2]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[2]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[2]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[2]);
        printf("gAlgorithmParam.nThresholdValOfMotion[3]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[3]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[3]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[3]);
        printf("gAlgorithmParam.nThresholdValOfMotion[4]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[4]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[4]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[4]);
        printf("gAlgorithmParam.nThresholdValOfMotion[5]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[5]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[5]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[5]);
        printf("gAlgorithmParam.nThresholdValOfMotion[6]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[6]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[6]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[6]);
        printf("gAlgorithmParam.nThresholdValOfMotion[7]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[7]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[7]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[7]);
        printf("gAlgorithmParam.nThresholdValOfMotion[8]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[8]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[8]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[8]);
        printf("gAlgorithmParam.nOffTime=%d\r\n", gAlgorithmParam.nOffTime);
    }
    else
    {
        printf("write init nv parameter\r\n");
        // 赋初值然后保存flash
        gAlgorithmParam.nMaxMotionLessRangeBin = PARAM_MOTION_MAX ;
        gAlgorithmParam.nMaxMotionRangeBin = PARAM_MOTIONLESS_MAX ;

        gAlgorithmParam.nThresholdValOfMotion[0] = PARAM_MOTION_SENSITIBITY_RANG0;
//        gAlgorithmParam.nThresholdValOfMotionLess[0] = PARAM_MOTIONLESS_SENSITIBITY_RANG0;

        gAlgorithmParam.nThresholdValOfMotion[1] = PARAM_MOTION_SENSITIBITY_RANG1;
//        gAlgorithmParam.nThresholdValOfMotionLess[1] = PARAM_MOTIONLESS_SENSITIBITY_RANG1;

        gAlgorithmParam.nThresholdValOfMotion[2] = PARAM_MOTION_SENSITIBITY_RANG2;
        gAlgorithmParam.nThresholdValOfMotionLess[2] = PARAM_MOTIONLESS_SENSITIBITY_RANG2;

        gAlgorithmParam.nThresholdValOfMotion[3] = PARAM_MOTION_SENSITIBITY_RANG3;
        gAlgorithmParam.nThresholdValOfMotionLess[3] = PARAM_MOTIONLESS_SENSITIBITY_RANG3;

        gAlgorithmParam.nThresholdValOfMotion[4] = PARAM_MOTION_SENSITIBITY_RANG4;
        gAlgorithmParam.nThresholdValOfMotionLess[4] = PARAM_MOTIONLESS_SENSITIBITY_RANG4;

        gAlgorithmParam.nThresholdValOfMotion[5] = PARAM_MOTION_SENSITIBITY_RANG5;
        gAlgorithmParam.nThresholdValOfMotionLess[5] = PARAM_MOTIONLESS_SENSITIBITY_RANG5;

        gAlgorithmParam.nThresholdValOfMotion[6] = PARAM_MOTION_SENSITIBITY_RANG6;
        gAlgorithmParam.nThresholdValOfMotionLess[6] = PARAM_MOTIONLESS_SENSITIBITY_RANG6;
		
		gAlgorithmParam.nThresholdValOfMotion[6] = PARAM_MOTION_SENSITIBITY_RANG6;
        gAlgorithmParam.nThresholdValOfMotionLess[6] = PARAM_MOTIONLESS_SENSITIBITY_RANG6;
		
		gAlgorithmParam.nThresholdValOfMotion[7] = PARAM_MOTION_SENSITIBITY_RANG7;
        gAlgorithmParam.nThresholdValOfMotionLess[7] = PARAM_MOTIONLESS_SENSITIBITY_RANG7;
		
		gAlgorithmParam.nThresholdValOfMotion[8] = PARAM_MOTION_SENSITIBITY_RANG8;
        gAlgorithmParam.nThresholdValOfMotionLess[8] = PARAM_MOTIONLESS_SENSITIBITY_RANG8;
		
        gAlgorithmParam.nOffTime = PARAM_OFF_TIME;

        printf("gAlgorithmParam.nMaxMotionLessRangeBin=%d\r\n", gAlgorithmParam.nMaxMotionLessRangeBin);
        printf("ggAlgorithmParam.nMaxMotionRangeBin=%d\r\n", gAlgorithmParam.nMaxMotionRangeBin);
        printf("gAlgorithmParam.nThresholdValOfMotion[0]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[0]);
        printf("gAlgorithmParam.nThresholdValOfMotion[1]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[1]);
        printf("gAlgorithmParam.nThresholdValOfMotion[2]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[2]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[2]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[2]);
        printf("gAlgorithmParam.nThresholdValOfMotion[3]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[3]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[3]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[3]);
        printf("gAlgorithmParam.nThresholdValOfMotion[4]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[4]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[4]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[4]);
        printf("gAlgorithmParam.nThresholdValOfMotion[5]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[5]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[5]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[5]);
        printf("gAlgorithmParam.nThresholdValOfMotion[6]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[6]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[6]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[6]);
        printf("gAlgorithmParam.nThresholdValOfMotion[7]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[7]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[7]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[7]);
        printf("gAlgorithmParam.nThresholdValOfMotion[8]=%d\r\n", gAlgorithmParam.nThresholdValOfMotion[8]);
        printf("gAlgorithmParam.nThresholdValOfMotionLess[8]=%d\r\n", gAlgorithmParam.nThresholdValOfMotionLess[8]);
        printf("gAlgorithmParam.nOffTime=%d\r\n", gAlgorithmParam.nOffTime);

        Algo_SaveParameter((uint32_t *)&gAlgorithmParam, sizeof(ALGORITHM_PARAM_T));
    }
}

static void CmdProc_Init(void)
{
    memset(&g_cmdRecv, 0, sizeof(g_cmdRecv));
	InitAlgorithmParam();
}

int32_t aiio_rd01_init(void)
{
    rd01_platform_init();

    //从flash读取配置信息，没有则写入
    Config_Init();

    CmdProcessEventGroup = xEventGroupCreate();
    SPIEventGroup = xEventGroupCreate();
    
    //创建spi队列
    SpiDataQueue = xQueueCreate(FUNC_QUEUE_SIZE, 48);
    if (SpiDataQueue == NULL)
    {
        printf("create queue failed!!!\r\n");
    }

    vTaskDelay(3);

    rd01_reg_init();

    xTaskCreate(cmd_process_task, "cmd_pro", 1024, NULL, CMD_PROCESS_TASK_PRIORITY, NULL);

    rd01_data_proc_init();  //数据初始化
    CmdProc_Init();

    xTaskCreate(queue_data_process_task, "data_pro", 1024, NULL, 14, &QueueTaskHandle);

    cs_interrupt_init();

    create_spi_init_task(bsp_spi_slave_init);

    return 0;
}

#endif

