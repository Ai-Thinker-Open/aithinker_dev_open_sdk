#include "device_init_config.h"
// #include "m_modelcheck.h"
// #include "com_callback.h"
// #include "cjs_R_S_decoder.h"
// #include "action_operation.h"
#include "m_ledcdevinfo.h"
#include "ledc_config.h"

#define FACTORY_RESET  "FactoryReset" //出厂设置

// extern int on_property_get(dev_property_p s);
// extern int on_property_set(dev_property_p s);
// extern void on_action_invoke(dev_action_p o);
// extern uint8_t switch_cou;


void Device_Initialize()
{
   bool config_net=0;
    //注册属性获取
    // sys_com_callback_registered(ID_PROPERTY_GET_CB,(void *)on_property_get);
    //注册属性设置
    // sys_com_callback_registered(ID_PROPERTY_SET_CB,(void *)on_property_set);
    //注册方法
    //sys_com_callback_registered(ID_ACTION_CB,(void *)on_action_invoke);
    
    //TODO:设备驱动初始化
    // ledc_init();
    //ledcDevInitialize();
    

    // if (0x55 == IsFactoryReset())
    // {
    //     //TODO: 设备重置
    //     config_net=1;
    //     ledcDevRestore();
    //     clear_flag_u8(FACTORY_RESET, 0);

    // }
    // else
    // {
    //     //TODO: 设备重启
    //     ledcDevRestart();
    // }    

    //启动配网灯
    // if(config_net==1)
    // {
    //     // ledc_pair_wifi_task_default_breathe_start();
    //     config_net=0;
    // }
    // else
    // {
    //     start_open_ledc();
    // }

    ledcDevRestart();
    updata_dev_info();
}

//设备恢复出厂设置
void Device_Restore()
{
    // clear_flag_u8(FACTORY_RESET, 0X55);
    // esp_restart();
}





