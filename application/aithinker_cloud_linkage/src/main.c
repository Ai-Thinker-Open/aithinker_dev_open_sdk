#include "aiio_adapter_include.h"

#ifndef CONFIG_BSP_LVGL_EN_CONFIG
#error "This example is not supported for current projects"
#endif
#include "touch_conf_user.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_demos.h"

#include "gui_guider.h"
#include "wifi_sta.h"
#include "mem.h"

#include "main.h"

extern struct mem_heap_s g_pmemheap;
extern void bflb_mem_usage(struct mem_heap_s *heap, struct meminfo *info);

user_info_t user_info;

lv_ui guider_ui;

/* lvgl log cb */
void lv_log_print_g_cb(const char *buf)
{
    aiio_log_i("[LVGL] %s", buf);
}

void test_lvgl_function(void *params)
{
    aiio_log_i("lvgl case\r\n");

    /* lvgl init */
    lv_log_register_print_cb(lv_log_print_g_cb);
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    // lv_demo_benchmark();
    // lv_demo_stress();
    // lv_demo_widgets();
    // ui_init();

    setup_ui(&guider_ui);
    events_init(&guider_ui);
    
    lv_task_handler();

    aiio_log_i("lvgl success\r\n");
    while (1)
    {
        lv_task_handler();
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
    }

}
void test_lvgl_func(void)
{
    static aiio_os_thread_handle_t *aiio_lvgl_thread = NULL;

    aiio_os_thread_create(&aiio_lvgl_thread, "test_lvgl", test_lvgl_function, 4096, NULL, 3);
    aiio_log_i("create lvgl task!");
}

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");
    
    user_wifi_sta_mode_init();
    user_sta_wifi_entry();
    // test_lvgl_func();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
