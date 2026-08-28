#include "stdio.h"
#include "string.h"
#include "aiio_error.h"
#include "aiio_os_port.h"
#include "aiio_log.h"
#include "pinctrl.h"
#include "soc_osal.h"
#include "common_def.h"

#define AIIO_OS_TIMER_OPT_BLOCK_MS_MAX 10 * 1000

#define AIIO_OS_WAIT_FOREVER (0xFFFFFFFF)
#define AIIO_OS_NO_WAIT (0)
#define AIIO_OS_WAIT_MS2TICK(ms) \
    (((ms) == AIIO_OS_WAIT_FOREVER) ? AIIO_OS_WAIT_FOREVER : (((ms) / portTICK_PERIOD_MS) + ((((ms) % portTICK_PERIOD_MS) + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)))

unsigned int critical_status = 0;


/************************************************************************************* */
// 这一部分用于管理消息队列的内容
// 链表节点定义
typedef struct Node {
    uint32_t queue_id;   // 用于查找
    uint32_t item;       // 实际保存的数据
    struct Node* next;
} Node;

// 创建节点
Node* create_node(uint32_t queue_id, uint32_t item) {
    Node* new_node = (Node*)aiio_os_malloc(sizeof(Node));
    if (!new_node) {
        aiio_log_w("内存分配失败！\n");
        return NULL;
    }
    new_node->queue_id = queue_id;
    new_node->item = item;
    new_node->next = NULL;
    return new_node;
}

// 尾插法插入
void insert_tail(Node** head, uint32_t queue_id, uint32_t item) {
    Node* new_node = create_node(queue_id, item);
    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }
}

// 查找节点：根据 queue_id
Node* search_node(Node* head, uint32_t queue_id) {
    while (head != NULL) {
        if (head->queue_id == queue_id)
            return head;
        head = head->next;
    }
    return NULL;
}

// 删除节点：根据 queue_id 删除
void delete_node(Node** head, uint32_t queue_id) {
    Node* temp = *head;
    Node* prev = NULL;

    while (temp != NULL) {
        if (temp->queue_id == queue_id) {
            if (prev == NULL) {
                *head = temp->next;
            } else {
                prev->next = temp->next;
            }
            aiio_os_free(temp);
            aiio_log_w("节点 queue_id = %d 已删除。\n", queue_id);
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    aiio_log_w("未找到 queue_id = %d 的节点。\n", queue_id);
}

// 打印链表
void print_list(Node* head) {
    while (head != NULL) {
        aiio_log_w("[queue_id: %d, item: %d] -> ", head->queue_id, head->item);
        head = head->next;
    }
    aiio_log_w("NULL\n");
}

// 释放整个链表
void free_list(Node* head) {
    while (head != NULL) {
        Node* temp = head;
        head = head->next;
        aiio_os_free(temp);
    }
}

static Node* head = NULL;
/************************************************************************************* */




int32_t aiio_os_is_inisr(void)
{
    return osal_in_interrupt();
}

int32_t aiio_os_tick_count(uint32_t *tick)
{
    if (tick == NULL)
    {
        return AIIO_ERROR;
    }

    // 这里不修改函数的结构了
    if (aiio_os_is_inisr())
    {
        // 无论是不是在中断中，都要获取滴答计数器的值
        *tick = osal_get_jiffies();
    }
    else
    {
        // 无论是不是在中断中，都要获取滴答计数器的值
        *tick = osal_get_jiffies();
    }

    return AIIO_OK;
}

int32_t aiio_os_thread_self(aiio_os_thread_handle_t *pthread)
{
    // 这个函数是没有适配的，只是能够让程序正常编译过
    pthread = pthread;

    return AIIO_ERROR;
}

int32_t aiio_os_thread_self_name(char *pthread_name)
{
    // 这个函数是没有适配的，只是能够让程序正常编译过
    pthread_name = pthread_name;

    return AIIO_ERROR;
}

int32_t aiio_os_tick_dealy(uint32_t tick)
{
    // 这个函数对应的毫秒级别的延迟
    osal_msleep(tick);

    return AIIO_OK;
}

int32_t aiio_os_mutex_create(aiio_os_mutex_handle_t *phandle)
{
    osal_mutex *mutex = (osal_mutex *)aiio_os_malloc(sizeof(osal_mutex));
    if (mutex == NULL)
    {
        return AIIO_ERROR;
    }

    int ret = osal_mutex_init(mutex);

    if (ret == OSAL_SUCCESS) // 成功
    {
        *phandle = (aiio_os_semaphore_handle_t)mutex;
        return AIIO_OK;
    }
    else
    {
        aiio_os_free(mutex);
        return AIIO_ERROR;
    }
}

int32_t aiio_os_mutex_delete(aiio_os_mutex_handle_t handle)
{
    if (handle == NULL)
    {
        return AIIO_ERROR;
    }

    // 将通用 void* 句柄转换为具体类型
    osal_mutex *mutex = (osal_mutex *)handle;

    // 调用底层销毁函数
    osal_mutex_destroy(mutex);

    // 释放信号量结构体内存
    aiio_os_free(mutex);

    return AIIO_OK;
}

int32_t aiio_os_mutex_put(aiio_os_mutex_handle_t handle)
{
    if (handle == NULL)
    {
        return AIIO_ERROR;
    }

    osal_mutex *mutex = (osal_mutex *)handle;

    // 对于参数的传入还需要优化
    if (aiio_os_is_inisr())
    {
        osal_mutex_unlock(mutex);
    }
    else
    {
        osal_mutex_unlock(mutex);
    }

    return AIIO_OK;
}

int32_t aiio_os_mutex_get(aiio_os_mutex_handle_t handle, uint32_t wait_ms)
{
    if (handle == NULL)
    {
        return AIIO_ERROR;
    }

    osal_mutex *mutex = (osal_mutex *)handle;

    int ret = 0;
    if (aiio_os_is_inisr())
    {
        ret = osal_mutex_lock_timeout(mutex, wait_ms);
    }
    else
    {
        ret = osal_mutex_lock_timeout(mutex, wait_ms);
    }

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_thread_create(aiio_os_thread_handle_t *pthread,
                              const char *name,
                              aiio_os_function_return_t (*function)(void *arg),
                              uint32_t stack_size,
                              void *arg,
                              int priority)
{
    osal_kthread_lock();

    osal_task *task_handle = osal_kthread_create((osal_kthread_handler)function, arg, name, stack_size);
    if (task_handle != NULL)
    {
        osal_kthread_set_priority(task_handle, priority);
        *pthread = (aiio_os_thread_handle_t)task_handle;
        osal_kthread_unlock();
        return AIIO_OK;
    }

    osal_kthread_unlock();
    return AIIO_ERROR;
}

int32_t aiio_os_thread_delete(aiio_os_thread_handle_t thread)
{
    osal_task *task_handle = (osal_task *)thread;

    osal_kthread_destroy(task_handle, 0);

    return AIIO_OK;
}

int32_t aiio_os_thread_start_scheduler(void)
{
    return AIIO_OK;
}

uint32_t aiio_os_ms2tick(uint32_t ms)
{
    // 这个函数传递什么内容就返回什么内容
    return ms;
}

uint32_t aiio_os_tick2ms(uint32_t tick)
{
    // 这个函数传递什么内容就返回什么内容
    return tick;
}

uint32_t aiio_os_get_free_heap_size()
{
    // 这个函数返回值就为0吧，能不能适配，这个要待定
    return 0;
}

uint32_t aiio_os_get_minimum_ever_free_heap_size()
{
    // 这个函数返回值就为0吧，能不能适配，这个要待定
    return 0;
}

int32_t aiio_os_queue_create(aiio_os_queue_handle_t *pqhandle, uint32_t q_len, uint32_t item_size)
{
    if (*pqhandle != NULL)
    {
        return AIIO_ERROR;
    }

    char p_name[] = "axk_bs21";
    unsigned long queue_id = 0;

    // 创建消息队列
    int ret = osal_msg_queue_create(p_name, item_size, &queue_id, 0, q_len);

    if (ret == OSAL_SUCCESS) // 成功
    {
        // 动态申请一个指针大小的内存，用于存储queue_id转换的指针
        unsigned long *handle_ptr = (unsigned long *)aiio_os_malloc(sizeof(unsigned long));
        if (handle_ptr == NULL)
        {
            return AIIO_ERROR; // 申请内存失败
        }

        // 将 queue_id 转换为指针，存储到动态分配的内存中
        *handle_ptr = queue_id;

        // pqhandle 指向这个动态分配的指针
        *pqhandle = (aiio_os_queue_handle_t)handle_ptr;

        insert_tail(&head, queue_id, item_size);

        return AIIO_OK;
    }
    else
    {
        pqhandle = NULL;
        return AIIO_ERROR;
    }
}

// int32_t aiio_os_queue_recv(aiio_os_queue_handle_t qhandle, void *msg, uint32_t wait_ms)
// {
//     if (qhandle == NULL || msg == NULL)
//     {
//         return AIIO_ERROR;
//     }

//     // 提取 queue_id（存储在动态分配的 unsigned long 中）
//     unsigned long *handle_ptr = (unsigned long *)qhandle;
//     unsigned long queue_id = *handle_ptr;

//     uint32_t buff_size = 102400;

//     int ret;

//     if (aiio_os_is_inisr())
//     {
//         ret = osal_msg_queue_read_copy(queue_id, (uint8_t *)msg, &buff_size, wait_ms);
//     }
//     else
//     {
//         ret = osal_msg_queue_read_copy(queue_id, (uint8_t *)msg, &buff_size, wait_ms);
//     }

//     // 返回 OSAL 的结果
//     return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
// }

int32_t aiio_os_queue_delete(aiio_os_queue_handle_t qhandle)
{
    if (qhandle == NULL)
    {
        return AIIO_ERROR;
    }

    // 强制转换为保存了 queue_id 的指针
    unsigned long *handle_ptr = (unsigned long *)qhandle;
    unsigned long queue_id = *handle_ptr;

    delete_node(&head, queue_id);

    // 删除消息队列
    osal_msg_queue_delete(queue_id);

    // 即使删除失败，也要释放内存，避免泄漏
    aiio_os_free(handle_ptr);

    return AIIO_OK;
}

// int32_t aiio_os_queue_send(aiio_os_queue_handle_t qhandle, const void *msg, uint32_t size, uint32_t wait_ms)
// {
//     if (qhandle == NULL || msg == NULL || size == 0)
//     {
//         return AIIO_ERROR;
//     }

//     // 从动态句柄中获取 queue_id
//     unsigned long *handle_ptr = (unsigned long *)qhandle;
//     unsigned long queue_id = *handle_ptr;

//     int ret = 0;

//     if (aiio_os_is_inisr())
//     {
//         ret = osal_msg_queue_write_copy(queue_id, (uint8_t *)msg, size, wait_ms);
//     }
//     else
//     {
//         ret = osal_msg_queue_write_copy(queue_id, (uint8_t *)msg, size, wait_ms);
//     }

//     return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
// }

int32_t aiio_os_queue_recv(aiio_os_queue_handle_t qhandle, void *msg, uint32_t wait_ms)
{
    if (qhandle == NULL || msg == NULL)
    {
        return AIIO_ERROR;
    }

    // 提取 queue_id（存储在动态分配的 unsigned long 中）
    unsigned long *handle_ptr = (unsigned long *)qhandle;
    unsigned long queue_id = *handle_ptr;

    uint32_t buff_size = 0;

    int ret;

    Node* found = search_node(head, queue_id);
    if (!found) {
        return AIIO_ERROR;
    }

    buff_size = found->item;

    if (aiio_os_is_inisr())
    {
        ret = osal_msg_queue_read_copy(queue_id, (uint8_t *)msg, &buff_size, wait_ms);
    }
    else
    {
        ret = osal_msg_queue_read_copy(queue_id, (uint8_t *)msg, &buff_size, wait_ms);
    }

    // 返回 OSAL 的结果
    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_queue_send(aiio_os_queue_handle_t qhandle, const void *msg, uint32_t wait_ms)
{
    if (qhandle == NULL || msg == NULL)
    {
        return AIIO_ERROR;
    }

    // 从动态句柄中获取 queue_id
    unsigned long *handle_ptr = (unsigned long *)qhandle;
    unsigned long queue_id = *handle_ptr;

    int ret = 0;

    Node* found = search_node(head, queue_id);
    if (!found) {
        return AIIO_ERROR;
    }

    if (aiio_os_is_inisr())
    {
        ret = osal_msg_queue_write_copy(queue_id, (uint8_t *)msg, found->item, wait_ms);
    }
    else
    {
        ret = osal_msg_queue_write_copy(queue_id, (uint8_t *)msg, found->item, wait_ms);
    }

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_queue_get_msgs_waiting(aiio_os_queue_handle_t qhandle)
{
    if (qhandle == NULL)
        return -1; // 错误返回值，表示队列无效

    // 取出 queue_id
    unsigned long *handle_ptr = (unsigned long *)qhandle;
    unsigned long queue_id = *handle_ptr;

    return osal_msg_queue_get_msg_num(queue_id);
}

int32_t aiio_os_enter_critical(void)
{
    // 这个进入临界区返回的状态用于释放临界区，如果在使用的时候多次调用进入临界区，那么返回的变量该如何保存（用大buf保存）
    critical_status = osal_irq_lock();

    return AIIO_OK;
}

int32_t aiio_os_exit_critical(void)
{
    // 这个退出临界区就是一个很大的问题
    osal_irq_restore(critical_status);

    return AIIO_OK;
}

void *aiio_os_malloc(size_t size)
{
    return osal_kmalloc(size, 0);
}

void aiio_os_free(void *prv)
{
    if (prv)
    {
        osal_kfree(prv);
    }
}

int32_t aiio_os_semaphore_create_counting(aiio_os_semaphore_handle_t *pmhandle, uint32_t maxcount, uint32_t initcount)
{
    if (initcount > maxcount)
    {
        return AIIO_ERROR;
    }

    osal_semaphore *semp = (osal_semaphore *)aiio_os_malloc(sizeof(osal_semaphore));
    if (semp == NULL)
    {
        return AIIO_ERROR;
    }

    int ret = osal_sem_init(semp, maxcount);

    if (ret == OSAL_SUCCESS) // 成功
    {
        for (uint32_t i = 0; i < initcount; ++i)
        {
            aiio_os_semaphore_get(semp, 0);
        }

        *pmhandle = (aiio_os_semaphore_handle_t)semp;
        return AIIO_OK;
    }
    else
    {
        aiio_os_free(semp);
        return AIIO_ERROR;
    }
}

int32_t aiio_os_semaphore_get(aiio_os_semaphore_handle_t mhandle, uint32_t wait_ms)
{
    if (mhandle == NULL)
    {
        return AIIO_ERROR;
    }

    osal_semaphore *semp = (osal_semaphore *)mhandle;

    int ret = 0;
    if (aiio_os_is_inisr())
    {
        ret = osal_sem_down_timeout(semp, wait_ms);
    }
    else
    {
        ret = osal_sem_down_timeout(semp, wait_ms);
    }

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_semaphore_put(aiio_os_semaphore_handle_t mhandle)
{
    if (mhandle == NULL)
    {
        return AIIO_ERROR;
    }

    osal_semaphore *semp = (osal_semaphore *)mhandle;

    if (aiio_os_is_inisr())
    {
        osal_sem_up(semp);
    }
    else
    {
        osal_sem_up(semp);
    }

    return AIIO_OK;
}

int32_t aiio_os_semaphore_getcount(aiio_os_semaphore_handle_t mhandle)
{
    mhandle = mhandle;
    return 0;
}

int32_t aiio_os_semaphore_delete(aiio_os_semaphore_handle_t mhandle)
{
    if (mhandle == NULL)
    {
        return AIIO_ERROR;
    }

    // 将通用 void* 句柄转换为具体类型
    osal_semaphore *semp = (osal_semaphore *)mhandle;

    // 调用底层销毁函数
    osal_sem_destroy(semp);

    // 释放信号量结构体内存
    aiio_os_free(semp);

    return AIIO_OK;
}

int32_t aiio_os_timer_create(aiio_os_timer_handle_t *ptimer, const char *name, uint32_t ms,
                             void (*call_back)(aiio_os_timer_handle_t), void *context,
                             aiio_os_timer_reload_t reload,
                             aiio_os_timer_activate_t activate)
{
    // 判断是否为单次触发定时器，如果是，则不自动重载，否则自动重载
    // 只能触发单次的定时器
    // int auto_reload = (reload == AIIO_OS_TIMER_ONE_SHOT) ? pdFALSE : pdTRUE;
    unused(name);    // 名字不使用
    unused(context); // 不使用
    unused(reload);  // 不使用

    osal_timer *timer = (osal_timer *)aiio_os_malloc(sizeof(osal_timer));

    timer->timer = NULL;
    timer->data = 0;
    timer->handler = (void (*)(unsigned long))call_back;
    timer->interval = ms; /* 1000ms */

    if (osal_timer_init(timer) != OSAL_SUCCESS)
    {
        return AIIO_ERROR;
    }

    // 如果设置为自动启动定时器，则调用启动函数
    if (activate == AIIO_OS_TIMER_AUTO_ACTIVATE)
    {
        // 启动定时器，超时时间设置为ticks
        if (osal_timer_start(timer) != OSAL_SUCCESS)
        {
            osal_timer_destroy(timer);

            aiio_os_free(timer);

            return AIIO_ERROR;
        }
    }

    *ptimer = (aiio_os_timer_handle_t)timer;

    return AIIO_OK;
}

int32_t aiio_os_timer_activate(aiio_os_timer_handle_t timer)
{
    if (timer == NULL)
    {
        return AIIO_ERROR;
    }

    int ret;

    // 将通用 void* 句柄转换为具体类型
    osal_timer *ptimer = (osal_timer *)timer;

    ret = osal_timer_start(ptimer);

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_timer_change(aiio_os_timer_handle_t timer, uint32_t ms)
{
    if (timer == NULL)
    {
        return AIIO_ERROR;
    }

    int ret;

    // 将通用 void* 句柄转换为具体类型
    osal_timer *ptimer = (osal_timer *)timer;

    ret = osal_timer_mod(ptimer, ms);

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_timer_is_active(aiio_os_timer_handle_t timer)
{
    unused(timer);

    return AIIO_ERROR;
}

void *aiio_os_timer_get_context(aiio_os_timer_handle_t timer)
{
    unused(timer);

    return NULL;
}

int32_t aiio_os_timer_reset(aiio_os_timer_handle_t timer)
{
    if (timer == NULL)
    {
        return AIIO_ERROR;
    }

    int ret;

    // 将通用 void* 句柄转换为具体类型
    osal_timer *ptimer = (osal_timer *)timer;

    ret = osal_timer_mod(ptimer, ptimer->interval);

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_timer_deactivate(aiio_os_timer_handle_t timer)
{
    if (timer == NULL)
    {
        return AIIO_ERROR;
    }

    int ret;

    // 将通用 void* 句柄转换为具体类型
    osal_timer *ptimer = (osal_timer *)timer;

    ret = osal_timer_stop(ptimer);

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_timer_delete(aiio_os_timer_handle_t timer)
{
    if (timer == NULL)
    {
        return AIIO_ERROR;
    }

    // 将通用 void* 句柄转换为具体类型
    osal_timer *ptimer = (osal_timer *)timer;

    // 调用底层销毁函数
    osal_timer_destroy(ptimer);

    // 释放信号量结构体内存
    aiio_os_free(ptimer);

    return AIIO_OK;
}

int32_t aiio_os_event_create(aiio_os_event_handle_t *event)
{
    osal_event *pevent = (osal_event *)aiio_os_malloc(sizeof(osal_event));
    if (pevent == NULL)
    {
        return AIIO_ERROR;
    }

    int ret = osal_event_init(pevent);

    if (ret == OSAL_SUCCESS) // 成功
    {
        *event = (aiio_os_event_handle_t)pevent;
        return AIIO_OK;
    }
    else
    {
        aiio_os_free(pevent);
        return AIIO_ERROR;
    }

    return AIIO_ERROR;
}

int32_t aiio_os_event_set_bit(aiio_os_event_handle_t event, const int32_t bits_to_set)
{
    if (event == NULL)
    {
        return AIIO_ERROR;
    }

    osal_event *pevent = (osal_event *)event;

    int ret = 0;
    if (aiio_os_is_inisr())
    {
        ret = osal_event_write(pevent, bits_to_set);
    }
    else
    {
        ret = osal_event_write(pevent, bits_to_set);
    }

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_event_wait_bits(aiio_os_event_handle_t event, const uint32_t bits_to_wait_for, const uint32_t clear_on_exit, const uint32_t wait_for_all_bit, uint32_t wait_ms)
{
    if (event == NULL)
    {
        return AIIO_ERROR;
    }

    osal_event *pevent = (osal_event *)event;

    int ret = 0;
    unsigned int mode = 0;

    // 设置等待全部或任意
    mode |= wait_for_all_bit ? OSAL_WAITMODE_AND : OSAL_WAITMODE_OR;

    // 设置是否读取后清除
    if (clear_on_exit)
    {
        mode |= OSAL_WAITMODE_CLR;
    }

    if (aiio_os_is_inisr())
    {
        ret = osal_event_read(pevent, bits_to_wait_for, wait_ms, mode);
    }
    else
    {
        ret = osal_event_read(pevent, bits_to_wait_for, wait_ms, mode);
    }

    // return (ret == bits_to_wait_for) ? AIIO_OK : AIIO_ERROR;
    return ret;
}

int32_t aiio_os_event_clear_bits(aiio_os_event_handle_t event, const uint32_t bits_to_clear)
{
    if (event == NULL)
    {
        return AIIO_ERROR;
    }

    osal_event *pevent = (osal_event *)event;

    int ret = 0;
    if (aiio_os_is_inisr())
    {
        ret = osal_event_clear(pevent, bits_to_clear);
    }
    else
    {
        ret = osal_event_clear(pevent, bits_to_clear);
    }

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_event_clear_bits_from_isr(aiio_os_event_handle_t event, const uint32_t bits_to_clear)
{
    if (event == NULL)
    {
        return AIIO_ERROR;
    }

    osal_event *pevent = (osal_event *)event;

    int ret = 0;
    if (aiio_os_is_inisr())
    {
        ret = osal_event_clear(pevent, bits_to_clear);
    }
    else
    {
        ret = osal_event_clear(pevent, bits_to_clear);
    }

    return (ret == OSAL_SUCCESS) ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_event_get_bits(aiio_os_event_handle_t event)
{
    event = event;
    return AIIO_ERROR;
}

int32_t aiio_os_event_get_bits_from_isr(aiio_os_event_handle_t event)
{
    event = event;
    return AIIO_ERROR;
}

int32_t aiio_os_event_delete(aiio_os_event_handle_t event)
{
    if (event == NULL)
    {
        return AIIO_ERROR;
    }

    // 将通用 void* 句柄转换为具体类型
    osal_event *pevent = (osal_event *)event;

    // 调用底层销毁函数
    osal_event_destroy(pevent);

    // 释放信号量结构体内存
    aiio_os_free(pevent);

    return AIIO_OK;
}
