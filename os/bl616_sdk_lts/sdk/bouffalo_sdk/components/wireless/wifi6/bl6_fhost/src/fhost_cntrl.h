/**
 ****************************************************************************************
 *
 * @file fhost_cntrl.h
 *
 * @brief Definition of control task for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */
#ifndef _FHOST_CNTRL_H_
#define _FHOST_CNTRL_H_

/**
 ****************************************************************************************
 * @defgroup FHOST_CNTRL FHOST_CNTRL
 * @ingroup FHOST
 * @brief Control task for fully hosted firmware
 *
 * @{
 ****************************************************************************************
 */

#include "cfgrwnx.h"
#include "fhost_rx.h"

struct cfgrwnx_msg_hdr;

struct reason_code {
    uint16_t reason_code;
    const char *action;
};

typedef struct {
    int proto;
    int pairwise_cipher;
    int group_cipher;
    int key_mgmt;
    int capabilities;
    size_t num_pmkid;
    const uint8_t *pmkid;
    int mgmt_group_cipher;
} wifi_wpa_ie_t;

typedef enum {
    WIFI_CIPHER_TYPE_NONE = 0,   /**< the cipher type is none */
    WIFI_CIPHER_TYPE_WEP40,      /**< the cipher type is WEP40 */
    WIFI_CIPHER_TYPE_WEP104,     /**< the cipher type is WEP104 */
    WIFI_CIPHER_TYPE_TKIP,       /**< the cipher type is TKIP */
    WIFI_CIPHER_TYPE_CCMP,       /**< the cipher type is CCMP */
    WIFI_CIPHER_TYPE_TKIP_CCMP,  /**< the cipher type is TKIP and CCMP */
    WIFI_CIPHER_TYPE_AES_CMAC128,/**< the cipher type is AES-CMAC-128 */
    WIFI_CIPHER_TYPE_SMS4,       /**< the cipher type is SMS4 */
    WIFI_CIPHER_TYPE_UNKNOWN,    /**< the cipher type is unknown */
} wifi_cipher_type_t;


/// Structure containing parameters to establish link with cntrl
struct fhost_cntrl_link
{
    /// CFGRWNX queue to get response from CNTRL task
    rtos_queue queue;
    /// Socket, for CNTRL task, to send events
    int sock_send;
    /// Socket to receive events from CNTRL task
    int sock_recv;
};

struct fhost_mod_params {
    uint8_t listen_itv;
};

/**
 ****************************************************************************************
 * @brief Send a message to the Control TASK.
 *
 * The message pushed to the control task will only contains pointer to the actual
 * message, thus memory pointed by @p msg_data must not be modified until control task
 * process the message.
 *
 * For now:
 * - for @ref FHOST_MSG_KE_WIFI messages, control task will free (with ke_free)
 * them once processed.
 * - for @ref FHOST_MSG_CFGRWNX messages, it is assumed that supplicant will always
 * wait for a response from control task before re-using/freeing message memory.
 *
 * @param[in] id       ID of message (type(@ref fhost_msg_type) + index).
 * @param[in] msg_data Buffer containing the message.
 * @param[in] len      Length, in bytes, of the message.
 * @param[in] timeout  Maximum duration, in ms, to wait if queue is full.
 *                     0 means do not wait and -1 means wait indefinitely.
 * @param[in] isr      Indicates if function is called from ISR.
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_write_msg(int id, void *msg_data, int len, int timeout, bool isr);

/**
 ****************************************************************************************
 * @brief Send a CFGRWNX command to the Control TASK and get the response.
 *
 * @param[in]     cmd    Pointer to the command header (followed by the parameters)
 * @param[in,out] resp   Pointer to the response header (followed by the space for the
 *                       response parameters). This parameter can be set to NULL if no
 *                       response is expected.
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_cfgrwnx_cmd_send(struct cfgrwnx_msg_hdr *cmd,
                                 struct cfgrwnx_msg_hdr *resp);

/**
 ****************************************************************************************
 * @brief Send a CFGRWNX event to the specified socket
 *
 * Event will be entirely copied to the socket so memory pointed by msg can be re-used
 * after calling this function.
 *
 * @param[in] msg_hdr  Pointer on the header of the event to send.
 * @param[in] sock     Socket to send CFGRWNX events
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_cfgrwnx_event_send(struct cfgrwnx_msg_hdr *msg_hdr, int sock);

/**
 ****************************************************************************************
 * @brief Wait until new event is available.
 *
 * This function is blocking until a new event is sent by the CNTRL task on the specified
 * link. This function is used to get the event ID and length without consuming it and
 * then @ref fhost_cntrl_cfgrwnx_event_get can be called with a buffer big enough to
 * retrieve the complete event or @ref fhost_cntrl_cfgrwnx_event_discard to discard it.
 *
 * @param[in]  link     Link with CNTRL task to use
 * @param[out] msg_hdr  Event header
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_cfgrwnx_event_peek_header(struct fhost_cntrl_link *link,
                                          struct cfgrwnx_msg_hdr *msg_hdr);

/**
 ****************************************************************************************
 * @brief Read oldest event available
 *
 * This function doesn't wait and it is meant to be called after @ref
 * fhost_cntrl_cfgrwnx_event_peek_header once the event ID and length are known.
 *
 * @param[in]  link   Link with CNTRL task to use
 * @param[out] event  Event buffer
 * @param[in]  len    Length, in bytes, of the @p event buffer
 *
 * @return Number of bytes written on success and < 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_cfgrwnx_event_get(struct fhost_cntrl_link *link, void *event, int len);

/**
 ****************************************************************************************
 * @brief Discard oldest event available.
 *
 * This function doesn't wait and it is meant to be called after @ref
 * fhost_cntrl_cfgrwnx_event_peek_header once the event ID is known.
 *
 * @param[in] link     Link with CNTRL task to use
 * @param[in] msg_hdr  Event header from @ref fhost_cntrl_cfgrwnx_event_peek_header
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_cfgrwnx_event_discard(struct fhost_cntrl_link *link,
                                      struct cfgrwnx_msg_hdr *msg_hdr);

/**
 ****************************************************************************************
 * @brief Change type of the MAC vif associated to a FHOST vif.
 *
 * Send CFGRWNX_SET_VIF_TYPE_CMD command to CNTRL task and wait for the response.
 * This will always delete the current MAC vif (if it exists) and create a new one
 * (unless type == VIF_UNKNOWN).
 *
 * @param[in] link           Link with CNTRL task to use
 * @param[in] fhost_vif_idx  Index of the FHOST vif
 * @param[in] type           Type to set on MAC vif
 * @param[in] p2p            P2P flag
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_set_mac_vif_type(struct fhost_cntrl_link *link, int fhost_vif_idx,
                                 enum mac_vif_type type, bool p2p);

#if NX_FHOST_MONITOR
/**
 ****************************************************************************************
 * @brief Configure a monitor interface
 *
 * Send message to control task to configure a monitor interface. The interface type
 * must have been changed to VIF_MONITOR (using @ref fhost_set_vif_type) before calling
 * this function.
 *
 * @param[in] link           Link with CNTRL task to use
 * @param[in] fhost_vif_idx  Index of the FHOST vif
 * @param[in] cfg            Configuration to apply
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_monitor_cfg(struct fhost_cntrl_link *link, int fhost_vif_idx,
                            struct fhost_vif_monitor_cfg *cfg);

#endif // NX_FHOST_MONITOR

/**
 ****************************************************************************************
 * @brief Create the Control task and the different queues associated to this task.
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_init(void);

/**
 ****************************************************************************************
 * @brief Send CFGRWNX_NEW_PEER_CANDIDATE_EVENT event
 *
 * @param[in] vif_idx         Index of the FHOST vif
 * @param[in] buf             buf Pointer to the RX buffer to forward
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_new_peer_candidate(int vif_idx, struct fhost_rx_buf_tag *buf);

/**
 ****************************************************************************************
 * @brief Send ME_CHAN_CONFIG_REQ to KE
 *
 * @param[in] channel_num         The max channel support
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_chan_config_update(int channel_num);

/**
 ****************************************************************************************
 * @brief Set listen interval
 *
 * set listen interval
 *
 * @param[in] listen interval val
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_set_listen_itv(uint8_t itv);

/**
 ****************************************************************************************
 * @brief Get listen interval
 *
 * get listen interval
 *
 * @return listen interval val.
 ****************************************************************************************
 */
uint8_t fhost_get_listen_itv(void);

/**
 ****************************************************************************************
 * @brief Restart fhost cntrl
 *
 * restart fhost cntrl
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_cntrl_restart(void);

/**
 * @}
 */
#endif /* _FHOST_CNTRL_H_ */
