/**
 ****************************************************************************************
 *
 * @file macif.h
 *
 * @brief Control interface for MAC software.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MACIF MACIF
 * @ingroup MACSW
 * @brief This define the interface used by MAC software to communicate with the
 * controlling layer.
 * Several implementation of this interface are available depending if MAC software is
 * controlled locally or from a remote CPU.
 * @{
 ****************************************************************************************
 */

#include "export/ke/ke_msg.h"
#include "ip/lmac/src/tx/tx_swdesc.h"

/**
 ****************************************************************************************
 * @brief Initialize the MAC interface
 *
 * @return 0 if success and !=0 is an error occurred
 ****************************************************************************************
 */
int macif_init(void);

/**
 ****************************************************************************************
 * @brief Template function for pushing a kmsg from the control layer to the wifi stack.
 *
 * Different messages (defined by parameter nb_msb)  will be copied into a Kmsg for the
 * wifi task, it can then be re-used after the call of this function.
 * May not be needed if control layer is using @ref KE_EVT_MACIF_MSG_BIT to indicate
 * new message to wifi stack.
 *
 * @param[in] msg_id    Message id.
 * @param[in] task_id   Id of the Wifi sub task destination.
 * @param[in] nb_msg    Number of messages to copy.
 * @param[in] ...       List of nb_msg (pointer to msg param, length in bytes)
 *
 * @return -1 allocation for message failed and 0 otherwise.
 ****************************************************************************************
 */
int macif_kmsg_push_t(int msg_id, int task_id, int nb_msg, ... );

/**
 ****************************************************************************************
 * @brief Push a kmsg from the control layer to the wifi stack.
 *
 * Memory pointed by @p param will be copied into a Kmsg for the wifi task, it can then
 * be re-used after the call of this function.
 * May not be needed if control layer is using @ref KE_EVT_MACIF_MSG_BIT to indicate
 * new message to wifi stack.
 *
 * @param[in] msg_id    Message id.
 * @param[in] task_id   Id of the Wifi sub task destination.
 * @param[in] param     Pointer to message parameters
 * @param[in] param_len Size, in bytes, on the @p param buffer
 *
 * @return -1 allocation for message failed and 0 otherwise.
 ****************************************************************************************
 */
int macif_kmsg_push(int msg_id, int task_id, void *param, int param_len);

/**
 ****************************************************************************************
 * @brief Kernel event handler for the handling of messages pushed by control layer
 *
 * This is wifi task's handler for @ref KE_EVT_MACIF_MSG_BIT.
 * It is assumed that control layer provides msg address by other means (e.g. by using a
 * global variable).
 * May not be needed if control layer is using @ref macif_kmsg_push to send message to
 * wifi stack.
 *
 * @param[in] dummy Parameter not used but required to follow the kernel event callback
 * format
 *
 ****************************************************************************************
 */
void macif_msg_evt(int dummy);

/**
 ****************************************************************************************
 * @brief Forward a kernel message to the MAC interface
 *
 * @param[in] ke_msg Pointer on the message
 *
 ****************************************************************************************
 */
void macif_kmsg_fwd(const struct ke_msg *ke_msg);

/**
 ****************************************************************************************
 * @brief Signal to the upper layers the presence of data in the host RX buffers.
 * This function typically triggers an interrupt to the host CPU. It is not used in
 * fully hosted partitioning.
 ****************************************************************************************
 */
void macif_rx_data_ind(void);

/**
 ****************************************************************************************
 * @brief Get the number of received packets above which the host shall be warned.
 *
 * @return The RX packet threshold
 *
 ****************************************************************************************
 */
uint8_t macif_rx_get_packet_threshold(void);

/**
 ****************************************************************************************
 * @brief Checks if a host buffer is currently available.
 *
 * @return true if a buffer is available, false otherwise.
 *
 ****************************************************************************************
 */
bool macif_rx_buf_check(void);

/**
 ****************************************************************************************
 * @brief Retrieve a RX host buffer address for future DMA transfer
 *
 * It just returns the pointer values set by the upper layers at init time and after
 * each reception.
 * The buffer is not available anymore after the call.
 * Prior to the call to this function, a call to @ref macif_rx_buf_check has to be
 * performed to ensure that there is a buffer available.
 *
 * @if full_doc
 * @param[out] host_id    HostId linked to the buffer
 * @endif
 *
 * @return Hostbuf address.
 *
 ****************************************************************************************
 */
#if NX_UMAC_PRESENT
uint32_t macif_rx_buf_get(uint32_t *host_id);
#else
uint32_t macif_rx_buf_get(void);
#endif //(NX_UMAC_PRESENT)

/**
 ****************************************************************************************
 * @brief Indicate to the lower layers that a buffer has just been pushed.
 * In case the RX flow control was activated, the WiFi task will be awaken in order to
 * resume the RX processing.
 ****************************************************************************************
 */
void macif_rx_buf_ind(void);

#if NX_UMAC_PRESENT
/**
 ****************************************************************************************
 * @brief Upload the list of prepared RX status descriptors to upper layers
 *
 * @param[in] desc_list Pointer to the list of RX status descriptors
 ****************************************************************************************
 */
void macif_rx_desc_upload(struct co_list *desc_list);
#endif

#if NX_UF_EN
/**
 ****************************************************************************************
 * @brief Indicate to the higher layers the presence of UF information.
 * @param[in] rx_vector   Pointer to the RX vector descriptor
 ****************************************************************************************
 */
void macif_uf_ind(struct rx_vector_desc *rx_vector);
#endif //NX_UF_EN

/**
 ****************************************************************************************
 * @brief Signal to the lower layers the presence of data in the TX queue passed as
 * parameter.
 * This function typically triggers an event in the WiFi stack. Used only in FullHost.
 *
 * @param[in] queue_idx Index of the queue in which data is available
 ****************************************************************************************
 */
void macif_tx_init(void);

/**
 ****************************************************************************************
 * @brief Signal to the lower layers the presence of data in the TX queue passed as
 * parameter.
 * This function typically triggers an event in the WiFi stack. Used only in FullHost.
 *
 * @param[in] queue_idx Index of the queue in which data is available
 ****************************************************************************************
 */
void macif_tx_data_ind(int queue_idx);

/**
 ****************************************************************************************
 * @brief Kernel event handler for the handling of the TX descriptor pushed by the upper
 * layers.
 *
 * @param[in] dummy Unused
 *
 ****************************************************************************************
 */
void macif_tx_evt(int dummy);

/**
 ****************************************************************************************
 * @brief Get the host address of the TX payload descriptor pattern
 * It is not used in fully hosted partitioning.
 *
 * @return The address of the TX descriptor pattern
 *
 ****************************************************************************************
 */
uint32_t macif_tx_pattern_addr_get(void);

/**
 ****************************************************************************************
 * @brief Get the buffered data for a given sta/tid
 *
 * @param[in] sta  STA index
 * @param[in] tid  TID
 *
 * @return number of bytes buffered
 *
 ****************************************************************************************
 */
uint32_t macif_buffered_get(uint8_t sta, uint8_t tid);

#if (NX_BEACONING && !NX_UMAC_PRESENT) || RW_MUMIMO_TX_EN
/**
 ****************************************************************************************
 * @brief Re-enable the user queues that were disabled
 *
 * @param[in] queue_idx     Index of the TX queue
 * @param[in] active_users  Bit field showing the users to re-enable
 *
 ****************************************************************************************
 */
void macif_tx_enable_users(int queue_idx, uint8_t active_users);

/**
 ****************************************************************************************
 * @brief Disable the user queues
 *
 * @param[in] queue_idx      Index of the TX queue
 * @param[in] disable_users  Bit field of users to disable
 *
 ****************************************************************************************
 */
void macif_tx_disable_users(int queue_idx, uint8_t disable_users);

/**
 ****************************************************************************************
 * @brief Test whether a user queue is enabled or not
 *
 * @param[in] queue_idx  Index of the TX queue
 * @param[in] users      Bit field of users to test
 * @return True if queues of all users are active and false otherwise
 ****************************************************************************************
 */
bool macif_tx_users_enabled(int queue_idx, uint8_t users);
#endif //(NX_BEACONING && !NX_UMAC_PRESENT) || RW_MUMIMO_TX_EN

#if NX_BEACONING && !NX_UMAC_PRESENT
/**
 ****************************************************************************************
 * @brief Get the status of the TX queue
 *
 * @param[in] queue_idx Index of the TX queue for which the event is called
 *
 * @return true if data is queued and false otherwise.
 ****************************************************************************************
 */
bool macif_tx_q_has_data(int queue_idx);

/**
 ****************************************************************************************
 * @brief Check how many tx descriptors are available on the requested queue for a VIF.
 * It is not used in fully hosted partitioning.
 *
 * @param[in] queue_idx Index of the TX queue
 * @param[in] vif_idx   Index of the Vif
 *
 * @return Number of txdesc ready on the TX queue for the specified vif
 *
 ****************************************************************************************
 */
uint8_t macif_tx_q_len(int queue_idx, int vif_idx);

/**
 ****************************************************************************************
 * @brief Push all the descriptors of a specific queue_idx/user_pos pair to the
 * TX path.
 *
 * The queue remains in the disabled state.
 *
 * @param[in] queue_idx Index of the TX queue
 * @param[in] user_pos  User position
 ****************************************************************************************
 */
void macif_tx_delayed_push_all(int queue_idx, uint8_t user);
#endif // NX_BEACONING && !NX_UMAC_PRESENT

#if RW_MUMIMO_TX_EN
/**
 ****************************************************************************************
 * @brief Kernel event handler for the handling of the TX descriptor delayed for
 * disabled users.
 *
 * @param[in] dummy Unused
 ****************************************************************************************
 */
void macif_tx_delayed_evt(int dummy);
#endif

/**
 ****************************************************************************************
 * @brief Initialize a new TX confirmation upload process.
 * This process shall be closed using @ref macif_tx_cfm_done before starting a new session.
 *
 * @param[in] access_category  Access category for the confirmations
 *
 ****************************************************************************************
 */
void macif_tx_cfm_start(uint8_t access_category);

/**
 ****************************************************************************************
 * @brief Push a TX confirmation for upload to upper layers
 *
 * @param[in] access_category  Access category for the confirmation
 * @param[in] txdesc Pointer to the TX descriptor for which a CFM is pushed
 *
 ****************************************************************************************
 */
void macif_tx_cfm_push(uint8_t access_category, struct txdesc *txdesc);

/**
 ****************************************************************************************
 * @brief Program the pending confirmation for upload.
 * This function shall be called only after having first initialized a confirmation
 * session using @ref macif_tx_cfm_start
 *
 * @param[in] access_category  Access category value
 * @param[in] poll Flag indicating if the function shall not return (true) until all
 *                 confirmations are uploaded
 ****************************************************************************************
 */
void macif_tx_cfm_done(uint8_t access_category, bool poll);

/**
 ****************************************************************************************
 * @brief Handler for DMA transfer of confirmations
 *
 ****************************************************************************************
 */
void macif_tx_cfm_dma_int_handler(void);

#if NX_RADAR_DETECT
/**
 ****************************************************************************************
 * @brief Indicate to the RD module that a radar event has been detected by the PHY.
 * @param[in] pulse_array   Pointer to the array of radar pulses
 * @return true if successful, false otherwise.
 ****************************************************************************************
 */
bool macif_rd_ind(struct radar_pulse_array_desc *pulse_array);
#endif //NX_RADAR_DETECT
/**
 ****************************************************************************************
 * @brief Handler for returning the size available for the API message
 *
 * @return the length of the API msg in bytes
 ****************************************************************************************
 */
uint32_t macif_max_api_msg_len (void);

/**
 ****************************************************************************************
 * @brief Convenient wrapper to ipc_e2a_msg that provides the maximum size that remains
 * in the msg for further data
 *
 *
 * @param[in] param_str parameter structure tag
 *
 * @return the max size available that can be allocated for a further use
 ****************************************************************************************
 */
#define MACIF_MAX_PARAM_LEN(param_str) (macif_max_api_msg_len() - sizeof_b(struct param_str))

/**
 * @}
 */
