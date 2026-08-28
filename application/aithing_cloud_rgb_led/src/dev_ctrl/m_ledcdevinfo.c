/*
  * AI-THINKER MIT License
  *
  * Copyright (c) 2017 <AI-THINKER SYSTEMS (SHENZHEN) PTE LTD>
  *
  * Permission is hereby granted for use on AI-THINKER SYSTEMS products only, in which case,
  * it is free of charge, to any person obtaining a copy of this software and associated
  * documentation files (the "Software"), to deal in the Software without restriction, including
  * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
  * to do so, subject to the following conditions:
  *
  *
  * The above copyright notice and this permission notice shall be included in all copies or
  * substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  *
  */
// #include "sys_com_def.h"

// #include "m_nvs.h"
// #include "m_modelcheck.h"


// #include "driver/i2c.h"

#include "RGBConverter.h"
#include "m_ledcdevinfo.h"
#include "ledc_config.h"
#include "aiio_adapter_include.h"

#include "math.h"
#include "stdint.h"
#include "stdio.h"

#define TAG "m_ledcdevinfo.h"

static dev_info dev_info_g = {0};

void dev_info_set(dev_info dev_info_t)
{
    int len = sizeof(dev_info);
    //aiio_log_i("dev_info_t.dev_json_info.LightMode:%d\n",dev_info_t.dev_json_info.LightMode);
    
    if (0 != aiio_nvs_set_blob("dev_info", &dev_info_t, len))
    {
        aiio_log_i("aiio_nvs_set_blob fail!!!!!\n");
    }
	aiio_log_i("set dev_info_t");
    aiio_log_i(
             "set dev_info_t %s:%d \n"
             "dev_switch:\t%d\n"
             "dev_c:\t%d\n"
             "dev_w:\t%d\n"
             "dev_r:\t%d\n"
             "dev_g:\t%d\n"
             "dev_b:\t%d\n"
             "LightSwitch:\t%d\n"
             "WorkMode:\t%d\n"
             "HSVColor:\t%d %d %d\n"
             "LightMode:\t%d\n"
             "Brightness:\t%d\n"
             "ColorTemperature:\t%d\n"
             "ColorSpeed:\t%d\n"
             "\r\n",
             __func__, __LINE__, dev_info_t.dev_switch,
             dev_info_t.dev_c, dev_info_t.dev_w, dev_info_t.dev_r,
             dev_info_t.dev_g, dev_info_t.dev_b,
             dev_info_t.dev_json_info.LightSwitch,
             dev_info_t.dev_json_info.WorkMode,
             dev_info_t.dev_json_info.HSVColor.Hue,
             dev_info_t.dev_json_info.HSVColor.Saturation,
             dev_info_t.dev_json_info.HSVColor.Value,
             dev_info_t.dev_json_info.LightMode,
             dev_info_t.dev_json_info.Brightness,
             dev_info_t.dev_json_info.ColorTemperature, dev_info_t.dev_json_info.ColorSpeed);
}

void dev_info_get(dev_info *dev_info_t)
{
    int len = sizeof(dev_info);
    size_t read_len = 0;

    memset(dev_info_t, 0, len); /* 内存清理，增加兼容性 */

    // if(0 != aiio_nvs_get_blob("dev_info", dev_info_t, len, &read_len))
    // {
    //     aiio_log_i("aiio_nvs_get_blob fail!!!!!\n");
    // }
    aiio_nvs_get_blob("dev_info", dev_info_t, len, &read_len);
    aiio_log_i(
             "get dev_info_t %s:%d \n"
             "dev_switch:\t%d\n"
             "dev_c:\t%d\n"
             "dev_w:\t%d\n"
             "dev_r:\t%d\n"
             "dev_g:\t%d\n"
             "dev_b:\t%d\n"
             "LightSwitch:\t%d\n"
             "WorkMode:\t%d\n"
             "HSVColor:\t%d %d %d\n"
             "LightMode:\t%d\n"
             "Brightness:\t%d\n"
             "ColorTemperature:\t%d\n"
             "ColorSpeed:\t%d\n"
             "\r\n",
             __func__, __LINE__,
             dev_info_t->dev_switch,
             dev_info_t->dev_c, dev_info_t->dev_w, dev_info_t->dev_r,
             dev_info_t->dev_g, dev_info_t->dev_b,
             dev_info_t->dev_json_info.LightSwitch,               
             dev_info_t->dev_json_info.WorkMode,
             dev_info_t->dev_json_info.HSVColor.Hue,
             dev_info_t->dev_json_info.HSVColor.Saturation,
             dev_info_t->dev_json_info.HSVColor.Value,
             dev_info_t->dev_json_info.LightMode,
             dev_info_t->dev_json_info.Brightness,
             dev_info_t->dev_json_info.ColorTemperature, dev_info_t->dev_json_info.ColorSpeed);
}

void dev_info_parse(dev_info *dev_info_t)
{
    uint32_t maxDutyCycle = LEDC_MAX_DUTY; //最大占空比
    uint32_t mono_pwm = 0;
    if (dev_info_t->dev_json_info.LightMode == MONO)
    {
        /* 冷暖灯输出总PWM */
        mono_pwm = (uint32_t)((maxDutyCycle / 100.0) * dev_info_t->dev_json_info.Brightness);
        aiio_log_i("mono_pwm=%d",mono_pwm);
        /* 冷灯输出PWM */
        dev_info_t->dev_c = mono_pwm / ((USER_COLOR_TEMPERATURE_MAX - USER_COLOR_TEMPERATURE_MIN) * 1.0) * (dev_info_t->dev_json_info.ColorTemperature - USER_COLOR_TEMPERATURE_MIN);
        /* 暖灯输出PWM */
        if (dev_info_t->dev_json_info.ColorTemperature == USER_COLOR_TEMPERATURE_MAX)
            dev_info_t->dev_w = 0;
        else
            dev_info_t->dev_w = mono_pwm - dev_info_t->dev_c;
        dev_info_t->dev_r = 0;
        dev_info_t->dev_g = 0;
        dev_info_t->dev_b = 0;
        aiio_log_i( "MONO r:%d,_g:%d,_b:%d ,w:%d,c:%d mod:%d\n", dev_info_t->dev_r,
                dev_info_t->dev_g, dev_info_t->dev_b,
                dev_info_t->dev_w, dev_info_t->dev_c,
                dev_info_t->dev_json_info.LightMode);
    }
    else if (dev_info_t->dev_json_info.LightMode == COLOR)
    {
        uint8_t rgb[3];
        /* 判断接收行为时APP控制还是语音控制 */
        if (dev_info_t->dev_voice_ctrl == 0)
        {
            /* HSV颜色模型转RGB颜色模型 */
            HSV2RGB(&rgb[0], &rgb[1], &rgb[2],
                    dev_info_t->dev_json_info.HSVColor.Hue,
                     dev_info_t->dev_json_info.HSVColor.Saturation,
                    dev_info_t->dev_json_info.HSVColor.Value);
            aiio_log_i( "\n---HSV2RGB r:%d,g:%d,b:%d\n", rgb[0], rgb[1], rgb[2]);
        }
        else if (dev_info_t->dev_voice_ctrl == 1)
        {
        /* RGB颜色模型直接赋值 */
            rgb[0] = dev_info_t->dev_json_info.RGBColor.Red;
            rgb[1] = dev_info_t->dev_json_info.RGBColor.Green;
            rgb[2] = dev_info_t->dev_json_info.RGBColor.Blue;
        }
        aiio_log_i( "ctrl:%d**_h:%d,_s:%d,_v%d\n",
                dev_info_t->dev_voice_ctrl,
                rgb[0], rgb[1], rgb[2]);
        /* 转换成占空比值 */
        dev_info_t->dev_r = pow( (20.158 *rgb[0])/ 255.0 ,3);
        dev_info_t->dev_g = pow( (20.158 *rgb[1])/ 255.0 ,3);    
        dev_info_t->dev_b = pow( (20.158 *rgb[2])/ 255.0 ,3);
        dev_info_t->dev_c = 0;
        dev_info_t->dev_w = 0;
        aiio_log_i( "\n---COLOR r:%d,_g:%d,_b:%d\n", dev_info_t->dev_r, dev_info_t->dev_g, dev_info_t->dev_b);
    }
    #if 0
    else if (dev_info_t->dev_json_info.LightMode == SCENE)
    {
        aiio_log_i("\n---WorkMode : %d\n", dev_info_t->dev_json_info.WorkMode);
        switch (dev_info_t->dev_json_info.WorkMode)
        {
            case READING:
                dev_info_t->dev_json_info.Brightness = 60;
                dev_info_t->dev_json_info.ColorTemperature = 4000;
                aiio_log_i("--READING-- : %d,%d\n", dev_info_t->dev_json_info.Brightness, dev_info_t->dev_json_info.ColorTemperature);
                break;
            case MUSIC:
                dev_info_t->dev_json_info.Brightness = 50;
                dev_info_t->dev_json_info.ColorTemperature = 3200;
                aiio_log_i("--MUSIC-- : %d,%d\n", dev_info_t->dev_json_info.Brightness, dev_info_t->dev_json_info.ColorTemperature);
                break;
            case SUNSET:
                dev_info_t->dev_json_info.Brightness = 50;
                dev_info_t->dev_json_info.ColorTemperature = 2500;
                aiio_log_i("--SUNSET-- : %d,%d\n", dev_info_t->dev_json_info.Brightness, dev_info_t->dev_json_info.ColorTemperature);
                break;
            case NIGHT_LIGHT:
                dev_info_t->dev_json_info.Brightness = 30;
                dev_info_t->dev_json_info.ColorTemperature = 2000;
                aiio_log_i("--NIGHT_LIGHT-- : %d,%d\n", dev_info_t->dev_json_info.Brightness, dev_info_t->dev_json_info.ColorTemperature);
                break;
            default:
                dev_info_t->dev_json_info.Brightness = 60;
                dev_info_t->dev_json_info.ColorTemperature = 4000;
                aiio_log_i("--PATTERN-- : %d,%d\n", dev_info_t->dev_json_info.Brightness, dev_info_t->dev_json_info.ColorTemperature);
                break;
        }
        /* 冷暖灯输出总PWM */
        mono_pwm = (uint32_t)((maxDutyCycle / 100.0) * dev_info_t->dev_json_info.Brightness);

        /* 冷灯输出PWM */
        dev_info_t->dev_c = mono_pwm / ((USER_COLOR_TEMPERATURE_MAX - USER_COLOR_TEMPERATURE_MIN) * 1.0) * (dev_info_t->dev_json_info.ColorTemperature - USER_COLOR_TEMPERATURE_MIN);

        /* 暖灯输出PWM */
        dev_info_t->dev_w = mono_pwm - dev_info_t->dev_c;
        dev_info_t->dev_r = 0;
        dev_info_t->dev_g = 0;
        dev_info_t->dev_b = 0;
    }
    #endif /* 取消情境模式 */
    aiio_log_i( "r:%d  g:%d  b:%d  c:%d  w:%d  mod:%d\r\n", dev_info_t->dev_r, dev_info_t->dev_g, dev_info_t->dev_b, dev_info_t->dev_c, dev_info_t->dev_w, dev_info_t->dev_json_info.LightMode);
    return;
}

void light_off(void)
{
    // uint32_t duty_update[5] ={0};
    // ledc_gradient_update_s(duty_update);
    ledc_off_all();
}

uint32_t ledc_gradient_update_s(uint32_t *target_duty)
{
    return ledc_status_update(target_duty);
}

uint32_t ledc_gradient_update_st(dev_info dev_info_t)
{
    
    dev_info_parse(&dev_info_t);
    uint32_t duty_update[5]={0};
    if (dev_info_t.dev_switch == 1)
    {
        if (dev_info_t.dev_json_info.LightMode == MONO)    /* 白光模式 */
        {

            duty_update[3]=dev_info_t.dev_c;
            duty_update[4]=dev_info_t.dev_w;
 
        }
        else if (dev_info_t.dev_json_info.LightMode == COLOR)   /* 彩光模式 */
        {
            duty_update[0]=dev_info_t.dev_r;
            duty_update[1]=dev_info_t.dev_g;
            duty_update[2]=dev_info_t.dev_b;
        }

        for(int i=0;i<5;i++)
        {
            aiio_log_d("duty_update[%d]  %d",i,duty_update[i]);
            duty_update[i] = duty_update[i] * 100 / 8196;
        }
        aiio_log_i( "ledc_gradient_update_st update");

        ledc_gradient_update_s(duty_update);
    }
    else
    {
        light_off();
    }

    //aiio_log_i( "ledc_gradient_update_st default");
    return 0;
}



void ledc_params_reset(dev_info *dev_info_t)
{
    //出厂设置
    if (dev_info_t == NULL)
        return ;
    dev_info_t->dev_json_info.LightMode = MONO; //灯模式  彩灯 or 冷暖灯
    dev_info_t->dev_json_info.LightSwitch = 1;
    dev_info_t->dev_json_info.WorkMode = 0; //工作场景

    dev_info_t->dev_json_info.ColorTemperature = 40;  //色温
    dev_info_t->dev_json_info.Brightness = 40;          //亮度

    dev_info_t->dev_json_info.HSVColor.Hue = 0;          //h
    dev_info_t->dev_json_info.HSVColor.Saturation = 100; //s
    dev_info_t->dev_json_info.HSVColor.Value = 40;       //h

    if (TEMPERATURE_CONNECT_BEFORE == 0 || TEMPERATURE_CONNECT_BEFORE == 1)
    {
        dev_info_t->dev_json_info.LightMode = MONO; //灯模式  彩灯 or 冷暖灯

        if (TEMPERATURE_CONNECT_BEFORE == 0)
            dev_info_t->dev_json_info.ColorTemperature = USER_COLOR_TEMPERATURE_MAX; //冷色光
        if (TEMPERATURE_CONNECT_BEFORE == 1)
            dev_info_t->dev_json_info.ColorTemperature = USER_COLOR_TEMPERATURE_MIN; //暖色光
    }
    else if (TEMPERATURE_CONNECT_BEFORE < 5)
    {
        dev_info_t->dev_json_info.LightMode = COLOR; //灯模式  彩灯 or 冷暖灯

        if (TEMPERATURE_CONNECT_BEFORE == 2)
            dev_info_t->dev_json_info.HSVColor.Hue = 0; //红色光
        if (TEMPERATURE_CONNECT_BEFORE == 3)
            dev_info_t->dev_json_info.HSVColor.Hue = 120; //绿色光
        if (TEMPERATURE_CONNECT_BEFORE == 4)
            dev_info_t->dev_json_info.HSVColor.Hue = 240; //蓝色光
    }
    dev_info_t->dev_json_info.LightSwitch = true;
    dev_info_t->dev_switch = 1;                 // 开灯，必须设置1
}

int ledcDevRestore(void)
{
        ledc_params_reset(&dev_info_g);
        dev_info_set(dev_info_g);
        ledc_gradient_update_st(dev_info_g); 
        return 0;
}

int ledcDevRestart(void)
{
    dev_info_get(&dev_info_g);
    return 0;
}

int start_open_ledc(void)
{
    ledc_gradient_update_st(dev_info_g); 
    return 0;
}




int ledcDevInitialize(void)
{
   vTaskDelay(50);
   updata_dev_info();

    //恢复出厂设置
    // if (0x55 == IsFactoryReset())
    // {
    //     aiio_log_i("brightness = %d\n", dev_info_g.dev_json_info.Brightness);
    //     aiio_log_i("ColorTemperature = %d\n", dev_info_g.dev_json_info.ColorTemperature);
    //     aiio_log_i("Hue = %d\n", dev_info_g.dev_json_info.HSVColor.Hue);
    //     aiio_log_i("Saturation = %d\n", dev_info_g.dev_json_info.HSVColor.Saturation);
    //     aiio_log_i("Value = %d\n", dev_info_g.dev_json_info.HSVColor.Value);
    // }
    // else
    // {
    //     dev_info_get(&dev_info_g);
    // }
    dev_info_set(dev_info_g);
/*
    if (0x55 == IsFactoryReset())
    {
        xTaskCreate(ledc_gradient_task, "ledc_gradienttask", 2048, NULL, 5, &ledc_gradient_task_handle);
    }
    else
    {
        //dev_info_parse(&dev_info_t);
        ledc_gradient_update_st(dev_info_g);
    }
*/
    return 0;
}

void get_dev_now_info(dev_info * dev_info_get)
{
	if(NULL == dev_info_get)
		return ;
	
	memcpy(dev_info_get,&dev_info_g,sizeof(dev_info));

    return ;
}

void set_dev_switch(unsigned int d)
{
    dev_info_g.dev_switch = d;
    aiio_log_i("*dev_switch*|%d|\r\n", dev_info_g.dev_switch);
    return ;
}

void set_dev_Brightness(unsigned int d)
{
    dev_info_g.dev_json_info.Brightness = d;
    dev_info dev_info_t = {0};
    dev_info_get(&dev_info_t);
    if(dev_info_t.dev_json_info.LightMode == MONO)
	    dev_info_g.dev_json_info.LightMode = MONO;             /* 疑问 */
    else if(dev_info_t.dev_json_info.LightMode == COLOR)
        dev_info_g.dev_json_info.LightMode = COLOR;
    aiio_log_i("*Brightness*|%d|\r\n", dev_info_g.dev_json_info.Brightness);
    return ;
}

void set_dev_Hue(unsigned int d)
{
    dev_info_g.dev_json_info.HSVColor.Hue = d;
	dev_info_g.dev_json_info.LightMode = COLOR;
    aiio_log_i("*Hue*|%d|\r\n", dev_info_g.dev_json_info.HSVColor.Hue);
    return ;
}

void set_dev_Saturation(unsigned int d)
{
    dev_info_g.dev_json_info.HSVColor.Saturation = d;
	dev_info_g.dev_json_info.LightMode = COLOR;
    aiio_log_i("*Saturation*|%d|\r\n", dev_info_g.dev_json_info.HSVColor.Saturation);
    return ;
}

void set_dev_Scene(unsigned int d)
{
    dev_info_g.dev_json_info.WorkMode = d;
	dev_info_g.dev_json_info.LightMode = SCENE;
    aiio_log_i("*WorkMode*|%d|\r\n", dev_info_g.dev_json_info.WorkMode);
    return ;
}

void set_dev_Value(unsigned int d)
{
    dev_info_g.dev_json_info.HSVColor.Value = d;
    dev_info_g.dev_json_info.LightMode = COLOR;
    aiio_log_i("*Value*|%d|\r\n", dev_info_g.dev_json_info.HSVColor.Value);
    return ;
}

void set_dev_Temperature(unsigned int d)
{
    dev_info_g.dev_json_info.ColorTemperature = d;
    printf("\n#####2model = %d\n", dev_info_g.dev_json_info.LightMode);
	dev_info_g.dev_json_info.LightMode = MONO;
    aiio_log_i("*ColorTemperature*|%d|\r\n", dev_info_g.dev_json_info.ColorTemperature);
    return ;
}

void set_dev_LightMode(unsigned int d)
{
    dev_info_g.dev_json_info.LightMode = d;
    printf("\n#####3model = %d\n", dev_info_g.dev_json_info.LightMode);
	// dev_info_g.dev_json_info.LightMode = 0;
    aiio_log_i("*LightMode*|%d|\r\n", dev_info_g.dev_json_info.LightMode);
    return ;
}

SemaphoreHandle_t  xSemaphore_light = NULL;
static void updata_dev_info_task(void *arg)
{
    BaseType_t xResult;
    xSemaphore_light = xSemaphoreCreateBinary();
    if(xSemaphore_light == NULL)
    {
        /* 没有创建成功，用户可以在这里加入,创建失败的处理机制 */
        aiio_log_e( "SemaphoreCreate fail");
    }

    if(dev_info_g.dev_json_info.LightMode != 0)
    {
        dev_info_g.dev_json_info.LightMode = MONO;
    }
    if(dev_info_g.dev_json_info.Brightness == 0 )
    {
        dev_info_g.dev_json_info.Brightness = 50;
    }
    if(dev_info_g.dev_json_info.ColorTemperature == 0)
    {
        dev_info_g.dev_json_info.ColorTemperature = 45;
    }
    if(dev_info_g.dev_json_info.HSVColor.Hue == 0)
    {
        dev_info_g.dev_json_info.HSVColor.Hue = 50;
    }
    if(dev_info_g.dev_json_info.HSVColor.Saturation == 0)
    {
        dev_info_g.dev_json_info.HSVColor.Saturation = 50;
    }
    if(dev_info_g.dev_json_info.HSVColor.Value == 0)
    {
        dev_info_g.dev_json_info.HSVColor.Value = 50;
    }

    while(1)
    {
		
		xResult = xSemaphoreTake(xSemaphore_light, portMAX_DELAY);
		if(xResult == pdTRUE)
		{
            aiio_log_i( "updata_dev_info_task running");
            printf("\n#####1model = %d\n", dev_info_g.dev_json_info.LightMode);
            /* 更新闪存缓存 */
            dev_info_set(dev_info_g);
            vTaskDelay(50);
            /* 更新标志置位 */
            ledc_gradient_update_st(dev_info_g);    // 如果成功，返回代码: OPERATION_OK
        }
        else
        {
            aiio_log_e("二值信号量接收失败");
        }
    }
    vTaskDelete(NULL);
}

//更新完整属性
void updata_dev_info(void)
{
    xTaskCreate(updata_dev_info_task, "updata_dev_info_task", 1024*2, NULL, 5, NULL);
}


