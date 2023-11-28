/**
 * @brief   Define the interface of the software timer
 * 
 * @file    aiio_softtimer.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        define some simply software timer interface
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-14          <td>1.0.0            <td>zhuolm             <td> The interface of software timer
 */
#include "aiio_softtimer.h"
#include "aiio_list.h"
#include "aiio_log.h"
#include "aiio_os_port.h"



typedef struct 
{
    AIIO_TIMER      timer_fd;                   /*!< The handle of software timer */
    bool            is_timer_on;                /*!< The launch status of software time*/
    int32_t         timer_type;                 /*!< The type of timer, the timer is AIIO_SINGLE_TIMER or AIIO_REPEAT_TIMER */
    uint32_t        start_time;                 /*!< The start tick of timer*/
    uint32_t        timeout;                    /*!< The timeout time ofg timer */
    uint32_t        last_diff_time;             /*!< The last different tick of timer */

}aiio_timer_t;


/* timer list */
static aiioList_t TimeList = {0};


static bool aiio_IsTimerTimeout(aiio_timer_t *timer, uint32_t curTick);
static int32_t aiio_CreateNewTimer(aiioList_t *timer_list);
static ListNode_t *aiio_FindTimer(aiioList_t *timer_list, AIIO_TIMER timer_fd);



bool aiio_TimerInit(void)
{
    if (!aiio_ListInit(&TimeList, AIIO_TIMER_MAX_NUM))
    {
        aiio_log_e("timer list init err");
        return false;
    }
    else
    {
        return true;
    }
}


AIIO_TIMER aiio_TimerAdd(int32_t timertype)
{
    aiio_timer_t timer;
    ListNode_t new_node;
    int32_t timer_fd = 0;

    memset((char *)&timer, 0, sizeof(aiio_timer_t));
    do
    {
        timer_fd = aiio_CreateNewTimer(&TimeList);
        if (timer_fd < 0)
        {
            aiio_log_e("timers' num has exceeded the limit");
            break;
        }
        else
        {
            timer.timer_fd = timer_fd;
            timer.timer_type = timertype;

            new_node.size = sizeof(aiio_timer_t);
            new_node.pbase = (uint8_t *)&timer;

            if (!aiio_ListAddNode(&TimeList, &new_node))
            {
                aiio_log_e("add timer to list err");
                break;
            }
            else
            {
                return timer.timer_fd;
            }
        }
    }
    while (0);
    
    return (-1);
}


int32_t aiio_TimerStart(AIIO_TIMER timer_fd, uint32_t ms)
{
    ListNode_t *node = NULL;
    uint32_t curTick = 0;
    aiio_timer_t *timer = NULL;

    aiio_os_tick_count(&curTick);
    if (!curTick)
    {
        aiio_log_e("get time err");
        return -1;
    }
    else
    {
        node = aiio_FindTimer(&TimeList, timer_fd);
        if (NULL != node)
        {
            timer = (aiio_timer_t *)node->pbase;

            if(NULL == timer)
            {
                aiio_log_e("timer is NULL \r\n");
                return -1;
            }
            if (ms <= 0)
            {
                aiio_log_e("timeout set error  \r\n");
                return -1;
            }
            else
            {
                timer->is_timer_on = true;
                timer->start_time = curTick;
                timer->timeout = ms;
                timer->last_diff_time = 0;
            }
            return 0;
        }
        else
        {
            aiio_log_e("timer is NULL \r\n");
            return -1;
        }   
    }
}


int32_t aiio_TimerStop(AIIO_TIMER timer_fd)
{
    ListNode_t *node = NULL;
    aiio_timer_t *timer = NULL;

    node = aiio_FindTimer(&TimeList, timer_fd);
    if (NULL != node)
    {
        timer = (aiio_timer_t *)node->pbase;

        if(NULL == timer)
        {
            aiio_log_e("timer is NULL \r\n");
            return -1;
        }

        timer->is_timer_on = false;
        timer->start_time = 0;
        timer->timeout = 0;
        timer->last_diff_time = 0;
        return 0;
    }
    else
    {
        aiio_log_e("timer is NULL \r\n");
        return -1;
    }   
}


int32_t aiio_TimerDel(AIIO_TIMER timer_fd)
{
    uint16_t i = 0;
    uint16_t node_num = 0;
    ListNode_t *node = NULL;

    if (AIIO_TIMER_MAX_NUM < timer_fd)
    {
        aiio_log_e("timer timer_fd err");
        return -1;
    }
    else if (AIIO_TIMER_DELETE_ALL == timer_fd)
    {
        node_num = aiio_ListGetNodeNum(&TimeList);
        for (i = node_num; i > 0; i--)
        {
            node = aiio_ListGetNodeFromIndex(&TimeList, (i - 1));
            if (!aiio_ListDelNode(&TimeList, node))
            {
                aiio_log_e("delete all timer from list err");
                return -1;
            }
        }
        return 0;
    }
    else
    {
        node = aiio_FindTimer(&TimeList, timer_fd);
        if (NULL != node)
        {
            if (!aiio_ListDelNode(&TimeList, node))
            {
                aiio_log_e("delete timer from list err");
                return -1;
            }
            else
            {
                timer_fd = -1;
                return 0;
            }
        }
        else
        {
            return -1;
        }
    }
}


int32_t aiio_CheckTimerTimeout(AIIO_TIMER timer_fd)
{
    ListNode_t *node = NULL;
    aiio_timer_t *timer = NULL;
    uint32_t curTick = 0;

    if(timer_fd < 0)
    {
        aiio_log_e("unknow timer \r\n");
        return -1;
    }

    aiio_os_tick_count(&curTick);
    if (!curTick)
    {
        aiio_log_e("get time err \r\n");
        curTick = 0;
        
        return -1;
    }
    else
    {
        node = aiio_FindTimer(&TimeList, timer_fd);
        if (NULL != node)
        {
            timer = (aiio_timer_t *)node->pbase;
            if(NULL == timer)
            {
                curTick = 0;
                
                return -1;
            }
            if (timer->is_timer_on)
            {
                if (aiio_IsTimerTimeout(timer, curTick))
                {
                    if (AIIO_SINGLE_TIMER == timer->timer_type)
                    {
                        timer->is_timer_on = false;
                        curTick = 0;
                        
                        return AIIO_TIMER_TIMEOUT;
                    }
                    else if (AIIO_REPEAT_TIMER == timer->timer_type)
                    {
                        timer->start_time = curTick;
                        curTick = 0;
                        
                        return AIIO_TIMER_TIMEOUT;
                    }
                    else
                    {
                        aiio_log_e("unknown timer type");
                        curTick = 0;
                        
                        return -1;
                    }
                }
                else
                {
                    curTick = 0;
                    
                    return AIIO_TIMER_RUNNING;
                }
            }
            else
            {
                curTick = 0;
                
                return AIIO_TIMER_STOP;
            }
        }
        else
        {
            curTick = 0;
            
            return -1;
        }
    }
}




static bool aiio_IsTimerTimeout(aiio_timer_t *timer, uint32_t curTick)
{
    uint32_t diffTime = 0;

    if(!timer)
    {
        aiio_log_e("param err \r\n");
        return false;
    }

    diffTime = curTick >= timer->start_time ? (curTick - timer->start_time) : (0xFFFFFFFF - timer->start_time + curTick);

    if (diffTime < timer->timeout)
    {
        if (timer->last_diff_time > diffTime)  /* timeout */
        {
            aiio_log_i("%ld %ld %ld %ld \r\n", timer->start_time, curTick, timer->timeout, timer->last_diff_time);
            timer->last_diff_time = 0;
            return true;
        }
        else
        {
            timer->last_diff_time = diffTime;
            return false;
        }
    }
    else
    {
        timer->last_diff_time = 0;
        return true;
    }
}


static int32_t aiio_CreateNewTimer(aiioList_t *timer_list)
{
    int32_t timer_fd = 0;
    aiio_timer_t *timer = NULL;
    bool isTimerUsed[AIIO_TIMER_MAX_NUM];
    uint16_t i = 0;
    uint16_t node_num = 0;
    ListNode_t *node = NULL;

    memset((char *)&isTimerUsed, 0, sizeof(isTimerUsed));

    node_num = aiio_ListGetNodeNum(timer_list);
    if (AIIO_TIMER_MAX_NUM <= node_num)
    {
        return (-1);
    }
    else
    {
        for (i = 0; i < node_num; i++)
        {
            node = aiio_ListGetNodeFromIndex(timer_list, i);
            if(!node)
            {
                continue;
            }
            timer = (aiio_timer_t *)node->pbase;
            if(timer)
            {
                isTimerUsed[timer->timer_fd] = true;
            }
        }

        for (timer_fd = 0; timer_fd < AIIO_TIMER_MAX_NUM; timer_fd++)
        {
            if (!isTimerUsed[timer_fd])
            {
                break;
            }
        }

        return timer_fd;
    }
}


static ListNode_t *aiio_FindTimer(aiioList_t *timer_list, AIIO_TIMER timer_fd)
{
    uint16_t i = 0;
    uint16_t node_num = 0;
    ListNode_t *node = NULL;
    aiio_timer_t *timer = NULL;

    node_num = aiio_ListGetNodeNum(timer_list);
    for (i = 0; i < node_num; i++)
    {
        node = aiio_ListGetNodeFromIndex(timer_list, i);
        if(!node)
        {
            continue;
        }
        timer = (aiio_timer_t *)node->pbase;
        if(timer != NULL)
        {
            if (timer->timer_fd == timer_fd)
            {
                return node;
            }
        }
    }
    return NULL;
}


