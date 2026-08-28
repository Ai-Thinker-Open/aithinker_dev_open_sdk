//rtl8711dx串口驱动
//标准库头文件
#include "stdarg.h"

//中间件头文件
#include "aiio_uart.h"
#include "aiio_error.h"
#include "aiio_chip_spec.h"
#include "aiio_log.h"

//sdk相关头文件
#include "serial_api.h"
#include "os_wrapper.h"
#include "ameba_soc.h"
#define USE_DEF_LOG (1) //是否使用默认log口，默认log口不会重新初始化log口

#define RTL8711_PIN_CALCULATE(a, b) ((a)<<5|(b))    //rtl8720中是 PIN_CALCULATE

typedef struct
{
    uint8_t inited : 1;
    uint8_t rx_irq_en : 1;
    uint8_t tx_irq_en : 1;
    uint8_t reserved : 5;
    serial_t Sobj;
    aiio_uart_int_callback irq_rx_bk;
} aiio_uart_dev_t;

static void rtl_set_debug_lev(void);
static int8_t uart_index_get(PinName tx);
static void _uart_irq(uint32_t id, SerialIrq event);
#if USE_DEF_LOG
static void rtl_log_uart_recive_task(void *Data);
static void rtl_log_uart_sema_give(void);
#else
static void log_uart_irq(uint32_t id, SerialIrq event);
#endif
static aiio_uart_dev_t s_uart_dev[2] = {0};
static serial_t *ai_rtl8711_LogSobj=NULL;    //log串口hal对象指针
#if USE_DEF_LOG
extern volatile UART_LOG_CTL		shell_ctl;
extern UART_LOG_BUF				shell_buf;
static rtos_task_t rtl_log_uart_recive_task_handle=NULL;
static rtos_sema_t	rtl_log_uart_sema = NULL;  //参考SDK中的 shell_sema
#endif
static uint8_t uart_in_at_mode_tag=0;   /*!< 0:normal mode 1:at mode*/

//log口初始化
int32_t aiio_uart_log_init(aiio_uart_config_t uart){
    rtl_set_debug_lev();
    #if USE_DEF_LOG
    LOGUART_SetBaud(LOGUART_DEV,uart.baud_rate);

    shell_ctl.ExecuteCmd = FALSE;
	shell_ctl.ExecuteEsc = TRUE; //don't check Esc anymore
	shell_ctl.GiveSema = rtl_log_uart_sema_give;
	if(NULL==rtl_log_uart_sema){
        rtos_sema_create_binary(&rtl_log_uart_sema);
    }else{
        aiio_log_d("sema inited\r\n");
    }
    if(NULL==rtl_log_uart_recive_task_handle){
        if (RTK_SUCCESS != rtos_task_create(&rtl_log_uart_recive_task_handle, "rtl_log_uart_recive_task", rtl_log_uart_recive_task, NULL, 2596, 5)) {
            aiio_log_e("create rtl_log_uart_recive_task error\r\n");
        }
    }else{
        aiio_log_d("rtl_log_uart_recive_task running\r\n");
    }
    return AIIO_OK;
    #else
    PinName tx_pin = 0;
    PinName rx_pin = 0;
    uint8_t uart_idx=0;

    switch (uart.uart_tx_pin)
    {
        case __AIIO_GPIO_NUM_5: //PB5
            tx_pin = RTL8711_PIN_CALCULATE(__AIIO_GPIO_B, __AIIO_GPIO_NUM_5);
            uart_idx=0;
            break;
        default:
            return AIIO_ERROR;
    }

    switch (uart.uart_rx_pin)
    {
        case __AIIO_GPIO_NUM_4: //PB4
            rx_pin = RTL8711_PIN_CALCULATE(__AIIO_GPIO_B, __AIIO_GPIO_NUM_4);
            break;
        default:
            break;
    }

    int8_t uart_index = uart_index_get(tx_pin);
    if (uart_index < __AIIO_UART0 || uart_index > __AIIO_UART1)
    {
        return AIIO_ERROR;
    }

    s_uart_dev[uart_index].inited = 1;
    s_uart_dev[uart_index].Sobj.uart_idx = uart_idx;
    serial_init(&s_uart_dev[uart_index].Sobj, tx_pin, rx_pin);
    serial_baud(&s_uart_dev[uart_index].Sobj, uart.baud_rate);
	serial_format(&s_uart_dev[uart_index].Sobj, 8, ParityNone, 1);

    s_uart_dev[uart_index].rx_irq_en = 1;
	serial_irq_handler(&s_uart_dev[uart_index].Sobj, log_uart_irq, (uint32_t)&s_uart_dev[uart_index].Sobj);
	serial_irq_set(&s_uart_dev[uart_index].Sobj, RxIrq, 1);
	serial_irq_set(&s_uart_dev[uart_index].Sobj, TxIrq, 0);

    ai_rtl8711_LogSobj = &s_uart_dev[uart_index].Sobj;

    return AIIO_OK;
    #endif
}

int32_t aiio_uart_log_send(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    int ch;
    char *str;
    char string[AIIO_LOG_MAX_LEN] = { 0 };
    str = string;

    if (0 < vsprintf(string, fmt, argp))
    {
        while ('\0' != (ch = *(str++)))
        {
            #if USE_DEF_LOG
            LOGUART_PutChar(ch);
            #else
            serial_putc(ai_rtl8711_LogSobj, ch);
            #endif
        }
    }

    va_end(argp);
    return AIIO_OK;
}

int32_t aiio_uart_param_check(aiio_uart_config_t uart)
{
    /*aiio_uart_log_send("\r\n\r\n\r\n[%s()-%d]uart_num=%d AIIO_UART_MAX=%d\r\n"
                    "uart_tx_pin=%d\r\n"
                    "uart_rx_pin=%d\r\n"
                    "baud_rate=%d\r\n"
                    "dataBits=%d\r\n"
                    "stopBits=%d\r\n"
                    "parity=%d\r\n\r\n\r\n",__func__,__LINE__,
                    uart.uart_num,AIIO_UART_MAX,
                    uart.uart_tx_pin,
                    uart.uart_rx_pin,
                    uart.baud_rate,
                    uart.dataBits,
                    uart.stopBits,
                    uart.parity);*/
    if( uart.uart_num > AIIO_UART1 || uart.uart_num < AIIO_UART0 ){
        return AIIO_UART_NUMBER_NOT_SUPPORT;
    }
    switch (uart.uart_tx_pin)
    {
        case __AIIO_GPIO_NUM_26:
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    switch (uart.uart_rx_pin)
    {
        case __AIIO_GPIO_NUM_25:
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    switch(uart.dataBits){
        case AIIO_UART_DATA_BITS_5:
        case AIIO_UART_DATA_BITS_6:
            return AIIO_UART_DATA_BITS_NOT_SUPPORT;
        case AIIO_UART_DATA_BITS_7:
        case AIIO_UART_DATA_BITS_8:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.stopBits){
        case AIIO_UART_STOP_BITS_1:
            break;
        case AIIO_UART_STOP_BITS_2:
            break;
        case AIIO_UART_STOP_BITS_1_5:
            return AIIO_UART_STOP_BITS_NOT_SUPPORT;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.parity){
        case AIIO_UART_PARITY_NONE:
        case AIIO_UART_PARITY_ODD:
        case AIIO_UART_PARITY_EVEN:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.flowControl){
        case AIIO_UART_FLOW_CONTROL_NONE:
            break;
        case AIIO_UART_FLOW_CONTROL_RTS:
        case AIIO_UART_FLOW_CONTROL_CTS:
        case AIIO_UART_FLOW_CONTROL_RTS_CTS:
            return AIIO_UART_FLOW_CONTROL_NOT_SUPPORT;
        default:
            return AIIO_PARAM_ERROR;
    }
    
    return AIIO_OK;
}

int32_t aiio_uart_init(aiio_uart_config_t uart)
{
    PinName tx_pin = 0;
    PinName rx_pin = 0;
    int data_bits=0;
    SerialParity parity=ParityNone;

    uart_in_at_mode_tag=0;
    //aiio_uart_log_send("\r\n\r\n\r\n[%s()-%d]baud_rate=%d dataBits=%d stopBits=%d parity=%d\r\n\r\n\r\n",__func__,__LINE__,uart.baud_rate,uart.dataBits,uart.stopBits,uart.parity);
    if( uart.uart_num > AIIO_UART1 || uart.uart_num < AIIO_UART0 ){
        return AIIO_UART_NUMBER_NOT_SUPPORT;
    }
    switch (uart.uart_tx_pin)
    {
        case __AIIO_GPIO_NUM_26:
            tx_pin = RTL8711_PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_26);
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    switch (uart.uart_rx_pin)
    {
        case __AIIO_GPIO_NUM_25:
            rx_pin = RTL8711_PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_25);
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    int8_t uart_index = uart_index_get(tx_pin);
    switch(uart.dataBits){
        case AIIO_UART_DATA_BITS_5:
        case AIIO_UART_DATA_BITS_6:
            return AIIO_UART_DATA_BITS_NOT_SUPPORT;
        case AIIO_UART_DATA_BITS_7:
        case AIIO_UART_DATA_BITS_8:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.stopBits){
        case AIIO_UART_STOP_BITS_1:
            data_bits=1;
            break;
        case AIIO_UART_STOP_BITS_2:
            data_bits=2;
            break;
        case AIIO_UART_STOP_BITS_1_5:
            return AIIO_UART_STOP_BITS_NOT_SUPPORT;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.parity){
        case AIIO_UART_PARITY_NONE:
            parity=ParityNone;
            break;
        case AIIO_UART_PARITY_ODD:
            parity=ParityOdd;
            break;
        case AIIO_UART_PARITY_EVEN:
            parity=ParityEven;
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.flowControl){
        case AIIO_UART_FLOW_CONTROL_NONE:
            break;
        case AIIO_UART_FLOW_CONTROL_RTS:
        case AIIO_UART_FLOW_CONTROL_CTS:
        case AIIO_UART_FLOW_CONTROL_RTS_CTS:
            return AIIO_UART_FLOW_CONTROL_NOT_SUPPORT;
        default:
            return AIIO_PARAM_ERROR;
    }
    s_uart_dev[uart_index].inited = 1;
    s_uart_dev[uart_index].Sobj.uart_idx = uart_index;
    serial_init(&s_uart_dev[uart_index].Sobj, tx_pin, rx_pin);
    serial_baud(&s_uart_dev[uart_index].Sobj, uart.baud_rate);
    serial_format(&s_uart_dev[uart_index].Sobj,uart.dataBits, parity,data_bits);
    if (uart.irq_rx_bk)
    {
        s_uart_dev[uart_index].rx_irq_en = 1;
        s_uart_dev[uart_index].irq_rx_bk = uart.irq_rx_bk;
        serial_irq_handler(&s_uart_dev[uart_index].Sobj, _uart_irq, (uint32_t)&s_uart_dev[uart_index].Sobj);
        serial_irq_set(&s_uart_dev[uart_index].Sobj, RxIrq, 1);
        serial_irq_set(&s_uart_dev[uart_index].Sobj, TxIrq, 0);
    }

    uart_in_at_mode_tag=uart.in_at_mode;

    return AIIO_OK;
}

int32_t aiio_uart_deinit(aiio_uart_config_t uart)
{
    if( uart.uart_num > AIIO_UART1 || uart.uart_num < AIIO_UART0 ){
        return AIIO_UART_NUMBER_NOT_SUPPORT;
    }
    if (!s_uart_dev[uart.uart_num].inited){
        return AIIO_ERROR;
    }

    serial_free(&s_uart_dev[uart.uart_num].Sobj);

    memset(&s_uart_dev[uart.uart_num], 0, sizeof(aiio_uart_dev_t));

    return AIIO_OK;
}

int32_t aiio_uart_send_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    uint16_t len = 0;

    if( uart_num > AIIO_UART1 || uart_num < AIIO_UART0 ){
        return AIIO_UART_NUMBER_NOT_SUPPORT;
    }
    if (!s_uart_dev[uart_num].inited){
        return AIIO_ERROR;
    }

    while (len < size)
    {
        if(uart_in_at_mode_tag){
            #if USE_DEF_LOG
            LOGUART_PutChar(buf[len]);
            #else
            serial_putc(ai_rtl8711_LogSobj, buf[len]);
            #endif
        }
        serial_putc(&s_uart_dev[uart_num].Sobj, buf[len++]);
    }

    return AIIO_OK;
}

int32_t aiio_uart_recv_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if( uart_num > AIIO_UART1 || uart_num < AIIO_UART0 ){
        return AIIO_UART_NUMBER_NOT_SUPPORT;
    }
    if (!s_uart_dev[uart_num].inited){
        return AIIO_ERROR;
    }
    if(s_uart_dev[uart_num].rx_irq_en){
        return AIIO_ERROR;
    }

    int32_t count=0;
    for(count=0;count<size;count++){
        buf[count] = serial_getc(&s_uart_dev[uart_num].Sobj);
    }

    return count;
}

//------------------------------------------------ 内部调用 -----------------------------------------
//设置sdk log打印登记
//原名称 app_init_debug()
static void rtl_set_debug_lev(void)
{
	u32 debug[LEVEL_NUMs];

	debug[LEVEL_ERROR] = 0xFFFFFFFF;
	debug[LEVEL_INFO]  = 0x0;
	debug[LEVEL_WARN]  = 0x0;
	debug[LEVEL_TRACE] = 0x0;

	// LOG_MASK(LEVEL_ERROR, debug[LEVEL_ERROR]);
	// LOG_MASK(LEVEL_WARN,  debug[LEVEL_WARN]);
	// LOG_MASK(LEVEL_INFO,  debug[LEVEL_INFO]);
	// LOG_MASK(LEVEL_TRACE, debug[LEVEL_TRACE]);
}

//根据pin脚获取当前串口号
static int8_t uart_index_get(PinName tx)
{
    if ( tx == _PA_26 )  //at
    {
        return __AIIO_UART1;
    }
    else if ( tx == _PB_20 )   //log
    {
        return __AIIO_UART0;
    }

    return -1;
}

static void _uart_irq(uint32_t id, SerialIrq event)
{
    serial_t *sobj = (void *)id;
    volatile char rc = 0;
    if (event == RxIrq)
    {
        rc = serial_getc(sobj);
        if (s_uart_dev[sobj->uart_idx].irq_rx_bk)
        {
            s_uart_dev[sobj->uart_idx].irq_rx_bk(&rc, 1);
        }
    }
}

#if USE_DEF_LOG
//参考 shell_task_ram()
static void rtl_log_uart_recive_task(void *Data)
{
    /* To avoid gcc warnings */
    (void) Data;
    u32 ret = FALSE;
    PUART_LOG_BUF pUartLogBuf = shell_ctl.pTmpLogBuf;
    //4 Set this for UartLog check cmd history
    shell_ctl.shell_task_rdy = 1;
    rtos_create_secure_context(RTOS_MINIMAL_SECURE_STACK_SIZE);
    do {
        rtos_sema_take(rtl_log_uart_sema, RTOS_MAX_DELAY);
        shell_loguartRx_dispatch();
        if (shell_ctl.ExecuteCmd) {
            aiio_log_d("recive data(%d)-%s-\r\n",pUartLogBuf->BufCount,pUartLogBuf->UARTLogBuf);
            if (s_uart_dev[1].irq_rx_bk)
            {
                s_uart_dev[1].irq_rx_bk(pUartLogBuf->UARTLogBuf,pUartLogBuf->BufCount);
                s_uart_dev[1].irq_rx_bk("\r\n",2);
            }
            shell_ctl.ExecuteCmd = FALSE;
        }else{
            aiio_log_e("cmd running\r\n");
        }
    } while (1);
}

//参考 shell_give_sema
static void rtl_log_uart_sema_give(void)
{
	if (shell_ctl.shell_task_rdy) {
		rtos_sema_give(rtl_log_uart_sema);
	}
}
#else
static void log_uart_irq(uint32_t id, SerialIrq event)
{
    serial_t *sobj = (void *)id;
    volatile char rc = 0;
    if (event == RxIrq)
    {
        rc = serial_getc(sobj);
        if (s_uart_dev[1].irq_rx_bk)
        {
            s_uart_dev[1].irq_rx_bk(&rc, 1);
        }
    }
}
#endif

