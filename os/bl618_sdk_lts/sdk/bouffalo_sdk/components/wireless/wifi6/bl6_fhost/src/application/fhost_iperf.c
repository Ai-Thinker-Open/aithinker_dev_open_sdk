/**
 ****************************************************************************************
 *
 * @file fhost_iperf.c
 *
 * @brief Definition of IPERF for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FHOST_IPERF
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "fhost_iperf.h"
#include "fhost.h"
#include "net_iperf_al.h"
#include "fhost_ipc.h"
#include "wifi_mgmr_ext.h"

#if NX_IPERF
/*
 * DEFINITIONS
 ****************************************************************************************
 */
/// Default Port
#define IPERF_DEFAULT_PORT  5001
/// Default test duration (in 0.1 seconds)
#define IPERF_DEFAULT_TIME_AMOUNT   100
/// Default buffer length (in bytes)
#define IPERF_DEFAULT_BUFFER_LEN    8 * 1024

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
static rtos_task_handle iperf_handle = NULL;

/// Unit of measurement of data
enum data_type{
    CONV_UNIT,
    CONV_KILO,
    CONV_MEGA,
    CONV_GIGA
};

/// Unit of measurements for bytes
const char* net_iperf_byte_lbl[] =
{
    [CONV_UNIT] = "Byte",
    [CONV_KILO] = "KByte",
    [CONV_MEGA] = "MByte",
    [CONV_GIGA] = "GByte",
};

/// Unit of measurements for bit
const char* net_iperf_bit_lbl[] =
{
    [CONV_UNIT] = "bit",
    [CONV_KILO] = "Kbits",
    [CONV_MEGA] = "Mbits",
    [CONV_GIGA] = "Gbits",
};

/// Long version of iperf helf
const char iperf_long_help[] = "\
Client/Server:\r\n\
  -i, --interval  #        seconds between periodic bandwidth reports\r\n\
  -l, --len       #[KM]    length of buffer to read or write (default 8 KB)\r\n\
  -p, --port      #        server port to listen on/connect to\r\n\
  -u, --udp                use UDP rather than TCP\r\n\
  -I              #        vif number, 0 for sta, 1 for ap, default is 0\r\n\r\n\
Client specific:\r\n\
  -b, --bandwidth #[KM]    for UDP, bandwidth to send at in bits/sec\r\n\
                         (default 1 Mbit/sec, implies -u)\r\n\
  -c, --client    <host>   run in client mode, connecting to <host>\r\n\
  -t, --time      #        time in seconds to transmit for (default 10 secs)\r\n\
  -S              #        type-of-service for outgoing packets.\r\n\
                           You may specify the value in hex with a '0x' prefix, in octal with a '0' prefix, or in decimal\r\n\
  -T              #        time-to-live for outgoing multicast packets\r\n\
  -w              #[KM]    TCP window size (ignored for now)\r\n\
Server specific:\r\n\
  -s, --server             run in server mode";

/// Table of iperf streams
struct fhost_iperf_stream streams[FHOST_IPERF_MAX_STREAMS];

/*
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief IPERF task implementation.
 ****************************************************************************************
 */
static RTOS_TASK_FCT(fhost_iperf_main)
{
    struct fhost_iperf_stream *iperf_stream = env;
    struct fhost_iperf_settings *iperf_settings = &(iperf_stream->iperf_settings);
    iperf_stream->active = true;

    int rssi = 0;
    wifi_mgmr_sta_rssi_get(&rssi);
    fhost_print(RTOS_TASK_NULL, "rssi: %ddB\r\n", rssi);

    if (!iperf_settings->flags.is_server)
    {
        if (iperf_settings->flags.is_udp)
        {
            // UDP Client
            if (net_iperf_udp_client_run(iperf_stream))
            {
                TRACE_APP(ERR, "IPERF: Failed to start UDP client");
                goto end;
            }
        }
        else
        {
            // TCP Client
            if (net_iperf_tcp_client_run(iperf_stream))
            {
                TRACE_APP(ERR, "IPERF: Failed to start TCP client");
                goto end;
            }
        }
    }
    else
    {
        if (iperf_settings->flags.is_udp)
        {
            // UDP Server
            if (net_iperf_udp_server_run(iperf_stream))
            {
                TRACE_APP(ERR, "IPERF : Failed to start UDP server");
                goto end;
            }
        }
        else
        {
            // TCP Server
            if (net_iperf_tcp_server_run(iperf_stream))
            {
                TRACE_APP(ERR, "IPERF: Failed to start TCP server");
                goto end;
            }
        }
    }


end:
    // Delete objects
    rtos_semaphore_delete(iperf_stream->to_semaphore);
    rtos_semaphore_delete(iperf_stream->iperf_task_semaphore);
    rtos_semaphore_delete(iperf_stream->send_buf_semaphore);
    rtos_mutex_delete(iperf_stream->iperf_mutex);
    iperf_stream->active = false;
    iperf_handle = NULL;
    iperf_stream->iperf_handle = NULL;
    rtos_task_delete(NULL);
}


/**
 ****************************************************************************************
 * @brief Search the stream id of a free iperf stream
 * Return the first free stream id in the table.
 *
 * @return iperf stream id, -1 if error
 ****************************************************************************************
 **/
static int fhost_iperf_find_free_stream_id(void)
{
    int stream_id;
    for (stream_id = 0; stream_id < FHOST_IPERF_MAX_STREAMS; stream_id++)
    {
        if (streams[stream_id].active == false)
        {
            return stream_id;
        }
    }
    return -1;
}

/**
 ****************************************************************************************
 * @brief Convert a float value into string according to the specified format
 *
 * For example:
 * - amount = 12000 format = 'K' => out_str = "11.72 KByte"
 * - amount = 12000 format = 'k' => out_str = "96 Kbits"
 *
 * @param[out] out_str              Pointer to the string buffer to write
 * @param[in] len                   Length of out_str buffer
 * @param[in] amount                Float value to convert in string based on the format
 * @param[in] format                Format to use for converting amount to string
 ****************************************************************************************
 **/
static void fhost_iperf_snprintf(char *out_str, uint32_t len, float amount, char format)
{
    uint8_t conv = CONV_UNIT;
    const char* suffix;
    int int_amount, dec_amount;
    bool is_bit_value = false;
    float round;

    switch ( format ) {
        case 'b':
            is_bit_value = true;
        case 'B':
            conv = CONV_UNIT;
            break;
        case 'k':
            is_bit_value = true;
        case 'K':
            conv = CONV_KILO;
            break;
        case 'm':
            is_bit_value = true;
        case 'M':
            conv = CONV_MEGA;
            break;
        case 'g':
            is_bit_value = true;
        case 'G':
            conv = CONV_GIGA;
            break;

        default:
        case 'a':
            is_bit_value = true;
            for (float tmp_amount = amount * 8;
                 tmp_amount >= 1000.0 && conv <= CONV_GIGA;
                 conv++)
            {
                tmp_amount /= 1000.0;
            }
            break;
        case 'A':
            for (float tmp_amount = amount;
                 tmp_amount >= 1024.0 && conv <= CONV_GIGA;
                 conv++)
            {
                tmp_amount /= 1024.0;
            }
            break;
    }

    if ( is_bit_value )
    {
        amount *= 8;
        suffix = net_iperf_bit_lbl[conv];
        while (conv-- > 0)
            amount /= 1000;
    }
    else
    {
        suffix = net_iperf_byte_lbl[conv];
        while (conv-- > 0)
            amount /= 1024;
    }

    int_amount = (int32_t) amount;
    if (amount < (float)9.995)
    {
        round = (float)0.005;
        amount += round;
        int_amount = (int32_t)amount;
        dec_amount = (amount * 100) - (int_amount * 100);
        dbg_snprintf((char *)out_str, len, "%d.%02d %s", int_amount, dec_amount, suffix);
    }
    else if (amount < (float)99.95)
    {
        round = (float)0.05;
        amount += round;
        int_amount = (int32_t)amount;
        dec_amount = (amount * 10) - (int_amount * 10);
        dbg_snprintf((char *)out_str, len, "%d.%d %s", int_amount, dec_amount, suffix);
    }
    else
    {
        round = (float)0.5;
        amount += round;
        int_amount = (int32_t)amount;
        dbg_snprintf((char *)out_str, len, "%4d %s", int_amount, suffix);
    }
}

void fhost_iperf_settings_init(struct fhost_iperf_settings *iperf_settings)
{
    memset(iperf_settings, 0, sizeof(*iperf_settings));

    // option, defaults
    iperf_settings->flags.is_time_mode  = true;
    iperf_settings->flags.is_server     = 0; // -s or -c
    iperf_settings->buf_len             = IPERF_DEFAULT_BUFFER_LEN; // -l
    iperf_settings->port                = IPERF_DEFAULT_PORT;     // -p,  ttcp port
    iperf_settings->amount              = IPERF_DEFAULT_TIME_AMOUNT; // -t
    iperf_settings->format              = 'a';
    iperf_settings->ttl                 = FHOST_IPERF_TTL;
    iperf_settings->vif_num             = MGMR_VIF_STA;
}

rtos_task_handle fhost_iperf_start(void *args)
{
    struct fhost_iperf_stream *iperf_stream = NULL;
    struct fhost_iperf_settings *iperf_settings = (struct fhost_iperf_settings *) args;
    int stream_id = fhost_iperf_find_free_stream_id();

    if (stream_id == -1)
    {
        fhost_print(RTOS_TASK_NULL, "Couldn't find free stream");
        return RTOS_TASK_NULL;
    }
    iperf_stream = &streams[stream_id];

    memset(iperf_stream, 0, sizeof(struct fhost_iperf_stream));

    iperf_stream->id = stream_id;
    iperf_stream->iperf_settings = *iperf_settings;

    if (rtos_semaphore_create(&iperf_stream->iperf_task_semaphore, 1, 0))
        goto end;

    if (rtos_semaphore_create(&iperf_stream->send_buf_semaphore, FHOST_IPERF_SEND_BUF_CNT, FHOST_IPERF_SEND_BUF_CNT))
        goto err_sem_net;

    if (rtos_semaphore_create(&iperf_stream->to_semaphore, 1, 0))
        goto err_sem_to;

    if (rtos_mutex_create(&iperf_stream->iperf_mutex))
        goto err_mutex;

    if (rtos_task_create(fhost_iperf_main, "IPERF", IPERF_TASK, FHOST_IPERF_STACK_SIZE,
                         iperf_stream, FHOST_IPERF_PRIORITY, &iperf_stream->iperf_handle))
        goto err_task_create;

    iperf_handle = iperf_stream->iperf_handle;
    return iperf_stream->iperf_handle;

    err_task_create:
    rtos_mutex_delete(iperf_stream->iperf_mutex);
    err_mutex:
    rtos_semaphore_delete(iperf_stream->to_semaphore);
    err_sem_to:
    rtos_semaphore_delete(iperf_stream->send_buf_semaphore);
    err_sem_net:
    rtos_semaphore_delete(iperf_stream->iperf_task_semaphore);
    end:
    return RTOS_TASK_NULL;
}

void fhost_iperf_init_stats(struct fhost_iperf_stream *stream)
{
    struct iperf_report *report = &stream->report;
    struct fhost_iperf_settings *settings = &stream->iperf_settings;

    memset(&report->stats, 0, sizeof(report->stats));
    memset(&report->last_stats, 0, sizeof(report->last_stats));

    iperf_current_time(&report->start_time);
    if (settings->flags.show_int_stats)
    {
        iperf_timeradd(&report->start_time, &settings->interval, &report->interval_target);
        report->last_interval = report->start_time;
    }

    if (!settings->flags.is_server && settings->flags.is_time_mode)
    {
        struct iperf_time duration;
        duration.sec = (uint32_t) settings->amount / 10;
        duration.usec = (uint32_t) 100000 * (settings->amount - (duration.sec * 10));
        iperf_timeradd(&report->start_time, &duration, &report->end_time);
    }
}

void fhost_iperf_print_interv_stats(struct fhost_iperf_stream *stream)
{
    struct fhost_iperf_settings *settings = &stream->iperf_settings;
    struct iperf_report *report = &stream->report;
    struct iperf_stats interval_stats;

    if (!settings->flags.show_int_stats ||
        iperf_timerafter(&report->interval_target, &report->packet_time))
        return;

    interval_stats.bytes = report->stats.bytes - report->last_stats.bytes;
    if (settings->flags.is_udp && settings->flags.is_server)
    {
        interval_stats.nb_datagrams = report->stats.nb_datagrams - report->last_stats.nb_datagrams;
        interval_stats.nb_error = report->stats.nb_error- report->last_stats.nb_error;
        interval_stats.nb_out_of_orded = report->stats.nb_out_of_orded - report->last_stats.nb_out_of_orded;
        if (interval_stats.nb_error > interval_stats.nb_out_of_orded)
            interval_stats.nb_error -= interval_stats.nb_out_of_orded;
        interval_stats.jitter_us = report->stats.jitter_us;
    }
    fhost_iperf_print_stats(stream, &report->last_interval, &report->packet_time, &interval_stats);

    report->last_stats = report->stats;
    report->last_interval = report->packet_time;
    iperf_timeradd(&report->interval_target, &settings->interval, &report->interval_target);
}


void fhost_iperf_print_stats(const struct fhost_iperf_stream *stream,
                             struct iperf_time *start_time,
                             struct iperf_time *end_time,
                             const struct iperf_stats *stats)
{
    const struct fhost_iperf_settings *iperf_settings = &stream->iperf_settings;
    const struct iperf_report *report = &stream->report;
    struct iperf_time duration_time;
    char data[11], bw[11];
    /* uint32_t int_amount, dec_amount; */
    /* float start_time_flt, end_time_flt; */
    uint64_t duration_usec;
    uint32_t start_sec, end_sec;
    int start_ds, end_ds;

    iperf_timersub(end_time, start_time, &duration_time);
    duration_usec = iperf_timerusec(&duration_time);

    // Convert in local time (i.e using report->start_time as reference) and in sec.ds format
    start_sec = start_time->sec - report->start_time.sec;
    start_ds = start_time->usec - report->start_time.usec + 50000;
    if (start_ds < 0)
    {
        start_sec--;
        start_ds += 1000000;
    }
    start_ds = start_ds / 100000;

    end_sec = end_time->sec - report->start_time.sec;
    end_ds = end_time->usec - report->start_time.usec + 50000;
    if (end_ds < 0)
    {
        end_sec--;
        end_ds += 1000000;
    }
    end_ds = end_ds / 100000;

    fhost_iperf_snprintf(data, sizeof(data), (float) stats->bytes, iperf_settings->format - 'a' + 'A');
    fhost_iperf_snprintf(bw, sizeof(bw), 1000000 * (float) stats->bytes / duration_usec, iperf_settings->format);

    if (stream->iperf_settings.flags.is_udp && iperf_settings->flags.is_server)
    {
        uint32_t jitter_sec = stats->jitter_us / 1000;
        float lost_percent;
        uint32_t lost_percent_int, lost_percent_dec;

        lost_percent = (100 * stats->nb_error) / (float) stats->nb_datagrams;
        lost_percent += (float)0.05;
        lost_percent_int = (uint32_t) lost_percent;
        lost_percent_dec = (uint32_t) (lost_percent * 10 - lost_percent_int * 10);

        if (!stream->report.last_stats.bytes)
            fhost_print(stream->iperf_handle,
                        "[ ID]  Interval      Transfer     Bandwidth       Jitter   Lost/Total Datagrams\n");

        fhost_print(stream->iperf_handle,
                    "[%3d] %2d.%01d-%2d.%01d sec  %s  %s/sec  %d.%03d ms   %d/%d (%d.%1d%%)\n",
                    stream->id, start_sec, start_ds, end_sec, end_ds, data, bw,
                    jitter_sec, stats->jitter_us - (jitter_sec * 1000), stats->nb_error,
                    stats->nb_datagrams, lost_percent_int, lost_percent_dec);
    }
    else
    {
        if (!stream->report.last_stats.bytes)
            fhost_print(stream->iperf_handle, "[ ID] Interval       Transfer     Bandwidth\n");

        fhost_print(stream->iperf_handle, "[%3d] %2d.%1d-%2d.%1d sec  %s  %s/sec\n",
                    stream->id, start_sec, start_ds, end_sec, end_ds, data, bw);
    }
}

int fhost_iperf_sigkill_handler(rtos_task_handle iperf_handle)
{
    int stream_id;
    struct fhost_iperf_stream *iperf_stream = NULL;
    const struct fhost_iperf_settings *iperf_settings;

    // Search iperf stream
    for (stream_id = 0; stream_id < FHOST_IPERF_MAX_STREAMS; stream_id++)
    {
        if (streams[stream_id].iperf_handle == iperf_handle)
        {
            iperf_stream = &streams[stream_id];
            break;
        }
    }

    if (stream_id == FHOST_IPERF_MAX_STREAMS)
    {
        fhost_print(iperf_handle, "Stream id %d not valid\n", stream_id);
        return FHOST_IPC_ERROR;
    }

    iperf_settings = &iperf_stream->iperf_settings;
    iperf_stream->active = false;
    if (iperf_settings->flags.is_udp)
        rtos_semaphore_signal(iperf_stream->iperf_task_semaphore, false);
    else
        net_iperf_tcp_close(iperf_stream);

    fhost_print(iperf_handle, "Close Iperf Success %d\n", stream_id);
    return FHOST_IPC_SUCCESS;
}

rtos_task_handle fhost_iperf_msg_handle_get(void)
{
    return iperf_handle;
}
#endif //NX_IPERF

/// @}
