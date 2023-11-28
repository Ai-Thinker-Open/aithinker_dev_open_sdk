/**
 * @brief   Declare the interface of the software timer
 * 
 * @file    aiio_softtimer.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        Declare some simply software timer interface
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-14          <td>1.0.0            <td>zhuolm             <td> The interface of software timer
 */
#ifndef __AIIO_SOFTTIMER_H_
#define __AIIO_SOFTTIMER_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/* The maximum the number of timers that can be created */
#define  AIIO_TIMER_MAX_NUM               20

/* The Flag of delete all timer*/
#define  AIIO_TIMER_DELETE_ALL            0xffffffff


/**
 * @brief   The type of timer
*/
#define AIIO_SINGLE_TIMER                 (0)
#define AIIO_REPEAT_TIMER                 (1)


/**
 * @brief   The status of timer
*/
#define     AIIO_TIMER_CANCEL             (0)
#define     AIIO_TIMER_EXIST              (1)
#define     AIIO_TIMER_STOP               (2)
#define     AIIO_TIMER_RUNNING            (3)
#define     AIIO_TIMER_TIMEOUT            (4)


typedef  int32_t  AIIO_TIMER;               /*!< Define the type of common timer, it mainly declare the handle of timer*/


/**
 * @brief   Initialize software timer
 * 
 * 
 * @return  true   timer initialize  success
 * @return  false   timer initialize fail
 */
bool aiio_TimerInit(void);


/**
 * @brief   Add new timer
 * 
 * @param[in]   timertype   appoint the type of timer, it is AIIO_SINGLE_TIMER or AIIO_REPEAT_TIMER
 * 
 * @return  AIIO_TIMER      Reture the handle of timer
 */
AIIO_TIMER aiio_TimerAdd(int32_t timertype);

/**
 * @brief   Launch timer
 * 
 * @param[in]   timer_fd    Appoint the handle of timer
 * @param[in]   ms          Appoint the start tick of timer
 * 
 * @return  int32_t         The timer is launched if return value is 0, otherwise isn't launched.
 */
int32_t aiio_TimerStart(AIIO_TIMER timer_fd, uint32_t ms);

/**
 * @brief   Stop timer
 * 
 * @param[in]   timer_fd    Appoint the handle of timer
 * 
 * @return  int32_t         The timer is stopped if return value is 0, otherwise isn't stopped.
 */
int32_t aiio_TimerStop(AIIO_TIMER timer_fd);

/**
 * @brief   Delete timer， when timer is delete, it can't be fined from timer list.
 * 
 * @note     When timer is delete, the handle of timer that should be valued for -1, it will be reture
 *           fail status or can't be fined when it is launched or checked again.
 * 
 * @param[in]   timer_fd    Appoint the handle of timer
 * 
 * @return  int32_t         The timer is deleted if return value is 0, otherwise isn't deleted.
 */
int32_t aiio_TimerDel(AIIO_TIMER timer_fd);


/**
 * @brief   Check the timer timeout
 * 
 * @param[in]   timer_fd    Appoint the handle of timer
 * 
 * @return  int32_t         Reture the status of timer
 */
int32_t aiio_CheckTimerTimeout(AIIO_TIMER timer_fd);



#endif

