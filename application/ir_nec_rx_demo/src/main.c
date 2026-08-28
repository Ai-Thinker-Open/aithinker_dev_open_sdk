/** @brief ir nec rx demo
 *
 *  @file        main.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/08/08          <td>V1.0.0          <td>hongjz          <td>ir nec rx demo fist version
 *  <table>
 *
 */

#include <stdio.h>
#include "aiio_ir.h"
#include "aiio_os_port.h"
#include "aiio_log.h"

#define IR_RX_PIN   (12)

static aiio_ir_rx_cfg_t cfg = {
    .mode = AIIO_IR_RX_NEC,
    .input_inverse = 1,
    .end_threshold = 9000,
    .data_threshold = 3400,
    .deglitch_enable = 0,
};

static void ir_nec_rx_task(void *param)
{
    uint64_t rx_data;
    uint8_t bit_cnt;
    aiio_ir_rx_gpio_init(0, IR_RX_PIN);
    aiio_ir_rx_init(&cfg);

    for (;;)
    {
        aiio_os_tick_dealy(10);

        aiio_err_t ret = aiio_ir_receive(&rx_data, &bit_cnt);
        if (ret == AIIO_OK)
        {
            aiio_log_i("receive bit: %u", bit_cnt);
            aiio_log_i("receive value: 0x%016lx", rx_data);
        }
    }
}

void aiio_main(void *params)
{
    aiio_log_i("IR NEC RX DEMO!");

    aiio_os_thread_create(NULL, "NEC_RX", ir_nec_rx_task, 4096, NULL, 10);

    aiio_os_thread_delete(NULL);
}

