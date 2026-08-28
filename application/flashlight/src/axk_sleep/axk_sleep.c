#include "axk_sleep.h"
#include "axk_at_basic.h"
#include "axk_at.h"
#include "axk_wifi_init.h"
#include "axk_nvs_info.h"

#include "aiio_log.h"

static int32_t axk_auto_sleep_set_info(const axk_sleep_info_t *info);

aiio_os_timer_handle_t sleep_timer = NULL;

static axk_sleep_info_t g_axk_sleep_info = {
    .enable = 1,
};

axk_sleep_t g_axk_sleep = {
    .set_info = axk_auto_sleep_set_info,
};

static int32_t axk_auto_sleep_set_info(const axk_sleep_info_t *info)
{
    int32_t ret = -1;

    if (info == NULL)
    {
        goto __exit;
    }

    memcpy(&g_axk_sleep_info, info, sizeof(axk_sleep_info_t));

    aiio_log_i("enable auto sleep: %u", g_axk_sleep_info.enable);

    ret = 0;

__exit:
    return ret;
}

int at_setup_auto_sleep_cmd(uint32_t argc, const char **argv)
{
    int ret = -1;

    if (argc > 2)
    {
        aiio_log_e("argc err(%d)", argc);
        goto __exit;
    }

    int ctrl = atoi(argv[0]);
    if (ctrl != 0 && ctrl != 1)
    {
        aiio_log_e("argv[0] err(%d)", ctrl);
        goto __exit;
    }

    if (ctrl != g_axk_sleep_info.enable)
    {
        if (ctrl == 0)
        {
            axk_sleep_timer_stop();
            g_axk_sleep_info.enable = ctrl;
        }
        else
        {
            g_axk_sleep_info.enable = ctrl;

            if (g_axk_wifi.got_ip == 1)
            {
                axk_sleep_timer_reset();
            }
        }

        //保存flash
        g_nvs_info.set_auto_sleep_info(g_axk_sleep_info);
    }

    AT_RESPONSE_OK;
    ret = 0;

__exit:
    return ret;
}

static void sleep_timer_cb(aiio_os_timer_handle_t param)
{
    axk_enter_sleep();
}

int32_t axk_auto_sleep_timer_init(void)
{
    return aiio_os_timer_create(&sleep_timer, "sleep_timer", 30000,
                                sleep_timer_cb, NULL,
                                AIIO_OS_TIMER_PERIODIC,
                                AIIO_OS_TIMER_NO_ACTIVATE);
}

int32_t axk_sleep_timer_reset(void)
{
    if (g_axk_sleep_info.enable == 0)
    {
        return 0;
    }

    if (sleep_timer != NULL)
    {
        aiio_log_i("sleep timer reset!");
        return aiio_os_timer_reset(sleep_timer);
    }

    return -1;
}

int32_t axk_sleep_timer_stop(void)
{
    if (g_axk_sleep_info.enable == 0)
    {
        return 0;
    }

    if (sleep_timer != NULL)
    {
        aiio_log_i("sleep timer stop!");
        return aiio_os_timer_deactivate(sleep_timer);
    }

    return -1;
}

int32_t axk_sleep_timer_change(uint32_t ms)
{
    if (g_axk_sleep_info.enable == 0)
    {
        return 0;
    }

    if (sleep_timer != NULL)
    {
        aiio_log_i("sleep timer change: %ld", ms);
        return aiio_os_timer_change(sleep_timer, ms);
    }

    return -1;
}

int32_t axk_sleep_timer_is_actic(void)
{
    return aiio_os_timer_is_active(sleep_timer);
}
