#ifndef __QYQ_FRAME_AT_H_
#define __QYQ_FRAME_AT_H_
#include "chip_include.h"
#ifndef __QYQ_FRAME_AT_C_
#define QYQ_FRAME_AT_EXT extern
#else
#define QYQ_FRAME_AT_EXT
#endif

// 控制参数数量
#define QYQ_FRAME_AT_ARGC_LIMIT 5
#define QYQ_FRAME_AT_RX_TIMEOUT 200

typedef int8_t (*qyq_frame_at_event_t)(unsigned int argc, const char **argv);

typedef enum qyq_frame_at_states {
    QYQ_FRAME_AT_INITIAL, // 初始化成功
    QYQ_FRAME_AT_RECEIVE, // 接收状态
    QYQ_FRAME_AT_ERROR,   // 错误状态
    QYQ_FRAME_AT_OK,      // 成功状态
    QYQ_FRAME_AT_PARSE,   // 解析状态
    QYQ_FRAME_AT_END,     // 结束状态
} qyq_frame_at_states_t;

typedef struct
{
    uint8_t *qyq_frame_at_name;                 // AT指令名字
    uint8_t qyq_frame_at_len;                   // AT指令长度
    qyq_frame_at_event_t qyq_frame_at_test_cb;  // AT测试指令，用来显示AT指令参数的合法范围
    qyq_frame_at_event_t qyq_frame_at_query_cb; // AT查询指令，用来查询AT指令当前设置的属性
    qyq_frame_at_event_t qyq_frame_at_setup_cb; // AT设置指令，用来设置AT指令的属性（有参数）
    qyq_frame_at_event_t qyq_frame_at_exe_cb;   // AT执行指令，用来执行AT指令
} qyq_frame_at_list_t;

typedef struct
{
    // AT 状态
    qyq_frame_at_states_t qyq_frame_at_states;
    // AT指令列表和列表大小
    qyq_frame_at_list_t *qyq_frame_at_list;
    uint8_t qyq_frame_at_list_size;
    // AT 时间操作
    uint32_t qyq_frame_at_tick_cnt;
    uint32_t qyq_frame_at_tick_bench;
    // AT数据接收
    uint8_t qyq_frame_at_rx_readready_status;
    uint16_t qyq_frame_at_rx_index;
    uint8_t *qyq_frame_at_rx_buf;
    uint16_t qyq_frame_at_rx_size;

    void (*qyq_frame_at_write)(uint8_t *buf, uint16_t len);
} qyq_frame_at_config_t;

typedef struct qyq_frame_at_type {
    qyq_frame_at_config_t *qyq_frame_at_config;

    int8_t (*qyq_frame_at_init)(struct qyq_frame_at_type *qyq_frame_at);
    int8_t (*qyq_frame_at_tick)(struct qyq_frame_at_type *qyq_frame_at);
    int8_t (*qyq_frame_at_run)(struct qyq_frame_at_type *qyq_frame_at);
    int8_t (*qyq_frame_at_recv)(struct qyq_frame_at_type *qyq_frame_at, uint8_t dat); // 这里的接收包含了解析处理，不建议放到中断中去处理
} qyq_frame_at_type_t;

QYQ_FRAME_AT_EXT int8_t qyq_frame_at_create(qyq_frame_at_type_t *qyq_frame_at, qyq_frame_at_config_t *qyq_frame_at_config);
#endif
