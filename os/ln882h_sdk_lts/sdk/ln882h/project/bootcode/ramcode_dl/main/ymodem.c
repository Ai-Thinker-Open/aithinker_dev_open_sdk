#include "proj_config.h"
#include "ln882h.h"
#include "mode_ctrl.h"
#include "ramcode_port.h"
#include "utils/crc16.h"
#include "ymodem.h"
#include <string.h>

/****************************************  variables  *********************************************/
uint8_t g_cache_buffer[CACHE_BUFFER_SIZE]       = {0};
uint8_t g_cache_tailing[YMODEM_PACKET_BIG_SIZE] = {0};

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
uint8_t g_cache_read_buf[CACHE_BUFFER_SIZE] = {0};
#endif

/****************************************  functions  *********************************************/
static uint32_t buf_filelen(unsigned char* ptr);

/**
 * @brief Erase ymodem receive area after receiving the first ymodem packet.
 *
 * @param file_length
 * @return int8_t return 0 on success, others on failure.
 */
static int8_t erase_ymodem_recv_area(uint32_t file_length)
{
    bootram_file_ctrl_t* cmd_ctrl     = bootram_file_get_handle();
    upgrade_ctrl_t*      upgrade_ctrl = &(cmd_ctrl->upgrade_ctrl);
    uint32_t len = ((file_length / SIZE_4KB) + ((file_length % SIZE_4KB) ? (1) : (0))) * SIZE_4KB;
    return bootram_flash_erase(upgrade_ctrl->start_addr, len);
}

void port_serial_flush(void)
{
    bootram_serial_flush();
}
void port_serial_putchar(uint8_t ch)
{
    bootram_serial_write(&ch, sizeof(ch));
}
int port_serial_getchar(uint8_t* ch, uint32_t timeout)
{
    int size = 0;
    while ((timeout--) > 0) {
        size = bootram_serial_read(ch, sizeof(unsigned char));
        if (size > 0) {
            return size;
        }
    }
    return size;
}

/*
 * @brief: Receive one frame data(data_len = 128 or data_len = 1024)
 * @param: ymode: ymodem context handle
 * @return:  0 -- receive success; -1 -- receive timeout;
 *          -2 -- CRC check fail;  -3 -- frame sequence number error.
 **/
static frame_state_enum_t ymodem_frame_receive(ymodem_t* ymode)
{
    uint32_t i      = 0;
    uint8_t* in_ptr = ymode->data_buf;
    uint16_t crc16  = 0;
    uint8_t  seqno, seqno_comp, crc_h, crc_l;

    // receive sequence number
    if (false == port_serial_getchar(&seqno, READ_TIMEOUT)) {
        return FRAME_TIMEOUT;
    }

    // receive sequence number complement code
    if (false == port_serial_getchar(&seqno_comp, READ_TIMEOUT)) {
        return FRAME_TIMEOUT;
    }

    if ((seqno + seqno_comp) != 0xFF) {
        return FRAME_SEQ_ERR;
    }

    // receive data0.1...128/1024
    for (i = 0; i < ymode->data_len; i++) {
        if (false == port_serial_getchar(in_ptr, READ_TIMEOUT)) {
            return FRAME_TIMEOUT;
        }
        in_ptr++;
    }

    // receive crc high 8 bit
    if (false == port_serial_getchar(&crc_h, READ_TIMEOUT)) {
        return FRAME_TIMEOUT;
    }

    // receive crc low 8 bit
    if (false == port_serial_getchar(&crc_l, READ_TIMEOUT)) {
        return FRAME_TIMEOUT;
    }

    // If there is any data, it is considered as an error
    if (false != port_serial_getchar(in_ptr, 1)) {
        return FRAME_NUM_ERR;
    }

    crc16 = crc16_ccitt((const char*)ymode->data_buf, ymode->data_len);
    if (crc16 != (crc_h << 8 | crc_l)) {
        return FRAME_CRC_ERR;
    }

    if (ymode->eot_count == 0) {
        if (seqno != (ymode->packet_count & 0xFF)) {
            return FRAME_SEQ_ERR;
        }
    }

    ymode->seqno = seqno;

    return FRAME_SUCCESS;
}

ymodem_state_enum_t ymodem_recieve_loop(ymodem_t* ymode, frame_rcv_complete_cb frame_cb)
{
    int      max_tries = MODEM_MAX_RETRIES;
    uint8_t* bufptr    = ymode->data_buf;
    uint8_t* namptr    = ymode->filename;

    uint32_t valid_data_count = 0;
    uint32_t errors           = 0;
    uint8_t  ch               = 0;
    bool     send_c           = true;
    bool     exit_ymodem      = false;

    if (frame_cb == NULL) {
        return YMODEM_FAIL;
    }

    memset(ymode, 0, sizeof(ymodem_t));  // init ymode params
    port_serial_flush();                 // flush port rx/tx fifo

    while ((max_tries-- > 0) && (exit_ymodem != true)) {
        if (send_c) {
            port_serial_putchar(MODEM_C);
        }

        if (port_serial_getchar(&ch, READ_TIMEOUT)) {
            switch (ch) {
                case MODEM_SOH:
                case MODEM_STX:
                    ymode->data_len =
                        (ch == MODEM_SOH) ? YMODME_PACKET_SMALL_SIZE : YMODEM_PACKET_BIG_SIZE;

                    switch (ymodem_frame_receive(ymode)) {
                        case FRAME_SUCCESS:
                            errors = 0;

                            /*---------------begin frame-------------*/
                            if (ymode->packet_count == 0) {
                                // file name
                                while (*bufptr != '\0') {
                                    *namptr++ = *bufptr++;
                                }
                                *namptr = '\0';
                                bufptr++;
                                while (*bufptr == ' ') {
                                    bufptr++;
                                }
                                // file length
                                ymode->filelen = buf_filelen(bufptr);

                                // format some flash sectors to save file.
                                if (0 != erase_ymodem_recv_area(ymode->filelen)) {
                                    port_serial_putchar(MODEM_CAN);  // cancel.
                                    port_serial_putchar(MODEM_CAN);
                                    exit_ymodem = true;
                                    return YMODEM_FAIL;
                                }

                                port_serial_putchar(MODEM_ACK);  // send ACK
                                port_serial_putchar(MODEM_C);
                                send_c = false;
                            }
                            /*----------------end frame--------------*/
                            else if ((ymode->eot_count > 1) && (ymode->data_len == YMODME_PACKET_SMALL_SIZE) &&
                                     (ymode->seqno == 0)) {
                                port_serial_putchar(MODEM_ACK);  // send ACK
                                exit_ymodem = true;              // exit ymodem.
                                return YMODEM_SUCCESS;
                            }
                            /*-----------normal data frame-----------*/
                            else {
                                valid_data_count += ymode->data_len;

                                if (valid_data_count > ymode->filelen) {
                                    ymode->valid_data_len =
                                        ymode->data_len - (valid_data_count - ymode->filelen);
                                }
                                else {
                                    ymode->valid_data_len = ymode->data_len;
                                }
                                ymode->rev_len += ymode->valid_data_len;

                                if (ymode->valid_data_len != frame_cb(ymode)) {
                                    port_serial_putchar(MODEM_CAN);
                                    port_serial_putchar(MODEM_CAN);
                                    exit_ymodem = true;  // exit ymodem.
                                    break;
                                }

                                port_serial_putchar(MODEM_ACK);  // send ACK
                            }

                            ymode->packet_count++;
                            break;

                        case FRAME_TIMEOUT:
                        case FRAME_CRC_ERR:
                        case FRAME_SEQ_ERR:
                        case FRAME_NUM_ERR:
                        default:
                            errors++;
                            if (errors > MODEM_MAX_ERRORS) {
                                port_serial_putchar(MODEM_CAN);
                                port_serial_putchar(MODEM_CAN);
                                exit_ymodem = true;  // exit ymodem.
                            }
                            else {
                                port_serial_flush();
                                port_serial_putchar(MODEM_NAK);  // send NAK
                            }
                            break;
                    }
                    break;

                case MODEM_EOT:
                    ymode->eot_count++;
                    port_serial_putchar(MODEM_NAK);  // send NAK
                    while (!port_serial_getchar(&ch, READ_TIMEOUT) || ch != MODEM_EOT) {
                    };  // waite sender's second EOT
                    ymode->eot_count++;
                    port_serial_putchar(MODEM_ACK);  // send ACK
                    port_serial_putchar(MODEM_C);    // send C
                    break;

                case MODEM_CAN:
                    if (port_serial_getchar(&ch, READ_TIMEOUT) && (ch == MODEM_CAN)) {
                        exit_ymodem = true;  // exit ymodem.
                    }
                    break;

                case MODEM_ABORT1:
                case MODEM_ABORT2:
                    port_serial_putchar(MODEM_CAN);
                    port_serial_putchar(MODEM_CAN);
                    exit_ymodem = true;  // exit ymodem.
                    break;

                default:
                    errors++;
                    if (errors > MODEM_MAX_ERRORS) {
                        port_serial_putchar(MODEM_CAN);
                        port_serial_putchar(MODEM_CAN);
                        exit_ymodem = true;  // exit ymodem.
                    }
                    break;
            }
        }
    }
    return YMODEM_FAIL;
}

static uint32_t buf_filelen(uint8_t* ptr)
{
    int datatype = 10, result = 0;

    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) {
        datatype = 16;
        ptr += 2;
    }

    for (; *ptr != '\0'; ptr++) {
        if ((*ptr >= '0') && (*ptr <= '9')) {
            result = result * datatype + *ptr - '0';
        }
        else {
            if (datatype == 10) {
                return result;
            }
            else {
                if ((*ptr >= 'A') && (*ptr <= 'F')) {
                    result = result * 16 + *ptr - 55;  // 55 = 'A'-10
                }
                else if ((*ptr >= 'a') && (*ptr <= 'f')) {
                    result = result * 16 + *ptr - 87;  // 87 = 'a'-10
                }
                else {
                    return result;
                }
            }
        }
    }
    return result;
}
