#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"


void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    aiio_assert(1);
    aiio_assert(0);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

