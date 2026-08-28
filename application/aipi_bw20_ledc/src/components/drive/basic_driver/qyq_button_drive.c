#define __QYQ_BUTTON_DRIVE_C_
#include "qyq_button_drive.h" // 包含头文件

static qyq_button_drive_callback_t button_callback = null;
static qyq_button_control_block_t *button_block_list = null;
static uint8_t button_size = 0; // SINGLELED数量

// 双击间隔为0
static uint16_t contimer_list[3];
static uint16_t longtimer_list[3];

static uint8_t statusin(void)
{
    if (GPIO_ReadDataBit(_PB_19) == 0)
    {
        return 1;
    }
    if (GPIO_ReadDataBit(_PB_20) == 0)
    {
        return 2;
    }
    if (GPIO_ReadDataBit(_PB_21) == 0)
    {
        return 3;
    }

    return 0;
}

static qyq_soft_button_control_block_t soft_button_block_list = {
    .button_number = 3,
    .contimer_value = contimer_list,
    .longtimer_value = longtimer_list,
    .statusin = statusin,
};

static void qyq_button_drive_task(void *para)
{
    uint16_t button_value = 0;

    while (1)
    {
        qyq_soft_button_get_buttonvalue(0, &button_value);

        if (button_value != 0)
        {
            button_block_list[(button_value - 1) % button_size].status = (button_value - 1) / button_size + 1;
            if (button_callback != null)
            {
                button_callback((button_value - 1) % button_size, (button_value - 1) / button_size + 1);
            }
        }
        else
        {
            for (uint8_t i = 0; i < button_size; i++)
            {
                button_block_list[i].status = 0;
            }
        }
        vTaskDelay(10);
    }
}

static void qyq_button_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = _PB_19; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            
    GPIO_Init(&GPIO_InitStructure);  

    GPIO_InitStructure.GPIO_Pin = _PB_20; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            
    GPIO_Init(&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = _PB_21; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            
    GPIO_Init(&GPIO_InitStructure);                       
}

/**
 * @brief 初始化按键，配置相关硬件资源
 *
 * @param block_list 按键控制块数组，用于存储按键状态和定时器信息
 * @param number 按键数量
 * @return int8_t 成功返回0，失败返回负值
 */
int8_t qyq_button_drive_init(qyq_button_control_block_t *block_list, uint8_t number)
{
    if (block_list == null)
    {
        return -1;
    }
    for (uint8_t i = 0; i < number; i++)
    {
        block_list[i].status = 0;
        contimer_list[i] = block_list[i].contimer;
        longtimer_list[i] = block_list[i].longtimer;
    }

    button_block_list = block_list;
    button_size = number;

    // 按键引脚初始化
    qyq_button_gpio_init();

    // GPIO 初始化
    qyq_soft_button_init(&soft_button_block_list, 1);

    // 创建一个10ms的任务用于按键的检测
    xTaskCreate(qyq_button_drive_task, (char *)"qyq_button_drive_task", 4096, NULL, 21, NULL);

    return 0;
}

/**
 * @brief 读取所有按键的状态
 *
 * @param button_value_list 存储所有按键值的数组，按键状态将写入此数组
 * @param button_number 按键数量
 * @return int8_t 成功返回0，失败返回负值
 */
int8_t qyq_button_drive_read_all(uint8_t *button_value_list, uint8_t button_number)
{
    if ((button_value_list == null) || (button_block_list == null) || (button_number != button_size))
    {
        return -1;
    }

    for (uint32_t i = 0; i < button_number; i++)
    {
        button_value_list[i] = button_block_list[i].status;
    }

    return 0;
}

/**
 * @brief 读取指定按键的状态
 *
 * @param button_id 指定要读取状态的按键ID
 * @param button_value 按键状态值，0为未按下，1为按下
 * @return int8_t 成功返回0，失败返回负值
 */
int8_t qyq_button_drive_read(uint8_t button_id, uint16_t *button_value)
{
    if ((button_id >= button_size) || (button_block_list == null) || (button_value == null))
    {
        return -1;
    }

    *button_value = button_block_list[button_id].status;

    return 0;
}

/**
 * @brief 注册按键回调函数
 *
 * @param callback 回调函数指针，按键按下或释放时调用
 * @return int8_t 成功返回0，失败返回负值
 * @note 注册的回调函数会在按键状态变化时触发
 */
int8_t qyq_button_drive_register_callback(qyq_button_drive_callback_t callback)
{
    if ((callback == null) || (button_callback != null))
    {
        return -1;
    }

    button_callback = callback;

    return 0;
}

/**
 * @brief 取消按键回调函数注册
 *
 * @return int8_t 成功返回0，失败返回负值
 */
int8_t qyq_button_drive_unregister_callback(void)
{
    if (button_callback == null)
    {
        return -1;
    }

    button_callback = null;

    return 0;
}
