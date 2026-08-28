#define __QYQ_TRILED_DRIVE_C_
#include "qyq_triled_drive.h"

volatile static uint8_t triled_number = 0; // TRILED数量
volatile static qyq_triled_control_block_t *triled_list = NULL;

#define LEDC_MODE LEDC_DMA_MODE

#define LED_NUM (150)                       // 定义常量 LED_NUM，表示 LED 的数量为 16 个
#define FRAME_NUM (1)                       // 定义常量 FRAME_NUM，表示帧的数量为 7
#define TEST_BUF_SIZE (LED_NUM * FRAME_NUM) // 定义常量 TEST_BUF_SIZE，为 LED_NUM 和 FRAME_NUM 的乘积，表示需要的测试缓冲区大小
uint32_t pixels[TEST_BUF_SIZE];             // 定义一个数组 pixels，用于存储 RGB 数据，大小为 TEST_BUF_SIZE，即 16 * 7 = 112 个 uint32_t 元素

typedef struct
{
    uint32_t *TxData;
    uint32_t TxLength;
    uint32_t Result;
} LEDC_OBJ;

typedef enum
{
    RESULT_COMPLETE = 1,
    RESULT_ERR = 2,
    RESULT_RUNNING = 3,
} LEDC_RESULT;

volatile LEDC_OBJ ledc_obj;
GDMA_InitTypeDef ledc_dma_t;

#define LEDC_PIN _PA_26
#define LED_T1H 800
#define LED_T1L 300
#define LED_T0H 300
#define LED_T0L 800
#define LED_RST 300000
#define F_INTERVAL_NS 1000000000
/* LEDC IP_CLK: 40MHz <--> 25ns */
#define NS2VAL(TIME) ((TIME) / 25)

float my_fabsf(float x)
{
    return (x < 0.0f) ? -x : x;
}

float my_fmodf(float x, float y)
{
    if (y == 0.0f)
        return 0.0f;  // 避免除0

    int n = (int)(x / y);   // 取整数部分（向0截断）
    return x - n * y;
}

static float my_fmaxf(float a, float b)
{
    return (a > b) ? a : b;
}

static float my_fminf(float a, float b)
{
    return (a < b) ? a : b;
}

/**
 * @brief  LEDC 中断处理函数
 *
 * 该函数处理 LEDC 中的各种中断，包括 CPU 请求中断、传输完成中断、等待数据超时中断、FIFO 溢出中断等。
 * 它根据不同的中断源，执行相应的处理操作，并在必要时调用回调函数或重置 LEDC。
 *
 * @param  param 指向 LEDC 对象的指针，用于获取 LEDC 的传输数据和状态信息。
 * @return 返回 0 表示处理完成。
 */
static uint32_t ledc_irq_handler(void *param)
{
    // 获取 LEDC 适配器对象，便于操作其中的数据
    LEDC_OBJ *ledc_adapter = (LEDC_OBJ *)param;
    uint32_t ledc_fifothr;        // 用于存储 FIFO 的数据量
    uint32_t InterruptStatus;     // 中断状态寄存器的值
    uint32_t *startaddr = NULL;   // 传输数据的起始地址
    static uint32_t Translen = 0; // 静态变量，记录已传输的数据长度

    // 禁用 LEDC 全局中断，防止处理过程中产生新的中断
    LEDC_INTConfig(LEDC_DEV, LEDC_BIT_GLOBAL_INT_EN, DISABLE);

    // 获取 LEDC 的中断状态寄存器的值
    InterruptStatus = LEDC_GetINT(LEDC_DEV);

    // 检查是否为 CPU 请求中断
    if (InterruptStatus & LEDC_BIT_FIFO_CPUREQ_INT)
    {
        // 打印中断状态，提示 CPU 请求中断
        // printf("[LEDC INT]CPU REQ %lx\n", InterruptStatus);

        // 清除 CPU 请求中断标志
        LEDC_ClearINT(LEDC_DEV, LEDC_BIT_FIFO_CPUREQ_INT);

        // 获取 FIFO 当前可以写入的数据量
        ledc_fifothr = LEDC_GetFIFOLevel(LEDC_DEV);

        // 计算数据传输的起始地址
        startaddr = ledc_adapter->TxData + Translen;

        // 如果剩余的数据长度大于 FIFO 的可用量，则发送 FIFO 大小的数据
        if ((ledc_adapter->TxLength - Translen) >= ledc_fifothr)
        {
            Translen += LEDC_SendData(LEDC_DEV, startaddr, ledc_fifothr);
        }
        else
        {
            // 如果剩余的数据长度小于 FIFO 的可用量，则发送剩余的数据
            Translen += LEDC_SendData(LEDC_DEV, startaddr, ledc_adapter->TxLength - Translen);
        }

        // 如果数据已全部发送完毕，重置传输长度计数器
        if ((ledc_adapter->TxLength == Translen))
        {
            Translen = 0;
        }

        // 重新启用 LEDC 全局中断
        LEDC_INTConfig(LEDC_DEV, LEDC_BIT_GLOBAL_INT_EN, ENABLE);
        return 0; // 返回 0 表示处理完成
    }

    // 检查是否为传输完成中断
    if (InterruptStatus & LEDC_BIT_LED_TRANS_FINISH_INT)
    {
        // 打印中断状态，提示传输完成
        // printf("[LEDC INT]TRANS DONE %lx\n\n", InterruptStatus);

        // 清除传输完成中断标志
        LEDC_ClearINT(LEDC_DEV, LEDC_BIT_LED_TRANS_FINISH_INT);

        // 设置传输结果为完成
        ledc_adapter->Result = RESULT_COMPLETE;

        // 执行 LEDC 软件复位
        LEDC_SoftReset(LEDC_DEV);

        // 调用传输完成的回调函数
        ledc_complete_callback();
    }

    // 检查是否为等待数据超时中断
    if (InterruptStatus & LEDC_BIT_WAITDATA_TIMEOUT_INT)
    {
        // 打印中断状态，提示等待数据超时
        // printf("[LEDC INT]wait data timeout %lx\n", InterruptStatus);

        // 清除等待数据超时中断标志
        LEDC_ClearINT(LEDC_DEV, LEDC_BIT_WAITDATA_TIMEOUT_INT);

        // 设置传输结果为错误
        ledc_adapter->Result = RESULT_ERR;

        // 执行 LEDC 软件复位
        LEDC_SoftReset(LEDC_DEV);
    }

    // 检查是否为 FIFO 溢出中断
    if (InterruptStatus & LEDC_BIT_FIFO_OVERFLOW_INT)
    {
        // 打印中断状态，提示 FIFO 溢出
        // printf("[LEDC INT]FIFO OF %lx\n", InterruptStatus);

        // 清除 FIFO 溢出中断标志
        LEDC_ClearINT(LEDC_DEV, LEDC_BIT_FIFO_OVERFLOW_INT);

        // 设置传输结果为错误
        ledc_adapter->Result = RESULT_ERR;

        // 执行 LEDC 软件复位
        LEDC_SoftReset(LEDC_DEV);
    }

    // 重新启用 LEDC 全局中断
    LEDC_INTConfig(LEDC_DEV, LEDC_BIT_GLOBAL_INT_EN, ENABLE);
    return 0; // 返回 0 表示处理完成
}

/**
 * @brief  初始化 WS2812 LED 驱动。
 *
 * 该函数配置 LEDC（LED 控制器）外设来驱动 WS2812 LED。它设置了时序参数、输出引脚，
 * 并启用了必要的中断以进行数据传输。
 */
static void qyq_triled_drive_bw20_init(void)
{
    LEDC_InitTypeDef LEDC_InitStruct;

    /* 启用 LEDC 时钟和功能
     * 确保 LEDC 外设已上电并准备使用。
     * APBPeriph_LEDC 启用 LEDC 外设的时钟，APBPeriph_LEDC_CLOCK 负责该时钟的控制。
     */
    RCC_PeriphClockCmd(APBPeriph_LEDC, APBPeriph_LEDC_CLOCK, ENABLE);

    /* 配置 LEDC 输出引脚
     * 将指定的引脚配置为 LEDC 模式，以输出数据驱动 WS2812 LED。
     */
    Pinmux_Config(LEDC_PIN, PINMUX_FUNCTION_LEDC);

    /* 初始化 LEDC 配置结构体 */
    LEDC_StructInit(&LEDC_InitStruct);

    /* 设置 WS2812 的 T0H、T0L、T1H、T1L 和复位时间的时序参数
     * 这些参数通过宏 `NS2VAL` 转换为相应的寄存器值，确保符合 WS2812 的时序要求。
     */
    LEDC_InitStruct.t0h_ns = NS2VAL(LED_T0H);
    LEDC_InitStruct.t0l_ns = NS2VAL(LED_T0L);
    LEDC_InitStruct.t1h_ns = NS2VAL(LED_T1H);
    LEDC_InitStruct.t1l_ns = NS2VAL(LED_T1L);
    LEDC_InitStruct.reset_ns = NS2VAL(LED_RST);
    LEDC_InitStruct.wait_data_time_ns = NS2VAL(LED_RST);
    LEDC_InitStruct.wait_time1_ns = NS2VAL(F_INTERVAL_NS);

    /* 配置传输模式和 LED 个数
     * 设置传输模式和需要控制的 WS2812 LED 数量。
     */
    LEDC_InitStruct.ledc_trans_mode = LEDC_MODE;
    LEDC_InitStruct.led_count = LED_NUM;
    LEDC_InitStruct.data_length = LED_NUM;

    /* 初始化 LEDC 外设，应用配置参数 */
    LEDC_Init(LEDC_DEV, &LEDC_InitStruct);

    /* 注册中断处理函数并启用中断
     * 配置中断处理函数 `ledc_irq_handler` 来处理 LEDC 的中断事件，并设置中断优先级为中等。
     */
    InterruptRegister((IRQ_FUN)ledc_irq_handler, LEDC_IRQ, (uint32_t)&ledc_obj, INT_PRI_MIDDLE);
    InterruptEn(LEDC_IRQ, INT_PRI_MIDDLE);
}

void ledc_complete_callback(void)
{
    // 打印 "new start"，表示回调函数被触发，LEDC 数据传输完成并开始新的操作
    // printf("new start\n");

    // // 检查当前 LEDC 的传输模式是否为 DMA 模式
    // if (LEDC_GetTransferMode(LEDC_DEV) == LEDC_DMA_MODE)
    // {
    //     DCache_CleanInvalidate((uint32_t)ledc_obj.TxData, (4 * ledc_obj.TxLength));
    //     // 如果是 DMA 模式，则使能 GDMA（通用 DMA）传输
    //     GDMA_Cmd(ledc_dma_t.GDMA_Index, ledc_dma_t.GDMA_ChNum, ENABLE);
    // }

    // 启动 LEDC 控制器以便进行新的数据传输
    // LEDC_Cmd(LEDC_DEV, ENABLE);
}

/**
 * @brief  LEDC irq handler to clear GDMA interrupt bits.
 * @return None.
 */
uint32_t ledc_dma_irq(void *param)
{
    // 强制转换参数（目前未使用）
    (void)param;

    // 打印调试信息，表示 LEDC 的 GDMA 中断处理程序被调用
    // printf("__LedcDmaIrqHandler__\n");

    // 清除 GDMA 中断状态，防止中断标志位未清除导致再次触发
    GDMA_ClearINT(ledc_dma_t.GDMA_Index, ledc_dma_t.GDMA_ChNum);

    // 返回 TRUE，表示中断处理成功
    return TRUE;
}

/**
 * @brief  初始化 LEDC 的 DMA 传输
 *
 * 该函数为 LEDC 分配 DMA 通道并配置 DMA 控制器，用于从内存向 LEDC 设备传输数据。
 *
 * @param  GDMA_InitStruct 指向 GDMA 初始化结构体的指针
 * @param  CallbackFunc DMA 中断的回调函数
 * @param  CallbackData 回调函数的参数
 * @param  GdmaScrAddr 源地址，指向要传输的数据
 * @return 如果初始化成功返回 _TRUE，否则返回 _FALSE
 */
bool ledc_dma_init(PGDMA_InitTypeDef GDMA_InitStruct,
                   IRQ_FUN CallbackFunc,
                   void *CallbackData,
                   uint32_t *GdmaScrAddr)
{
    uint8_t gdma_chnl;    // 用于存储分配的 GDMA 通道号
    uint8_t ledc_fifothr; // 用于存储 LEDC FIFO 的阈值

    // 断言，确保 GDMA 初始化结构体不为空
    assert_param(GDMA_InitStruct != NULL);

    // 为 LEDC 的传输分配一个 GDMA 通道
    gdma_chnl = GDMA_ChnlAlloc(0, (IRQ_FUN)CallbackFunc, (uint32_t)CallbackData, 4);
    if (gdma_chnl == 0xFF)
    { /* 如果没有可用的 DMA 通道，返回 _FALSE */
        return FALSE;
    }

    // 分配并初始化 GDMA 结构体，首先将其内存清零
    _memset((void *)GDMA_InitStruct, 0, sizeof(GDMA_InitTypeDef));

    // 设置 GDMA 控制器的索引（第 0 个 GDMA 控制器）
    GDMA_InitStruct->GDMA_Index = 0;
    // 设置使用的 GDMA 通道号
    GDMA_InitStruct->GDMA_ChNum = gdma_chnl;
    // 设置传输方向为从内存到外设
    GDMA_InitStruct->GDMA_DIR = TTFCMemToPeri_PerCtrl;
    // 设置目标握手接口为 LEDC 传输接口
    GDMA_InitStruct->GDMA_DstHandshakeInterface = GDMA_HANDSHAKE_INTERFACE_LEDC_TX;
    // 设置中断类型为块传输、全传输完成和错误中断
    GDMA_InitStruct->GDMA_IsrType = (BlockType | TransferType | ErrType);

    // 设置源地址为传输数据的起始地址
    GDMA_InitStruct->GDMA_SrcAddr = (uint32_t)(GdmaScrAddr);
    // 设置目标地址为 LEDC 数据寄存器
    GDMA_InitStruct->GDMA_DstAddr = (uint32_t)&LEDC_DEV->LEDC_DATA_REG;
    // 设置目标地址的增量模式为不改变
    GDMA_InitStruct->GDMA_DstInc = NoChange;
    // 设置源地址的增量模式为递增
    GDMA_InitStruct->GDMA_SrcInc = IncType;

    // 获取 LEDC FIFO 的阈值（可存储的数据量）
    ledc_fifothr = LEDC_GetFIFOLevel(LEDC_DEV);
    if (ledc_fifothr == 8)
    {
        // 如果 FIFO 阈值为 8，设置源和目标的传输大小为 8
        GDMA_InitStruct->GDMA_SrcMsize = MsizeEight;
        GDMA_InitStruct->GDMA_DstMsize = MsizeEight;
    }
    else
    {
        // 否则将 FIFO 阈值设置为 15，设置传输大小为 16
        LEDC_SetFIFOLevel(LEDC_DEV, 15);
        GDMA_InitStruct->GDMA_SrcMsize = MsizeSixteen;
        GDMA_InitStruct->GDMA_DstMsize = MsizeSixteen;
    }

    // 设置源数据宽度和目标数据宽度均为 4 字节
    GDMA_InitStruct->GDMA_SrcDataWidth = TrWidthFourBytes;
    GDMA_InitStruct->GDMA_DstDataWidth = TrWidthFourBytes;

    // 返回 _TRUE 表示初始化成功
    return TRUE;
}

/**
 * @brief  LEDC GDMA deinit to release DMA channel.
 * @return None.
 */
void ledc_dma_deinit(void)
{
    // 禁用 GDMA 通道的 DMA 传输
    GDMA_Cmd(0, ledc_dma_t.GDMA_ChNum, DISABLE);

    // 释放分配给 LEDC 的 GDMA 通道
    GDMA_ChnlFree(0, ledc_dma_t.GDMA_ChNum);
}

/**
 * @brief  控制 WS2812 灯光显示。
 *
 * 该函数准备传输数据，通过 LEDC 外设和 DMA 模式驱动 WS2812 灯带，并等待传输完成或发生错误。
 */
static void qyq_ws2812_start(void)
{
    ledc_obj.Result = RESULT_RUNNING;

    // 检查当前 LEDC 的传输模式是否为 DMA 模式
    if (LEDC_GetTransferMode(LEDC_DEV) == LEDC_DMA_MODE)
    {
        DCache_CleanInvalidate((uint32_t)ledc_obj.TxData, (4 * ledc_obj.TxLength));
        // 如果是 DMA 模式，则使能 GDMA（通用 DMA）传输
        GDMA_Cmd(ledc_dma_t.GDMA_Index, ledc_dma_t.GDMA_ChNum, ENABLE);
    }

    /* 启用 LEDC，开始传输 */
    LEDC_Cmd(LEDC_DEV, ENABLE);

    /* 等待传输完成或发生错误 */
    while (ledc_obj.Result == RESULT_RUNNING)
        ;
    // {
    //     vTaskDelay(5);
    // }
    // vTaskDelay(5);
}

static void qyq_ws2812_data_update(void)
{
    for (uint32_t i = 0; i < triled_number; i++)
    {
        // 奇数帧设置为关闭状态（黑色）
        if (triled_list[i].status)
        {
            pixels[i] = (triled_list[i].green << 16) | (triled_list[i].red << 8) | triled_list[i].blue;
        }
        else
        {
            pixels[i] = 0;
        }
        // printf("value:0x%08x\r\n", pixels[i]);
    }
}

/**
 * @brief 将HSV颜色空间转换为RGB颜色空间。
 *
 * @param h Hue（色调），范围为0-360。
 * @param s Saturation（饱和度），范围为0-1。
 * @param v Value（亮度），范围为0-1。
 * @param r 输出的红色分量，范围为0-255。
 * @param g 输出的绿色分量，范围为0-255。
 * @param b 输出的蓝色分量，范围为0-255。
 */
void qyq_triled_drive_hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float c = v * s;
    // float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float x = c * (1 - my_fabsf(my_fmodf(h / 60.0f, 2) - 1));
    float m = v - c;
    float r_prime, g_prime, b_prime;

    if (h >= 0 && h < 60)
    {
        r_prime = c;
        g_prime = x;
        b_prime = 0;
    }
    else if (h >= 60 && h < 120)
    {
        r_prime = x;
        g_prime = c;
        b_prime = 0;
    }
    else if (h >= 120 && h < 180)
    {
        r_prime = 0;
        g_prime = c;
        b_prime = x;
    }
    else if (h >= 180 && h < 240)
    {
        r_prime = 0;
        g_prime = x;
        b_prime = c;
    }
    else if (h >= 240 && h < 300)
    {
        r_prime = x;
        g_prime = 0;
        b_prime = c;
    }
    else
    {
        r_prime = c;
        g_prime = 0;
        b_prime = x;
    }

    *r = (uint8_t)((r_prime + m) * 255);
    *g = (uint8_t)((g_prime + m) * 255);
    *b = (uint8_t)((b_prime + m) * 255);
}

/**
 * @brief 将RGB颜色空间转换为HSV颜色空间。
 *
 * @param r 红色分量，范围为0-255。
 * @param g 绿色分量，范围为0-255。
 * @param b 蓝色分量，范围为0-255。
 * @param h 输出的色调（Hue），范围为0-360。
 * @param s 输出的饱和度（Saturation），范围为0-1。
 * @param v 输出的亮度（Value），范围为0-1。
 */
void qyq_triled_drive_rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *v)
{
    float r_prime = r / 255.0f;
    float g_prime = g / 255.0f;
    float b_prime = b / 255.0f;

    float c_max = my_fmaxf(my_fmaxf(r_prime, g_prime), b_prime);
    float c_min = my_fminf(my_fminf(r_prime, g_prime), b_prime);
    // float c_max = fmaxf(fmaxf(r_prime, g_prime), b_prime);
    // float c_min = fminf(fminf(r_prime, g_prime), b_prime);
    float delta = c_max - c_min;

    // Hue
    if (delta == 0)
    {
        *h = 0;
    }
    else if (c_max == r_prime)
    {
        *h = 60 * my_fmodf(((g_prime - b_prime) / delta), 6);
        // *h = 60 * fmodf(((g_prime - b_prime) / delta), 6);
    }
    else if (c_max == g_prime)
    {
        *h = 60 * (((b_prime - r_prime) / delta) + 2);
    }
    else
    {
        *h = 60 * (((r_prime - g_prime) / delta) + 4);
    }

    if (*h < 0)
    {
        *h += 360;
    }

    // Saturation
    if (c_max == 0)
    {
        *s = 0;
    }
    else
    {
        *s = delta / c_max;
    }

    // Value
    *v = c_max;
}

/**
 * @brief 初始化TRILED驱动
 * @param triledlist 指向TRITRILED控制块数组的指针，用于存储多个TRILED的配置信息
 * @param trilednumber 需要初始化的TRILED数量
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_init(qyq_triled_control_block_t *triledlist, uint8_t trilednumber)
{
    if (triledlist == NULL)
    {
        return -2;
    }

    triled_number = trilednumber;
    triled_list = triledlist;

    for (uint32_t i = 0; i < trilednumber; i++)
    {
        triled_list[i].status = 0;
        triled_list[i].red = 0;
        triled_list[i].green = 0;
        triled_list[i].blue = 0;
    }

    // 数据buf初始化
    qyq_ws2812_data_update();

    // 初始化
    qyq_triled_drive_bw20_init();

    /* 设置传输数据和传输长度 */
    ledc_obj.Result = RESULT_RUNNING;                 // 标记传输状态为运行中
    ledc_obj.TxData = pixels;                         // 指定传输数据的起始地址
    ledc_obj.TxLength = trilednumber;                 // 设置传输数据的长度，基于帧数和 LED 数量
    LEDC_SetTotalLength(LEDC_DEV, ledc_obj.TxLength); // 设置 LEDC 的总传输长度

    if (LEDC_MODE)
    {
        /* 如果是 DMA 模式 */
        ledc_dma_init(&ledc_dma_t, ledc_dma_irq, NULL, ledc_obj.TxData); // 初始化 DMA 传输，配置 DMA 中断和传输数据

        // 清除并失效传输数据的 DCache 缓存，确保数据的一致性
        DCache_CleanInvalidate((uint32_t)ledc_obj.TxData, (4 * ledc_obj.TxLength));

        // 初始化 GDMA（通用 DMA），使能 DMA 通道
        GDMA_Init(ledc_dma_t.GDMA_Index, ledc_dma_t.GDMA_ChNum, &ledc_dma_t);
        GDMA_Cmd(ledc_dma_t.GDMA_Index, ledc_dma_t.GDMA_ChNum, ENABLE);
    }

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 打开指定的TRILED
 * @param triled_id 需要打开的TRILED的ID
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_turn_on(uint8_t triled_id)
{
    if (triled_id >= triled_number)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    triled_list[triled_id].status = 1;

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 打开所有的TRILED
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_turn_onall(void)
{
    uint8_t i = 0;
    for (i = 0; i < triled_number; i++)
    {
        triled_list[i].status = 1;
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 关闭指定的TRILED
 * @param triled_id 需要关闭的TRILED的ID
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_turn_off(uint8_t triled_id)
{
    if (triled_id >= triled_number)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    triled_list[triled_id].status = 0;

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 关闭所有的TRILED
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_turn_offall(void)
{
    uint8_t i = 0;
    for (i = 0; i < triled_number; i++)
    {
        triled_list[i].status = 0;
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 切换指定的TRILED状态（打开->关闭或关闭->打开）
 * @param triled_id 需要切换状态的TRILED的ID
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_toggle(uint8_t triled_id)
{
    if (triled_id >= triled_number)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    if (triled_list[triled_id].status == 0)
    {
        triled_list[triled_id].status = 1;
    }
    else
    {
        triled_list[triled_id].status = 0;
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 获取指定TRILED的当前状态
 * @param triled_id 需要查询的TRILED的ID
 * @return int8_t 返回TRILED的当前状态，1表示打开，0表示关闭，负数表示错误
 */
int8_t qyq_triled_drive_get_state(uint8_t triled_id, qyq_triled_control_block_t *triledstatus)
{
    if (triled_id >= triled_number)
    {
        return -1;
    }

    if (triledstatus == NULL)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    triledstatus->status = triled_list[triled_id].status;
    triledstatus->red = triled_list[triled_id].red;
    triledstatus->green = triled_list[triled_id].green;
    triledstatus->blue = triled_list[triled_id].blue;

    return 0;
}

/**
 * @brief 设置指定TRILED的颜色
 * @param triled_id 需要设置颜色的TRILED的ID
 * @param red 红色分量值（0-255）
 * @param green 绿色分量值（0-255）
 * @param blue 蓝色分量值（0-255）
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_set_color(uint8_t triled_id, uint8_t red, uint8_t green, uint8_t blue)
{
    if (triled_id >= triled_number)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    triled_list[triled_id].red = red;
    triled_list[triled_id].green = green;
    triled_list[triled_id].blue = blue;

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 设置指定TRILED的颜色
 * @param triled_id 需要设置颜色的TRILED的ID
 * @param red 红色分量值（0-255）
 * @param green 绿色分量值（0-255）
 * @param blue 蓝色分量值（0-255）
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
int8_t qyq_triled_drive_set_allcolor(uint8_t red, uint8_t green, uint8_t blue)
{
    if (triled_list == NULL)
    {
        return -2;
    }

    for (uint32_t i = 0; i < triled_number; i++)
    {
        triled_list[i].red = red;
        triled_list[i].green = green;
        triled_list[i].blue = blue;
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

/**
 * @brief 设置指定三色LED的颜色（HSV格式）。
 *
 * 该函数通过指定色调 (Hue)、饱和度 (Saturation) 和亮度 (Value)，
 * 将颜色设置到指定的三色LED上。
 *
 * @param triled_id 指定的三色LED的ID。
 * @param hue       色调，范围为 0-360，表示颜色的基本类型（如红、绿、蓝）。
 * @param sat       饱和度，范围为 0.0-1.0，表示颜色的纯度，0 表示灰色，1 表示最纯的颜色。
 * @param value     亮度，范围为 0.0-1.0，表示颜色的明暗程度，0 表示黑色，1 表示最亮。
 *
 * @return int8_t 返回 0 表示成功，负值表示失败。
 */
int8_t qyq_triled_drive_set_hsv_color(uint8_t triled_id, float hue, float sat, float value)
{
    if (triled_id >= triled_number)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    qyq_triled_drive_hsv_to_rgb(hue, sat, value, &triled_list[triled_id].red, &triled_list[triled_id].green, &triled_list[triled_id].blue);

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

int8_t qyq_triled_drive_set_rgb_numcolor(uint32_t num, uint8_t r_color, uint8_t g_color, uint8_t b_color)
{
    if (num >= triled_number)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    for (uint32_t i = 0; i < triled_number; i++)
    {
        triled_list[i].red = 0;
        triled_list[i].green = 0;
        triled_list[i].blue = 0;
    }

    for (uint32_t i = 0; i < num; i++)
    {
        triled_list[i].red = r_color;
        triled_list[i].green = g_color;
        triled_list[i].blue = b_color;
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

int8_t qyq_triled_drive_set_hsv_numcolor(uint32_t num, float hue, float sat, float value)
{
    if (num > triled_number)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    for (uint32_t i = 0; i < triled_number; i++)
    {
        triled_list[i].red = 0;
        triled_list[i].green = 0;
        triled_list[i].blue = 0;
    }

    for (uint32_t i = 0; i < num; i++)
    {
        qyq_triled_drive_hsv_to_rgb(hue, sat, value, &triled_list[i].red, &triled_list[i].green, &triled_list[i].blue);
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}
/**
 * @brief 设置所有三色LED的颜色（HSV格式）。
 *
 * 该函数通过指定色调 (Hue)、饱和度 (Saturation) 和亮度 (Value)，
 * 将颜色设置到所有三色LED上。
 *
 * @param hue   色调，范围为 0-360，表示颜色的基本类型（如红、绿、蓝）。
 * @param sat   饱和度，范围为 0.0-1.0，表示颜色的纯度，0 表示灰色，1 表示最纯的颜色。
 * @param value 亮度，范围为 0.0-1.0，表示颜色的明暗程度，0 表示黑色，1 表示最亮。
 *
 * @return int8_t 返回 0 表示成功，负值表示失败。
 */
int8_t qyq_triled_drive_set_hsv_allcolor(float hue, float sat, float value)
{
    if (triled_list == NULL)
    {
        return -2;
    }

    for (uint32_t i = 0; i < triled_number; i++)
    {
        qyq_triled_drive_hsv_to_rgb(hue, sat, value, &triled_list[i].red, &triled_list[i].green, &triled_list[i].blue);
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}

int8_t qyq_triled_drive_set_hsv_numintervals_color(uint32_t start, uint32_t end, float hue, float sat, float value)
{
    if (end >= triled_number)
    {
        return -1;
    }
    if (start > end)
    {
        return -1;
    }

    if (triled_list == NULL)
    {
        return -2;
    }

    for (uint32_t i = 0; i < triled_number; i++)
    {
        triled_list[i].red = 0;
        triled_list[i].green = 0;
        triled_list[i].blue = 0;
    }

    for (uint32_t i = start; i <= end; i++)
    {
        qyq_triled_drive_hsv_to_rgb(hue, sat, value, &triled_list[i].red, &triled_list[i].green, &triled_list[i].blue);
    }

    // 更新数据
    qyq_ws2812_data_update();

    // 启动传输
    qyq_ws2812_start();

    return 0;
}
