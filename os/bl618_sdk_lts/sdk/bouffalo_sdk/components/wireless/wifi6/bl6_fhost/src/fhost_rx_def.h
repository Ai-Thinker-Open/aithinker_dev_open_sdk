/**
 ****************************************************************************************
 *
 * @file fhost_rx_def.h
 *
 * @brief Definitions of the fully hosted RX task.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#ifndef _FHOST_RX_DEF_H_
#define _FHOST_RX_DEF_H_

/**
 ****************************************************************************************
 * @addtogroup FHOST_RX
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

/*
 * DEFINITIONS
 ****************************************************************************************
 */
/// Number of RX buffers
#if NX_AMPDU_RX
#define FHOST_RX_BUF_CNT            (NX_MAX_BA_RX * NX_AMPDU_RX_BUF_SIZE + 2)
#else
#define FHOST_RX_BUF_CNT             4
#endif
/// Number of elements in the RX descriptor queue
#define FHOST_RX_QUEUE_DESC_ELT_CNT (FHOST_RX_BUF_CNT * 2)
/// Size of a RX buffer
#define FHOST_RX_BUF_SIZE           (RX_MAX_AMSDU_SUBFRAME_LEN + 1)

/// @}

#endif // _FHOST_RX_DEF_H_

