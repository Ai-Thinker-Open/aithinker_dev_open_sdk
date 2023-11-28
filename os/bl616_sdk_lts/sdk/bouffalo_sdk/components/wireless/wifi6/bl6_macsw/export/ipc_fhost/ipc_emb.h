/**
 ****************************************************************************************
 *
 * @file ipc_emb.h
 *
 * @brief IPC module context on emb side for fully hosted configuration.
 * In this case IPC is only use for debug/test purpose
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */

#ifndef _IPC_EMB_H_
#define _IPC_EMB_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "export/common/co_int.h"
#include "export/common/co_bool.h"

/**
 ****************************************************************************************
 * @addtogroup IPC
 * @{
 ****************************************************************************************
 */
// Specific ID to indicate asynchronous indication
/// ID used for error indication
#define IPC_EMB_MSG_IND 0xffff0000
/// ID used for printf
#define IPC_EMB_PRINT_MSG 0xffff0001

/// to remove
__INLINE void ipc_emb_radar_event_ind(void) {};
/// to remove
__INLINE uint32_t ipc_emb_hostradarbuf_get(void) {return 0;}


/**
 ****************************************************************************************
 * @brief Initialize the IPC on the embedded side.
 *
 * @warning Since this function sets some Init values in IPC Shared memory it must not be
 * called before ipc_host_init() which resets all IPC Shared memory at boot.
 *
 ****************************************************************************************
 */
void ipc_emb_init(void);

/**
 ****************************************************************************************
 * @brief Interrupt handler for Host to embedded side message
 *
 ****************************************************************************************
 */
void ipc_emb_msg_irq(void);

/**
 ****************************************************************************************
 * @brief Acknowledge message reception to Host
 *
 ****************************************************************************************
 */
void ipc_emb_ack_msg(void);

/**
 ****************************************************************************************
 * @brief Forward a Application message to the Host
 *
 * Message will be uploaded using DMA transfer, and @ref ipc_emb_msg_dma_int_handler is
 * called when the transfer is done.
 *
 * @param[in] id   ID of message. For response (aka confirmatiom), id must have the same
 *                 value as the command.
 *                 For asynchronous indication, id must be @ref IPC_EMB_MSG_IND.
 * @param[in] len  Length, in bytes, of message
 * @param[in] data Message to upload
 *
 ****************************************************************************************
 */
void ipc_emb_msg_fwd(int id, int len, uint32_t *data);

/**
 ****************************************************************************************
 * @brief Handles the DMA interrupt generated when the MSG from Embedded side to
 * Host has been transferred.
 *
 * Transfer is started when @ref ipc_emb_msg_fwd is called.
 * It triggers IPC interrupt for Host to indicate that a msg has been written
 ****************************************************************************************
 */
void ipc_emb_msg_dma_int_handler(void);

/**
 ****************************************************************************************
 * @brief Send an EMB string to the APP - IPC API function
 *
 * If @p poll is not set, message will be uploaded using DMA transfer.
 * In both cases @ref ipc_emb_dbg_dma_int_handler is called when the transfer is done.
 *
 * @param[in]   poll    Poll to ensure that print has gone when we exit the function
 * @param[in]   len     Length of the message
 * @param[in]   string  Pointer to the message
 *                      Start must be word aligned (assumed to be the sprintf buffer)
 *                      The string may be truncated if it is too long.
 *
 ****************************************************************************************
 */
void ipc_emb_print_fwd(bool poll, const uint32_t len, char *string);

/**
 ****************************************************************************************
 * @brief Handles the DMA interrupt generated when the Debug message from Embedded side to
 * Application has been transferred.
 *
 * Transfer is started when @ref ipc_emb_print_fwd is called.
 * It triggers IPC interrupt for Host to indicate that a dbg msg has been written
 ****************************************************************************************
 */
void ipc_emb_dbg_dma_int_handler(void);

/**
 ****************************************************************************************
 * @brief Retrieve a hostbuf address for debug dump transfer
 *
 * @return Hostbuf address.
 *
 ****************************************************************************************
 */
uint32_t ipc_emb_hostdbgdumpbuf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve next configuration section
 *
 * @param[in]  start Address of the previous section (set to NULL on first call)
 * @param[out] id    Id of the section returned
 * @param[out] len   Size, in bytes, of the section (Including potential padding)
 *
 * @return Address of configuration Section, and NULL when all sections have been read.
 *
 ****************************************************************************************
 */
void *ipc_emb_config_get_next(void *start, uint16_t *id, uint16_t *len);

/// @} IPC

#endif // _IPC_EMB_H_
