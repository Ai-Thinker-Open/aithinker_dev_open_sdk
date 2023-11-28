/**
 ****************************************************************************************
 *
 * @file fhost_wpa.c
 *
 * @brief Helper functions for wpa_supplicant task management
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#include "rtos.h"
#include "sys/socket.h"
#include "fhost.h"
#include "fhost_wpa.h"
#include "fhost_cntrl.h"
#include "fhost_rx.h"
#include "mac_frame.h"

#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"

#include "platform_al.h"
#include "net_al_ext.h"

#define FHOST_TIMEOUTMS_ADD_NETWORK       7001//100
#define FHOST_TIMEOUTMS_DISABLE_NETWORK   7002//100
#define FHOST_TIMEOUTMS_ENABLE_NETWORK    7001//100

#define FHOST_TIMEOUTMS_INTERFACE_ADD     8002//300
#define FHOST_TIMEOUTMS_SET_NETWORK       8003//300
#define FHOST_TIMEOUTMS_INTERFACE_REMOVE  8004//300

#define FHOST_TIMEOUTMS_DEL_STA           8005//100

/**
 ****************************************************************************************
 * @addtogroup FHOST_WPA
 * @{
 ****************************************************************************************
 */
/// AKM string used by wpa_supplicant
const char *wpa_akm_str[] = {
    [MAC_AKM_NONE] = "NONE",
    [MAC_AKM_PRE_RSN] = NULL,
    [MAC_AKM_8021X] = "EAP",
    [MAC_AKM_PSK] = "WPA-PSK",
    [MAC_AKM_FT_8021X] = "FT_EAP",
    [MAC_AKM_FT_PSK] = "FT-PSK",
    [MAC_AKM_8021X_SHA256] = "WPA-EAP-SHA256",
    [MAC_AKM_PSK_SHA256] = "WPA-PSK-SHA256",
    [MAC_AKM_TDLS] = "TDLS",
    [MAC_AKM_SAE] = "SAE",
    [MAC_AKM_FT_OVER_SAE] = "FT-SAE",
    [MAC_AKM_8021X_SUITE_B] = "WPA-EAP-SUITE-B",
    [MAC_AKM_8021X_SUITE_B_192] = "WPA-EAP-SUITE-B-192",
    [MAC_AKM_FILS_SHA256] = "FILS-SHA256",
    [MAC_AKM_FILS_SHA384] = "FILS-SHA384",
    [MAC_AKM_FT_FILS_SHA256] = "FT-FILS-SHA256",
    [MAC_AKM_FT_FILS_SHA384] = "FT-FILS-SHA384",
    [MAC_AKM_OWE] = "OWE",
    [MAC_AKM_WAPI_CERT] = "WAPI-CERT",
    [MAC_AKM_WAPI_PSK] = "WAPI-PSK",
};

/// Cipher suites string used by wpa_supplicant
const char *wpa_cipher_str[] = {
    [MAC_CIPHER_WEP40] = "WEP40",
    [MAC_CIPHER_TKIP] = "TKIP",
    [MAC_CIPHER_CCMP] = "CCMP",
    [MAC_CIPHER_WEP104] = "WEP104",
    [MAC_CIPHER_WPI_SMS4] = "WPI_SMS4",
    [MAC_CIPHER_BIP_CMAC_128] = "AES-128-CMAC",
    [MAC_CIPHER_GCMP_128] = "GCMP",
    [MAC_CIPHER_GCMP_256] = "GCMP-256",
    [MAC_CIPHER_CCMP_256] = "CCMP-256",
    [MAC_CIPHER_BIP_GMAC_128] = "BIP-GMAC-128",
    [MAC_CIPHER_BIP_GMAC_256] = "BIP-GMAC-256",
    [MAC_CIPHER_BIP_CMAC_256] = "BIP-CMAC-256",
};

/// wpa_supplicant callback definition
struct fhost_wpa_event_cb
{
    /// The callback function
    fhost_wpa_cb_t func;
    /// The callback private parameter
    void *arg;
    /// The events bitfield for this callback
    uint32_t events;
};

/// Private structure for @ref fhost_wpa_wait_event
struct fhost_wpa_target_event
{
    /// Event expected
    enum fhost_wpa_event event;
    /// Task to notify when event is received
    rtos_task_handle task;
};

/// Maximum number of callback allowed
#define FHOST_WPA_EVENT_CB_CNT 2

/// wpa_supplicant parameters for one FHOST interface
struct fhost_wpa_vif_tag
{
    /// Interface Name
    char iface_name[NET_AL_MAX_IFNAME];
    /// state
    enum fhost_wpa_state state;
    /// Table of callback for WPA event
    struct fhost_wpa_event_cb cb[FHOST_WPA_EVENT_CB_CNT];
    /// Network ID WPA is connected to
    int network_id;
    /// MGMT RX filter
    uint32_t rx_filter;
};

/// wpa_supplicant global parameters
struct fhost_wpa_tag
{
    /// handle of WPA task
    rtos_task_handle task;
    /// Socket for WPA control interface
    int ctrl_sock;
    /// Mutex lock to send a command to WPA task
    rtos_mutex ctrl_mutex;
    /// Interfaces status
    struct fhost_wpa_vif_tag vifs[NX_VIRT_DEV_MAX];
};

/// wpa_supplicant configuration for all FHOST interfaces
static struct fhost_wpa_tag fhost_wpa;

/// main function of wpa task
void wpa_supplicant_main(void *env);

static void fhost_wpa_event_process(enum fhost_wpa_event event, void *param,
                                    int param_len, int fhost_vif_idx);

/**
 ****************************************************************************************
 * @brief Get wpa configuration structure for an interface
 *
 * @param[in] fhost_vif_idx  Index of the FHOST interface
 * @return pointer to configuration structure for the interface or NULL for invalid index
 ****************************************************************************************
 */
__INLINE struct fhost_wpa_vif_tag *fhost_wpa_get_vif(int fhost_vif_idx)
{
    if ((fhost_vif_idx < 0) || (fhost_vif_idx >= NX_VIRT_DEV_MAX))
        return NULL;
    return &fhost_wpa.vifs[fhost_vif_idx];
}

/**
 ****************************************************************************************
 * @brief Reset interface parameters to its default values.
 *
 * @param[in] wpa_vif  WPA interface structure for the interface
 ****************************************************************************************
 */
static void fhost_wpa_vif_reset(struct fhost_wpa_vif_tag *wpa_vif)
{
    wpa_vif->state = FHOST_WPA_STATE_STOPPED;
    wpa_vif->network_id = -1;
    wpa_vif->rx_filter = 0xFFFFFFF;
}

/**
 ****************************************************************************************
 * @brief Reset all WPA environment to its default values.
 ****************************************************************************************
 */
static void fhost_wpa_reset()
{
    struct fhost_wpa_vif_tag *wpa_vif;
    int i;

    fhost_wpa.task = RTOS_TASK_NULL;
    fhost_wpa.ctrl_sock = -1;

    for (i = 0, wpa_vif = fhost_wpa.vifs; i < NX_VIRT_DEV_MAX; i++, wpa_vif++)
    {
        fhost_wpa_vif_reset(wpa_vif);
    }
}

/**
 ****************************************************************************************
 * @brief Process function for @ref FHOST_WPA_STARTED event
 *
 * This event is sent by WPA task after its initialization, and this function opens and
 * connects a UDP socket to send commands.
 *
 * @param[in] port  Control port id
 * @return 0 In case of success and !=0 if the control interface cannot be opened.
 * In this case the wpa_supplicant is killed.
 ****************************************************************************************
 */
static int fhost_wpa_started(int port)
{
#ifndef NO_GLOBAL_SOCKET
    fhost_wpa.ctrl_sock = fhost_open_loopback_udp_sock(port);

    if (fhost_wpa.ctrl_sock < 0)
    {
        TRACE_FHOST(ERR, "Failed to connect to WPA ctrl interface (port=%d)", port);

        // Delete task (this will leak memory)
        rtos_task_delete(fhost_wpa.task);

        // And process a fake EXIT event to clean everything
        fhost_wpa_event_process(FHOST_WPA_EXIT, (void *)-2, 0, FHOST_WPA_GLOBAL_VIF);
        return -1;
    }
#endif

    fhost_task_ready(SUPPLICANT_TASK);
    TRACE_FHOST(WPA_INFO, "WPA task started");
    return 0;
}

/**
 ****************************************************************************************
 * @brief Process function for @ref FHOST_WPA_EXIT event
 *
 * @param[in] exit_code  Exit code return by WPA task
 ****************************************************************************************
 */
static void fhost_wpa_exit(int exit_code)
{
    TRACE_FHOST(WPA_INFO, "WPA task exit (status = %d)", (int16_t)exit_code);

#ifndef NO_GLOBAL_SOCKET
    if (fhost_wpa.ctrl_sock >= 0)
        close(fhost_wpa.ctrl_sock);
#endif

    fhost_rx_set_mgmt_cb(NULL, NULL);
    fhost_wpa_reset();
}

/**
 ******************************************************************************
 * @brief Generic wpa event callback to notify a waiting task when expected
 * event is received
 *
 * Registered by @ref fhost_wpa_wait_event_register and unregistered by @ref
 * fhost_wpa_wait_event_unregister. The function unregister itself when the
 * waiting task is notified.
 *
 * @param[in] fhost_vif_idx  Index of the FHOST interface
 * @param[in] event          Event generated by the WPA task
 * @param[in] event_param    Event parameter (not used)
 * @param[in] arg            Registered private parameter, in this case info on
 *                           the expected event and the waiting task
 ******************************************************************************
 */
static void fhost_wpa_wait_event(int fhost_vif_idx, enum fhost_wpa_event event,
                                 void *event_param, void *arg)
{
    struct fhost_wpa_target_event *target = arg;


    if (MGMR_VIF_STA == fhost_vif_idx) {
        switch (event) {
            case FHOST_WPA_CONNECTED:
                PLATFORM_HOOK(prevent_sleep, PSM_EVENT_CONNECT, 0);
                break;
            case FHOST_WPA_DISCONNECTED:
                #ifdef LP_APP
                PLATFORM_HOOK(prevent_sleep, PSM_EVENT_CONNECT, 0);
                PLATFORM_HOOK(prevent_sleep, PSM_EVENT_DISCONNECT, 0);
                #endif
                break;
            default:
                break;
        }
    }
    if (MGMR_VIF_AP == fhost_vif_idx) {
        switch (event) {
            case FHOST_WPA_CONNECTED:
                platform_post_event(EV_WIFI, CODE_WIFI_ON_AP_STARTED);
                break;
            case FHOST_WPA_DISCONNECTED:
                platform_post_event(EV_WIFI, CODE_WIFI_ON_AP_STOPPED);
                break;
            default:
                break;
        }
    }

    fhost_wpa_cb_unregister(fhost_vif_idx, fhost_wpa_wait_event);
    rtos_task_notify(target->task, false);
}

/**
 ******************************************************************************
 * @brief Generic function to register WPA event callback that will notify
 * the calling task when a specific event occurs.
 *
 * Register @ref fhost_wpa_wait_event as callback. After calling this function
 * the user should call @ref rtos_task_wait_notification to wait for the
 * selected event. To avoid deadlock, the events @ref FHOST_WPA_INTERFACE_REMOVED
 * and @ref FHOST_WPA_EXIT are always included in the event set.
 * The caller must take care of race condition between wpa events and callback
 * registration.
 *
 * @param[in] fhost_vif_idx  Index of the FHOST interface.
 * @param[in] event          Expected Event.
 * @param[in] target         Registered as callback private parameter. No need
 *                           to initialize it but the pointer MUST be valid
 *                           until rtos_task_wait_notification is called.
 * @return 0 on success and != 0 otherwise
 ******************************************************************************
 */
static int fhost_wpa_wait_event_register(int fhost_vif_idx, enum fhost_wpa_event event,
                                         struct fhost_wpa_target_event *target)
{
    target->event = event;
    target->task = rtos_get_task_handle();

    if (rtos_task_init_notification(target->task))
        return -1;

    return fhost_wpa_cb_register(fhost_vif_idx,
                                 CO_BIT(FHOST_WPA_CONNECTED) |
                                 CO_BIT(FHOST_WPA_DISCONNECTED) | 
                                 CO_BIT(FHOST_WPA_EXIT) |
                                 CO_BIT(FHOST_WPA_INTERFACE_REMOVED),
                                 fhost_wpa_wait_event, target);
}

/**
 ******************************************************************************
 * @brief Unregister WPA event callback
 *
 * Unregister the function registered by @ref fhost_wpa_wait_event_register.
 * There no need to call this function when a notification has been received.
 *
 * @param[in] fhost_vif_idx  Index of the FHOST interface
 * @return 0 on success and != 0 otherwise
 ******************************************************************************
 */
static int fhost_wpa_wait_event_unregister(int fhost_vif_idx)
{
    return fhost_wpa_cb_unregister(fhost_vif_idx, fhost_wpa_wait_event);
}

/**
 ****************************************************************************************
 * @brief Call registered callback when event is received from WPA task
 *
 * Loop over all registered callbacks and call them if associated to this event.
 *
 * @param[in] fhost_vif_idx  Index of the FHOST interface
 * @param[in] event          Event from WPA task
 * @param[in] param          Event parameter
 ****************************************************************************************
 */
static void fhost_wpa_call_event_cb(int fhost_vif_idx, enum fhost_wpa_event event,
                                    void *param)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);
    struct fhost_wpa_event_cb *cb = wpa_vif->cb;
    bool reset_cb = (event == FHOST_WPA_EXIT);
    int i;

    for (i = 0 ; i < FHOST_WPA_EVENT_CB_CNT; i++, cb++)
    {
        if (cb->events & CO_BIT(event))
            cb->func(fhost_vif_idx, event, param, cb->arg);

        if (reset_cb)
        {
            cb->events = 0;
            cb->func = NULL;
            cb->arg = NULL;
        }
    }
}

/**
 ****************************************************************************************
 * @brief Process events form WPA task
 *
 * @note This function is called in the context of the WPA task, and as such it cannot
 * block upon WPA task (e.g. it cannot send WPA command).
 *
 * @param[in] event          Event from WPA task
 * @param[in] param          Event parameter
 * @param[in] param_len      Length, in bytes, of the param buffer
 * @param[in] fhost_vif_idx  Index of the FHOST interface
 ****************************************************************************************
 */
static void fhost_wpa_event_process(enum fhost_wpa_event event, void *param,
                                    int param_len, int fhost_vif_idx)
{
    if (fhost_vif_idx == FHOST_WPA_GLOBAL_VIF)
    {
        int i;

        switch (event)
        {
            case FHOST_WPA_EXIT:
                fhost_wpa_exit((int)param);
                break;
            case FHOST_WPA_STARTED:
                if (fhost_wpa_started((int)param))
                    return;
                break;
            default:
                return;
        }

        // For global event callback registered on all interfaces
        for (i = 0 ; i < NX_VIRT_DEV_MAX; i++)
        {
            fhost_wpa_call_event_cb(i, event, param);
        }
    }
    else
    {
        struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);

        if (!wpa_vif)
            return;

        switch (event)
        {
            case FHOST_WPA_CONNECTED:
                TRACE_FHOST(WPA_FSM, "{FVIF-%d} enter FHOST_WPA_STATE_CONNECTED", fhost_vif_idx);
                wpa_vif->state = FHOST_WPA_STATE_CONNECTED;
                break;
            case FHOST_WPA_DISCONNECTED:
                TRACE_FHOST(WPA_FSM, "{FVIF-%d} enter FHOST_WPA_STATE_NOT_CONNECTED", fhost_vif_idx);
                wpa_vif->state = FHOST_WPA_STATE_NOT_CONNECTED;
                break;
            case FHOST_WPA_INTERFACE_ADDED:
            case FHOST_WPA_INTERFACE_REMOVED:
                break;
            default:
                return;
        }

        fhost_wpa_call_event_cb(fhost_vif_idx, event, param);
    }
}

/**
 ****************************************************************************************
 * @brief Callback function for non processed MGMT frames
 *
 * Management frames not processed by the wifi task are forwarded to wpa_supplicant
 *
 * @param[in] info  Frame information.
 * @param[in] arg   Callback parameter (unused)
 ****************************************************************************************
 */
static void fhost_wpa_rx_cb(struct fhost_frame_info *info, void *arg)
{
    struct mac_hdr *hdr = (struct mac_hdr *)info->payload;
    struct fhost_wpa_vif_tag *wpa_vif;
    struct fhost_vif_tag *fhost_vif;
    struct cfgrwnx_rx_mgmt_event event;

    if ((unsigned)info->fvif_idx >= NX_VIRT_DEV_MAX)
    {
        TRACE_FHOST(WPA_INFO, "Ignore Management frame received on invalid VIF");
        return;
    }

    wpa_vif = &fhost_wpa.vifs[info->fvif_idx];
    fhost_vif = &fhost_env.vif[info->fvif_idx];

    if ((info->payload == NULL) ||
        (wpa_vif->state == FHOST_WPA_STATE_STOPPED) ||
        ((hdr->fctl & MAC_FCTRL_TYPE_MASK) != MAC_FCTRL_MGT_T) ||
        (wpa_vif->rx_filter & CO_BIT(MAC_FCTRL_SUBTYPE(hdr->fctl))))
        return;

    if (fhost_vif->conn_sock < 0)
        return;

    event.hdr.id = CFGRWNX_RX_MGMT_EVENT;
    event.hdr.len = sizeof(event);
    event.fhost_vif_idx = info->fvif_idx;
    event.freq = info->freq;
    event.rssi = info->rssi;
    event.length = info->length;
    event.payload = rtos_malloc(event.length);
    if (event.payload == NULL)
        return;

    memcpy(event.payload, info->payload, event.length);
    fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock);
}

/**
 ****************************************************************************************
 * @brief Send command to wpa_supplicant task
 *
 * @param[in] wpa_vif  WPA structure for the interface, NULL for global command.
 * @param[in] cmd      Command string (must be NULL terminated)
 * @return 0 if command has been successfully sent to WPA task and != 0 otherwise
 ****************************************************************************************
 */
static int fhost_wpa_send_cmd(struct fhost_wpa_vif_tag *wpa_vif, char *cmd, int ctrl_sock)
{
    struct iovec iovec[4];
    struct msghdr msghdr;

    memset(&msghdr, 0, sizeof(msghdr));
    msghdr.msg_iov = iovec;

    if (wpa_vif)
    {
        iovec[0].iov_base = "IFNAME=";
        iovec[0].iov_len = 7;
        iovec[1].iov_base = wpa_vif->iface_name;
        iovec[1].iov_len = strlen(wpa_vif->iface_name);
        iovec[2].iov_base = " ";
        iovec[2].iov_len = 1;
        iovec[3].iov_base = cmd;
        iovec[3].iov_len = strlen(cmd);
        msghdr.msg_iovlen = 4;
    }
    else
    {
        iovec[0].iov_base = cmd;
        iovec[0].iov_len = strlen(cmd);
        msghdr.msg_iovlen = 1;
    }

    if (sendmsg(ctrl_sock, &msghdr, 0) < 0)
        return -1;

    return 0;
}

/**
 ****************************************************************************************
 * @brief Retrieve response from wpa_supplicant task
 *
 * The function first waits up to @p timeout_ms ms for wpa_supplicant to send data on the
 * ctrl interface. An error is returned if this is not the case.
 * Then the response is read in the provided buffer. If no buffer is provided (or if the
 * buffer is too small) a temporary buffer is used to retrieve at up 4 characters.
 * The function then check if the response starts with the "FAIL" string. If so the
 * function returns 1.
 * In any cases the response is also copied in the @p resp buffer (as much as possible)
 * and the size written in updated in @p resp_len.
 *
 * @param[in]     resp        Buffer to retrieve the response.
 * @param[in,out] resp_len    Size, in bytes, of the response buffer.
 *                            If no error is reported, it is updated with the size
 *                            actually written in the response buffer.
 * @param[in]     timeout_ms  Timeout, in ms, allowed to the wpa_supplicant task to
 *                            respond (<0 means wait forever).
 *
 * @return <0 if an error occurred, 1 if response starts with "FAIL" and 0 otherwise.
 ****************************************************************************************
 */
static int fhost_wpa_get_resp(char *resp, int *resp_len, int timeout_ms, int ctrl_sock)
{
    struct timeval timeout;
    fd_set fds;
    char tmp_resp[4];
    char *buf;
    int res, buf_len;
    int recv_flags = 0;

    if (!resp || !resp_len || (*resp_len < 4))
    {
        // Use tmp_resp as dummy buffer
        buf = tmp_resp;
        buf_len = sizeof(tmp_resp);
    }
    else
    {
        buf = resp;
        buf_len = *resp_len;
    }

    if (timeout_ms >= 0)
    {
        FD_ZERO(&fds);
        FD_SET(ctrl_sock, &fds);

        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = timeout_ms % 1000 * 1000;

        if (select(ctrl_sock + 1, &fds, NULL, NULL, &timeout) <= 0) {
            printf("select error timeout %d\r\n", timeout_ms);
            return -1;
        }

        recv_flags = MSG_DONTWAIT;
    }

    // printf("receive with timeout start %d\r\n", timeout_ms);
    res = recv(ctrl_sock, buf, buf_len, recv_flags);
    // printf("receive with timeout Done %d, res = %d\r\n", timeout_ms, res);
    if (res < 0)
        return -1;

    TRACE_BUF_FHOST(WPA_CMD, "RESP: %pBs", res, buf);

    if (resp && resp_len)
    {
        if (buf == tmp_resp)
        {
            if (res < *resp_len)
                *resp_len = res;
            memcpy(resp, tmp_resp, *resp_len);
        }
        else
            *resp_len = res;
    }

    if (strncmp(buf, "FAIL", 4) == 0)
        res = 1;
    else
        res = 0;

    return res;
}

/*
 ****************************************************************************************
 * Public functions
 ****************************************************************************************
 */
int fhost_wpa_init(void)
{
    memset(&fhost_wpa, 0, sizeof(fhost_wpa));
    fhost_wpa_reset();

    if (rtos_mutex_create(&fhost_wpa.ctrl_mutex) ||
        rtos_task_create(wpa_supplicant_main, "WPA", SUPPLICANT_TASK,
                         FHOST_WPA_STACK_SIZE, NULL, FHOST_WPA_PRIORITY, &fhost_wpa.task))
    {
        TRACE_FHOST(ERR, "Failed to create WPA task");
        return -1;
    }

    fhost_rx_set_mgmt_cb(NULL, fhost_wpa_rx_cb);
    return 0;
}

int fhost_wpa_add_vif(int fhost_vif_idx)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);

    if (!wpa_vif ||
        (wpa_vif->state != FHOST_WPA_STATE_STOPPED) ||
        (fhost_vif_name(fhost_vif_idx, wpa_vif->iface_name,
                        sizeof(wpa_vif->iface_name)) < 0))
        return -1;

    if (fhost_wpa_execute_cmd(FHOST_WPA_GLOBAL_VIF, NULL, 0, FHOST_TIMEOUTMS_INTERFACE_ADD, "INTERFACE_ADD %s",
                              wpa_vif->iface_name))
    {
        TRACE_FHOST(ERR, "{FVIF-%d} Failed to add WPA interface", fhost_vif_idx);
        return -1;
    }

    wpa_vif->state = FHOST_WPA_STATE_NOT_CONNECTED;

    if (fhost_vif_idx == MGMR_VIF_STA &&
        fhost_wpa_execute_cmd(fhost_vif_idx, NULL, 0, 300, "STA_AUTOCONNECT %u", !wifiMgmr.disable_autoreconnect)) {
        printf("%s: set autoconnect failed\r\n", __func__);
    }

    TRACE_FHOST(WPA_INFO, "{FVIF-%d} WPA interface added", fhost_vif_idx);
    return 0;
}

int fhost_wpa_remove_vif(int fhost_vif_idx)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);

    if (!wpa_vif)
        return -1;

    if (wpa_vif->state == FHOST_WPA_STATE_STOPPED)
        return 0;

    if (fhost_wpa_execute_cmd(FHOST_WPA_GLOBAL_VIF, NULL, 0, FHOST_TIMEOUTMS_INTERFACE_REMOVE, "INTERFACE_REMOVE %s",
                              wpa_vif->iface_name))
    {
        TRACE_FHOST(ERR, "{FVIF-%d} Failed to remove WPA interface", fhost_vif_idx);
        return -1;
    }

    wpa_vif->state = FHOST_WPA_STATE_STOPPED;
    TRACE_FHOST(WPA_INFO, "{FVIF-%d} WPA interface removed", fhost_vif_idx);
    return 0;
}

enum fhost_wpa_state fhost_wpa_get_state(int fhost_vif_idx)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);

    if (!wpa_vif)
        return FHOST_WPA_STATE_STOPPED;

    return wpa_vif->state;
}

int fhost_wpa_cb_register(int fhost_vif_idx, int events, fhost_wpa_cb_t cb_func, void *cb_arg)
{
    struct fhost_wpa_event_cb *cb;
    int i;

    if ((unsigned)fhost_vif_idx >= NX_VIRT_DEV_MAX)
        return -1;

    cb = fhost_wpa.vifs[fhost_vif_idx].cb;

    for (i = 0; i < FHOST_WPA_EVENT_CB_CNT; i++, cb++)
    {
        if (cb->events == 0)
        {
            cb->events = events;
            cb->func = cb_func;
            cb->arg = cb_arg;
            return 0;
        }
    }

    return -1;
}

int fhost_wpa_cb_unregister(int fhost_vif_idx, fhost_wpa_cb_t cb_func)
{
    struct fhost_wpa_event_cb *cb;
    int i;

    if ((unsigned)fhost_vif_idx >= NX_VIRT_DEV_MAX)
        return -1;

    cb = fhost_wpa.vifs[fhost_vif_idx].cb;

    for (i = 0; i < FHOST_WPA_EVENT_CB_CNT; i++, cb++)
    {
        if (cb->func == cb_func)
        {
            cb->events = 0;
            cb->func = NULL;
            cb->arg = NULL;
            return 0;
        }
    }

    return -1;
}

int fhost_wpa_send_event(enum fhost_wpa_event event, void *param, int param_len, int fhost_vif_idx)
{
    if (((event <= FHOST_WPA_STARTED) && (fhost_vif_idx != FHOST_WPA_GLOBAL_VIF)) ||
        ((event > FHOST_WPA_STARTED) && (fhost_vif_idx > NX_VIRT_DEV_MAX)) ||
        (event >= FHOST_WPA_LAST))
        return -1;

    fhost_wpa_event_process(event, param, param_len, fhost_vif_idx);
    return 0;
}

int fhost_wpa_execute_cmd(int fhost_vif_idx, char *resp_buf, int *resp_buf_len,
                          int timeout_ms, const char *fmt, ...)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);
    va_list args;
    char cmd[128];
    unsigned int cmd_len;
    int res = -1;

#ifdef NO_GLOBAL_SOCKET
    struct sockaddr_in cntrl;
    struct sockaddr_in wpa;
    int ctrl_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(ctrl_sock < 0) {
        TRACE_FHOST(ERR, "app -> wpa supplicant socket create fail");
        return res;
    }

    cntrl.sin_family = AF_INET;
    cntrl.sin_addr.s_addr = htonl(INADDR_ANY);
    cntrl.sin_port =  htons(0);
    if (bind(ctrl_sock, (struct sockaddr *)&cntrl, sizeof(cntrl)) < 0)
        goto end;

    wpa.sin_family = AF_INET;
    wpa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    wpa.sin_port = htons(9878);
    if (connect(ctrl_sock, (struct sockaddr *)&wpa, sizeof(wpa)) < 0)
        goto end;
#else
    int ctrl_sock = fhost_wpa.ctrl_sock;
#endif

    if ((fhost_vif_idx < FHOST_WPA_GLOBAL_VIF) &&
        (!wpa_vif || (wpa_vif->state == FHOST_WPA_STATE_STOPPED)))
        goto end;

    // Format command
    va_start(args, fmt);
    cmd_len = dbg_vsnprintf(cmd, sizeof(cmd), fmt, args);
    va_end(args);

    if (cmd_len >= sizeof(cmd))
    {
        TRACE_FHOST(ERR, "WPA Command truncated. need %d bytes", cmd_len);
        goto end;
    }

    rtos_mutex_lock(fhost_wpa.ctrl_mutex);

    TRACE_BUF_FHOST(WPA_CMD, "CMD: %pBs", cmd_len, cmd);

    // Send it
    if (fhost_wpa_send_cmd(wpa_vif, cmd, ctrl_sock) < 0) {
        rtos_mutex_unlock(fhost_wpa.ctrl_mutex);
        goto end;
    }

    // Retrieve respond
    res = fhost_wpa_get_resp(resp_buf, resp_buf_len, timeout_ms, ctrl_sock);

    rtos_mutex_unlock(fhost_wpa.ctrl_mutex);
  end:
#ifdef NO_GLOBAL_SOCKET
    close(ctrl_sock);
#endif
    return res;
}

int fhost_wpa_create_network(int fhost_vif_idx, char *net_cfg, char *ssid_cfg, char *key_cfg, bool enable, int timeout_ms)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);
    char res[5], *tok;
    int res_len;

    if (!net_cfg || !ssid_cfg || !wpa_vif || fhost_wpa_add_vif(fhost_vif_idx))
        return -1;

    // Create and configure network block
    res_len = sizeof(res) - 1;
    if (fhost_wpa_execute_cmd(fhost_vif_idx, res, &res_len, FHOST_TIMEOUTMS_ADD_NETWORK, "ADD_NETWORK"))
        return -1;
    res[res_len] = '\0';
    wpa_vif->network_id = atoi(res);

#if 0
    // Enable H2E
    res_len = sizeof(res) - 1;
    if (fhost_wpa_execute_cmd(fhost_vif_idx, res, &res_len, 1000, "SET sae_pwe 2"))
        return -1;
#endif

    tok = strtok(net_cfg, ";");
    while (tok)
    {
        res_len = sizeof(res);
        printf("Exec %s\r\n", tok);
        if (fhost_wpa_execute_cmd(fhost_vif_idx, res, &res_len, FHOST_TIMEOUTMS_SET_NETWORK, "SET_NETWORK %d %s",
                                  wpa_vif->network_id, tok))
        {
            printf("SET_NETWORK (%s) failed\r\n", tok);
            fhost_wpa_remove_vif(fhost_vif_idx);
            return -1;
        }
        tok = strtok(NULL, ";");
    }

    printf("Exec %s\r\n", ssid_cfg);
    res_len = sizeof(res);
    if (fhost_wpa_execute_cmd(fhost_vif_idx, res, &res_len, FHOST_TIMEOUTMS_SET_NETWORK, "SET_NETWORK %d %s",
                              wpa_vif->network_id, ssid_cfg))
    {
        printf("SET_NETWORK (%s) failed\r\n", ssid_cfg);
        fhost_wpa_remove_vif(fhost_vif_idx);
        return -1;
    }

    if (*key_cfg) {
        printf("Exec %s\r\n", key_cfg);
        res_len = sizeof(res);
        if (fhost_wpa_execute_cmd(fhost_vif_idx, res, &res_len, FHOST_TIMEOUTMS_SET_NETWORK, "SET_NETWORK %d %s",
                                  wpa_vif->network_id, key_cfg))
        {
            printf("SET_NETWORK (%s) failed\r\n", key_cfg);
            fhost_wpa_remove_vif(fhost_vif_idx);
            return -1;
        }
    }
    printf("WPA network %d: created and configured", wpa_vif->network_id);

    // Connect to AP if requested
    if (enable && fhost_wpa_enable_network(fhost_vif_idx, timeout_ms))
    {
        fhost_wpa_remove_vif(fhost_vif_idx);
        return -1;
    }

    return 0;
}

int fhost_wpa_enable_network(int fhost_vif_idx, int timeout_ms)
{
    struct fhost_wpa_target_event target;
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);

    if (!wpa_vif || (wpa_vif->network_id < 0))
        return -1;

    if (wpa_vif->state == FHOST_WPA_STATE_CONNECTED)
        return 0;

    if (timeout_ms &&
        fhost_wpa_wait_event_register(fhost_vif_idx, FHOST_WPA_CONNECTED, &target))
        return -1;

    if (fhost_wpa_execute_cmd(fhost_vif_idx, NULL, NULL, FHOST_TIMEOUTMS_ENABLE_NETWORK, "ENABLE_NETWORK %d ",
                              wpa_vif->network_id))
    {
        fhost_wpa_wait_event_unregister(fhost_vif_idx);
        return -1;
    }

    if (!timeout_ms)
        return 0;

    // Wait until connection is completed.
    if (rtos_task_wait_notification(timeout_ms) == 0)
    {
        TRACE_FHOST(WPA_INFO, "WPA network %d: connection timeout", wpa_vif->network_id);
        fhost_wpa_wait_event_unregister(fhost_vif_idx);
        return -1;
    }
    else if (wpa_vif->state == FHOST_WPA_STATE_STOPPED)
    {
        TRACE_FHOST(WPA_INFO, "WPA Task exit during connection");
        return -1;
    }
    else if (wpa_vif->state == FHOST_WPA_STATE_CONNECTED)
    {
        TRACE_FHOST(WPA_INFO, "WPA network %d: connected", wpa_vif->network_id);
        return 0;
    } 
    else if (wpa_vif->state == FHOST_WPA_STATE_NOT_CONNECTED)
    {
        TRACE_FHOST(WPA_INFO, "WPA network %d: not connected", wpa_vif->network_id);
        return 0;
    }

    return 0; 
}

int fhost_wpa_disable_network(int fhost_vif_idx)
{
    struct fhost_wpa_target_event target;
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);

    if (!wpa_vif || (wpa_vif->network_id < 0))
        return -1;

    if (wpa_vif->state != FHOST_WPA_STATE_CONNECTED)
        return 0;

    if (fhost_wpa_wait_event_register(fhost_vif_idx, FHOST_WPA_DISCONNECTED, &target))
        return -1;

    if (fhost_wpa_execute_cmd(fhost_vif_idx, NULL, NULL, FHOST_TIMEOUTMS_DISABLE_NETWORK, "DISABLE_NETWORK %d ",
                              wpa_vif->network_id))
    {
        fhost_wpa_wait_event_unregister(fhost_vif_idx);
        return -1;
    }

    if (rtos_task_wait_notification(1000) == 0)
    {
        TRACE_FHOST(WPA_INFO, "WPA network %d: disconnection timeout", wpa_vif->network_id);
        fhost_wpa_wait_event_unregister(fhost_vif_idx);
        return -1;
    }
    else if (wpa_vif->state == FHOST_WPA_STATE_STOPPED)
    {
        TRACE_FHOST(WPA_INFO, "WPA Task exit during disconnection");
        return -1;
    }

    TRACE_FHOST(WPA_INFO, "WPA network %d: disconnected", wpa_vif->network_id);
    return 0;
}

int fhost_wpa_set_mgmt_rx_filter(int fhost_vif_idx, uint32_t filter)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);
    if (!wpa_vif)
        return -1;

    wpa_vif->rx_filter = filter;
    return 0;
}

int fhost_wpa_get_mgmt_rx_filter(int fhost_vif_idx)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);
    if (!wpa_vif)
        return 0;

    return wpa_vif->rx_filter;
}

int fhost_wpa_akm_name(uint32_t akm, char *buf, int len)
{
    unsigned int i;
    int written = 1;

    if (len < 2)
        return -1;
    len -= 2; // keep space for ';' and '\0'

    for (i = 0; i < CO_ARRAY_SIZE(wpa_akm_str); i++)
    {
        if ((akm & CO_BIT(i)) && wpa_akm_str[i])
        {
            int akm_len = strlen(wpa_akm_str[i]);
            if (len < (akm_len + 1))
                return -1;
            *buf++ = ' ';
            memcpy(buf, wpa_akm_str[i], akm_len);
            buf += akm_len;
            len -= akm_len + 1;
            written += akm_len + 1;
        }
    }

    *buf++ = ';';
    *buf = 0;
    return written;
}

int fhost_wpa_cipher_name(uint32_t cipher, char *buf, int len)
{
    unsigned int i;
    int written = 1;

    if (len < 2)
        return -1;
    len -= 2; // keep space for ';' and '\0'

    for (i = 0; i < CO_ARRAY_SIZE(wpa_cipher_str); i++)
    {
        if ((cipher & CO_BIT(i)) && wpa_cipher_str[i])
        {
            int cipher_len = strlen(wpa_cipher_str[i]);
            if (len < (cipher_len + 1))
                return -1;
            *buf++ = ' ';
            memcpy(buf, wpa_cipher_str[i], cipher_len);
            buf += cipher_len;
            len -= cipher_len + 1;
            written += cipher_len + 1;
        }
    }

    *buf++ = ';';
    *buf = 0;
    return written;
}

int fhost_wpa_get_supplicant_state(int fhost_vif_idx)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);
    char res[5];
    int res_len;
    int state;

    res_len = sizeof(res) - 1;
    fhost_wpa_execute_cmd(fhost_vif_idx, res, &res_len, 7004, "GET_STATUS %d ",
                            wpa_vif->network_id);
    res[res_len] = '\0';
    state = atoi(res);

#if 0
    switch (state) {
        case 0:
            printf("WPA_DISCONNECTED\r\n");
            break;
        case 1:
            printf("WPA_INTERFACE_DISABLED\r\n");
            break;
        case 2:
            printf("WPA_INACTIVE\r\n");
            break;
        case 3:
            printf("WPA_SCANNING\r\n");
            break;
        case 4:
            printf("WPA_AUTHENTICATING\r\n");
            break;
        case 5:
            printf("WPA_ASSOCIATING\r\n");
            break;
        case 6:
            printf("WPA_ASSOCIATED\r\n");
            break;
        case 7:
            printf("WPA_4WAY_HANDSHAKE\r\n");
            break;
        case 8:
            printf("WPA_GROUP_HANDSHAKE\r\n");
            break;
        case 9:
            printf("WPA_COMPLETED\r\n");
            break;
    }
#endif

    return state;
}

int fhost_wpa_ap_sta_del(int fhost_vif_idx, uint8_t *mac_addr)
{
    struct fhost_wpa_vif_tag *wpa_vif = fhost_wpa_get_vif(fhost_vif_idx);

    if (!wpa_vif || (wpa_vif->network_id < 0) || !mac_addr)
        return -1;

    if (wpa_vif->state != FHOST_WPA_STATE_CONNECTED)
        return -1;

    if (fhost_wpa_execute_cmd(fhost_vif_idx, NULL, NULL, FHOST_TIMEOUTMS_DEL_STA, "AP_STA_DEL %d %s",
                              wpa_vif->network_id, mac_addr))
    {
        printf("Del sta error\r\n");
        return -1;
    }

    return 0;
}

/**
 * @}
 */
