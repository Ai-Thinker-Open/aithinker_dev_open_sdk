#define __QYQ_FRAME_AT_C_
#include "qyq_frame_at.h"

static int qyq_frame_at_string_split(char *strp, uint32_t strsize, char ch, char *argv[], uint32_t argcM)
{
    int ch_index = 0;
    int argc_index = 0;
    unsigned char splitflag = 0;

    if ((!strsize) || (!argcM))
        return 0;

    argv[argc_index++] = &strp[ch_index];
    for (ch_index = 0; ch_index < strsize; ch_index++)
    {
        if (strp[ch_index] == ch)
        {
            strp[ch_index] = '\0';
            splitflag = 1;
        }
        else if (splitflag == 1)
        {
            splitflag = 0;
            argv[argc_index++] = &strp[ch_index];
            if (argc_index >= argcM)
                break;
        }
        else
        {
            splitflag = 0;
        }
    }

    return argc_index;
}

static int8_t qyq_frame_at_parse_at(struct qyq_frame_at_type *qyq_frame_at, uint8_t size)
{
    int8_t ret = -1;
    int index = 0;       // 索引
    uint16_t offset = 0; // 基准
    char *ptr = null;    // 字符串指针

    int argc = QYQ_FRAME_AT_ARGC_LIMIT;
    char *argv[QYQ_FRAME_AT_ARGC_LIMIT] = {(char *)0};

    // 判断指令集是否以\r\n结尾
    if ((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf[size - 2] != '\r') || (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf[size - 1] != '\n'))
    {
        return -1;
    }

    // 判断是否是AT指令
    if (strstr((const char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf, "AT") == null)
    {
        return -1;
    }

    // AT指令匹配
    for (index = 0; index < qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list_size; index++)
    {
        ptr = strstr((const char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf, (const char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_name);
        if (ptr != null)
        {
            ptr += strlen((const char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_name);
            offset = ptr - (char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf;
            break;
        }
    }

    // 判断指令是否定义
    if (index >= qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list_size)
    {
        return -1;
    }

    /* 解析查询命令 */
    if ((ptr[0] == '?') && (ptr[1] == '\r') && (ptr[2] == '\n'))
    {
        if (null != qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_query_cb)
        {
            ret = qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_query_cb(argc, (const char **)argv);
        }
    }
    else if ((ptr[0] == '\r') && (ptr[1] == '\n'))
    { /* 解析执行命令 */
        if (null != qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_exe_cb)
        {
            ret = qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_exe_cb(argc, (const char **)argv);
        }
    }
    else if ((ptr[0] == '!') && (ptr[1] == '\r') && (ptr[2] == '\n'))
    {
        /*AT 测试指令*/
        if (null != qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_test_cb)
        {
            ret = qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_test_cb(argc, (const char **)argv);
        }
    }
    else if (ptr[0] == '=')
    { /* 解析设置命令 */
        ptr += 1;
        argc = qyq_frame_at_string_split((char *)ptr, size - offset, ',', argv, argc);
        if (null != qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_setup_cb)
        {
            ret = qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[index].qyq_frame_at_setup_cb(argc, (const char **)argv);
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static int8_t qyq_frame_at_init(struct qyq_frame_at_type *qyq_frame_at)
{
    uint8_t i = 0, j = 0;
    if (qyq_frame_at == null)
    {
        return -1;
    }

    if (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list == null || qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list_size == 0)
    {
        return -1;
    }

    for (i = 0; i < qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list_size; i++)
    {
        if (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_len != (int)strlen((const char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_name))
        {
            return -1;
        }

        for (j = 0; j < qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list_size; j++)
        {
            if (i == j)
            {
                continue;
            }

            if (((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_len == qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_len) &&
                 (strcmp((const char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_name, (const char *)qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_name) == 0)) ||
                ((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_test_cb != NULL) && (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_test_cb == qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_test_cb)) ||
                ((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_query_cb != NULL) && (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_query_cb == qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_query_cb)) ||
                ((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_setup_cb != NULL) && (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_setup_cb == qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_setup_cb)) ||
                ((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_exe_cb != NULL) && (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[j].qyq_frame_at_exe_cb == qyq_frame_at->qyq_frame_at_config->qyq_frame_at_list[i].qyq_frame_at_exe_cb)))
            {
                return -1;
            }
        }
    }

    if ((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf == null) || (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_size == 0))
    {
        return -1;
    }

    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_INITIAL;
    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt = 0;
    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_bench = 0;
    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index = 0;
    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_readready_status = 0;
    return 0;
}

static int8_t qyq_frame_at_recv(struct qyq_frame_at_type *qyq_frame_at, uint8_t dat)
{
    if (qyq_frame_at == null)
    {
        return -1;
    }

    // qyq_frame_at_write(&dat,1);

    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_bench = qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt - 1;

    if (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index >= qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_size)
    {
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_readready_status = 0;
        // 表示数据满了，返回状态信息
        return -2;
    }
    // 更新时间和数据
    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf[qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index++] = dat;
    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_readready_status = 1;

    return 0;
}

static int8_t qyq_frame_at_run(struct qyq_frame_at_type *qyq_frame_at)
{
    int8_t ret = 0;
    if (qyq_frame_at == null)
    {
        return -1;
    }
    switch (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states)
    {
    case QYQ_FRAME_AT_INITIAL:
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_bench = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_readready_status = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_RECEIVE;
        break;
    case QYQ_FRAME_AT_RECEIVE:
        // 判断接收是否就绪
        if (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_readready_status)
        {
            if (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index > 0)
            {
                if ((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt - qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_bench) > QYQ_FRAME_AT_RX_TIMEOUT)
                {
                    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_PARSE;
                }
            }
        }
        break;
    case QYQ_FRAME_AT_ERROR:
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_write("ERROR\r\n", strlen("ERROR\r\n"));
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_END;
        break;
    case QYQ_FRAME_AT_OK:
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_write("OK\r\n", strlen("OK\r\n"));
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_END;
        break;
    case QYQ_FRAME_AT_PARSE:
        ret = qyq_frame_at_parse_at(qyq_frame_at, qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index);
        if (ret == 0)
        {
            qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_OK;
        }
        else
        {
            qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_ERROR;
        }
        break;
    case QYQ_FRAME_AT_END:
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_readready_status = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_states = QYQ_FRAME_AT_RECEIVE;
        break;
    default:
        break;
    }
    return ret;
}

/*
static int8_t qyq_frame_at_recv(struct qyq_frame_at_type *qyq_frame_at, uint8_t dat)
{
    int8_t ret = 0;

    if (qyq_frame_at == null)
    {
        return -1;
    }
    // 判断是否处于接收状态
    if(qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_status == 0)
    {
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_status = 1;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_bench = qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt;
    }
    // 超时和最大BUF长度机制
    if (((qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt - qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_bench) > QYQ_FRAME_AT_RX_TIMEOUT) || (qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index >= qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_size))
    {
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_flag = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_status = 0;

        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_write("ERROR\r\n", strlen("ERROR\r\n"));
        return -1;
    }
    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_bench = qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt;

    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_buf[qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index++] = dat;

    if ((dat == '\n') && qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_flag)
    {
        ret = qyq_frame_at_parse(qyq_frame_at, qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index);
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_status = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_flag = 0;
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_index = 0;
    }
    else if (dat == '\r')
    {
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_flag = 1;
    }
    else
    {
        qyq_frame_at->qyq_frame_at_config->qyq_frame_at_rx_flag = 0;
    }

    return ret;
}
*/
static int8_t qyq_frame_at_tick(struct qyq_frame_at_type *qyq_frame_at)
{
    if (qyq_frame_at == null)
    {
        return -1;
    }

    qyq_frame_at->qyq_frame_at_config->qyq_frame_at_tick_cnt++;

    return 0;
}

int8_t qyq_frame_at_create(qyq_frame_at_type_t *qyq_frame_at, qyq_frame_at_config_t *qyq_frame_at_config)
{
    if (qyq_frame_at == null)
    {
        return -1;
    }
    if (qyq_frame_at_config == null)
    {
        return -1;
    }

    qyq_frame_at->qyq_frame_at_config = qyq_frame_at_config;

    qyq_frame_at->qyq_frame_at_init = qyq_frame_at_init;
    qyq_frame_at->qyq_frame_at_tick = qyq_frame_at_tick;
    qyq_frame_at->qyq_frame_at_run = qyq_frame_at_run;
    qyq_frame_at->qyq_frame_at_recv = qyq_frame_at_recv;

    return 0;
}
