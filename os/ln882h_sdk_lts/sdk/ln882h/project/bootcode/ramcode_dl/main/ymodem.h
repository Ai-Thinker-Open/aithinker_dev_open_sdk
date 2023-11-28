#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>
#include <stdbool.h>

#define MODEM_SOH                (0x01) /* start of 128-byte data packet */
#define MODEM_STX                (0x02) /* start of 1024-byte data packet */
#define MODEM_EOT                (0x04) /* end of transmission */
#define MODEM_ACK                (0x06) /* acknowledge */
#define MODEM_NAK                (0x15) /* negative acknowledge */
#define MODEM_CAN                (0x18) /* two of these in succession aborts transfer */
#define MODEM_C                  (0x43) /* 'C' == 0x43, request 16-bit CRC */
#define MODEM_ABORT1             (0x41) /* 'A' == 0x41, abort by user */
#define MODEM_ABORT2             (0x61) /* 'a' == 0x61, abort by user */

#define YMODME_PACKET_SMALL_SIZE (128)
#define YMODEM_PACKET_BIG_SIZE   (1024 * 16)

// put ymodem packets into `g_cache_buffer` before program to flash.
#define CACHE_BUFFER_SIZE        (1024 * 64)

#define MODEM_MAX_RETRIES        (0xFFFFFFF)  //
#define READ_TIMEOUT             0xFFFF
#define MODEM_FILE_NAME_SIZE     64
#define MODEM_MAX_ERRORS         20

typedef enum {
    FRAME_SUCCESS = 0,
    FRAME_TIMEOUT = -1,
    FRAME_CRC_ERR = -2,
    FRAME_SEQ_ERR = -3,
    FRAME_NUM_ERR = -4,
} frame_state_enum_t;

typedef enum {
    YMODEM_SUCCESS = 0,
    YMODEM_FAIL    = -1,
} ymodem_state_enum_t;

/* *-------------------- X/YModem frame format---------------------*
 * |  0    |  1    |   2   |  3      | ... | n+3     | n+4  | n+5  |
 * |---------------------------------------------------------------|
 * | start | seqno | !seqno| data[0] | ... | data[n] | crc0 | crc1 |
 * *---------------------------------------------------------------*
 */
typedef struct
{
    uint32_t packet_count;                      // how many ymodem packets have been received.
    uint32_t data_len;                          //
    uint16_t valid_data_len;                    // valid data length in one block.
    uint8_t  eot_count;                         // EOT
    uint8_t  seqno;                             // seq no
    uint8_t  data_buf[YMODEM_PACKET_BIG_SIZE];  //
    uint32_t filelen;                           // file length (in bytes) of current receiving file.
    uint32_t rev_len;                           // received data length
    uint8_t  filename[MODEM_FILE_NAME_SIZE];
} ymodem_t;

extern uint8_t g_cache_buffer[CACHE_BUFFER_SIZE];
extern uint8_t g_cache_tailing[YMODEM_PACKET_BIG_SIZE];

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
extern uint8_t g_cache_read_buf[CACHE_BUFFER_SIZE];
#endif

typedef uint32_t (*frame_rcv_complete_cb)(ymodem_t* ymode);
ymodem_state_enum_t ymodem_recieve_loop(ymodem_t* ymode, frame_rcv_complete_cb receive_complete);

#endif
