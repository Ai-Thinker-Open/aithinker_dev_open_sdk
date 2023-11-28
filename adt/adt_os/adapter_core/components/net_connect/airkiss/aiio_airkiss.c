/** @brief      airkiss application interface.
 *
 *  @file       aiio_airkiss.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       airkiss application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>airkiss Init
 *  <tr><td>2023/05/18      <td>1.0.1       <td>wusen       <td>Fixed the WIFI distribution event conflict bug && Fixed the airkiss stop bug 
 *  </table>
 *
 */
#include "aiio_airkiss.h"

int32_t aiio_airkiss_register_cb(aiio_airkiss_callback_t cb)
{
    return AIIO_OK;
}

void aiio_airkiss_wifi_event(int32_t evt_code, void *param)
{

}

int32_t aiio_airkiss_start(void)
{
    return AIIO_OK;
}

int32_t aiio_airkiss_stop(void)
{
    return AIIO_OK;
}
