#ifndef LEDC_INIT_CONFIG_H__
#define LEDC_INIT_CONFIG_H__

// #include "driver/ledc.h"
// #include "driver/gpio.h"
#include "stdbool.h"
#include "stdint.h"
#include "math.h"

#define LEDC_MAX_DUTY          (uint32_t )pow(2,13)

void ledc_init(void);
void ledc_off_all(void);
int ledc_status_update(uint32_t *target_duty);
void ledc_pair_on(void);
void ledc_pair_off(void);
void ledc_pair_wifi_task_default_breathe_start(void);
bool get_ledc_net_status(void);
int ledc_rgb_ctrl(int ch,int rgb_val);
int ledc_duty_ctrl(int ch,int duty_val);
#endif