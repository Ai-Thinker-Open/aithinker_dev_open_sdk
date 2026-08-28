
// #include "esp_log.h"
// #include "sys_com_def.h"

// #include "m_modelcheck.h"

#include "device_init_config.h"
#include "ledc_config.h"
#include "m_ledcdevinfo.h"

#define TAG "ledc_config"

#include "aiio_cloud_led.h"
#include "stdint.h"

/**
 * @ 配网时灯颜色
 * 4:白光
 * 3:暖光
 * 1:红光
 * 0:绿光
 * 2:蓝光
 */
#define WIFI_CONNECT_BEFORE (3) //初始的灯光选择



#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE

#define LEDC_TEST_CH_NUM       (5)



#define GREEN_IO    12        
#define RED_IO      4
#define BLUE_IO     14

#define CW_IO       13
#define WW_IO       5


// #define GREEN_IO   2//12         
// #define RED_IO     4//4
// #define BLUE_IO    5//14

#define MAX_CHANGE_TIME 1000  //ms
#define MIN_CHANGE_TIME 20   

static int sg_gpio_group[LEDC_TEST_CH_NUM]={GREEN_IO,RED_IO,BLUE_IO,WW_IO,CW_IO};
// ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM];


// void ledc_init(void)
// {
//     ledc_timer_config_t ledc_timer = {
//         .duty_resolution = LEDC_MAX_DUTY_BIT, // resolution of PWM duty
//         .freq_hz = 5000,                      // frequency of PWM signal
//         .speed_mode = LEDC_HS_MODE,           // timer mode
//         .timer_num = LEDC_HS_TIMER            // timer index
//     };

//     ledc_timer_config(&ledc_timer);

//     for(int ch=0;ch<LEDC_TEST_CH_NUM;ch++)
//     {
//         ledc_channel[ch].channel=ch;
//         ledc_channel[ch].duty       = 0;
//         ledc_channel[ch].gpio_num   = sg_gpio_group[ch],
//         ledc_channel[ch].speed_mode=LEDC_HS_MODE;
//         ledc_channel[ch].timer_sel=LEDC_HS_TIMER;
//         ledc_channel_config(&ledc_channel[ch]);
//     }

//     ledc_fade_func_install(0);

//     ESP_LOGI(TAG,"LED init succesfull");

//     ledc_off_all();
// }


void ledc_off_all(void)
{
    for(int ch=0;ch<LEDC_TEST_CH_NUM;ch++)
    {   
        aiio_cloud_led_pwm_set(ch, 0);
    }  
    // aiio_red_led_status(0);
    // aiio_green_led_status(0);
    // aiio_blue_led_status(0);
    // aiio_cloud_led_all_off();
}

int ledc_duty_ctrl(int ch,int duty8192_val)
{
    if(ch>=0 &&ch<LEDC_TEST_CH_NUM)
    {
        aiio_cloud_led_pwm_set(ch, duty8192_val);
        return 0;
    }
        return -1;
}

int ledc_rgb_ctrl(int ch,int rgb_val)
{
    int duty= (float)rgb_val/255 *8192;
    if(ch>=0 &&ch<LEDC_TEST_CH_NUM)
    {
        aiio_cloud_led_pwm_set(ch, duty);
        return 0;
    }
        return -1;
}


int ledc_status_update(uint32_t *target_duty)
{
    static int last_target_duty[5]={0};
    for (int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) 
    {

        #if 0
        int tmp_change_time=abs( last_target_duty[ch]- target_duty[ch] );
        if(tmp_change_time<MIN_CHANGE_TIME)
            tmp_change_time=MIN_CHANGE_TIME;

        ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, tmp_change_time);
        ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);

        #else


        int tmp_change_time=abs( last_target_duty[ch]- target_duty[ch] )/8;
        // aiio_log_d("(%d)tmp_change_time %d",ch,tmp_change_time);
        if(tmp_change_time<MIN_CHANGE_TIME)
            tmp_change_time=MIN_CHANGE_TIME;
        last_target_duty[ch]=target_duty[ch];
        // ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
        //         ledc_channel[ch].channel, target_duty[ch], tmp_change_time);
        // ledc_fade_start(ledc_channel[ch].speed_mode,
        //         ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);

        aiio_cloud_led_pwm_set(ch, target_duty[ch]);

        #endif
    }

    return 0;
}


static bool ledc_net_task_run=0;

bool get_ledc_net_status(void)
{
    return ledc_net_task_run;
}

// static void ledc_breath_start(void *arg)
// {
    
//     int timems=1000;
//     int sum_time=120*timems/1000;
//     int sum_count=0;
//     ledc_net_task_run=1;
//     while (1)
//     {
//         ledc_set_fade_with_time(ledc_channel[WIFI_CONNECT_BEFORE].speed_mode,
//                     ledc_channel[WIFI_CONNECT_BEFORE].channel, 3072, timems/2);
//         ledc_fade_start(ledc_channel[WIFI_CONNECT_BEFORE].speed_mode,
//                     ledc_channel[WIFI_CONNECT_BEFORE].channel, LEDC_FADE_NO_WAIT);

//         vTaskDelay(timems/2 / portTICK_PERIOD_MS);

//         ledc_set_fade_with_time(ledc_channel[WIFI_CONNECT_BEFORE].speed_mode,
//                     ledc_channel[WIFI_CONNECT_BEFORE].channel, 0, timems/2);
//         ledc_fade_start(ledc_channel[WIFI_CONNECT_BEFORE].speed_mode,
//                     ledc_channel[WIFI_CONNECT_BEFORE].channel, LEDC_FADE_NO_WAIT);

//         vTaskDelay(timems/2 / portTICK_PERIOD_MS);

//         if(++sum_count>sum_time)
//         {
 
//             ledcDevRestore();
//             break;
//         }

//     }
//     ledc_net_task_run=0;
//     vTaskDelete(NULL);
// }


// TaskHandle_t ledc_gradient_task_handle = NULL;

// void ledc_pair_wifi_task_default_breathe_start(void)
// {
//     xTaskCreate(ledc_breath_start, "ledc_gradienttask", 2048, NULL, 6, &ledc_gradient_task_handle);
// }





