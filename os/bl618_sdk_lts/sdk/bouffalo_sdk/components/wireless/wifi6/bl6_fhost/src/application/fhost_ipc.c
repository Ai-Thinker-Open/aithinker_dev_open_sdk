 /**
 ****************************************************************************************
 *
 * @file fhost_ipc.c
 *
 * @brief Definition of IPC task for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#include "rwnx_config.h"
#include "fhost.h"
#include "fhost_ipc.h"
#include "fhost_wpa.h"
#include "fhost_cntrl.h"
#include "fhost_config.h"
#if NX_PING
#include "fhost_ping.h"
#endif
#if NX_IPERF
#include "fhost_iperf.h"
#endif
#include "fhost_connect.h"
#include "fhost_rx.h"
#include "ipc_emb.h"
#include "sys/socket.h"
#include "cfgrwnx.h"

#include "mac_frame.h"
#include "net_al.h"
#include "platform_al.h"

#include "wifi_mgmr.h"
#include "wifi_mgmr_ext.h"

/**
 ****************************************************************************************
 * @addtogroup FHOST_IPC
 * @{
 ****************************************************************************************
 */

/// IPC task message queue size
#define FHOST_IPC_QUEUE_SIZE 3

/// Max number of characters to write in one line
#define LINE_MAX_SZ 150

/**
 * Wifi configuration
 */


static uint8_t FHOST_CFG_HE_MCS_data[1] = {1}; // 0=MCS0-7, 1=MCS0-9, 2=MCS0-11
static uint16_t tx_agg_lifetime = 0x4000;
#ifdef CFG_BL_WIFI_UAPSD_ENABLE
static uint32_t PS_UAPSD_TIMEOUT = 50;
static uint32_t PS_UAPSD_QUEUES = ((1 << AC_VO) | (1 << AC_VI) | (1 << AC_BE) | (1 << AC_BK));
#endif
struct fhost_config_item example_config[] = {
    {FHOST_CFG_TX_LFT,          2, &tx_agg_lifetime},
    {FHOST_CFG_HE_MCS,          1, FHOST_CFG_HE_MCS_data},
#ifdef CFG_BL_WIFI_UAPSD_ENABLE
    {FHOST_CFG_UAPSD_TIMEOUT,   4, &PS_UAPSD_TIMEOUT},
    {FHOST_CFG_UAPSD_QUEUES,    4, &PS_UAPSD_QUEUES},
#endif
    {FHOST_CFG_END,             0, NULL},
};

/// Master message queue for IPC messages
static rtos_queue cmd_queue;
static rtos_queue queue;

/// Link to control task
struct fhost_cntrl_link *cntrl_link;

/// Local buffer used to process and respond to IPC message
#define IPC_MSG_BUF_SIZE 256
struct ipc_fhost_msg
{
    uint32_t id;   ///< Id of the message (not used)
    uint32_t len;  ///< Length in bytes
    uint32_t data[IPC_MSG_BUF_SIZE/4]; ///< Message data
    uint32_t pattern; ///< unique pattern
};
static struct ipc_fhost_msg ipc_msg;

/// IPC command description
struct fhost_ipc_cmd {
    /// process function
    int (*exec) (char *params);
    /// name of the command
    char *name;
    /// List of parameters (to report command usage to host)
    char *params;
};

/// Rate Control configuration
struct fhost_rc_conf {
    /// String describing the RC configuration
    char line[LINE_MAX_SZ + 1];
    /// Index of the RC configuration
    int r_idx;
};

static const struct fhost_ipc_cmd ipc_commands[];

/// Command message ID associated to RTOS Task handle
struct fhost_task_msg_id {
    /// task handle
    rtos_task_handle task_handle;
    /// sigkill callback
    fhost_ipc_sigkill_cb sigkill_cb;
    /// message id
    uint32_t msg_id;
};

/// Length of @ref task_handle_msg_id
#define FHOST_TASK_HANDLE_LEN 5
/// Array of message IDs associated to RTOS task handles.
static struct fhost_task_msg_id task_handle_msg_id[FHOST_TASK_HANDLE_LEN];

/// Task that is waiting notification upon fw dump. It is static as only
/// the IPC task can be in such case after receiving mactrace command.
static rtos_task_handle task_waiting_error_ind = NULL;

/**
 ****************************************************************************************
 * @brief Extract token from parameter list
 *
 * Extract the first parameter of the string. Parameters are separatd with space unless
 * it starts with " (or ') in which case it extract the token until " (or ') is reached.
 * " (or ') are then removed from the token.
 *
 * @param[in, out] params Pointer to parameters string to parse. Updated with remaining
 *                        parameters to parse.
 * @return pointer on first parameter
 ****************************************************************************************
 */
static char *fhost_ipc_next_token(char **params)
{
    char *ptr = *params, *next;
    char sep = ' ';

    if (!ptr)
        return NULL;

    if ((ptr[0] == '"') || (ptr[0] == '\''))
    {
        sep = ptr[0];
        ptr++;
    }

    next = strchr(ptr, sep);
    if (next)
    {
        *next++ = '\0';
        while (*next == ' ')
            next++;
        if (*next == '\0')
            next = NULL;
    }
    *params = next;
    return ptr;
}

/**
 ****************************************************************************************
 * @brief Convert string to upper case
 *
 * @param[in, out] str   String to convert
 * @param[in]      stop  Where to stop conversion (if NULL convert the whole string)
 ****************************************************************************************
 */
static void fhost_ipc_upper(char *str, char *stop)
{
    char *ptr = str;
    char c;

    if (stop)
    {
        c = *stop;
        *stop = 0;
    }

    while (*ptr)
    {
        if ((*ptr >= 'a') && (*ptr <= 'z'))
            *ptr -= 'a' - 'A';
        ptr++;
    }

    if (stop)
        *stop = c;
}

/**
 ****************************************************************************************
 * @brief Convert string containing ip address
 *
 * The string may should be of the form a.b.c.d/e (/e being optional)
 *
 * @param[in]  str   String to parse
 * @param[out] ip    Updated with the numerical value of the ip address
 * @param[out] mask  Updated with the numerical value of the network mask
 *                   (or 32 if not present)
 * @return 0 if string contained what looks like a valid ip address and -1 otherwise
 ****************************************************************************************
 */
static int fhost_ipc_parse_ip4(char *str, uint32_t *ip, uint32_t *mask)
{
    char *token;
    uint32_t a, i, j;

    #define check_is_num(_str)  for (j = 0 ; j < strlen(_str); j++) {  \
            if (_str[j] < '0' || _str[j] > '9')                        \
                return -1;                                             \
        }

    // Check if mask is present
    token = strchr(str, '/');
    if (token && mask) {
        *token++ = '\0';
        check_is_num(token);
        a = atoi(token);
        if (a == 0 || a > 32)
            return -1;
        *mask = (1<<a) - 1;
    }
    else if (mask)
    {
        *mask = 0xffffffff;
    }

    // parse the ip part
    *ip = 0;
    for (i = 0; i < 4; i ++)
    {
        if (i < 3)
        {
            token = strchr(str, '.');
            if (!token)
                return -1;
            *token++ = '\0';
        }
        check_is_num(str);
        a = atoi(str);
        if (a > 255)
            return -1;
        str = token;
        *ip += (a << (i * 8));
    }

    return 0;
}

/**
 ****************************************************************************************
 * @brief Convert string containing MAC address
 *
 * The string may should be of the form xx:xx:xx:xx:xx:xx
 *
 * @param[in]  str   String to parse
 * @param[out] addr  Updated with MAC address
 * @return 0 if string contained what looks like a valid MAC address and -1 otherwise
 ****************************************************************************************
 */
int fhost_ipc_parse_mac_addr(char *str, struct mac_addr *addr)
{
    char *ptr = str;
    uint32_t i;

    if (!str || strlen(str) < 17 || !addr)
        return -1;

    for (i = 0 ; i < 6 ; i++)
    {
        char *next;
        long int hex = strtol(ptr, &next, 16);
        if (((unsigned)hex > 255) || ((hex == 0) && (next == ptr)) ||
            ((i < 5) && (*next != ':')) ||
            ((i == 5) && (*next != '\0')))
            return -1;

        ((uint8_t *)addr)[i] = (uint8_t)hex;
        ptr = ++next;
    }

    return 0;
}

/**
 ****************************************************************************************
 * @brief print AKM suite name
 *
 * @param[in] suite  AKM suite value
 * @param[in] sep    Whether to print separator first
 ****************************************************************************************
 */
void fhost_ipc_print_akm(enum mac_akm_suite suite, bool sep)
{
    if (sep)
        fhost_print(RTOS_TASK_NULL, "-");

    switch (suite)
    {
        case MAC_AKM_8021X:
            fhost_print(RTOS_TASK_NULL, "8021X");
            break;
        case MAC_AKM_PSK:
            fhost_print(RTOS_TASK_NULL, "PSK");
            break;
        case MAC_AKM_FT_8021X:
            fhost_print(RTOS_TASK_NULL, "FT_8021X");
            break;
        case MAC_AKM_FT_PSK:
            fhost_print(RTOS_TASK_NULL, "FT_PSK");
            break;
        case MAC_AKM_8021X_SHA256:
            fhost_print(RTOS_TASK_NULL, "8021X_SHA256");
            break;
        case MAC_AKM_PSK_SHA256:
            fhost_print(RTOS_TASK_NULL, "PSK_SHA256");
            break;
        case MAC_AKM_TDLS:
            fhost_print(RTOS_TASK_NULL, "TDLS");
            break;
        case MAC_AKM_SAE:
            fhost_print(RTOS_TASK_NULL, "SAE");
            break;
        case MAC_AKM_FT_OVER_SAE:
            fhost_print(RTOS_TASK_NULL, "FT_OVER_SAE");
            break;
        case MAC_AKM_8021X_SUITE_B:
            fhost_print(RTOS_TASK_NULL, "8021X_SUITE_B");
            break;
        case MAC_AKM_8021X_SUITE_B_192:
            fhost_print(RTOS_TASK_NULL, "8021X_SUITE_B_192");
            break;
        case MAC_AKM_FILS_SHA256:
            fhost_print(RTOS_TASK_NULL, "FILS_SHA256");
            break;
        case MAC_AKM_FILS_SHA384:
            fhost_print(RTOS_TASK_NULL, "FILS_SHA384");
            break;
        case MAC_AKM_FT_FILS_SHA256:
            fhost_print(RTOS_TASK_NULL, "FT_FILS_SHA256");
            break;
        case MAC_AKM_FT_FILS_SHA384:
            fhost_print(RTOS_TASK_NULL, "FT_FILS_SHA384");
            break;
        case MAC_AKM_OWE:
            fhost_print(RTOS_TASK_NULL, "OWE");
            break;
        default:
            break;
    }
}

/**
 ****************************************************************************************
 * @brief print Cipher suite name
 *
 * @param[in] suite  Cipher suite value
 * @param[in] sep    Whether to print separator first
 ****************************************************************************************
 */
void fhost_ipc_print_cipher(enum mac_cipher_suite suite, bool sep)
{
    if (sep)
        fhost_print(RTOS_TASK_NULL, "-");

    switch (suite)
    {
        case MAC_CIPHER_WEP40:
            fhost_print(RTOS_TASK_NULL, "WEP40");
            break;
        case MAC_CIPHER_TKIP:
            fhost_print(RTOS_TASK_NULL, "TKIP");
            break;
        case MAC_CIPHER_CCMP:
            fhost_print(RTOS_TASK_NULL, "CCMP");
            break;
        case MAC_CIPHER_WEP104:
            fhost_print(RTOS_TASK_NULL, "WEP104");
            break;
        case MAC_CIPHER_WPI_SMS4:
            fhost_print(RTOS_TASK_NULL, "WPI_SMS4");
            break;
        case MAC_CIPHER_BIP_CMAC_128:
            fhost_print(RTOS_TASK_NULL, "BIP_CMAC_128");
            break;
        case MAC_CIPHER_GCMP_128:
            fhost_print(RTOS_TASK_NULL, "GCMP_128");
            break;
        case MAC_CIPHER_GCMP_256:
            fhost_print(RTOS_TASK_NULL, "GCMP_256");
            break;
        case MAC_CIPHER_CCMP_256:
            fhost_print(RTOS_TASK_NULL, "CCMP_256");
            break;
        case MAC_CIPHER_BIP_GMAC_128:
            fhost_print(RTOS_TASK_NULL, "BIP_GMAC_128");
            break;
        case MAC_CIPHER_BIP_GMAC_256:
            fhost_print(RTOS_TASK_NULL, "BIP_GMAC_256");
            break;
        case MAC_CIPHER_BIP_CMAC_256:
            fhost_print(RTOS_TASK_NULL, "BIP_CMAC_256");
            break;
        default:
            break;
    }
}

/**
 ****************************************************************************************
 * @brief read AKM from string and convert it to bitfield
 *
 * Several AKM can be present in the string, separated by coma.
 * Invalid AKM are ignored.
 *
 * @param[in]  akm_str  String containing AKM list
 * @param[out] akm      Bitfield of AKM read
 * @param[in]  show_error  Print error message if invalid cipher found
 *
 * @return number of invalid AKM found
 ****************************************************************************************
 */
int fhost_ipc_read_akm(char *akm_str, uint32_t *akm, bool show_error)
{
    char *next_akm, *cur_akm = akm_str;
    int ret = 0;

    fhost_ipc_upper(akm_str, NULL);
    next_akm = strchr(cur_akm, ',');

    *akm = 0;
    while (cur_akm)
    {
        if (strncmp(cur_akm, "OPEN", 4) == 0)
        {
            *akm |= CO_BIT(MAC_AKM_NONE);
        }
        else if (strncmp(cur_akm, "WEP", 3) == 0)
        {
            *akm |= CO_BIT(MAC_AKM_PRE_RSN);
        }
        else if ((strncmp(cur_akm, "RSN", 3) == 0) ||
                 (strncmp(cur_akm, "WPA2", 4) == 0))
        {
            *akm |= CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_PSK_SHA256);
        }
        else if ((strncmp(cur_akm, "SAE", 3) == 0) ||
                 (strncmp(cur_akm, "WPA3", 4) == 0))
        {
            *akm |= CO_BIT(MAC_AKM_SAE);
        }
        else if (strncmp(cur_akm, "WPA", 4) == 0)
        {
            *akm |= CO_BIT(MAC_AKM_PRE_RSN) | CO_BIT(MAC_AKM_PSK);
        }
        else
        {
            ret++;
        }

        cur_akm = next_akm;
        if (cur_akm)
        {
            cur_akm++; //skip coma
            next_akm = strchr(cur_akm, ',');
        }
    }

    if (ret && show_error)
    {
        fhost_print(RTOS_TASK_NULL, "[%s] Only the following AKM are supported:\n"
                    "OPEN: For open AP\n"
#if 0 // not supported now
                    "WEP: For AP with WEP security\n"
#endif
                    "WPA: For AP with WPA/PSK security (pre WPA2)\n"
                    "RSN (or WPA2): For AP with WPA2/PSK security\n"
                    "SAE (or WPA3): For AP with WPA3/PSK security\n"
                    , akm_str);
    }
    return ret;
}

/**
 ****************************************************************************************
 * @brief read Cipher from string and convert it to bitfield
 *
 * Several Ciphers can be present in the string, separated by coma.
 * Invalid Cipher are ignored.
 *
 * @param[in]  cipher_str  String containing Cipher list
 * @param[out] cipher      Bitfield of Cipher read
 * @param[in]  show_error  Print error message if invalid cipher found
 *
 * @return number of invalid Cipher found
 ****************************************************************************************
 */
static int fhost_ipc_read_cipher(char *cipher_str, uint32_t *cipher, bool show_error)
{
    char *next_cipher, *cur_cipher = cipher_str;
    int ret = 0;

    fhost_ipc_upper(cipher_str, NULL);
    next_cipher = strchr(cur_cipher, ',');

    *cipher = 0;
    while (cur_cipher)
    {
        if (strncmp(cur_cipher, "CCMP-256", 8) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_CCMP_256);
        }
        else if (strncmp(cur_cipher, "CCMP", 4) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_CCMP);
        }
        else if (strncmp(cur_cipher, "TKIP", 4) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_TKIP);
        }
        else if (strncmp(cur_cipher, "WEP40", 5) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_WEP40);
        }
        else if (strncmp(cur_cipher, "WEP104", 6) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_WEP104);
        }
        else if (strncmp(cur_cipher, "SMS4", 4) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_WPI_SMS4);
        }
        else if (strncmp(cur_cipher, "GCMP-256", 8) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_GCMP_256);
        }
        else if (strncmp(cur_cipher, "GCMP", 4) == 0)
        {
            *cipher |= CO_BIT(MAC_CIPHER_GCMP_128);
        }
        else
        {
            ret++;
        }

        cur_cipher = next_cipher;
        if (cur_cipher)
        {
            cur_cipher++; //skip coma
            next_cipher = strchr(cur_cipher, ',');
        }
    }

    if (ret && show_error)
    {
        fhost_print(RTOS_TASK_NULL, "[%s] Only the following cipher suites are supported:\n"
                    "CCMP, TKIP, WEP40, WEP104, SMS4, CCMP-256, GCMP, GCMP-256", cipher_str);
    }

    return ret;
}

#if NX_BEACONING
/**
 ****************************************************************************************
 * @brief read AP operation mode and convert it
 *
 * @param[in]  mode_str    String containing Operation mode
 * @param[out] mode        Mode read
 * @param[in]  show_error  Print error message if invalid mode found
 *
 * @return 0 on success and -1 otherwise
 ****************************************************************************************
 */
static int fhost_ipc_read_mode(char *mode_str, enum fhost_ap_mode *mode, bool show_error)
{
    int ret = 0;

    fhost_ipc_upper(mode_str, NULL);

    if (strncmp(mode_str, "HT", 2) == 0)
    {
        *mode = FHOST_AP_HT;
    }
    else if (strncmp(mode_str, "VHT", 3) == 0)
    {
        *mode = FHOST_AP_VHT;
    }
    else if (strncmp(mode_str, "HE", 2) == 0)
    {
        *mode = FHOST_AP_HE;
    }
    else if (strncmp(mode_str, "NON-HT", 6) == 0)
    {
        *mode = FHOST_AP_NON_HT;
    }
    else if (strncmp(mode_str, "AUTO", 4) == 0)
    {
        *mode = FHOST_AP_AUTO;
    }
    else
    {
        if (show_error)
            fhost_print(RTOS_TASK_NULL, "[%s] Only the following mode are supported:\n"
                        "AUTO, NON-HT(11g/11a), HT(11n), VHT(11ac), HE(11ax)\n", mode_str);
        ret = -1;
    }
    return ret;
}
#endif // NX_BEACONING

/**
 ****************************************************************************************
 * @brief Find the interface index
 *
 * @param[in] name Name of the interace
 * @return Index of the FHOST wifi interface or -1 if interface doesn't exist.
 ****************************************************************************************
 */
int fhost_search_itf(char *name)
{
    int fhost_vif_idx = fhost_vif_idx_from_name(name);

    if (fhost_vif_idx >= 0)
        return fhost_vif_idx;

    fhost_print(RTOS_TASK_NULL, "Cannot find interface %s\n", name);
    return -1;
 }

/**
 ****************************************************************************************
 * @brief Find the first valid network interface.
 *
 * @return Index of the FHOST wifi interface.
 ****************************************************************************************
 */
int fhost_search_first_valid_itf()
{
    struct fhost_status status;
    fhost_get_status(&status);

    if (status.vif_active_cnt)
        return status.vif_first_active;

    fhost_print(RTOS_TASK_NULL, "No active interface\n");
    return -1;
 }

#if NX_FHOST_MONITOR
/**
 ****************************************************************************************
 * @brief callback function
 *
 * Extract received packet informations (frame length, type, mac addr ...) in monitor mode
 *
 * @param[in] info  RX Frame information.
 * @param[in] arg   Not used
 ****************************************************************************************
 */
static void fhost_monitor_cb(struct fhost_frame_info *info, void *arg)
{
    if (info->payload == NULL)
    {
        TRACE_APP(INF, "Unsupported frame: length = %d", info->length)
        printf("Unsupported frame: length = %dr\r\n", info->length);
    }
    else
    {
        struct mac_hdr *hdr __MAYBE_UNUSED = (struct mac_hdr *)info->payload;
        TRACE_APP(INF, "%pM %pM %pM %fc SN:%d length = %d", TR_MAC(hdr->addr1.array),
                  TR_MAC(hdr->addr2.array), TR_MAC(hdr->addr3.array), hdr->fctl, hdr->seq >> 4,
                  info->length);
        printf("SN:%d length = %d\r\n", hdr->seq >> 4, info->length);
    }
}
#endif // NX_FHOST_MONITOR

/**
 ****************************************************************************************
 * @brief Search a fhost_task_msg_id in task_handle_msg_id by RTOS task handle
 *
 * @param[in] handle            RTOS task handle
 *
 * @return pointer to fhost_task_msg_id on success, NULL if error.
 ****************************************************************************************
 */
static struct fhost_task_msg_id *fhost_search_task_hdl_msg(rtos_task_handle handle)
{
    int i;
    struct fhost_task_msg_id *ret = NULL;

    for (i = 0; i < FHOST_TASK_HANDLE_LEN; i++)
    {
        if (task_handle_msg_id[i].task_handle == handle)
            ret = &task_handle_msg_id[i];
    }

    return ret;
}

/**
 ****************************************************************************************
 * @brief Search a fhost_task_msg_id in task_handle_msg_id by message ID
 *
 * @param[in] msg_id           Message ID
 *
 * @return pointer to fhost_task_msg_id on success, NULL if error.
 ****************************************************************************************
 */
static struct fhost_task_msg_id *fhost_search_task_msg_id(uint32_t msg_id)
{
    int i;
    struct fhost_task_msg_id *ret = NULL;

    for (i = 0; i < FHOST_TASK_HANDLE_LEN; i++)
    {
        if (task_handle_msg_id[i].msg_id == msg_id)
            ret = &task_handle_msg_id[i];
    }

    return ret;
}

/**
 ****************************************************************************************
 * @brief Search a free fhost_task_msg_id in task_handle_msg_id
 *
 * @return pointer to fhost_task_msg_id on success, NULL if error.
 ****************************************************************************************
 */
static struct fhost_task_msg_id *fhost_search_free_task_hdl_msg()
{
    return fhost_search_task_hdl_msg(RTOS_TASK_NULL);
}

/**
 ****************************************************************************************
 * @brief Search one connected station based on the mac address
 *
 * @param[in] addr      MAC Address of the connected STA
 * @return pointer to sta_info_tag on success, NULL if error.
 ****************************************************************************************
 */
static void *fhost_search_connected_sta_by_mac(struct mac_addr *addr)
{
    int i;
    struct fhost_vif_tag *fhost_vif;

    for (i = 0; i < NX_VIRT_DEV_MAX; i++)
    {
        void *sta;
        if (fhost_env.vif[i].mac_vif == NULL)
            continue;

        fhost_vif = &fhost_env.vif[i];
        sta = co_list_pick(mac_vif_get_sta_list(fhost_vif->mac_vif));

        while (sta != NULL)
        {
            if (MAC_ADDR_CMP(sta_getp_mac_addr(sta), addr))
                return sta;

            sta = sta_get_list_next(sta);
        }
    }
    return NULL;
}

/**
 ****************************************************************************************
 * @brief Start an ipc task saving the task handle.
 *
 * @param[in] msg_id            Message ID
 * @param[in] start_task_func   Function that starts the ipc task
 * @param[in] start_task_args   Argument pointer to pass to start_task_func
 * @param[in] sigkill_func      Callback for SIGKILL signal
 *
 * @return FHOST_IPC_NO_RESP if successful, FHOST_IPC_ERROR otherwise.
 ****************************************************************************************
 */
static int fhost_ipc_start_task(int msg_id, fhost_ipc_start_task_cb start_task_func,
                                void *start_task_args, fhost_ipc_sigkill_cb sigkill_func)
{
    struct fhost_task_msg_id *task_hdl_msg_ptr = fhost_search_free_task_hdl_msg();
    if (!task_hdl_msg_ptr)
        return FHOST_IPC_ERROR;

    if ((task_hdl_msg_ptr->task_handle = start_task_func(start_task_args)))
    {
        task_hdl_msg_ptr->msg_id = msg_id;
        task_hdl_msg_ptr->sigkill_cb = sigkill_func;
    }
    else
    {
        return FHOST_IPC_ERROR;
    }

    return FHOST_IPC_NO_RESP;
}

/**
 ****************************************************************************************
 * @brief Process function for 'list_cmd' command
 *
 * Simply list the commands sending a print message.
 *
 * @param[in] params Not used
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_list(char *params)
{
    uint8_t i;

    fhost_print(RTOS_TASK_NULL, "====Fhost Commands====\r\n");
    for (i = 0; ipc_commands[i].exec != NULL; i++)
    {
        fhost_print(RTOS_TASK_NULL, "%-25s: %s\r\n",
                    ipc_commands[i].name,ipc_commands[i].params);
    }

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'status' command
 *
 * @param[in] params Optional command parameter
 *                   Valid parameter: "chan", "vif"
 *
 * @return FHOST_IPC_SUCCESS on success and FHOST_IPC_ERROR if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_status(char *params)
{
    struct fhost_status status;
    bool show_vif = false, show_chan = false;
    char *token;
    int i;

    token = fhost_ipc_next_token(&params);
    while (token)
    {
        if (!strncmp("vif", token, 3))
            show_vif = true;
        else if (!strncmp("chan", token, 4))
            show_chan = true;
        else
            fhost_print(RTOS_TASK_NULL, "Ignore invalid parameter %s\n", token);
        token = fhost_ipc_next_token(&params);
    }
    if (!show_chan && !show_vif)
        show_vif = true;

    fhost_get_status(&status);

    if (show_chan)
    {
        const struct mac_chan_def *chan_def;
        fhost_print(RTOS_TASK_NULL, "Available Channels:\n");

        for (chan_def = &(status.chan_2g4[0]), i = 0;
             i < (status.chan_2g4_cnt + status.chan_5g_cnt);
             chan_def++, i++)
        {
            if (i == status.chan_2g4_cnt)
                chan_def = &(status.chan_5g[0]);

            fhost_print(RTOS_TASK_NULL, "ch: %3d, freq: %dMhz, TX pwr: %2d dBm, flags: %s%s\n",
                        phy_freq_to_channel(chan_def->band, chan_def->freq),
                        chan_def->freq, chan_def->tx_power,
                        chan_def->flags & CHAN_NO_IR ? "[NO_IR]" : "",
                        chan_def->flags & CHAN_DISABLED ? "[DISABLED]" : "");
        }
        fhost_print(RTOS_TASK_NULL, "\n");
    }

    if (show_vif)
    {
        for (i = 0; i < status.vif_max_cnt; i++)
        {
            struct fhost_vif_status vif_status;
            char vif_name[4];
            fhost_get_vif_status(i, &vif_status);
            memset(vif_name, 0, sizeof(vif_name));
            fhost_vif_name(i, vif_name, sizeof(vif_name) - 1);
            fhost_print(RTOS_TASK_NULL, "%s: VIF[%d] : ", vif_name, i);
            switch (vif_status.type)
            {
                case VIF_STA:
                    fhost_print(RTOS_TASK_NULL, "STA");
                    break;
                case VIF_AP:
                    fhost_print(RTOS_TASK_NULL, "AP");
                    break;
                case VIF_MONITOR:
                    fhost_print(RTOS_TASK_NULL, "MONITOR");
                    break;
                case VIF_MESH_POINT:
                    fhost_print(RTOS_TASK_NULL, "MESH");
                    break;
                default:
                    fhost_print(RTOS_TASK_NULL, "INACTIVE");
                    break;
            }
            fhost_print(RTOS_TASK_NULL, ", MAC: %02x:%02x:%02x:%02x:%02x:%02x, ",
                        vif_status.mac_addr[0], vif_status.mac_addr[1],
                        vif_status.mac_addr[2], vif_status.mac_addr[3],
                        vif_status.mac_addr[4], vif_status.mac_addr[5]);
            if (vif_status.type != VIF_UNKNOWN)
            {
                if (vif_status.chan.prim20_freq != 0)
                {
                    fhost_print(RTOS_TASK_NULL, "Operating Channel: %dMHz@%dMHz\n",
                                vif_status.chan.prim20_freq,
                                (1 << vif_status.chan.type) * 20);
                }
                else
                {
                    fhost_print(RTOS_TASK_NULL, " No Operating Channel\n");
                }
            } else {
                fhost_print(RTOS_TASK_NULL, "\n");
            }
        }
    }

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'wpa_supplicant' command
 *
 * Create a wpa supplicant task
 *
 * @param[in] params Not used
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_wpa(char *params)
{
    char *token, *next = params;
    int fhost_vif_idx = -1;

    if ((token = fhost_ipc_next_token(&next)))
    {
        if (!strcmp("-i", token))
        {
            token = fhost_ipc_next_token(&next);
            fhost_vif_idx = fhost_search_itf(token);
            if (fhost_vif_idx < 0)
                return FHOST_IPC_ERROR;
        }
    }
    else
    {
        fhost_vif_idx = fhost_search_first_valid_itf();
        if (fhost_vif_idx < 0)
            return FHOST_IPC_ERROR;
    }

    if (fhost_wpa_add_vif(fhost_vif_idx))
        return FHOST_IPC_ERROR;

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'wpa_cli' command
 *
 * Send a command to the wpa_supplicant task. The task must have been started first with
 * 'wpa_supplicant' command.
 *
 * @param[in] params Command to send to wpa_supplicant task
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_wpa_cli(char *params)
{
    int res, resp_len, fhost_vif_idx;
    char *token, *next = params;
    char *resp, *cmd;

    if (strstr(params, "-i"))
    {
        // skip "-i"
        token = fhost_ipc_next_token(&next);
        token = fhost_ipc_next_token(&next);

        fhost_vif_idx = fhost_search_itf(token);
    }
    else
    {
        fhost_vif_idx = fhost_search_first_valid_itf();
    }

    if (fhost_vif_idx < 0)
        return FHOST_IPC_ERROR;

    if (!next)
        return FHOST_IPC_SHOW_USAGE;

    // convert command to upper case
    cmd = next;
    fhost_ipc_upper(cmd, strchr(cmd, ' '));

    // for backward compatibilty (mainly for MVT)
    if (strncmp(cmd, "TERMINATE", 9) == 0)
    {
        if (fhost_wpa_remove_vif(fhost_vif_idx))
            res = FHOST_IPC_ERROR;
        else
            res = FHOST_IPC_SUCCESS;
    }
    else
    {
        resp_len = 2048;
        resp = rtos_malloc(resp_len + 1);
        if (!resp)
            return FHOST_IPC_ERROR;

        res = fhost_wpa_execute_cmd(fhost_vif_idx, resp, &resp_len, 10000, cmd);
        if (res >= 0)
        {
            resp[resp_len] = '\0';
            fhost_print_buf(RTOS_TASK_NULL, resp);
            if (res > 0)
                res = FHOST_IPC_ERROR;
            else
                res = FHOST_IPC_SUCCESS;
        }
        else
        {
            res = FHOST_IPC_ERROR;
        }

        rtos_free(resp);
    }

    return res;
}

/**
 ****************************************************************************************
 * @brief Process function for 'mactrace' command
 *
 * Send fw command to trigger dump.
 *
 * @param[in] params Error name
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_mactrace(char *params)
{
#if 0
//move to mac
    struct mm_dbg_trigger_req req;

    if (params)
    {
        size_t len = strlen(params);
        if (len > sizeof(req.error))
            len = (sizeof(req.error) - 1);
        memcpy(req.error, params, len);
        req.error[len] = 0;
    }
    else
    {
        strcpy(req.error, "Force trigger");
    }

    // Wait until fw actually start the dump to complete the command
    task_waiting_error_ind = rtos_get_task_handle();

    if (macif_kmsg_push(MM_DBG_TRIGGER_REQ, TASK_MM, &req, sizeof(req)) ||
        !rtos_task_wait_notification(1000))
    {
        task_waiting_error_ind = NULL;
        return FHOST_IPC_ERROR;
    }

#endif
    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'free' command
 *
 * Provides information about memory usage.
 *
 * @param[in] params Not used
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_free(char *params)
{
    int total, used, free, max_used;

    rtos_heap_info(&total, &free, &max_used);
    used = total - free;
    max_used = total - max_used;

    fhost_print(RTOS_TASK_NULL, "RTOS HEAP: free=%d used=%d max_used=%d/%d\n",
                free, used, max_used, total);

    return FHOST_IPC_SUCCESS;
}

#if NX_IPERF
/**
 ****************************************************************************************
 * @brief Process function for 'iperf' command
 *
 * Start an iperf server on the specified port.
 *
 * @param[in] params parameters passed to iperf command
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
int fhost_ipc_iperf(char *params)
{
    char conv, *token, *substr, *next = params;
    struct fhost_iperf_settings iperf_settings;
    bool client_server_set = 0;
    struct fhost_task_msg_id *task_msg_id;
    int ret;

    token = fhost_ipc_next_token(&next);

    if (!token) {
        goto help;
    }
    if (!strcmp("stop", token))
    {
        task_msg_id = fhost_search_task_hdl_msg(fhost_iperf_msg_handle_get());
        if (task_msg_id->task_handle)
        {
            ret = task_msg_id->sigkill_cb(task_msg_id->task_handle);
            task_msg_id = RTOS_TASK_NULL;
            return ret;
        }

        return FHOST_IPC_SUCCESS;
    }

    fhost_iperf_settings_init(&iperf_settings);

    do
    {
        if (token[0] != '-')
            return FHOST_IPC_SHOW_USAGE;

        //TODO[AAL]: Add support long options

        switch(token[1])
        {
            case ('b'): // UDP bandwidth
            case ('w'): // TCP window size
            {
                char *decimal_str;
                int decimal = 0;
                char opt = token[1];
                uint64_t value;

                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }

                decimal_str = strchr(token, '.');
                if (decimal_str)
                {
                    int fact = 100;
                    decimal_str++;
                    while (*decimal_str >= '0' && *decimal_str <= '9')
                    {
                        decimal += (*decimal_str - '0') * fact;
                        if (fact == 1)
                            break;
                        fact = fact / 10;
                        decimal_str++;
                    }
                }

                value = atoi(token);
                conv = token[strlen(token) - 1];

                // convert according to [Gg Mm Kk]
                switch (conv)
                {
                    case 'G':
                    case 'g':
                        value *= 1000000000;
                        value += decimal * 1000000;
                        break;
                    case 'M':
                    case 'm':
                        value *= 1000000;
                        value += decimal * 1000;
                        break;
                    case 'K':
                    case 'k':
                        value *= 1000;
                        value += decimal;
                        break;
                    default:
                        break;
                }

                if (opt == 'b')
                {
                    iperf_settings.udprate = value;
                    iperf_settings.flags.is_udp = true;
                    iperf_settings.flags.is_bw_set = true;
                    // if -l has already been processed, is_buf_len_set is true so don't overwrite that value.
                    if (!iperf_settings.flags.is_buf_len_set)
                        iperf_settings.buf_len = FHOST_IPERF_DEFAULT_UDPBUFLEN;
                }
                else
                {
                    // TCP window is ignored for now
                }
                break;
            }
            case ('c'): // Client mode with server host to connect to
            {
                if (client_server_set)
                    return FHOST_IPC_SHOW_USAGE;

                iperf_settings.flags.is_server = 0;
                client_server_set = true;

                if (!(token = fhost_ipc_next_token(&next)))
                    return FHOST_IPC_SHOW_USAGE;

                if (fhost_ipc_parse_ip4(token, &iperf_settings.host_ip, NULL))
                {
                    fhost_print(RTOS_TASK_NULL, "invalid IP address %s\n", token);
                    return FHOST_IPC_ERROR;
                }
                break;
            }
            case ('f'): // format to print in
            {
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }
                iperf_settings.format = token[0];
                break;
            }
            case ('i'): // Interval between periodic reports
            {
                uint32_t interval = 0;
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }

                substr = strchr(token, '.');

                if (substr)
                {
                    *substr++ = '\0';
                    interval += atoi(substr);
                }

                interval += atoi(token) * 10;
                if (interval < 5)
                {
                    fhost_print(RTOS_TASK_NULL, "interval must be greater than or "
                                "equal to 0.5. Interval set to 0.5\n");
                    interval = 5;
                }

                iperf_settings.interval.sec = interval / 10;
                iperf_settings.interval.usec = 100000 * (interval - (iperf_settings.interval.sec * 10));
                iperf_settings.flags.show_int_stats = true;
                break;
            }
            case ('l'): //Length of each buffer
            {
                uint32_t udp_min_size = sizeof(struct iperf_UDP_datagram);

                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }

                iperf_settings.buf_len = atoi( token );
                iperf_settings.flags.is_buf_len_set = true;
                if (iperf_settings.flags.is_udp && iperf_settings.buf_len < udp_min_size)
                {
                    iperf_settings.buf_len = udp_min_size;
                    fhost_print(RTOS_TASK_NULL, "buffer length must be greater than or "
                                "equal to %d in UDP\n", udp_min_size);
                }
                break;
            }
            case ('n'): // amount mode (instead of time mode)
            {
                iperf_settings.flags.is_time_mode = false;
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }
                iperf_settings.amount = atoi( token );
                break;
            }
            case ('p'): //server port
            {
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }
                iperf_settings.port = atoi( token );
                break;
            }
            case ('s'): // server mode
            {
                if (client_server_set)
                    return FHOST_IPC_SHOW_USAGE;
                iperf_settings.flags.is_server = 1;
                client_server_set = true;
                break;
            }
            case ('t'): // time mode (instead of amount mode)
            {
                iperf_settings.flags.is_time_mode = true;
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }
                iperf_settings.amount = 0;
                substr = strchr(token, '.');
                if (substr)
                {
                    *substr++ = '\0';
                    iperf_settings.amount += atoi(substr);
                }

                iperf_settings.amount += atoi(token) * 10;
                break;
            }
            case ('u'): // UDP instead of TCP
            {
                // if -b has already been processed, UDP rate will be non-zero, so don't overwrite that value
                if (!iperf_settings.flags.is_udp)
                {
                    iperf_settings.flags.is_udp = true;
                    iperf_settings.udprate = FHOST_IPERF_DEFAULT_UDPRATE;
                }

                // if -l has already been processed, is_buf_len_set is true, so don't overwrite that value.
                if (!iperf_settings.flags.is_buf_len_set)
                {
                    iperf_settings.buf_len = FHOST_IPERF_DEFAULT_UDPBUFLEN;
                }
                break;
            }
            case ('S'): // IP type-of-service
            {
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }
                // the zero base allows the user to specify
                // hexadecimals: "0x#"
                // octals: "0#"
                // decimal numbers: "#"
                iperf_settings.tos = strtol( token, NULL, 0 );
                break;
            }
            case ('T'): // TTL
            {
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }
                iperf_settings.ttl = atoi(token);
                break;
            }
            case ('X'): // Peer version detect
            {
                iperf_settings.flags.is_peer_ver = true;
                break;
            }
            case ('I'): // vif num
            {
                uint16_t vif_num;
                if (!(token = fhost_ipc_next_token(&next)))
                {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }

                vif_num = atoi(token);
                if (vif_num == MGMR_VIF_STA) {
                    iperf_settings.vif_num = MGMR_VIF_STA;
                } else if (vif_num == MGMR_VIF_AP) {
                    iperf_settings.vif_num = MGMR_VIF_AP;
                } else {
                    fhost_print(RTOS_TASK_NULL, iperf_long_help);
                    return FHOST_IPC_ERROR;
                }
                break;
            }
            case ('h'): // Long Help
            {
help:
                fhost_print(RTOS_TASK_NULL, iperf_long_help);
                return FHOST_IPC_SUCCESS;
            }
            default:
            {
                return FHOST_IPC_SHOW_USAGE;
            }
        }
    } while ((token = fhost_ipc_next_token(&next)));

    if (!client_server_set)
        return FHOST_IPC_SHOW_USAGE;

    return fhost_ipc_start_task(ipc_msg.id, fhost_iperf_start, &iperf_settings, fhost_iperf_sigkill_handler);
}
#endif // NX_IPERF


/**
 ****************************************************************************************
 * @brief Process function for 'sigkill' command
 *
   @verbatim
    sigkill <cmd_id> is used to stop a running command
   @endverbatim
 *
 * @param[in] params command id
 *
 * @return FHOST_IPC_SUCCESS on success and FHOST_IPC_ERROR if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_sigkill(char *params)
{
    struct fhost_task_msg_id *task_msg_id;
    char *token, *next = params;
    int msg_id, ret;

    token = fhost_ipc_next_token(&next);
    if (!token)
        return FHOST_IPC_SHOW_USAGE;

    msg_id = atoi(token);

    task_msg_id = fhost_search_task_msg_id(msg_id);
    if (task_msg_id->task_handle)
    {
        ret = task_msg_id->sigkill_cb(task_msg_id->task_handle);
        task_msg_id = RTOS_TASK_NULL;
        return ret;
    }

    return FHOST_IPC_ERROR;
}

#if 0
void update_arp_entry(void)
{
    ip4_addr_t adrs;
    struct eth_addr test_ethaddr3 = {{0x7c,0x8a,0xe1,0x6f,0xa8,0x28}};

    IP4_ADDR(&adrs, 10,28,31,200);
    etharp_add_static_entry(&adrs, &test_ethaddr3);
}
#endif

/**
 ****************************************************************************************
 * @brief Process function for 'ip' command
 *
 * ip command can be used for.
 * - address managing:
   @verbatim
      ip addr add <ip>/<mask> [gw] <itf-name>
      ip addr del <itf-name>
      ip addr [show [itf-name]]
   @endverbatim
 *
 * @param[in] params Port number
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_ip(char *params)
{
    char *token, *next = params;
    int fhost_vif_idx = -1;

    //update_arp_entry();
    token = fhost_ipc_next_token(&next);
    if (!strcmp("addr", token))
    {
        token = fhost_ipc_next_token(&next);
        if (token && !strcmp("add", token))
        {
            struct fhost_vif_ip_addr_cfg ip_cfg;
            char *itf, *ip_str, *gw_str;

            memset(&ip_cfg, 0, sizeof(ip_cfg));
            ip_str = fhost_ipc_next_token(&next);
            gw_str = fhost_ipc_next_token(&next);
            itf = fhost_ipc_next_token(&next);
            if (!itf)
            {
                itf = gw_str;
                gw_str = NULL;
            }

            fhost_vif_idx = fhost_search_itf(itf);
            if (fhost_vif_idx < 0)
                return FHOST_IPC_ERROR;

            ip_cfg.mode = IP_ADDR_STATIC_IPV4;
            if (fhost_ipc_parse_ip4(ip_str, &ip_cfg.ipv4.addr, &ip_cfg.ipv4.mask) != 0)
                return FHOST_IPC_ERROR;

            if (gw_str && fhost_ipc_parse_ip4(gw_str, &ip_cfg.ipv4.gw, NULL))
                return FHOST_IPC_ERROR;

            if (fhost_set_vif_ip(fhost_vif_idx, &ip_cfg))
                return FHOST_IPC_ERROR;
        }
        else if (token && !strcmp("del", token))
        {
            struct fhost_vif_ip_addr_cfg ip_cfg;

            token = fhost_ipc_next_token(&next);
            fhost_vif_idx = fhost_search_itf(token);
            if (fhost_vif_idx < 0)
                return FHOST_IPC_ERROR;

            ip_cfg.mode = IP_ADDR_NONE;
            fhost_set_vif_ip(fhost_vif_idx, &ip_cfg);
        }
        else
        {
            int i;

            if (token && !strcmp("show", token))
                token = fhost_ipc_next_token(&next);

            if (token)
            {
                fhost_vif_idx = fhost_search_itf(token);
                if (fhost_vif_idx < 0)
                    return FHOST_IPC_ERROR;
            }

            for(i = 0 ; i < NX_VIRT_DEV_MAX ; i++)
            {
                if ((fhost_vif_idx < 0) || (i == fhost_vif_idx))
                {
                    char fmt[] = "[%d] %s: MAC=%02x:%02x:%02x:%02x:%02x:%02x ip=%d.%d.%d.%d/%d %s%s\n";
                    char ifname[NET_AL_MAX_IFNAME];
                    char *state, *connected;
                    struct fhost_vif_ip_addr_cfg ip_cfg;
                    const uint8_t *mac_addr;

                    if (fhost_env.vif[i].mac_vif &&
                        (mac_vif_get_type(fhost_env.vif[i].mac_vif) != VIF_UNKNOWN))
                    {
                        state = "UP";
                        if (mac_vif_get_active(fhost_env.vif[i].mac_vif))
                            connected = ",CONNECTED";
                        else
                            connected = "";
                    }
                    else
                    {
                        state = "DOWN";
                        connected = "";
                    }
                    mac_addr = net_if_get_mac_addr(fhost_to_net_if(i));

                    if (fhost_vif_name(i, ifname, NET_AL_MAX_IFNAME) < 0)
                        return FHOST_IPC_ERROR;


                    if (fhost_get_vif_ip(i, &ip_cfg))
                        return FHOST_IPC_ERROR;

                    fhost_print(RTOS_TASK_NULL, fmt, i, ifname,
                                mac_addr[0], mac_addr[1], mac_addr[2],
                                mac_addr[3], mac_addr[4], mac_addr[5],
                                ip_cfg.ipv4.addr & 0xff, (ip_cfg.ipv4.addr >> 8) & 0xff,
                                (ip_cfg.ipv4.addr >> 16) & 0xff, (ip_cfg.ipv4.addr >> 24) & 0xff,
                                32 - co_clz(ip_cfg.ipv4.mask), state, connected);
                }
            }
        }
    }
    else
        return FHOST_IPC_SHOW_USAGE;

    return FHOST_IPC_SUCCESS;
}


/**
 ****************************************************************************************
 * @brief Process function for 'scan' command
 *
 * @param[in] params Scan parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_scan(char *params)
{
    int nb_res, fhost_vif_idx;
    struct mac_scan_result result;

    if (params && strstr(params, "-i"))
    {
        char *token, *next = params;
        struct fhost_vif_status status;

        token = fhost_ipc_next_token(&next);
        token = fhost_ipc_next_token(&next);

        fhost_vif_idx = fhost_search_itf(token);
        if (fhost_get_vif_status(fhost_vif_idx, &status))
            return FHOST_IPC_ERROR;

        if (fhost_get_vif_status(fhost_vif_idx, &status) ||
            ((status.type == VIF_UNKNOWN) &&
             fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_STA, false)))
            return FHOST_IPC_ERROR;
    }
    else
    {
        fhost_vif_idx = fhost_search_first_valid_itf();
    }
    if (fhost_vif_idx < 0)
        return FHOST_IPC_ERROR;

    nb_res = fhost_scan(cntrl_link, fhost_vif_idx);
#if 0
    fhost_print(RTOS_TASK_NULL, "Got %d scan results\n", nb_res);

    nb_res = 0;
    while(fhost_get_scan_results(cntrl_link, nb_res++, 1, &result))
    {
        bool sep = false;
        uint16_t mfp;
        int i;

        if (result.ssid.length < sizeof(result.ssid.array))
            result.ssid.array[result.ssid.length] = '\0';
        else
            result.ssid.array[result.ssid.length - 1] = '\0';

        fhost_print(RTOS_TASK_NULL, "(%d dBm) CH=%3d BSSID=%02x:%02x:%02x:%02x:%02x:%02x SSID=%s ",
                    result.rssi, phy_freq_to_channel(result.chan->band, result.chan->freq),
                    ((uint8_t *)result.bssid.array)[0], ((uint8_t *)result.bssid.array)[1],
                    ((uint8_t *)result.bssid.array)[2], ((uint8_t *)result.bssid.array)[3],
                    ((uint8_t *)result.bssid.array)[4], ((uint8_t *)result.bssid.array)[5],
                    (char *)result.ssid.array);

        if (result.akm & CO_BIT(MAC_AKM_NONE))
        {
            fhost_print(RTOS_TASK_NULL, "[OPEN]\n");
            continue;
        }

        if (result.akm == CO_BIT(MAC_AKM_PRE_RSN))
        {
            fhost_print(RTOS_TASK_NULL, "[WEP]\n");
            continue;
        }

        if (result.akm & CO_BIT(MAC_AKM_WAPI_CERT) ||
            result.akm & CO_BIT(MAC_AKM_WAPI_PSK))
        {
            bool cert = result.akm & CO_BIT(MAC_AKM_WAPI_CERT);
            bool psk = result.akm & CO_BIT(MAC_AKM_WAPI_PSK);
            fhost_print(RTOS_TASK_NULL, "[WAPI:%s%s%s %s/%s]\n",
                        cert ? "CERT" : "",
                        cert && psk ? "-" : "",
                        psk ? "PSK" : "",
                        result.group_cipher == CO_BIT(MAC_CIPHER_WPI_SMS4) ? "SMS4" : "?",
                        result.pairwise_cipher == CO_BIT(MAC_CIPHER_WPI_SMS4) ? "SMS4" : "?");
            continue;
        }

        if (result.akm & CO_BIT(MAC_AKM_PRE_RSN))
            fhost_print(RTOS_TASK_NULL, " [WPA:");
        else
            fhost_print(RTOS_TASK_NULL, " [RSN:");

        for (i = MAC_AKM_8021X ; i <= MAC_AKM_OWE ; i++)
        {
            if (result.akm & CO_BIT(i))
            {
                fhost_ipc_print_akm(i, sep);
                sep = true;
            }
        }

        fhost_print(RTOS_TASK_NULL, " ");
        sep = false;
        for (i = MAC_CIPHER_WEP40 ; i <= MAC_CIPHER_BIP_CMAC_256 ; i++)
        {
            if (result.pairwise_cipher & CO_BIT(i))
            {
                fhost_ipc_print_cipher(i, sep);
                sep = true;
            }
        }
        fhost_print(RTOS_TASK_NULL, "/");

        // remove MFP cipher from group
        mfp = (CO_BIT(MAC_CIPHER_BIP_CMAC_128) | CO_BIT(MAC_CIPHER_BIP_GMAC_128) |
               CO_BIT(MAC_CIPHER_BIP_GMAC_256) | CO_BIT(MAC_CIPHER_BIP_CMAC_256));

        mfp = result.group_cipher & mfp;
        result.group_cipher &= ~(mfp);

        if (result.group_cipher)
            fhost_ipc_print_cipher(31 - co_clz(result.group_cipher), false);
        else
            fhost_print(RTOS_TASK_NULL, "?");

        if (mfp)
        {
            fhost_print(RTOS_TASK_NULL, "][MFP:");
            sep = false;
            for (i = MAC_CIPHER_BIP_CMAC_128 ; i <= MAC_CIPHER_BIP_CMAC_256 ; i++)
            {
                  if (mfp & CO_BIT(i))
                  {
                      fhost_ipc_print_cipher(i, sep);
                      sep = true;
                  }
            }
        }

        fhost_print(RTOS_TASK_NULL, "]\n");
    }
#endif

    return FHOST_IPC_SUCCESS;
}

#if NX_PING
/**
 ****************************************************************************************
 * @brief Process function for 'ping' command
 *
 * Ping command can be used to test the reachability of a host on an IP network.
 *
   @verbatim
   ping <dst_ip> [-s pksize (bytes)] [-r rate (pkt/sec)] [-d duration (sec)] [-Q tos]
   ping stop <id1> [<id2> ... <id8>]
   @endverbatim
 *
 * Note that -s, -r, -d, -t are options for ping command. We could choose any of them to
 * configure. If not configured, it will set the default values at layer net_tg_al.
 *
 * @param[in] params ping command above
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_ping(char *params)
{
    char *token, *next = params;
    uint32_t rip = 0, ret;
    token = fhost_ipc_next_token(&next);
    u32_t rate = 0, pksize = 0, duration = 0, tos = 0;
    struct fhost_task_msg_id *task_hdl_msg_ptr;
    bool background = false;
    struct fhost_ping_task_args args;

    if (!strcmp("stop", token))
    {
        u32_t stream_id;
        struct fhost_ping_stream* ping_stream;
        int ret = FHOST_IPC_SUCCESS;

        token = fhost_ipc_next_token(&next);
        if (!token)
            return FHOST_IPC_SHOW_USAGE;

        stream_id = atoi(token);
        ping_stream = fhost_ping_find_stream_profile(stream_id);

        if (ping_stream)
        {
            if (ping_stream->background)
            {
                task_hdl_msg_ptr = fhost_search_task_hdl_msg(ping_stream->ping_handle);
                task_hdl_msg_ptr->msg_id = ipc_msg.id;
                ret = FHOST_IPC_NO_RESP;
            }
            fhost_ping_stop(ping_stream);
        }
        else
        {
            fhost_print(RTOS_TASK_NULL, "Invalid stream_id %d", stream_id);
            ret = FHOST_IPC_ERROR;
        }

        return ret;
    }


    do
    {
        // Handle all options of ping command
        if (token[0] == '-')
        {
            switch(token[1])
            {
                case ('s'):
                    token = fhost_ipc_next_token(&next);
                    if (!token)
                        return FHOST_IPC_SHOW_USAGE;
                    pksize = atoi(token);
                    break;
                case ('r'):
                    token = fhost_ipc_next_token(&next);
                    if (!token)
                        return FHOST_IPC_SHOW_USAGE;
                    rate = atoi(token);
                    break;
                case ('d'):
                    token = fhost_ipc_next_token(&next);
                    if (!token)
                        return FHOST_IPC_SHOW_USAGE;
                    duration = atoi(token);
                    break;
                case ('Q'):
                    token = fhost_ipc_next_token(&next);
                    if (!token)
                        return FHOST_IPC_SHOW_USAGE;
                    tos = atoi(token);
                    break;
                case ('G'):
                    background = true;
                    break;
                default:
                    return FHOST_IPC_SHOW_USAGE;
            }
        }
        // If it's neither options, nor IP address, then the input is wrong
        else if (fhost_ipc_parse_ip4(token, &rip, NULL))
        {
            fhost_print(RTOS_TASK_NULL, "Invalid IP address: %s\n", token);
            return FHOST_IPC_ERROR;
        }
    } while ((token = fhost_ipc_next_token(&next)));

    // IP destination should be set by the command
    if (rip == 0)
        return FHOST_IPC_SHOW_USAGE;

    args.rip = rip;
    args.rate = rate;
    args.pksize = pksize;
    args.duration = duration;
    args.tos = tos;
    args.background = background;
    ret = fhost_ipc_start_task(ipc_msg.id, fhost_ping_start, &args, fhost_ping_sigkill_handler);

    if (ret == FHOST_IPC_ERROR)
    {
        fhost_print(RTOS_TASK_NULL, "Send ping error\n");
        return ret;
    }

    if (background)
        return FHOST_IPC_SUCCESS;
    else
        return FHOST_IPC_NO_RESP;
}
#endif //NX_PING

/**
 ****************************************************************************************
 * @brief Process function for 'tg' command
 *
 * TG command can be used for the traffic generation and reception
 * First a stream must be configured with the following parameter:
 * - id (< @ref FHOST_TG_MAX_TRAFFIC_STREAMS)
 * - profile (@ref profile_id)
 * - direction (@ref FHOST_TG_DIRECT_SEND, @ref FHOST_TG_DIRECT_RECV)
 * - remote ip and port
 * - local ip and port
 * - rate (in pkt/s, 0 for RX)
 * - packet size (in bytes, 0 for RX)
 * - duration (in seconds, 0 for RX)
 * - tos (Type Of Service) (basically 'tid << 5', 0 for RX).
 *
 * Once a stream has been configured, it can be started/stopped
 *
   @verbatim
     tg config <id> <profile> <direction> <remote_ip> <remote_port> <local_ip> <local_port>
               <rate> <pksize> <duration> <tos>
     tg start  <id1> [<id2> ... <id8>]
     tg stop   <id1> [<id2> ... <id8>]
   @endverbatim
 *
 * @param[in] params TG command above
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
int fhost_ipc_tg(char *params)
{
    #if NX_TG
    char *token, *next = params;

    token = fhost_ipc_next_token(&next);
    if (!strcmp("config", token))
    {
        char *id, *prof, *direction, *remote_ip, *remote_port, *local_ip,
        *local_port, *rate, *pksize, *duration, *tos;
        u32_t stream_id;
        uint32_t rip, lip;
        char fmt[] = "Configure stream %d for %s\n";

        id = fhost_ipc_next_token(&next);
        prof = fhost_ipc_next_token(&next);
        direction = fhost_ipc_next_token(&next);
        remote_ip = fhost_ipc_next_token(&next);
        remote_port = fhost_ipc_next_token(&next);
        local_ip = fhost_ipc_next_token(&next);
        local_port = fhost_ipc_next_token(&next);
        rate = fhost_ipc_next_token(&next);
        pksize = fhost_ipc_next_token(&next);
        duration = fhost_ipc_next_token(&next);
        tos = fhost_ipc_next_token(&next);

        if (!id || !prof || !direction ||
            !remote_ip || fhost_ipc_parse_ip4(remote_ip, &rip, NULL) || !remote_port ||
            !local_ip || fhost_ipc_parse_ip4(local_ip, &lip, NULL) || !local_port ||
            !pksize || !duration || !tos)
            return FHOST_IPC_SHOW_USAGE;

        stream_id = atoi(id);

        if (fhost_tg_config(stream_id, atoi(prof), atoi(direction), rip, atoi(remote_port),
                            lip, atoi(local_port), atoi(rate), atoi(pksize), atoi(duration),
                            atoi(tos)))
        {
            fhost_print(RTOS_TASK_NULL, "Configuration error\n");
            return FHOST_IPC_ERROR;
        }

        if (atoi(direction) == FHOST_TG_DIRECT_SEND)
        {
            fhost_print(RTOS_TASK_NULL, fmt, stream_id, "sending\n");
        }
        else
        {
            fhost_print(RTOS_TASK_NULL, fmt, stream_id, "receiving\n");
        }
    }
    else if (!strcmp("start", token))
    {
        char *id[FHOST_TG_MAX_TRAFFIC_STREAMS + 1];
        u32_t stream_id;
        char fmt[40] = "Start stream id : ";
        int i = 0;
        int j = 0;

        id[i] = fhost_ipc_next_token(&next);
        // If there's the first ID, try to collect all the rest
        while (id[i] != NULL && i < FHOST_TG_MAX_TRAFFIC_STREAMS)
        {
            id[++i] = fhost_ipc_next_token(&next);
        }

        if (!id[0])
            return FHOST_IPC_SHOW_USAGE;

        // In this loop, we count all the IDs of streams to start
        for (j = 0; j <= i - 1; j++)
        {
            stream_id = atoi(id[j]);
            if (fhost_tg_start(stream_id))
            {
                fhost_print(RTOS_TASK_NULL, "Can't start stream : %d\n", stream_id);
                return FHOST_IPC_ERROR;
            }
            strcat(strcat(fmt, id[j]), " ");
            // If not, ASSERT (!hal_machw_time_past(value)) at mm_timer.c:116
            rtos_task_suspend(500);
        }
        strcat(fmt, "\n");
        fhost_print(RTOS_TASK_NULL, fmt);
    }
    else if (!strcmp("stop", token))
    {
        u32_t stream_id;
        struct fhost_tg_stats *stats = NULL;

        token = fhost_ipc_next_token(&next);
        if (!token)
            return FHOST_IPC_SHOW_USAGE;

        do
        {
            stream_id = atoi(token);
            stats = fhost_tg_stop(stream_id);

            if (stats == NULL)
            {
                fhost_print(RTOS_TASK_NULL, "Can't stop stream : %d\n", stream_id);
                return FHOST_IPC_ERROR;
            }

            rtos_task_suspend(1);
            fhost_print(RTOS_TASK_NULL,
                        "STOP stream ID : %d\n"
                        "TX frames : %d\n"
                        "RX frames : %d\n"
                        "TX bytes : %d\n"
                        "RX bytes : %d\n"
                        "Throughput : %d bps\n"
                        "Expected : %d\n"
                        "Lost : %d\n"
                        "Unordered : %d\n"
                        "RTT : %d us\n",
                        stream_id,
                        stats->tx_frames,
                        stats->rx_frames,
                        stats->tx_bytes,
                        stats->rx_bytes,
                        stats->throughput,
                        stats->expected,
                        stats->lost,
                        stats->unordered,
                        stats->rt_time);
        } while((token = fhost_ipc_next_token(&next)));
    }
    else
        return FHOST_IPC_SHOW_USAGE;


    return FHOST_IPC_SUCCESS;
    #else
    return FHOST_IPC_ERROR;
    #endif // NX_TG
}


/**
 ****************************************************************************************
 * @brief Process function for 'monitor' command
 *
 * monitor command can be used to start the monitor mode
 *
   @verbatim
     monitor start <itf> <freq> <20|40|80|80+80|160> <center freq 1> <center freq 2>
     monitor stop  <itf>
   @endverbatim
 *
 * @param[in] params monitor_start/stop commands above
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_monitor(char *params)
{
    #if NX_FHOST_MONITOR
    const struct {
        const char *name;
        u8_t val;
    } bwmap[] = {
        { .name = "20", .val = PHY_CHNL_BW_20, },
        { .name = "40", .val = PHY_CHNL_BW_40, },
        { .name = "80", .val = PHY_CHNL_BW_80, },
        { .name = "80+80", .val = PHY_CHNL_BW_80P80, },
        { .name = "160", .val = PHY_CHNL_BW_160, },
    };

    char *token, *next = params;
    int fhost_vif_idx = 0;

    token = fhost_ipc_next_token(&next);
    if (!strcmp("start", token))
    {
        char *itf;
        struct fhost_vif_monitor_cfg cfg;
        struct mac_chan_def *chan;
        unsigned int i, freq_offset;

        itf = fhost_ipc_next_token(&next);

        // get the interface index
        fhost_vif_idx = fhost_search_itf(itf);
        if (fhost_vif_idx < 0)
            return FHOST_IPC_ERROR;

        // frequency
        token = fhost_ipc_next_token(&next);
        cfg.chan.prim20_freq = atoi(token);
        chan = fhost_chan_get(cfg.chan.prim20_freq);
        if (chan == NULL)
        {
            fhost_print(RTOS_TASK_NULL, "Invalid freq %d", cfg.chan.prim20_freq);
            return FHOST_IPC_ERROR;
        }
        cfg.chan.band = chan->band;
        cfg.chan.tx_power = chan->tx_power;

        // by default 20Mhz bandwidth
        cfg.chan.type = PHY_CHNL_BW_20;
        cfg.chan.center1_freq = cfg.chan.prim20_freq;
        cfg.chan.center2_freq = 0;

        // bw
        if ((token = fhost_ipc_next_token(&next)))
        {
            for (i = 0; i < CO_ARRAY_SIZE(bwmap); i++)
            {
                if (strcmp(bwmap[i].name, token) == 0)
                {
                    cfg.chan.type = bwmap[i].val;
                    break;
                }
            }
        }

        // center1_freq
        if ((token = fhost_ipc_next_token(&next)))
        {
            cfg.chan.center1_freq = atoi(token);
        }

        if (cfg.chan.center1_freq > cfg.chan.prim20_freq)
            freq_offset = cfg.chan.center1_freq - cfg.chan.prim20_freq;
        else
            freq_offset = cfg.chan.prim20_freq - cfg.chan.center1_freq;

        switch(cfg.chan.type)
        {
            case PHY_CHNL_BW_20:
                if (freq_offset != 0)
                {
                    fhost_print(RTOS_TASK_NULL, "monitor_start :"
                                "Center frequency of primary channel different from "
                                "frequency of primary channel in 20MHz (%d != %d)\n",
                                cfg.chan.center1_freq, cfg.chan.prim20_freq);
                    return FHOST_IPC_ERROR;
                }
                break;
            case PHY_CHNL_BW_40:
                if (freq_offset != 10)
                {
                    fhost_print(RTOS_TASK_NULL, "monitor_start :"
                                "Center frequency of primary channel different from "
                                "frequency of primary channel +/- 10 in 40MHz (%d != %d)\n",
                                cfg.chan.center1_freq, cfg.chan.prim20_freq);
                    return FHOST_IPC_ERROR;
                }
                break;
            case PHY_CHNL_BW_80P80:
                //center2_freq
                if ((token = fhost_ipc_next_token(&next)))
                {
                    cfg.chan.center2_freq = atoi(token);
                }
                else
                {
                    fhost_print(RTOS_TASK_NULL, "monitor_start :"
                                "Center frequency of secondary channel must be set\n");
                    return FHOST_IPC_ERROR;
                }

                //adjacent channel rejection
                if ((cfg.chan.center1_freq - cfg.chan.center2_freq == 80) ||
                    (cfg.chan.center2_freq - cfg.chan.center1_freq == 80))
                {
                    fhost_print(RTOS_TASK_NULL, "monitor_start :"
                                "Adjacent channel is not allowed, use 160MHz bandwidth\n");
                    return FHOST_IPC_ERROR;
                }

                // no break
            case PHY_CHNL_BW_80:
                if ((freq_offset != 10) && (freq_offset != 30))
                {
                    fhost_print(RTOS_TASK_NULL, "monitor_start :"
                                "Center frequency of primary channel different from "
                                "frequency of primary channel +/- 10 and frequency of"
                                "primary channel +/- 30 (%d != %d)\n",
                                cfg.chan.center1_freq, cfg.chan.prim20_freq);
                    return FHOST_IPC_ERROR;
                }
                break;
            case PHY_CHNL_BW_160:
                if ((freq_offset != 10) && (freq_offset != 30) &&
                    (freq_offset != 50) && (freq_offset != 70))
                {
                    fhost_print(RTOS_TASK_NULL, "monitor_start :"
                            "Center frequency of primary channel must belong to the range:"
                            "frequency of primary channel +/- [10, 30, 50, 70]\n");
                    return FHOST_IPC_ERROR;
                }
                break;
            default:
                fhost_print(RTOS_TASK_NULL, "monitor_start :"
                            "Invalid bandwidth %d\n", cfg.chan.type);
                return FHOST_IPC_ERROR;
        }

        if (fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_MONITOR, false))
        {
            fhost_print(RTOS_TASK_NULL, "Error while enabling monitor mode\n");
            return FHOST_IPC_ERROR;
        }

        cfg.uf = true;
        cfg.cb = fhost_monitor_cb;
        cfg.cb_arg = NULL;

        if (fhost_cntrl_monitor_cfg(cntrl_link, fhost_vif_idx, &cfg))
        {
            fhost_print(RTOS_TASK_NULL, "Error while configuring monitor mode\n");
            return FHOST_IPC_ERROR;
        }

    }
    else if (!strcmp("stop", token))
    {
        char *itf = fhost_ipc_next_token(&next);

        if (!itf)
            return FHOST_IPC_SHOW_USAGE;

        // get the interface index
        fhost_vif_idx = fhost_search_itf(itf);
        if (fhost_vif_idx < 0)
            return FHOST_IPC_ERROR;

        if (fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_STA, false))
        {
            fhost_print(RTOS_TASK_NULL, "Error while disabling monitor mode\n");
            return FHOST_IPC_ERROR;
        }
    }
    else
        return FHOST_IPC_SHOW_USAGE;

    return FHOST_IPC_SUCCESS;
    #else
    return FHOST_IPC_ERROR;
    #endif // NX_FHOST_MONITOR
}


/**
 ****************************************************************************************
 * @brief Process function for 'sys_stats' command
 *
 * Send fw command to get system statistics.
 *
 * @param[in] params Not used
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_sys_stats(char *params)
{
    #if NX_SYS_STAT
    struct cfgrwnx_msg cmd;
    struct cfgrwnx_sys_stats_resp resp;
    uint32_t stats_time, sleep_int, sleep_frac, doze_int, doze_frac;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SYS_STATS_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SYS_STATS_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr))
        return FHOST_IPC_ERROR;

    if (resp.stats.stats_time == 0)
        return FHOST_IPC_ERROR;

    stats_time = resp.stats.stats_time;
    sleep_int = ((resp.stats.cpu_sleep_time * 100) / stats_time);
    sleep_frac = (((resp.stats.cpu_sleep_time * 100) % stats_time) * 10) / stats_time;
    doze_int = ((resp.stats.doze_time * 100) / stats_time);
    doze_frac = (((resp.stats.doze_time * 100) % stats_time) * 10) / stats_time;

    fhost_print(RTOS_TASK_NULL, "System statistics:\n"
                                "  CPU sleep [%%]: %d.%d\n"
                                "  Doze      [%%]: %d.%d\n", sleep_int, sleep_frac,
                                doze_int, doze_frac);
    return FHOST_IPC_SUCCESS;
    #else
    return FHOST_IPC_ERROR;
    #endif //NX_SYS_STAT
    return FHOST_IPC_ERROR;
}

/**
 ****************************************************************************************
 * @brief Process function for 'dhcpc' command
 *
 * Start/stop dhcp client. Use interface at index 0 if no interface is provided.
 *
   @verbatim
     dhcpc start|stop [<itf>]
   @endverbatim
 *
 * @param[in] params parameters passed to dhcpc command
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_dhcpc(char *params)
{
    char *token, *itf, *next = params;
    int fhost_vif_idx = -1;

    token = fhost_ipc_next_token(&next);
    if (!strcmp("start", token) || !strcmp("stop", token))
    {
        struct fhost_vif_ip_addr_cfg ip_cfg;

        if ((itf = fhost_ipc_next_token(&next)))
        {
            fhost_vif_idx = fhost_search_itf(itf);
            if (fhost_vif_idx < 0)
                return FHOST_IPC_ERROR;
        }
        else
        {
            // Get the first interface available
            fhost_vif_idx = fhost_search_first_valid_itf();
            if (fhost_vif_idx < 0)
                return FHOST_IPC_ERROR;
        }

        if (!strcmp("start", token))
        {
            ip_cfg.mode = IP_ADDR_DHCP_CLIENT;
            ip_cfg.default_output = true;
            ip_cfg.dhcp.to_ms = 5000;

            if (fhost_set_vif_ip(fhost_vif_idx, &ip_cfg))
            {
                fhost_print(RTOS_TASK_NULL, "dhcpc start error\n");
                return FHOST_IPC_ERROR;
            }
        }
        else
        {
            ip_cfg.mode = IP_ADDR_NONE;
            if (fhost_set_vif_ip(fhost_vif_idx, &ip_cfg))
            {
                fhost_print(RTOS_TASK_NULL, "dhcpc stop error\n");
                return FHOST_IPC_ERROR;
            }
        }
    }
    else
        return FHOST_IPC_SHOW_USAGE;

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'list_features' command
 *
 * Send fw command to get FW/PHY supported features.
 *
 * @param[in] params Not used
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_list_features(char *params)
{
    struct cfgrwnx_msg cmd;
    struct cfgrwnx_list_features_resp resp;
    uint32_t sys_feat;
    uint8_t nss, chbw;
    char *ldpc, *he;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_LIST_FEATURES_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;
    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_LIST_FEATURES_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr))
        return FHOST_IPC_ERROR;

    sys_feat = resp.version.features;

    #define PRINT_IPC_FEAT(feat)                                        \
        (sys_feat & CO_BIT(MM_FEAT_##feat##_BIT) ? "["#feat"]" : "")

    fhost_print(RTOS_TASK_NULL, "FW features: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
                PRINT_IPC_FEAT(BCN),
                PRINT_IPC_FEAT(RADAR),
                PRINT_IPC_FEAT(PS),
                PRINT_IPC_FEAT(UAPSD),
                PRINT_IPC_FEAT(AMPDU),
                PRINT_IPC_FEAT(AMSDU),
                PRINT_IPC_FEAT(P2P),
                PRINT_IPC_FEAT(P2P_GO),
                PRINT_IPC_FEAT(UMAC),
                PRINT_IPC_FEAT(VHT),
                PRINT_IPC_FEAT(HE),
                PRINT_IPC_FEAT(BFMEE),
                PRINT_IPC_FEAT(BFMER),
                PRINT_IPC_FEAT(WAPI),
                PRINT_IPC_FEAT(MFP),
                PRINT_IPC_FEAT(MU_MIMO_RX),
                PRINT_IPC_FEAT(MU_MIMO_TX),
                PRINT_IPC_FEAT(MESH),
                PRINT_IPC_FEAT(TDLS),
                PRINT_IPC_FEAT(ANT_DIV));
    #undef PRINT_IPC_FEAT

    nss = phy_get_nss() + 1;
    chbw = 20 * (1 << phy_get_bw());
    ldpc = phy_ldpc_rx_supported() ? "[LDPC]" : "";
    he = phy_he_supported() ? "[HE]" : "";

    fhost_print(RTOS_TASK_NULL, "PHY features:[NSS=%d][CHBW=%d]%s%s\n", nss, chbw, ldpc, he);

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'connect' command
 *
 * Start connection to an AP in a separated task.
 * @verbatim
   connect [-i <itf>] -s <SSID> [-k <key>] [-b <bssid>] [-f <freq>[,freq]] [-a <akm>]
   [-u <pairwise cipher list>] [-g <group cipher list>] [-t <timeout_ms>]
   @endverbatim
 *
 * @param[in] params  Connection parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_connect(char *params)
{
    struct fhost_ipc_connect_cfg cfg;
    char *token, *next = params;

    memset(&cfg, 0, sizeof(cfg));

    cfg.cfg.timeout_ms = 5000000;
    while ((token = fhost_ipc_next_token(&next)))
    {
        char option;

        if ((token[0] != '-') | (token[2] != '\0'))
            return FHOST_IPC_SHOW_USAGE;

        option = token[1];
        token = fhost_ipc_next_token(&next);
        if (!token)
            return FHOST_IPC_SHOW_USAGE;

        switch(option)
        {
            case 'i':
            {
                cfg.fhost_vif_idx = fhost_search_itf(token);
                if (cfg.fhost_vif_idx < 0)
                    return FHOST_IPC_ERROR;
                break;
            }
            case 's':
            {
                size_t ssid_len = strlen(token);
                if (ssid_len > sizeof(cfg.cfg.ssid.array))
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid SSID");
                    return FHOST_IPC_ERROR;
                }

                memcpy(cfg.cfg.ssid.array, token, ssid_len);
                cfg.cfg.ssid.length = ssid_len;

                wifi_mgmr_sta_ssid_set(token);

                break;
            }
            case 'b':
            {
                if (fhost_ipc_parse_mac_addr(token, &cfg.cfg.bssid))
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid BSSID");
                    return FHOST_IPC_ERROR;
                }
                break;
            }
            case 'k':
            {
                size_t key_len = strlen(token);
                if ((key_len + 1) > sizeof(cfg.cfg.key))
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid Key");
                    return FHOST_IPC_ERROR;
                }
                strcpy(cfg.cfg.key, token);
                wifi_mgmr_sta_passphr_set(token);
                break;
            }
            case 'f':
            {
                unsigned int i;
                char *next_freq = strchr(token, ',');
                for (i = 0 ; i <  CO_ARRAY_SIZE(cfg.cfg.freq); i++)
                {
                    cfg.cfg.freq[i] = atoi(token);
                    if (!next_freq)
                        break;
                    *next_freq++ = '\0';
                    token = next_freq;
                    next_freq = strchr(token, ',');
                }
                break;
            }
            case 'a':
            {
                if (fhost_ipc_read_akm(token, &cfg.cfg.akm, true))
                    return FHOST_IPC_ERROR;

                break;
            }
            case 't':
            {
                cfg.cfg.timeout_ms = atoi(token);
                break;
            }
            case 'u':
            case 'g':
            {
                uint32_t *cipher;
                if (option == 'u')
                    cipher = &cfg.cfg.unicast_cipher;
                else
                    cipher = &cfg.cfg.group_cipher;

                if (fhost_ipc_read_cipher(token, cipher, true))
                    return FHOST_IPC_ERROR;

                break;
            }
            default:
                return FHOST_IPC_SHOW_USAGE;
        }
    }

    if (cfg.cfg.ssid.length == 0)
        return FHOST_IPC_SHOW_USAGE;

    cfg.link = cntrl_link;

    return fhost_ipc_start_task(ipc_msg.id, fhost_ipc_connect_task_start, &cfg,
                                fhost_ipc_connect_task_kill);
}

static int fhost_ipc_disconnect(char *params)
{
    int res, fhost_vif_idx;
    struct mac_scan_result result;

    if (params && strstr(params, "-i")) {
        char *token, *next = params;
        struct fhost_vif_status status;

        token = fhost_ipc_next_token(&next);
        token = fhost_ipc_next_token(&next);

        fhost_vif_idx = fhost_search_itf(token);
        if (fhost_get_vif_status(fhost_vif_idx, &status))
            return FHOST_IPC_ERROR;

        if (fhost_get_vif_status(fhost_vif_idx, &status) ||
            ((status.type == VIF_UNKNOWN) &&
             fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_STA, false)))
            return FHOST_IPC_ERROR;
    } else {
        fhost_vif_idx = fhost_search_first_valid_itf();
    }

    if (fhost_vif_idx < 0) {
        return FHOST_IPC_ERROR;
    }

    // Disable network connection
    if (fhost_wpa_disable_network(fhost_vif_idx)) {
       res = FHOST_IPC_ERROR;
    }
    if (fhost_wpa_remove_vif(fhost_vif_idx)) {
       res = FHOST_IPC_ERROR;
    }

    return FHOST_IPC_SUCCESS;
}
/**
 ****************************************************************************************
 * @brief Process function for 'powersave' command
 *
 * Enable/disable power save mode.
 * @verbatim
   powersave [-i <itf>] [-u <uapsd_queue>] <on|off|dyn>
   @endverbatim
 *
 * @param[in] params  PS parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_powersave(char *params)
{
    #if NX_POWERSAVE
    struct cfgrwnx_set_ps_mode cmd;
    struct cfgrwnx_resp resp;
    int fhost_vif_idx = -1;
    char *token, *next = params;
    bool mode_set = false;
    int uapsd_queue = -1;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SET_PS_MODE_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SET_PS_MODE_RESP;

    while ((token = fhost_ipc_next_token(&next)))
    {
        if (token[0] == '-')
        {
            switch (token[1]) {
                case 'i':
                    token = fhost_ipc_next_token(&next);
                    if (!token)
                        return FHOST_IPC_SHOW_USAGE;
                    fhost_vif_idx = fhost_search_itf(token);
                    if (fhost_vif_idx < 0)
                        return FHOST_IPC_ERROR;
                    break;
                case 'u':
                    token = fhost_ipc_next_token(&next);
                    if (!token)
                        return FHOST_IPC_SHOW_USAGE;
                    uapsd_queue = strtol( token, NULL, 0 );
                    break;
                default:
                    return FHOST_IPC_SHOW_USAGE;
            }
        }
        else if (strncmp(token, "on", 2) == 0)
        {
            // the correct way to configure ps mode is via ME_CONFIG_REQ but for
            // testing bypass this
            me_env_set_ps_mode(PS_MODE_ON);
            cmd.enabled = true;
            mode_set = true;
        }
        else if (strncmp(token, "dyn", 3) == 0)
        {
            // the correct way to configure ps mode is via ME_CONFIG_REQ but for
            // testing bypass this
            me_env_set_ps_mode(PS_MODE_ON_DYN);
            cmd.enabled = true;
            mode_set = true;
        }
        else if (strncmp(token, "off", 3) == 0)
        {
            cmd.enabled = false;
            mode_set = true;
        }
        else
        {
            return FHOST_IPC_SHOW_USAGE;
        }
    }

    if (!mode_set)
        return FHOST_IPC_SHOW_USAGE;

    if (fhost_vif_idx == -1)
        fhost_vif_idx = fhost_search_first_valid_itf();
    if (fhost_vif_idx < 0)
        return FHOST_IPC_ERROR;

    if (uapsd_queue != -1)
        fhost_vif_set_uapsd_queues(fhost_vif_idx, (uint8_t)uapsd_queue);

    cmd.fhost_vif_idx = fhost_vif_idx;
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr))
        return FHOST_IPC_ERROR;

    return FHOST_IPC_SUCCESS;
    #else
    return FHOST_IPC_ERROR;
    #endif // NX_POWERSAVE
}

/**
 ****************************************************************************************
 * @brief Process function for 'ap' command
 *
 * Start an AP
 * @verbatim
   ap [-i <itf>] -s <SSID> -f <freq>[+-@] [-a <akm>[,<akm 2>]] [-k <key>]
      [-b bcn_int[,dtim_period]] [-u <unicast cipher>[,<unicast cipher 2>]]
      [-g <group cipher>] [-m <mfp: 0|1|2>] [-t <AP mode>] [-o <option=value>]
      [-n <isolation mode:on|off>]
   @endverbatim
 * The '+/-' after the frequency allow to configure a 40MHz channel with the secondary
 * channel being the upper/lower one. The '@' allow to configure a 80 MHz channel, this
 * is only allowed for valid primary channel and center freq is automatically computed.
 *
 * @param[in] params  Connection parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_ap(char *params)
{
    #if NX_BEACONING
    struct fhost_vif_ap_cfg cfg;
    struct fhost_vif_ip_addr_cfg ip_cfg;
    int fhost_vif_idx = fhost_search_first_valid_itf();
    char *token, *next = params;
    int res = FHOST_IPC_ERROR;

    memset(&cfg, 0, sizeof(cfg));

    while ((token = fhost_ipc_next_token(&next)))
    {
        char option;

        if ((token[0] != '-') | (token[2] != '\0'))
        {
            res = FHOST_IPC_SHOW_USAGE;
            goto end;
        }

        option = token[1];
        token = fhost_ipc_next_token(&next);
        if (!token)
        {
            res = FHOST_IPC_SHOW_USAGE;
            goto end;
        }

        switch(option)
        {
            case 'i':
            {
                fhost_vif_idx = fhost_search_itf(token);
                if (fhost_vif_idx < 0)
                    goto end;
                break;
            }
            case 's':
            {
                size_t ssid_len = strlen(token);
                if (ssid_len > sizeof(cfg.ssid.array))
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid SSID");
                    goto end;
                }

                memcpy(cfg.ssid.array, token, ssid_len);
                cfg.ssid.length = ssid_len;
                break;
            }
            case 'k':
            {
                size_t key_len = strlen(token);
                if ((key_len + 1) > sizeof(cfg.key))
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid Key");
                    goto end;
                }
                strcpy(cfg.key, token);
                break;
            }
            case 'f':
            {
                int len = strlen(token) - 1;
                struct mac_chan_def *chan = NULL;
                int offset = 0;
                if (token[len] == '+')
                {
                    token[len] = 0;
                    offset = 10;
                    cfg.chan.type = PHY_CHNL_BW_40;
                }
                else if (token[len] == '-')
                {
                    token[len] = 0;
                    offset = -10;
                    cfg.chan.type = PHY_CHNL_BW_40;
                }
                else if (token[len] == '@')
                {
                    token[len] = 0;
                    cfg.chan.type = PHY_CHNL_BW_80;
                }
                else
                {
                    cfg.chan.type = PHY_CHNL_BW_20;
                }

                cfg.chan.prim20_freq = atoi(token);
                chan = fhost_chan_get(cfg.chan.prim20_freq);
                if (!chan)
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid channel\n");
                    goto end;
                }

                if (cfg.chan.prim20_freq >= PHY_FREQ_5G)
                    cfg.chan.band = PHY_BAND_5G;
                else
                    cfg.chan.band = PHY_BAND_2G4;

                if (cfg.chan.type == PHY_CHNL_BW_80)
                {
                    if ((cfg.chan.prim20_freq < 5180) ||
                        (cfg.chan.prim20_freq > 5805))
                    {
                        fhost_print(RTOS_TASK_NULL, "Invalid primary for 80MHz channel\n");
                        goto end;
                    }
                    offset = (cfg.chan.prim20_freq - 5180) % 80;
                    if (offset < 20)
                        offset = 30;
                    else if (offset < 40)
                        offset = 10;
                    else if (offset < 60)
                        offset = -10;
                    else
                        offset = -30;
                }
                cfg.chan.center1_freq = cfg.chan.prim20_freq + offset;
                break;
            }
            case 'a':
            {
                if (fhost_ipc_read_akm(token, &cfg.akm, true))
                    goto end;

                break;
            }
            case 'u':
            case 'g':
            {
                uint32_t *cipher;
                if (option == 'u')
                    cipher = &cfg.unicast_cipher;
                else
                    cipher = &cfg.group_cipher;

                if (fhost_ipc_read_cipher(token, cipher, true))
                    goto end;

                break;
            }
            case 'b':
            {
                char *dtim = strchr(token, ',');
                if (dtim)
                {
                    *dtim++ = 0;
                    cfg.dtim_period = atoi(dtim);
                }
                cfg.bcn_interval = atoi(token);

                break;
            }
            case 'm':
            {
                cfg.mfp = atoi(token);
                break;
            }
            case 't':
            {
                if (fhost_ipc_read_mode(token, &cfg.mode, true))
                    goto end;
                break;
            }
            case 'o':
            {
                int opt_len = strlen(token) + 2;
                char *tmp = strchr(token, '=');

                if (!tmp)
                {
                    res = FHOST_IPC_SHOW_USAGE;
                    goto end;
                }
                *tmp = ' ';

                tmp = cfg.extra_cfg;
                if (tmp)
                    opt_len += strlen(tmp);

                cfg.extra_cfg = rtos_malloc(opt_len);
                if (!cfg.extra_cfg)
                {
                    fhost_print(RTOS_TASK_NULL, "no more memory\n");
                    cfg.extra_cfg = tmp;
                    goto end;
                }

                if (tmp)
                {
                    dbg_snprintf(cfg.extra_cfg, opt_len, "%s%s;", tmp, token);
                    rtos_free(tmp);
                }
                else
                {
                    dbg_snprintf(cfg.extra_cfg, opt_len, "%s;", token);
                }
                break;
            }
            case 'n':
            {
                if (strncmp(token, "on", 2) == 0)
                    cfg.isolation_mode = true;
                else
                    cfg.isolation_mode = false;
                break;
            }
            default:
            {
                fhost_print(RTOS_TASK_NULL, "Invalid option %c\n", option);
                res = FHOST_IPC_SHOW_USAGE;
                goto end;
            }
        }
    }

    if (fhost_vif_idx < 0)
        goto end;

    if ((cfg.ssid.length == 0) || (cfg.chan.prim20_freq == 0))
    {
         res = FHOST_IPC_SHOW_USAGE;
         goto end;
    }

    // try to select the best AKM if not set
    if (cfg.akm == 0)
    {
        if (strlen(cfg.key) == 0)
            cfg.akm = CO_BIT(MAC_AKM_NONE);
        else if (strlen(cfg.key) == 5)
            cfg.akm = CO_BIT(MAC_AKM_PRE_RSN);
        else
            cfg.akm = CO_BIT(MAC_AKM_PSK);
    }

    // Save IP configuration (if any)
    fhost_get_vif_ip(fhost_vif_idx, &ip_cfg);

    // (Re)Set interface type to AP
    if (fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_UNKNOWN, false) ||
        fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_AP, false))
        goto end;

    if (fhost_ap_cfg(fhost_vif_idx, &cfg, false))
    {
        fhost_print(RTOS_TASK_NULL, "Failed to start AP, check your configuration");
        goto end;
    }

    // Reconfigure IP only if it was static
    if (ip_cfg.mode == IP_ADDR_STATIC_IPV4)
    {
        ip_cfg.default_output = true;
        fhost_set_vif_ip(fhost_vif_idx, &ip_cfg);
    }
    
void net_dhcpd_start(net_if_t *net_if);
    net_dhcpd_start(fhost_to_net_if(fhost_vif_idx));

    res = FHOST_IPC_SUCCESS;

  end:
    if (cfg.extra_cfg)
        rtos_free(cfg.extra_cfg);

    return res;

    #else
    return FHOST_IPC_ERROR;
    #endif // NX_BEACONING
}

/**
 ****************************************************************************************
 * @brief Compare indexes of rate control configurations
 *
 * @param[in] a         First rate control configuration
 * @param[in] b         Second rate control configuration
 * @return 1 if the index of a is higher than the index of b
 ****************************************************************************************
 */
static int fhost_rc_compare_idx(const void *a, const void *b)
{
    struct fhost_rc_conf *idx_a = *(struct fhost_rc_conf **) a;
    struct fhost_rc_conf *idx_b = *(struct fhost_rc_conf **) b;

    if (idx_a->r_idx < idx_b->r_idx)
        return -1;
    else
        return 1;
}

/**
 ****************************************************************************************
 * @brief Convert RC table index to Rate configuration
 *
 * @param[in] idx      Index of the RC table
 * @param[in] is_rx    Indicates whether the conversion is for RX stats
 * @param[out] r_cfg   Rate configuration to fill
 * @param[out] ru_size RU size for the HE TB
 *
 * @return 0 if successful, != 0 otherwise
 ****************************************************************************************
 */
static int fhost_idx_to_rate_cfg(int idx, bool is_rx, union fhost_rc_rate_ctrl_info *r_cfg,
                                 int *ru_size)
{
    union fhost_rc_mcs_index {
        struct {
            uint32_t mcs : 3;
            uint32_t nss : 2;
        } ht;
        struct {
            uint32_t mcs : 4;
            uint32_t nss : 3;
        } vht;
        struct {
            uint32_t mcs : 4;
            uint32_t nss : 3;
        } he;
    };
    uint16_t first_ht, first_vht, first_he_su , first_he_er __MAYBE_UNUSED, first_he_mu __MAYBE_UNUSED;
    uint8_t he_mu_rates_per_mcs __MAYBE_UNUSED, he_su_rates_per_mcs __MAYBE_UNUSED,
            he_er_rates_per_mcs __MAYBE_UNUSED, vht_rates_per_mcs, ht_rates_per_mcs;
    uint8_t max_vht_mcs, max_he_mcs __MAYBE_UNUSED;

    #if NX_FHOST_RX_STATS
    if (is_rx)
    {
        first_ht =  fhost_conf_rx.first_ht;
        first_vht = fhost_conf_rx.first_vht;
        #if NX_HE
        first_he_su = fhost_conf_rx.first_he_su;
        first_he_er = fhost_conf_rx.first_he_er;
        #else
        first_he_su = fhost_conf_rx.n_rates;
        #endif
        first_he_mu = fhost_conf_rx.first_he_mu;
        ht_rates_per_mcs = fhost_conf_rx.ht_rates_per_mcs;
        vht_rates_per_mcs = fhost_conf_rx.vht_rates_per_mcs;
        he_su_rates_per_mcs = fhost_conf_rx.he_su_rates_per_mcs;
        he_er_rates_per_mcs = fhost_conf_rx.he_er_rates_per_mcs;
        he_mu_rates_per_mcs = fhost_conf_rx.he_mu_rates_per_mcs;
        max_vht_mcs = fhost_conf_rx.max_vht_mcs;
        max_he_mcs = fhost_conf_rx.max_he_mcs;
    }
    else
    #endif
    {
        first_ht = FIRST_HT;
        first_vht = FIRST_VHT;
        first_he_su = FIRST_HE_SU;
        first_he_er = FIRST_HE_ER;
        first_he_mu = FIRST_HE_MU;
        he_er_rates_per_mcs = 3;
        he_su_rates_per_mcs = 4 * 3;
        vht_rates_per_mcs = 4 * 2;
        ht_rates_per_mcs = 2 * 2;
        he_mu_rates_per_mcs = 6 * 3;
        max_vht_mcs = 10;
        max_he_mcs = 12;
    }

    r_cfg->value = 0;
    if (idx < FIRST_OFDM)
    {
        r_cfg->format_mod_tx = FORMATMOD_NON_HT;
        r_cfg->gi_and_pre_type_tx = (idx & 1) << 1;
        r_cfg->mcs_idx = idx / 2;
    }
    else if (idx < first_ht)
    {

        r_cfg->format_mod_tx = FORMATMOD_NON_HT;
        r_cfg->mcs_idx =  idx - N_CCK + 4;
    }
    else if (idx < first_vht)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_ht;
        r_cfg->format_mod_tx = FORMATMOD_HT_MF;
        r->ht.nss = idx / (8*ht_rates_per_mcs);
        r->ht.mcs = (idx % (8*ht_rates_per_mcs)) / ht_rates_per_mcs;
        r_cfg->bw_tx = ((idx % (8*ht_rates_per_mcs)) % ht_rates_per_mcs) / 2;
        r_cfg->gi_and_pre_type_tx = idx & 1;
    }
    else if (idx < first_he_su)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_vht;
        r_cfg->format_mod_tx = FORMATMOD_VHT;
        r->vht.nss = idx / (max_vht_mcs*vht_rates_per_mcs);
        r->vht.mcs = (idx % (max_vht_mcs*vht_rates_per_mcs)) / vht_rates_per_mcs;
        r_cfg->bw_tx = ((idx % (max_vht_mcs*vht_rates_per_mcs)) % vht_rates_per_mcs) / 2;
        r_cfg->gi_and_pre_type_tx = idx & 1;
    }
    #if NX_HE
    else if (idx < first_he_mu)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_he_su;
        r_cfg->format_mod_tx = FORMATMOD_HE_SU;
        // Jochen 2023/3/6: use msb of nss[2:0] to indicate dcm
        // r->vht.nss = idx / (max_he_mcs*he_su_rates_per_mcs);
        r_cfg->dcm_tx = idx / (4*max_he_mcs*he_su_rates_per_mcs);
        r->vht.nss = (idx % (4*max_he_mcs*he_su_rates_per_mcs)) / (max_he_mcs*he_su_rates_per_mcs);
        r->vht.mcs = (idx % (max_he_mcs*he_su_rates_per_mcs)) / he_su_rates_per_mcs;
        r_cfg->bw_tx = ((idx % (max_he_mcs*he_su_rates_per_mcs)) % he_su_rates_per_mcs) / 3;
        r_cfg->gi_and_pre_type_tx = idx % 3;
    }
    else if (idx < first_he_er)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        if (ru_size == NULL)
            return -1;

        idx -= first_he_mu;
        r_cfg->format_mod_tx = FORMATMOD_HE_MU;
        r->vht.nss = idx / (max_he_mcs*he_mu_rates_per_mcs);
        r->vht.mcs = (idx % (max_he_mcs*he_mu_rates_per_mcs)) / he_mu_rates_per_mcs;
        *ru_size = ((idx % (max_he_mcs*he_mu_rates_per_mcs)) % he_mu_rates_per_mcs)/ 3;
        r_cfg->gi_and_pre_type_tx = idx % 3;
        r_cfg->bw_tx = 0;
    }
    else
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_he_er;
        r_cfg->format_mod_tx = FORMATMOD_HE_ER;
        // Jochen 2023/3/6: add dcm tx option
        r_cfg->dcm_tx = idx / (2 * 3 * he_er_rates_per_mcs);
        r_cfg->bw_tx = (idx % (2 * 3 *he_er_rates_per_mcs)) / (3 * he_er_rates_per_mcs);
        if (ru_size != NULL)
            *ru_size = idx / (3 * he_er_rates_per_mcs);
        r_cfg->gi_and_pre_type_tx = idx % 3;
        r->vht.mcs = (idx % (3 * he_er_rates_per_mcs)) / he_er_rates_per_mcs;
        r->vht.nss = 0;
    }
    #endif // NX_HE
    return 0;
}

/**
 ****************************************************************************************
 * @brief Write one rate configuration on a string buffer passed as parameter
 *
 * @param[in] buf       String buffer where to write the Rate Control result
 * @param[in] size      Size of the string buffer
 * @param[in] format    Format of the rate configuration
 * @param[in] nss       Number of spatial streams of the rate configuration
 * @param[in] mcs       MCS index of the rate configuration
 * @param[in] bw        Bandwidth of the rate configuration
 * @param[in] sgi       Short Guard interval setting of the rate configuration
 * @param[in] pre       Preamble type of the rate configuration
 * @param[in] is_rx     Indicates whether we are printing RX stats
 * @param[out] r_idx    Index of the rate configuration
 * @return Length of the written string
 ****************************************************************************************
 */
static int fhost_print_rate(char *buf, int size, int format, int nss, int mcs, int bw,
                            int sgi, int pre, int dcm, bool is_rx, int *r_idx)
{
    static const int ru_size_he_er[] = { 242, 106 };
    static const int ru_size_he_mu[] = { 26, 52, 106, 242, 484, 996 };

    int res = 0;
    int bitrates_cck[4] = { 10, 20, 55, 110 };
    int bitrates_ofdm[8] = { 6, 9, 12, 18, 24, 36, 48, 54 };
    char he_gi[3][4] = { "0.8", "1.6", "3.2" };
    uint16_t first_ht, first_vht, first_he_su, first_he_er, first_he_mu;
    uint8_t max_vht_mcs, max_he_mcs, ht_rates_per_mcs, vht_rates_per_mcs;
    uint8_t he_su_rates_per_mcs, he_er_rates_per_mcs, he_mu_rates_per_mcs;

    #if NX_FHOST_RX_STATS
    if (is_rx)
    {
        first_ht = fhost_conf_rx.first_ht;
        first_vht = fhost_conf_rx.first_vht;
        first_he_su = fhost_conf_rx.first_he_su;
        first_he_mu = fhost_conf_rx.first_he_mu;
        first_he_er = fhost_conf_rx.first_he_er;

        max_vht_mcs = fhost_conf_rx.max_vht_mcs;
        max_he_mcs = fhost_conf_rx.max_he_mcs;
        ht_rates_per_mcs = fhost_conf_rx.ht_rates_per_mcs;
        vht_rates_per_mcs = fhost_conf_rx.vht_rates_per_mcs;
        he_su_rates_per_mcs = fhost_conf_rx.he_su_rates_per_mcs;
        he_mu_rates_per_mcs = fhost_conf_rx.he_mu_rates_per_mcs;
        he_er_rates_per_mcs = fhost_conf_rx.he_er_rates_per_mcs;
    }
    else
    #endif
    {
        first_ht = FIRST_HT;
        first_vht = FIRST_VHT;
        first_he_su = FIRST_HE_SU;
        first_he_mu = FIRST_HE_MU;
        first_he_er = FIRST_HE_ER;
        he_su_rates_per_mcs = 4 * 3;
        vht_rates_per_mcs = 4 * 2;
        ht_rates_per_mcs  = 2 * 2;
        he_mu_rates_per_mcs = 6 * 3;
        he_er_rates_per_mcs = 3;
        max_vht_mcs = 10;
        max_he_mcs = 12;
    }

    if (format < FORMATMOD_HT_MF)
    {
        if (mcs < 4)
        {
            if (r_idx)
            {
                *r_idx = (mcs * 2) + pre;
                res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
            }
            res += dbg_snprintf(&buf[res], size - res, "L-CCK/%cP%11c%2u.%1uM   ",
                                pre > 0 ? 'L' : 'S', ' ', bitrates_cck[mcs] / 10,
                                bitrates_cck[mcs] % 10);
        }
        else
        {
            mcs -= 4;
            if (r_idx)
            {
                *r_idx = FIRST_OFDM + mcs;
                res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
            }
            res += dbg_snprintf(&buf[res], size - res, "L-OFDM%13c%2u.0M   ",
                                ' ', bitrates_ofdm[mcs]);
        }
    }
    else if (format < FORMATMOD_VHT)
    {
        if (r_idx)
        {
            *r_idx = first_ht + nss * 8 * ht_rates_per_mcs + mcs * ht_rates_per_mcs +
                      bw * 2 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        mcs += nss * 8;
        res += dbg_snprintf(&buf[res], size - res, "HT%d/%cGI%11cMCS%-2d   ",
                            20 * (1 << bw), sgi ? 'S' : 'L', ' ', mcs);
    }
    else if (format == FORMATMOD_VHT)
    {
        if (r_idx)
        {
            *r_idx = first_vht + nss * max_vht_mcs * vht_rates_per_mcs +
                     mcs * vht_rates_per_mcs + bw * 2 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "VHT%d/%cGI%*cMCS%d/%1d  ",
                            20 * (1 << bw), sgi ? 'S' : 'L', bw > 2 ? 9 : 10, ' ', mcs,
                            nss + 1);
    }
    else if (format == FORMATMOD_HE_SU)
    {
        if (r_idx)
        {
            *r_idx = first_he_su + nss * max_he_mcs * he_su_rates_per_mcs +
                     mcs * he_su_rates_per_mcs+ bw * 3 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "HE%d/GI%s%4s%*cMCS%d/%1d%*c",
                            20 * (1 << bw), he_gi[sgi], dcm ? "/DCM" : "",
                            bw > 2 ? 4 : 5, ' ', mcs, nss + 1, mcs > 9 ? 1 : 2, ' ');
    }
    else if (format == FORMATMOD_HE_MU)
    {
        if (r_idx)
        {
            *r_idx = first_he_mu + nss * max_he_mcs * he_mu_rates_per_mcs +
                     mcs * he_mu_rates_per_mcs + bw * 3 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "HEMU-%d/GI%s%*cMCS%d/%1d%*c",
                            ru_size_he_mu[bw], he_gi[sgi], bw > 1 ? 5 : 6, ' ', mcs, nss + 1,
                            mcs > 9 ? 1 : 2, ' ');
    }
    else // HE ER
    {
        if (r_idx)
        {
            *r_idx = first_he_er + bw * 3 * he_er_rates_per_mcs +
                     mcs * he_er_rates_per_mcs + sgi;
            res = dbg_snprintf(buf, size - res, "%3d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "HEER-%d/GI%s%4s%1cMCS%d/NSS%1d%2c",
                            ru_size_he_er[bw], he_gi[sgi], dcm ? "/DCM" : "",
                            ' ', mcs, nss + 1, ' ');
    }

    return res;
}

/**
 ****************************************************************************************
 * @brief Extract parameters from rate configuration and print it on a buffer
 *
 * @param[in] buf           String buffer where to write the Rate Control result
 * @param[in] size          Size of the string buffer
 * @param[in] rate_config   Rate configuration of the sample
 * @param[in] ru_size       RU size for the HE TB
 * @param[in] is_rx         Indicates whether we are printing RX stats
 * @param[out] r_idx        Index of the rate configuration
 * @return Length of the written string
 ****************************************************************************************
 */
static int fhost_print_rate_from_cfg(char *buf, int size, uint32_t rate_config,
                                     uint8_t ru_size, bool is_rx, int *r_idx)
{
    uint8_t ft, pre, gi, bw, nss, mcs, dcm;

    ft = export_rc_get_format_mod(rate_config);
    mcs = export_rc_get_mcs_index(rate_config);
    nss = export_rc_get_nss(rate_config);
    bw = export_rc_get_bw(rate_config);
    pre = export_rc_get_pre_type(rate_config);
    gi = export_rc_get_sgi(rate_config) | (pre << 1);
    dcm = 0;
    if (ft >= FORMATMOD_HE_SU)
    {
        dcm = (rate_config & RC_DCM_MOD_MASK) >> RC_DCM_MOD_OFT;
        if ((ft == FORMATMOD_HE_MU) || (ft == FORMATMOD_HE_ER))
            bw = ru_size;
    }
    return fhost_print_rate(buf, size, ft, nss, mcs, bw, gi, pre, dcm, is_rx, r_idx);
}

#if NX_FHOST_RX_STATS & 0
// move to mac
/**
 ****************************************************************************************
 * @brief Print RX statistics about a sta passed as parameter
 *
 * @param[in] sta_idx       Index of the STA
 *
 * return 0 if successful, != 0 otherwise
 ****************************************************************************************
 */
static int fhost_ipc_print_rx_stats(int sta_idx)
{
    char hist[] = "##################################################";
    int hist_len = sizeof(hist) - 1, len = 0;
    int8_t rssi[2];
    uint8_t mac_addr[MAC_ADDR_LEN];
    struct rx_vector_1 *last_rx;
    unsigned int mcs, gi, nss, pre, bw, dcm = 0;
    uint32_t i;
    char buf[LINE_MAX_SZ];
    // Extract Mac address
    MAC_ADDR_EXTRACT(mac_addr, sta_mgmt_get_peer_addr(sta_idx));
    fhost_print(RTOS_TASK_NULL, "\nRX rate info for %02x:%02x:%02x:%02x:%02x:%02x:\n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                mac_addr[5]);

    if (!rx_stats[sta_idx])
    {
        fhost_print(RTOS_TASK_NULL, "\nThis station has not been initialized\n");
        return -1;
    }

    // Display Statistics
    for (i = 0 ; i < fhost_conf_rx.n_rates ; i++ )
    {
        if (rx_stats[sta_idx]->table[i]) {
            union fhost_rc_rate_ctrl_info r_cfg;
            int percent = (((uint64_t)rx_stats[sta_idx]->table[i]) * 1000) / rx_stats[sta_idx]->cpt;
            int p;
            int ru_size = 0;

            if(fhost_idx_to_rate_cfg(i, true, &r_cfg, &ru_size))
                return -1;

            fhost_print_rate_from_cfg(buf, LINE_MAX_SZ, r_cfg.value, ru_size, true, NULL);
            p = (percent * hist_len) / 1000;
            fhost_print(RTOS_TASK_NULL,"%s: %9d(%2d.%1d%%)%.*s\n",
                        buf, rx_stats[sta_idx]->table[i], percent / 10, percent % 10, p, hist);
        }
    }

    // Display detailed info of the last received rate
    last_rx = &rx_stats[sta_idx]->last_rx;

    fhost_print(RTOS_TASK_NULL,
                "\nLast received rate\n"
                "  type         rate    LDPC STBC BEAMFM DCM DOPPLER %s\n",
                (phy_get_nrx() > 0) ? "rssi1(dBm) rssi2(dBm)" : "rssi(dBm)");

    bw = last_rx->ch_bw;
    pre = last_rx->pre_type;
    #if NX_MAC_VER >= 20
    if (last_rx->format_mod >= FORMATMOD_HE_SU) {
        mcs = last_rx->he.mcs;
        nss = last_rx->he.nss;
        gi = last_rx->he.gi_type;
        if ((last_rx->format_mod == FORMATMOD_HE_MU) ||
            (last_rx->format_mod == FORMATMOD_HE_ER))
            bw = last_rx->he.ru_size;
        dcm = last_rx->he.dcm;
    } else
    #endif
    if (last_rx->format_mod == FORMATMOD_VHT) {
        #if NX_MAC_VER >= 20
        mcs = last_rx->vht.mcs;
        nss = last_rx->vht.nss;
        gi = last_rx->vht.short_gi;
        #else
        mcs = last_rx->mcs;
        nss = last_rx->stbc ? last_rx->n_sts/2 : last_rx->n_sts;
        gi = last_rx->short_gi;
        #endif
    } else if (last_rx->format_mod >= FORMATMOD_HT_MF) {
        #if NX_MAC_VER >= 20
        mcs = last_rx->ht.mcs % 8;
        nss = last_rx->ht.mcs / 8;;
        gi = last_rx->ht.short_gi;
        #else
        mcs = last_rx->mcs % 8;
        nss = last_rx->mcs / 8;;
        gi = last_rx->short_gi;
        #endif
    } else {
        mcs = rxv2macrate[last_rx->leg_rate];
        nss = 0;
        gi = 0;
    }

    len = fhost_print_rate(buf, LINE_MAX_SZ - len, last_rx->format_mod, nss, mcs, bw, gi, pre, dcm, true, NULL);

    // flags for HT/VHT/HE
    #if NX_MAC_VER >= 20
    if (last_rx->format_mod >= FORMATMOD_HE_SU) {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c     %c    %c     %c",
                            last_rx->he.fec ? 'L' : ' ',
                            last_rx->he.stbc ? 'S' : ' ',
                            last_rx->he.beamformed ? 'B' : ' ',
                            last_rx->he.dcm ? 'D' : ' ',
                            last_rx->he.doppler ? 'D' : ' ');
    } else
    #endif
    if (last_rx->format_mod == FORMATMOD_VHT) {
        #if NX_MAC_VER >= 20
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c     %c           ",
                            last_rx->vht.fec ? 'L' : ' ',
                            last_rx->vht.stbc ? 'S' : ' ',
                            last_rx->vht.beamformed ? 'B' : ' ');
        #else
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c     %c           ",
                            last_rx->fec_coding ? 'L' : ' ',
                            last_rx->stbc ? 'S' : ' ',
                            !last_rx->smoothing ? 'B' : ' ');
        #endif
    } else if (last_rx->format_mod >= FORMATMOD_HT_MF) {
        #if NX_MAC_VER >= 20
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c                  ",
                            last_rx->ht.fec ? 'L' : ' ',
                            last_rx->ht.stbc ? 'S' : ' ');
        #else
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c                  ",
                            last_rx->fec_coding ? 'L' : ' ',
                            last_rx->stbc ? 'S' : ' ');
        #endif
    } else {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "                         ");
    }

    hal_desc_get_rssi(last_rx, rssi);
    if (phy_get_nrx() > 0) {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "       %-4d       %d\n",
                            rssi[0], rssi[1]);
    } else {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "      %d\n", rssi[0]);
    }

    fhost_print(RTOS_TASK_NULL, "%s\n", buf);
    return 0;
}
#endif // NX_FHOST_RX_STATS

/**
 ****************************************************************************************
 * @brief Print Rate Control statistics for a station passed as parameter
 *
 * @param[in] sta_idx    Station index
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_rc_print_stats_sta(int sta_idx, bool is_rx)
{
#if 0
    // todo move to mac
    struct me_rc_stats_cfm res;
    struct fhost_rc_conf *st, *st_sorted[RC_MAX_N_SAMPLE];
    struct cfgrwnx_rc cmd;
    struct cfgrwnx_rc_result resp;
    uint8_t i, mac_addr[MAC_ADDR_LEN];

    // If RX stats
    if (is_rx)
    {
        #if NX_FHOST_RX_STATS
        if (fhost_ipc_print_rx_stats(sta_idx))
            return FHOST_IPC_ERROR;
        #else
        fhost_print(RTOS_TASK_NULL, "RX Statistics not enabled in FW\n");
        return FHOST_IPC_ERROR;
        #endif
        return FHOST_IPC_SUCCESS;
    }

    // prepare CFGRWNX_RC_CMD to send
    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_RC_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;
    cmd.sta_idx = sta_idx;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_RC_RESP;

    // Send CFGRWNX_RC_CMD
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
    {
        resp.cfm.no_samples = 0;
        return FHOST_IPC_SUCCESS;
    }

    res = resp.cfm;

    // Allocate RC configurations table
    st = rtos_malloc(sizeof(struct fhost_rc_conf) * res.no_samples);
    if (!st)
        return FHOST_IPC_ERROR;

    // Print mac address
    MAC_ADDR_EXTRACT(mac_addr, sta_mgmt_get_peer_addr(sta_idx));
    fhost_print(RTOS_TASK_NULL, "####### STA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                mac_addr[5]);

    // Parse and print every RC configuration
    for (i = 0; i < res.no_samples; i++)
    {
        unsigned int tp, eprob;
        uint16_t len;

        len = fhost_print_rate_from_cfg(st[i].line, LINE_MAX_SZ,
                                           res.rate_stats[i].rate_config, 0, false,
                                           &(st[i].r_idx));

        if (res.sw_retry_step != 0)
        {
            len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                                res.retry_step_idx[res.sw_retry_step] == i ? '*' : ' ');
        }
        else
        {
            len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, " ");
        }
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                            res.retry_step_idx[0] == i ? 'T' : ' ');
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                            res.retry_step_idx[1] == i ? 't' : ' ');
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c ",
                            res.retry_step_idx[2] == i ? 'P' : ' ');

        tp = res.tp[i] / 10;
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, " %4u.%1u", tp / 10,
                            tp % 10);

        eprob = ((res.rate_stats[i].probability * 1000) >> 16) + 1;

        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len,
                            "  %4u.%1u %5u(%6u)  %6u", eprob / 10, eprob % 10,
                            res.rate_stats[i].success, res.rate_stats[i].attempts,
                            res.rate_stats[i].sample_skipped);

        st_sorted[i] = &st[i];
    }
    fhost_print(RTOS_TASK_NULL, "\nTX rate info for %02X:%02X:%02X:%02X:%02X:%02X:\n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    fhost_print(RTOS_TASK_NULL,
                " #  type           rate             tpt   eprob    ok(   tot)   skipped\n");

    // Sort RC statistics
    // qsort(st_sorted, res.no_samples, sizeof(struct fhost_rc_conf *), fhost_rc_compare_idx);

    for (i = 0; i < res.no_samples; i++)
        fhost_print(RTOS_TASK_NULL, "%s\n", st_sorted[i]->line);

    // display HE TB statistics if any
    if (res.rate_stats[RC_HE_STATS_IDX].rate_config != 0)
    {
        unsigned int tp, eprob;
        struct rc_rate_stats *rate_stats = &res.rate_stats[RC_HE_STATS_IDX];
        int ru_index = rate_stats->ru_and_length & 0x07;
        int ul_length = rate_stats->ru_and_length >> 3;

        fhost_print(RTOS_TASK_NULL, "\nHE TB rate info:\n");
        fhost_print(RTOS_TASK_NULL,
                    "    type           rate             tpt   eprob    ok(   tot)   ul_length\n    ");

        fhost_print_rate_from_cfg(st[0].line, LINE_MAX_SZ, rate_stats->rate_config, ru_index, false, NULL);
        fhost_print(RTOS_TASK_NULL, "%s\n", st[0].line);

        tp = res.tp[RC_HE_STATS_IDX] / 10;
        fhost_print(RTOS_TASK_NULL, "      %4u.%1u", tp / 10, tp % 10);

        eprob = ((rate_stats->probability * 1000) >> 16) + 1;
        fhost_print(RTOS_TASK_NULL, "  %4u.%1u %5u(%6u)  %6u\n",
                   eprob / 10, eprob % 10, rate_stats->success,
                   rate_stats->attempts, ul_length);
    }

    fhost_print(RTOS_TASK_NULL, "\n MPDUs AMPDUs AvLen trialP");
    fhost_print(RTOS_TASK_NULL, "\n%6u %6u %3d.%1d %6u\n", res.ampdu_len,
                res.ampdu_packets, res.avg_ampdu_len >> 16,
                ((res.avg_ampdu_len * 10) >> 16) % 10, res.sample_wait);

    // free buffers
    rtos_free(st);

#endif
    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Print Rate Control statistics for every station connected to
 * a VIF passed as parameter
 *
 * @param[in] fvif_idx      Index of virtual interface in FHOST
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_rc_print_stats(int fvif_idx, bool is_rx)
{
    uint8_t i, idx_table_size;
    uint8_t idx_table[NX_REMOTE_STA_MAX];
    struct fhost_vif_tag *fhost_vif;

    fhost_vif = &fhost_env.vif[fvif_idx];

    // Get VIF status
    idx_table_size = fhost_get_sta_idx(fhost_vif, NX_REMOTE_STA_MAX, idx_table);

    for (i = 0; i < idx_table_size; i++)
    {
        if (fhost_rc_print_stats_sta(idx_table[i], is_rx) == FHOST_IPC_ERROR)
            return FHOST_IPC_ERROR;
    }
    if (!idx_table_size)
        fhost_print(RTOS_TASK_NULL, "No results\n");

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Set TX Rate Control for a certain station
 *
 * @param[in] sta               Pointer to the STA entry
 * @param[in] fixed_rate_idx    Rate control index to use
 * @return FHOST_IPC_SUCCESS if successful, FHOST_IPC_ERROR otherwise
 ****************************************************************************************
 */
static int fhost_rc_set_rate_sta(void *sta, int fixed_rate_idx)
{
    struct cfgrwnx_rc_set_rate cmd;
    struct cfgrwnx_resp resp;
    union fhost_rc_rate_ctrl_info rate_config;
    char buf[LINE_MAX_SZ];

    // Convert rate index into rate configuration
    if ((fixed_rate_idx < 0) || (fixed_rate_idx >= TOT_RATES))
    {
        // disable fixed rate
        rate_config.value = (uint32_t)-1;
    }
    else if (fhost_idx_to_rate_cfg(fixed_rate_idx, false, &rate_config, NULL))
        return FHOST_IPC_ERROR;

    printf("ridx %d => rate (format %d,dcm %d,bw %d,mcs %d,gi %d)\n",
        fixed_rate_idx, rate_config.format_mod_tx,
        rate_config.dcm_tx, rate_config.bw_tx, rate_config.mcs_idx, rate_config.gi_and_pre_type_tx
        );

    fhost_print_rate_from_cfg(buf, LINE_MAX_SZ, rate_config.value,
                              0, false, NULL);

    fhost_print(RTOS_TASK_NULL, "Fix rate to %s\n", buf);

    // prepare CFGRWNX_RC_SET_RATE_CMD to send
    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_RC_SET_RATE_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;
    cmd.sta_idx = sta_get_staid(sta);
    cmd.fixed_rate_cfg = (uint16_t) rate_config.value;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_RC_SET_RATE_RESP;

    // Send CFGRWNX_RC_SET_RATE_CMD
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
    {
        return FHOST_IPC_ERROR;
    }
    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Set TX Rate Control for all the stations associated to a VIF
 *
 * @param[in] vif_idx           Index of the virtual interface
 * @param[in] fixed_rate_idx    Rate control index to use
 * @return FHOST_IPC_SUCCESS if successful, FHOST_IPC_ERROR otherwise
 ****************************************************************************************
 */

static int fhost_rc_set_rate(int fvif_idx, int fixed_rate_idx)
{
    struct sta_info_tag *sta;
    struct fhost_vif_tag *fhost_vif = &fhost_env.vif[fvif_idx];

    sta = (struct sta_info_tag *)co_list_pick(mac_vif_get_sta_list(fhost_vif->mac_vif));

    while (sta != NULL)
    {
        if (fhost_rc_set_rate_sta(sta, fixed_rate_idx) == FHOST_IPC_ERROR)
            return FHOST_IPC_ERROR;

        sta = sta_get_list_next(sta);
    }
    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'rc' command
 *
 * Print the Rate Control Table
 * @verbatim
 rc [-m <mac_addr> | -i <itf>] [-f <idx>]
 @endverbatim
 *
 * @param[in] params  Vif interface
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_rc(char *params)
{
    struct sta_info_tag *sta = NULL;
    char *token, *next = params;
    int fhost_vif_idx = INVALID_VIF_IDX;
    int fixed_rate_idx = -1;
    bool is_print = true;
    bool is_rx = false;
    uint8_t i;

    token = fhost_ipc_next_token(&next);

    // Parsing arguments
    while (token)
    {
        // Search a VIF by VIF index
        if(!strcmp("-i", token))
        {
            token = fhost_ipc_next_token(&next);
            fhost_vif_idx = fhost_search_itf(token);
            if (fhost_vif_idx < 0)
                return FHOST_IPC_ERROR;
        }
        // Search a STA by MAC Address
        else if (!strcmp("-m", token))
        {
            struct mac_addr mac_addr;

            token = fhost_ipc_next_token(&next);
            if (fhost_ipc_parse_mac_addr(token, &mac_addr))
            {
                fhost_print(RTOS_TASK_NULL, "Invalid Mac address");
                return FHOST_IPC_ERROR;
            }

            sta = fhost_search_connected_sta_by_mac(&mac_addr);
            if (!sta)
            {
                fhost_print(RTOS_TASK_NULL, "Mac address not found");
                return FHOST_IPC_ERROR;
            }
        }
        // Set fixed rate
        else if (!strcmp("-f", token))
        {
            fixed_rate_idx = atoi(fhost_ipc_next_token(&next));
            is_print = false;
        }
        // print rx stats
        else if (!strcmp("-r", token))
            is_rx = true;

        token = fhost_ipc_next_token(&next);
    }

    // Action is printing
    if (is_print)
    {
        // A sta has been provided (-m option)
        if (sta)
        {
            return fhost_rc_print_stats_sta(sta_get_staid(sta), is_rx);
        }
        // A vif has been provided (-i option)
        else if (fhost_vif_idx != INVALID_VIF_IDX)
        {
            return fhost_rc_print_stats(fhost_vif_idx, is_rx);
        }
        // No vif or sta has been provided (no options)
        else
        {
            // Print rates for every VIF
            for (i = 0; i < NX_VIRT_DEV_MAX; i++)
            {
                if ((fhost_env.vif[i].mac_vif != NULL) &&
                    (fhost_rc_print_stats(i, is_rx) != FHOST_IPC_SUCCESS))
                    return FHOST_IPC_ERROR;
            }
            return FHOST_IPC_SUCCESS;
        }
    }
    // Action is setting rate (-f option)
    else
    {
        // A sta has been provided (-m option)
        if (sta)
            return fhost_rc_set_rate_sta(sta, fixed_rate_idx);
        // A vif has been provided (-i option)
        else if (fhost_vif_idx != INVALID_VIF_IDX)
            return fhost_rc_set_rate(fhost_vif_idx, fixed_rate_idx);
        // No vif or sta has been provided (no options)
        else
        {
            for (i = 0; i < NX_VIRT_DEV_MAX; i++)
            {
                if ((fhost_env.vif[i].mac_vif != NULL) &&
                    (fhost_rc_set_rate(i, fixed_rate_idx) == FHOST_IPC_ERROR))
                    return FHOST_IPC_ERROR;
            }
            return FHOST_IPC_SUCCESS;
        }
    }

    return FHOST_IPC_SHOW_USAGE;
}
/**
****************************************************************************************=======
 * @brief Process function for 'p2p_noa' command
 *
 * Send NOA command to setup the Notice Of Absence protocol in P2P
 *
 * @param[in] params  NOA configuration parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_ipc_p2p_noa(char *params)
{
    struct cfgrwnx_p2p_noa_cmd cmd;
    struct cfgrwnx_resp resp;
    int fhost_vif_idx = fhost_search_first_valid_itf();
    char *token, *next = params;

    uint8_t count = 0;
    uint32_t duration = 0, interval = 0;
    bool dyn_noa = false;

    while ((token = fhost_ipc_next_token(&next)))
    {
        char option;

        if ((token[0] != '-') | (token[2] != '\0'))
        {
            return FHOST_IPC_SHOW_USAGE;
        }

        option = token[1];
        token = fhost_ipc_next_token(&next);
        if (!token)
        {
            return FHOST_IPC_SHOW_USAGE;
        }

        switch(option)
        {
            case 'c':
            {
                count = atoi(token);
                break;
            }
            case 'i':
            {
                interval = atoi(token);
                break;
            }
            case 'd':
            {
                duration = atoi(token);
                break;
            }
            case 'D':
            {
                dyn_noa = atoi(token);
                break;
            }

            default:
            {
                fhost_print(RTOS_TASK_NULL, "Invalid option %c\n", option);
                return FHOST_IPC_SHOW_USAGE;
            }
        }
    }

    if (duration >= interval)
    {
        fhost_print(RTOS_TASK_NULL, "Invalid p2p NOA config: interval=%d <= duration=%d\n",
                    interval, duration);
        return FHOST_IPC_ERROR;
    }

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_P2P_NOA_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;

    cmd.fhost_vif_idx = fhost_vif_idx;
    cmd.count = count;
    cmd.interval_us = interval * TU_DURATION;
    cmd.duration_us = duration * TU_DURATION;
    cmd.dyn_noa = dyn_noa;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_P2P_NOA_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr))
        return FHOST_IPC_ERROR;

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process function for 'mesh' command
 *
 * Start a mesh point
 * @verbatim
   mesh [-i <itf>] -s <SSID> -f <freq in MHz> [-a <akm>[,<akm 2>]] [-k <key>]
        [-b bcn_int[,dtim_period]] [-u <unicast cipher>[,<unicast cipher 2>]]
        [-g <group cipher>] [-m <mfp: 0|1|2>] [-r <basic rates>] [-o <option=value>]
   @endverbatim
 *
 * @param[in] params  Connection parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */

static int fhost_ipc_mesh(char *params)
{
    #if RW_MESH_EN
    struct fhost_vif_ap_cfg cfg;
    int fhost_vif_idx = fhost_search_first_valid_itf();
    char *token, *next = params;
    int res = FHOST_IPC_ERROR;

    memset(&cfg, 0, sizeof(cfg));

    while ((token = fhost_ipc_next_token(&next)))
    {
        char option;

        if ((token[0] != '-') | (token[2] != '\0'))
        {
            return FHOST_IPC_SHOW_USAGE;
        }

        option = token[1];
        token = fhost_ipc_next_token(&next);
        if (!token)
        {
            return FHOST_IPC_SHOW_USAGE;
        }

        switch(option)
        {
            case 'i':
            {
                fhost_vif_idx = fhost_search_itf(token);
                if (fhost_vif_idx < 0)
                    goto end;
                break;
            }
            case 's':
            {
                size_t ssid_len = strlen(token);
                if (ssid_len > sizeof(cfg.ssid.array))
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid SSID");
                    goto end;
                }

                memcpy(cfg.ssid.array, token, ssid_len);
                cfg.ssid.length = ssid_len;
                break;
            }
            case 'k':
            {
                size_t key_len = strlen(token);
                if ((key_len + 1) > sizeof(cfg.key))
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid Key");
                    goto end;
                }
                strcpy(cfg.key, token);
                break;
            }
            case 'f':
            {
                int len = strlen(token) - 1;
                struct mac_chan_def *chan = NULL;
                int offset = 0;
                if (token[len] == '+')
                {
                    token[len] = 0;
                    offset = 10;
                    cfg.chan.type = PHY_CHNL_BW_40;
                }
                else if (token[len] == '-')
                {
                    token[len] = 0;
                    offset = -10;
                    cfg.chan.type = PHY_CHNL_BW_40;
                }
                else if (token[len] == '@')
                {
                    token[len] = 0;
                    cfg.chan.type = PHY_CHNL_BW_80;
                }
                else
                {
                    cfg.chan.type = PHY_CHNL_BW_20;
                }

                cfg.chan.prim20_freq = atoi(token);
                chan = fhost_chan_get(cfg.chan.prim20_freq);
                if (!chan)
                {
                    fhost_print(RTOS_TASK_NULL, "Invalid channel\n");
                    goto end;
                }

                if (cfg.chan.prim20_freq >= PHY_FREQ_5G)
                    cfg.chan.band = PHY_BAND_5G;
                else
                    cfg.chan.band = PHY_BAND_2G4;

                if (cfg.chan.type == PHY_CHNL_BW_80)
                {
                    if ((cfg.chan.prim20_freq < 5180) ||
                        (cfg.chan.prim20_freq > 5805))
                    {
                        fhost_print(RTOS_TASK_NULL, "Invalid primary for 80MHz channel\n");
                        goto end;
                    }
                    offset = (cfg.chan.prim20_freq - 5180) % 80;
                    if (offset < 20)
                        offset = 30;
                    else if (offset < 40)
                        offset = 10;
                    else if (offset < 60)
                        offset = -10;
                    else
                        offset = -30;
                }
                cfg.chan.center1_freq = cfg.chan.prim20_freq + offset;
                break;
            }
            case 'a':
            {
                if (fhost_ipc_read_akm(token, &cfg.akm, true))
                    goto end;

                break;
            }
            case 'u':
            case 'g':
            {
                uint32_t *cipher;
                if (option == 'u')
                    cipher = &cfg.unicast_cipher;
                else
                    cipher = &cfg.group_cipher;

                if (fhost_ipc_read_cipher(token, cipher, true))
                    goto end;

                break;
            }
            case 'b':
            {
                char *dtim = strchr(token, ',');
                if (dtim)
                {
                    *dtim++ = 0;
                    cfg.dtim_period = atoi(dtim);
                }
                cfg.bcn_interval = atoi(token);

                break;
            }
            case 'm':
            {
                cfg.mfp = atoi(token);
                break;
            }
            case 't':
            {
                if (fhost_ipc_read_mode(token, &cfg.mode, true))
                    goto end;
                break;
            }
            case 'r':
            {
                int i = 0;
                cfg.basic_rates[i++] = atoi(token);

                char *next_copy;
                next_copy = next;
                while ((token = fhost_ipc_next_token(&next_copy)))
                {
                    if ((token[0] == '-') | (token[0] == '\0'))
                    {
                        break;
                    }
                    fhost_ipc_next_token(&next);
                    cfg.basic_rates[i++] = atoi(token);
                }
                break;
            }
            case 'o':
            {
                int opt_len = strlen(token) + 2;
                char *tmp = strchr(token, '=');

                if (!tmp)
                {
                    res = FHOST_IPC_SHOW_USAGE;
                    goto end;
                }
                *tmp = ' ';

                tmp = cfg.extra_cfg;
                if (tmp)
                    opt_len += strlen(tmp);

                cfg.extra_cfg = rtos_malloc(opt_len);
                if (!cfg.extra_cfg)
                {
                    fhost_print(RTOS_TASK_NULL, "no more memory\n");
                    cfg.extra_cfg = tmp;
                    goto end;
                }

                if (tmp)
                {
                    dbg_snprintf(cfg.extra_cfg, opt_len, "%s%s;", tmp, token);
                    rtos_free(tmp);
                }
                else
                {
                    dbg_snprintf(cfg.extra_cfg, opt_len, "%s;", token);
                }
                break;
            }
            default:
            {
                fhost_print(RTOS_TASK_NULL, "Invalid option %c\n", option);
                return FHOST_IPC_SHOW_USAGE;
            }
        }
    }

    if (fhost_vif_idx < 0)
        goto end;

    if ((cfg.ssid.length == 0) || (cfg.chan.prim20_freq == 0))
    {
         res = FHOST_IPC_SHOW_USAGE;
         goto end;
    }

    // try to select the best AKM if not set
    if (cfg.akm == 0)
    {
        if (strlen(cfg.key) == 0)
            cfg.akm = CO_BIT(MAC_AKM_NONE);
        else
            cfg.akm = CO_BIT(MAC_AKM_SAE);
    }

    // Do not set the VIF type to MESH_POINT here as it does not work.
    if (fhost_ap_cfg(fhost_vif_idx, &cfg, true))
    {
        fhost_print(RTOS_TASK_NULL, "Failed to start MESH, check your configuration");
        goto end;
    }

    res = FHOST_IPC_SUCCESS;

  end:
    if (cfg.extra_cfg)
        rtos_free(cfg.extra_cfg);

    return res;

    #else
    return FHOST_IPC_ERROR;
    #endif // RW_MESH_EN
}

/**
 ****************************************************************************************
 * @brief Process function for 'ftm' command
 *
 * Start a Fine Timing Measurement
 * @verbatim
   ftm

   @endverbatim
 *
 * @param[in] params  Connection parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */

static int fhost_ipc_ftm(char *params)
{
    #if NX_FTM_INITIATOR
    struct mac_ftm_results res;

    if (fhost_ftm(cntrl_link, fhost_search_first_valid_itf(), &res))
        return FHOST_IPC_ERROR;

    fhost_print(RTOS_TASK_NULL, "Number of FTM responders found: %d\n",
                res.nb_ftm_rsp);

    for (int i = 0; i < res.nb_ftm_rsp; i++)
    {
        fhost_print(RTOS_TASK_NULL, "BSSID=%02x:%02x:%02x:%02x:%02x:%02x RTT=%d \n",
        ((uint8_t *)res.meas[i].addr.array)[0], ((uint8_t *)res.meas[i].addr.array)[1],
        ((uint8_t *)res.meas[i].addr.array)[2], ((uint8_t *)res.meas[i].addr.array)[3],
        ((uint8_t *)res.meas[i].addr.array)[4], ((uint8_t *)res.meas[i].addr.array)[5],
        res.meas[i].rtt);
    }

    return FHOST_IPC_SUCCESS;
    #else
    return FHOST_IPC_ERROR;
    #endif // NX_FTM_INITIATOR
}

static int fhost_ipc_rate(char *params)
{
    char *token, *next = params;
    int msg_id;

    token = fhost_ipc_next_token(&next);
    if (!token)
        return FHOST_IPC_SHOW_USAGE;

    msg_id = atoi(token);

    printf("rate is %x:%d\r\n", msg_id, msg_id);
extern uint32_t g_fw_rate;
    g_fw_rate = msg_id;

    return FHOST_IPC_SUCCESS;
}

void fhost_ipc_help(void)
{
    if (wifiMgmr.ready != 1) {
        return;
    }
    fhost_ipc_list(NULL);
}

/// Array of supported IPC command
static const struct fhost_ipc_cmd ipc_commands[] = {
    {fhost_ipc_list, "list_cmd", ""},
    {fhost_ipc_status, "status", "[chan] [vif]"},
    {fhost_ipc_rate, "rate", "rate"},
    {fhost_ipc_wpa, "wpa_supplicant", "[-i <itf>]"},
    {fhost_ipc_wpa_cli, "wpa_cli", "[-i <itf>] <cmd>"},
    {fhost_ipc_mactrace, "mactrace", ""},
    {fhost_ipc_free, "free", ""},
#if NX_IPERF
    {fhost_ipc_iperf, "iperf", "-s|-c <host>|-h [options (use -h for details)]\n"
     "                           stop"},
#endif
    {fhost_ipc_ip, "ip",
     "addr [show [itf]]>\n"
     "                           addr add <ip>/<mask> <itf>\n"
     "                           addr del <itf>"},
    {fhost_ipc_scan, "scan", "[-i <itf>]"},
#if NX_PING
    {fhost_ipc_ping, "ping",
     "[-s <pkt_size>] [-r <rate>] [-d (duration)] [-Q <ToS>] [-G (background)] <dest_ip>\n"
     "                           stop <id>"},
#endif
    {fhost_ipc_monitor, "monitor",
     "start <itf> <freq> [20|40|80|80+80|160] [<center1_freq>] [<center2_freq>]\n"
     "                           stop  <itf>"},
    {fhost_ipc_sys_stats, "sys_stats", ""},
    {fhost_ipc_sigkill, "sigkill", "<cmd_id>"},
    {fhost_ipc_dhcpc, "dhcpc", "start|stop [<itf>]"},
    {fhost_ipc_list_features, "list_features", ""},
    {fhost_ipc_connect, "connect", "[-i <itf>] -s <SSID> [-k <key>] [-b <bssid>]\n"
     "                           [-f <freq>[,freq]] [-a <akm>] [-t <timeout_ms>]\n"
     "                           [-u <unicast cipher list>] [-g <group cipher list>]"},
    {fhost_ipc_disconnect, "disconnect", "[-i <itf>]" },
    {fhost_ipc_powersave, "powersave", "[-i <itf>] [-u <uapsd_queues>] <off|on|dyn>"},
    {fhost_ipc_ap, "ap", "[-i <itf>] -s <SSID> -f <freq>[+-@] [-a <akm>[,<akm 2>]] [-k <key>]\n"
     "                           [-b bcn_int[,dtim_period]] [-m <mfp: 0|1|2>] [-t <OP mode>]\n"
     "                           [-u <unicast cipher list>] [-g <group cipher>] [-o <opt>=<val>]\n"
     "                           [-n <isolation mode:on|off>]"},
    {fhost_ipc_rc, "rc", "[-m <mac_addr> | -i <itf>] [-f <idx>][-r]"},
    {fhost_ipc_p2p_noa, "p2p_noa", "[-c <count>] [-i <interval in TU>] [-d <duration in TU>] [-D <dynamic control: 0|1>]"},
    {fhost_ipc_mesh, "mesh", "[-i <itf>] [-s <ssid>] [-f <frequency in MHz>] [-a <akm>[,<akm 2>]] [-k <key>]\n"
     "                           [-b bcn_int[,dtim_period]] [-m <mfp: 0|1|2>] [-t <OP mode>]\n"
     "                           [-u <unicast cipher list>] [-g <group cipher>] [-o <opt>=<val>]\n"
     "                           [-r <basic rates>]"},
    {fhost_ipc_ftm, "ftm", "to define"},
    {NULL, "", ""}
};


/**
 ****************************************************************************************
 * @brief Send a print message to the host.
 *
 * Copy the message id of the ipc message received from the host into the print message.
 * Message will be uploaded using DMA transfer, and @ref ipc_emb_msg_dma_int_handler is
 * called when the transfer is done.
 *
 * @param[in] handle         RTOS task handle
 * @param[in] ipc_print_msg  Pointer to message to send to host
 * @param[in] len            Length of the message
 *
 * @return 0 on success and != 0 if error occured.
 ****************************************************************************************
 */
static void fhost_send_print_msg(rtos_task_handle handle,
                                 struct ipc_fhost_msg *ipc_print_msg,
                                 uint16_t len)
{
#if 0
    if (handle != RTOS_TASK_NULL)
    {
        struct fhost_task_msg_id *task_hdl_msg_ptr = fhost_search_task_hdl_msg(handle);

        if (!task_hdl_msg_ptr)
            return;

        *ipc_print_msg->data = task_hdl_msg_ptr->msg_id;
    }
    else
    {
        *ipc_print_msg->data = ipc_msg.id;
    }

    len += sizeof(ipc_msg.id);
    if (len > sizeof(ipc_print_msg->data))
    {
        len = sizeof(ipc_print_msg->data);
    }
    ipc_emb_msg_fwd(IPC_EMB_PRINT_MSG, len, ipc_print_msg->data);
#else
    int i;
    uint8_t *buf = (uint8_t *)ipc_print_msg->data + sizeof(ipc_msg.id);

    i = 0;
    puts((const char*)buf);
#endif
}

int fhost_ipc_write_cmd(int msg)
{
    return rtos_queue_write(cmd_queue, &msg, -1, false);
}

int fhost_ipc_read_cmd(void)
{
    int msg;

    rtos_queue_read(cmd_queue, &msg, -1, false);

    return msg;
}

void *fhost_cmd_tab_complete(char *inbuf, unsigned int *bp, int cli_cmd, int *fhost_cmd)
{
    int         i, m;
    const char *fm = NULL;

    /* show matching commands */
    for (i = 0, m = 0; i < sizeof(ipc_commands)/sizeof(struct fhost_ipc_cmd) ; i++) {
        const struct fhost_ipc_cmd *cmd = &ipc_commands[i];
        if (cmd->exec != NULL) {
            if (!strncmp(inbuf, cmd->name, *bp)) {
                m++;
                if (m == 1) {
                    fm = cmd->name;
                    if (cli_cmd != 0) {
                        fhost_print(RTOS_TASK_NULL, "%s ", cmd->name);
                    }
                } else if (m == 2)
                    if (cli_cmd != 0) {
                        fhost_print(RTOS_TASK_NULL, "%s ", cmd->name);
                    } else {
                        fhost_print(RTOS_TASK_NULL, "%s %s ", fm, cmd->name);
                    }
                else
                    fhost_print(RTOS_TASK_NULL, "%s ", cmd->name);
            }
        }
    }

    *fhost_cmd = m;

    if (m == 1 && fm) {
        return (void *)fm;
    } else {
        return NULL;
    }
}

/**
 ****************************************************************************************
 * @brief Process an IPC message
 *
 * The message is recopied locally, and message is acknowledged to the host.
 *
 * Then the command name is extracted and checked among all registered commands.
 * If the command is found the corresponding process function is called with the rest of
 * the message as parameters.
 * After return of the process function, command status is uploaded as it to the host
 * unless the process function returned FHOST_IPC_NO_RESP. In the latter case, the
 * a new task has been created to execute the command "in background" allowing new
 * command to be executed. Response to host will send once the "background" task finish
 * (using @ref fhost_ipc_send_cmd_cfm)
 *
 * If the command is not found, an error message is also reported to the host.
 *
 * @param[in] msg Message to process
 *
 ****************************************************************************************
 */
static void fhost_ipc_msg_process(struct fhost_msg *msg)
{
    char *command, *param;
    const struct fhost_ipc_cmd *cmd;
    uint32_t res;

    ipc_msg.len = 0;
    ipc_msg.id = FHOST_MSG_INDEX(msg->id);
    memcpy(ipc_msg.data, msg->data, msg->len);
    ipc_emb_ack_msg();

    command = (char *)ipc_msg.data;
    param = strchr(command, ' ');
    if (param)
    {
        *param++ = '\0';
        while (*param == ' ')
            param++;
    }
    else
    {
        /* be sure to have \0 in command */
        command[msg->len - 1] = '\0';
    }

    cmd = ipc_commands;
    while (cmd->exec)
    {
        if (!strcmp(command, cmd->name))
            break;
        cmd++;
    }

    if (cmd->exec)
    {
        fhost_ipc_write_cmd(0);
        fhost_print(RTOS_TASK_NULL, "\r\n");
        res = (uint32_t) cmd->exec(param);
        /* Add default response */
        if (res == FHOST_IPC_NO_RESP)
        {
            return;
        }
        else if (res == FHOST_IPC_SHOW_USAGE)
        {
            fhost_print(RTOS_TASK_NULL, "Usage:\n%s %s\n",
                        cmd->name, cmd->params);
        }
    }
    else
    {
        fhost_ipc_write_cmd(-1);
        res = FHOST_IPC_UNKWN_CMD;
    }

    //ipc_emb_msg_fwd(ipc_msg.id, sizeof(res), &res);
}

/**
 ****************************************************************************************
 * @brief IPC task runtime initialization
 *
 * Initialization when IPC task is first ran:
 * - IPC task is created with lower priority to be sure that control task is executed
 *   first to create the network interface. Now we can move to the "real" priority
 * - Assign default IP address to all network interface
 ****************************************************************************************
 */
static void fhost_ipc_init()
{
    struct fhost_vif_tag *fhost_vif;
    int idx;

    for (idx = 0, fhost_vif = &fhost_env.vif[0]; idx < NX_VIRT_DEV_MAX; idx++, fhost_vif++)
    {
        struct fhost_vif_ip_addr_cfg ip_cfg;
        uint8_t mac_addr1 = fhost_vif->mac_addr.array[0] >> 8;

        ip_cfg.mode = IP_ADDR_STATIC_IPV4;
        ip_cfg.ipv4.addr = 0;
        ip_cfg.ipv4.mask = 0;
        ip_cfg.ipv4.gw = 0;
        ip_cfg.ipv4.dns = 0;

        if (idx == 0)
            ip_cfg.default_output = true;
        else
            ip_cfg.default_output = false;

        if (fhost_set_vif_ip(idx, &ip_cfg)) {
            TRACE_APP(ERR, "Failed to configure IP address for FVIF-%d", idx);
        }
    }

    cntrl_link = fhost_cntrl_cfgrwnx_link_open();
    if (cntrl_link == NULL)
    {
        TRACE_APP(ERR, "Failed to open link with control task");
        ASSERT_ERR(0);
    }
}

/**
 ****************************************************************************************
 * @brief IPC task main loop
 *
 * IPC task may received command from the host via IPC, or information event from the
 * supplicant task.
 *
 ****************************************************************************************
 */
static RTOS_TASK_FCT(fhost_ipc_task)
{
    struct fhost_msg msg;

    fhost_wait_ready();
    fhost_ipc_init();

    platform_post_event(EV_WIFI, CODE_WIFI_ON_MGMR_DONE);

    for( ;; )
    {
        rtos_queue_read(queue, &msg, -1, false);

        switch (FHOST_MSG_TYPE(msg.id))
        {
            case FHOST_MSG_IPC:
                fhost_ipc_msg_process(&msg);
                break;
            default:
                TRACE_APP(ERR, "Task IPC: unsupported message received (%d/%d)",
                          FHOST_MSG_TYPE(msg.id),
                          FHOST_MSG_INDEX(msg.id));
                break;
        }
    }
}

/****************************************************************************************
 * Task interface
 ***************************************************************************************/
/*
 * When using IPC, the main application runs a very basic shell.
 * It receives command from the host, processes them and uploads a response to host.
 * A command may be processed locally by the IPC task or offloaded to another task
 * (e.g. wpa_supplicant task)
 */
int fhost_application_init(void)
{
    u8_t i;
    rtos_task_handle task_handle;

extern int bl_wifi6_cli_init(void);
    bl_wifi6_cli_init();

    for (i= 0; i < FHOST_TASK_HANDLE_LEN; i++)
    {
        task_handle_msg_id[i].task_handle = NULL;
    }

    if (rtos_queue_create(sizeof(struct fhost_msg), FHOST_IPC_QUEUE_SIZE, &queue))
        return 2;
    
    if (rtos_queue_create(sizeof(int), FHOST_IPC_QUEUE_SIZE, &cmd_queue))
        return 2;


    if (rtos_task_create(fhost_ipc_task, "IPC task", APPLICATION_TASK,
                         FHOST_IPC_STACK_SIZE, NULL, FHOST_IPC_PRIORITY, &task_handle))
        return 1;

    return 0;
}

/*
 * When using IPC, message is uploaded to host using DMA transfer.
 */
int fhost_print_buf(rtos_task_handle handle, const char *buf)
{
    struct ipc_fhost_msg ipc_print_msg;
    char *data;
    uint16_t rem_len = (strlen(buf) + 1);
    uint16_t len = 0;

    while (rem_len)
    {
        buf += len;
        len = rem_len > (sizeof(ipc_print_msg.data) - sizeof(ipc_msg.id))
                        ? (sizeof(ipc_print_msg.data) - sizeof(ipc_msg.id)) : rem_len;

        data = (char *) ipc_print_msg.data + sizeof(ipc_msg.id);
        memcpy(data, buf, len);
        fhost_send_print_msg(handle, &ipc_print_msg, len);
        rem_len -= len;
    }

    return 0;
}

/*
 * When using IPC, message is uploaded to host using DMA transfer.
 */
int fhost_print(rtos_task_handle handle, const char *fmt, ...)
{
    va_list args;
    struct ipc_fhost_msg ipc_print_msg;
    uint32_t len, offset = 0, remain = sizeof(ipc_print_msg.data) - sizeof(ipc_msg.id);
    char* data;

    do
    {
        data = (char *) ipc_print_msg.data;
        data += sizeof(ipc_msg.id);
        va_start(args, fmt);
        len = dbg_vsnprintf_offset((char*) data, remain, offset, fmt, args);
        va_end(args);

        if (len >= offset + remain)
        {
            fhost_send_print_msg(handle, &ipc_print_msg, remain - 1);
        }
        else
        {
            fhost_send_print_msg(handle, &ipc_print_msg, len - offset);
        }

        //Increase offset by remain to write the next chunk in data
        offset += remain - 1;
    } while (len >= offset);
    return 0;
}

int fhost_ipc_write_msg(int id, void *msg_data, int len, bool isr)
{
    struct fhost_msg msg;

    msg.id   =  FHOST_MSG_ID(FHOST_MSG_IPC, id);
    msg.len  = len;
    msg.data = msg_data;

    return rtos_queue_write(queue, &msg, -1, isr);
}

/*
void fhost_ipc_error_ind(void)
{
    char ind[16];
    unsigned int len;

    len = dbg_snprintf(ind, sizeof(ind), "fwdump %d", sizeof(struct dbg_debug_dump_tag));
    if (len < sizeof(ind))
        len ++;
    else
        len = sizeof(ind);
    ipc_emb_msg_fwd(IPC_EMB_MSG_IND, len, (uint32_t *)ind);

    if (task_waiting_error_ind)
    {
        rtos_task_notify(task_waiting_error_ind, false);
        task_waiting_error_ind = NULL;
    }
}
*/

void fhost_ipc_send_cmd_cfm(rtos_task_handle handle, uint32_t status)
{
    struct fhost_task_msg_id *task_hdl_msg_ptr = fhost_search_task_hdl_msg(handle);

    if (!task_hdl_msg_ptr)
        return;

    ipc_emb_msg_fwd(task_hdl_msg_ptr->msg_id, sizeof(status), &status);
    task_hdl_msg_ptr->task_handle = RTOS_TASK_NULL;
    task_hdl_msg_ptr->sigkill_cb = NULL;
}

/*
 * fhost_config_get_next_item: Must return configuration item one by one.
 * In this example confguration is saved in the global variable example_config
 */
void fhost_config_get_next_item(struct fhost_config_item *item)
{
    static int index;

    // First call, initialize index
    if (item->data == NULL)
        index = 0;

    // Check that we don't read outside of example_config table
    if (index > (sizeof(example_config) / sizeof(struct fhost_config_item)))
    {
        item->id = FHOST_CFG_END;
    }
    else
    {
        // Copy one configuration parameter
        *item = example_config[index];
    }

    // Update index for next call
    index++;
}

/*
 * XXX Caution free resp_result pointed memory when it's done
 *
 * */
int fhost_ipc_wpa_cli_api(int fhost_vif_idx, char *cmd, char **resp_result)
{
    int res, resp_len;
    char *resp;

    //*resp_result is NOT allowed to be NULL
    if (*resp_result) {
        return FHOST_IPC_ERROR;
    }

    if (fhost_vif_idx < 0) {
        fhost_vif_idx = fhost_search_first_valid_itf();
    }

    if (fhost_vif_idx < 0)
        return FHOST_IPC_ERROR;

    // for backward compatibilty (mainly for MVT)
    if (strncmp(cmd, "TERMINATE", 9) == 0)
    {
        if (fhost_wpa_remove_vif(fhost_vif_idx))
            res = FHOST_IPC_ERROR;
        else
            res = FHOST_IPC_SUCCESS;
    }
    else
    {
        resp_len = 2048;
        resp = rtos_malloc(resp_len + 1);
        if (!resp)
            return FHOST_IPC_ERROR;

        res = fhost_wpa_execute_cmd(fhost_vif_idx, resp, &resp_len, 10000, cmd);
        if (res >= 0)
        {
            resp[resp_len] = '\0';
#if 0
            fhost_print_buf(RTOS_TASK_NULL, resp);
#endif
            if (res > 0)
                res = FHOST_IPC_ERROR;
            else
                res = FHOST_IPC_SUCCESS;
        }
        else
        {
            res = FHOST_IPC_ERROR;
        }
        *resp_result = resp;

    }

    return res;
}

int fhost_ipc_wpa_cli_api_free(char *resp_result)
{
    rtos_free(resp_result);
    return 0;
}

#ifndef BL_WIFI_LP_FW 
static char _cmd_buf[256];
int _extra_command(char *cmd_string)
{
    int len;
    static int id;

    if (wifiMgmr.ready != 1) {
        return -1;
    }

    len = strlen(cmd_string);
    if ((len < sizeof(_cmd_buf))  && (len != 0)) {
        memcpy(_cmd_buf, cmd_string, len);
        _cmd_buf[len] = '\0';
        fhost_ipc_write_msg(id++, (void *)_cmd_buf, len + 1, 0);
        return fhost_ipc_read_cmd();
    } else {
        return -1;
    }
}

#endif

/**
 * @}
 */
