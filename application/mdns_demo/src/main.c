#include "aiio_adapter_include.h"
#include "wifi_connect.h"


void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    int32_t i_ret=start_connect_wifi();
    if(i_ret<0){
        aiio_log_e("wifi start error");
    }
    aiio_log_i("wifi start");
    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

