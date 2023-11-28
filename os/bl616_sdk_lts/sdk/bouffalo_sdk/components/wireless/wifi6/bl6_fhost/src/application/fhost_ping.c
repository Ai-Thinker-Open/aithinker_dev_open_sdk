 /**
 ****************************************************************************************
 *
 * @file fhost_ping.c
 *
 * @brief Definition of ping task for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FHOST_PING
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "fhost_ping.h"
#include "net_tg_al.h"
#include "fhost.h"
#include "fhost_ipc.h"
#include "fhost_rtos.h"

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
/// Table of ping streams
struct fhost_ping_stream p_streams[FHOST_PING_MAX_STREAMS];

/*
 * FUNCTIONS
 ****************************************************************************************
 */
/// main function of ping send task
static RTOS_TASK_FCT(ping_send_main)
{
    struct fhost_ping_stream *ping_stream = env;
    ip_addr_t rip;
    u32_t ip = ping_stream->prof.dst_ip;
    int packet_loss = 0;
    int res, cmd_ret = FHOST_IPC_SUCCESS;

    // Wait for semaphore from fhost_ping_start to ensure the flag active is at 1
    rtos_semaphore_wait(ping_stream->ping_semaphore, -1);

    // run the ping command
    res = net_ping_send(ping_stream);

    // Wait for semaphore to lock the statistics print for background execution
    // Background task: semaphore signalled in fhost_ping_stop
    // Foreground task: semaphore signalled in fhost_ping_start
    rtos_semaphore_wait(ping_stream->ping_semaphore, -1);
    ping_stream->active = false;

    if (res >= 0)
    {
        struct fhost_ping_stats* stats;
        int ms, us;

        stats = &ping_stream->stats;
        IP4_ADDR(&rip,  ip & 0xff, (ip >> 8) & 0xff,
                 (ip >> 16) & 0xff, (ip >> 24) & 0xff);
        fhost_print(rtos_get_task_handle(),
                    "--- %s ping statistics ---\n",
                    ipaddr_ntoa(&rip));
        if (stats->tx_frames)
        {
            packet_loss = ((stats->tx_frames - stats->rx_frames) * 100) / stats->tx_frames;
        }

        ms = stats->rt_time / 1000;
        us = stats->rt_time - (ms * 1000);
        fhost_print(rtos_get_task_handle(),
                    "%d packets transmitted, %d received, "
                    "%d%% packet loss, time %d ms\n"
                    "%d bytes transmitted, %d bytes received, rtt avg %d.%03d ms\n",
                    stats->tx_frames, stats->rx_frames,
                    packet_loss, stats->time / 1000, stats->tx_bytes, stats->rx_bytes,
                    ms, us);
        if (res > 0)
        {
            fhost_print(rtos_get_task_handle(), "%d packets skipped\n", res);
        }
    }
    else
    {
        fhost_print(rtos_get_task_handle(), "ping internal error\n");
        TRACE_APP(ERR, "PING: PING_SEND_MAIN ERR net ping send");
        cmd_ret = FHOST_IPC_ERROR;
    }

    rtos_semaphore_delete(ping_stream->ping_semaphore);
    rtos_task_delete(NULL);
}

/**
 ****************************************************************************************
 * @brief Search the stream id of a free ping stream
 * It will return the first free one in the table.
 *
 * @return ping stream id, -1 if error
 ****************************************************************************************
 **/
static int fhost_ping_find_free_stream_id(void)
{
    int stream_id;
    for (stream_id = 0; stream_id < FHOST_PING_MAX_STREAMS; stream_id++)
    {
        if (p_streams[stream_id].active == false)
        {
            return stream_id;
        }
    }
    return -1;
}

struct fhost_ping_stream *fhost_ping_find_stream_profile(uint32_t stream_id)
{
    if (stream_id < FHOST_PING_MAX_STREAMS)
    {
        return &p_streams[stream_id];
    }
    else
    {
        return NULL;
    }
}

rtos_task_handle fhost_ping_start(void *args)
{
    struct fhost_ping_task_args *ping_args = (struct fhost_ping_task_args *) args;
    struct fhost_ping_stream *ping_stream = NULL;
    int stream_id = fhost_ping_find_free_stream_id();
    char fmt[] = "Send ping ID %d \n";

    if (stream_id == -1)
    {
        fhost_print(RTOS_TASK_NULL, "Couldn't find free stream");
        return RTOS_TASK_NULL;
    }

    ping_stream = fhost_ping_find_stream_profile(stream_id);

    memset(ping_stream, 0, sizeof(struct fhost_ping_stream));

    ping_stream->id = stream_id;
    ping_stream->prof.dst_ip = ping_args->rip;
    ping_stream->prof.rate = ping_args->rate;
    ping_stream->prof.pksize = ping_args->pksize;
    ping_stream->prof.duration = ping_args->duration;
    ping_stream->prof.tos = ping_args->tos;
    ping_stream->background = ping_args->background;

    if (rtos_semaphore_create(&ping_stream->ping_semaphore, 2, 0))
        return RTOS_TASK_NULL;

    if (rtos_task_create(ping_send_main, "PING_SEND", PING_SEND_TASK,
                         FHOST_PING_SEND_STACK_SIZE, ping_stream, FHOST_PING_SEND_PRIORITY,
                         &ping_stream->ping_handle))
    {
        rtos_semaphore_delete(ping_stream->ping_semaphore);
        return RTOS_TASK_NULL;
    }
    ping_stream->active = true;
    // Signal the semaphore to ping_send_main to ensure the flag active is at 1
    rtos_semaphore_signal(ping_stream->ping_semaphore, false);
    if (!ping_stream->background)
    {
        // Signal the ping semaphore to print ping statistics
        rtos_semaphore_signal(ping_stream->ping_semaphore, false);
    }

    fhost_print(RTOS_TASK_NULL, fmt, ping_stream->id);
    return ping_stream->ping_handle;
}

void fhost_ping_stop(struct fhost_ping_stream *ping_stream)
{
    // Cannot stop a stream which is inactive
    if (!ping_stream->active)
    {
        fhost_print(RTOS_TASK_NULL,
                    "Cannot stop the stream %d because it is inactive", ping_stream->id);
       return;
    }

    fhost_print(RTOS_TASK_NULL, "Stop ping ID : %d\n", ping_stream->id);

    if (ping_stream->background)
    {
        // Signal the ping semaphore to print ping statistics
        rtos_semaphore_signal(ping_stream->ping_semaphore, false);
    }
    ping_stream->active = false;
}

int fhost_ping_sigkill_handler(rtos_task_handle ping_handle)
{
    int stream_id, ret;
    struct fhost_ping_stream *ping_stream = NULL;

    // Search ping stream
    for (stream_id = 0; stream_id < FHOST_PING_MAX_STREAMS; stream_id++)
    {
        if (p_streams[stream_id].ping_handle == ping_handle)
        {
            ping_stream = &p_streams[stream_id];
            break;
        }
    }

    if (stream_id == FHOST_PING_MAX_STREAMS)
    {
        fhost_print(ping_handle, "Stream id %d not valid\n", stream_id);
        return FHOST_IPC_ERROR;
    }

    if (ping_stream->background)
        ret = FHOST_IPC_NO_RESP;
    else
        ret = FHOST_IPC_SUCCESS;

    fhost_ping_stop(ping_stream);

    return ret;
}

/// @}
