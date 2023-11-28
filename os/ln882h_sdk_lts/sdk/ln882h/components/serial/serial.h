#ifndef __SERIAL_H__
#define __SERIAL_H__
#include "proj_config.h"

#include "serial_hw.h"
#include "utils/fifo/fifobuf.h"

typedef void (* serial_rx_callbcak)(void);

/** Serial handle structure */
typedef struct Serial
{
    serial_port_id_t       port_id;   /** Physical port id */
    int8_t                 is_open;
    fifo_buffer_t          txfifo;
    fifo_buffer_t          rxfifo;
    int32_t                rxtimeout;
    int32_t                txtimeout;
    volatile uint32_t      status;    /** Holds the flags defined above.  Will be 0 when no errors have occurred. */
    serial_rx_callbcak     rx_callback;
    struct SerialHardware *hw;        /** Low-level interface to hardware. */
} Serial_t;


void   serial_init(Serial_t *fd, serial_port_id_t port_id, uint32_t baudrate, serial_rx_callbcak cb);
void   serial_deinit(Serial_t *fd);

size_t serial_read(Serial_t *fd, void *_buf, size_t size);
size_t serial_write(Serial_t *fd, const void *_buf, size_t size);

int    serial_putchar(Serial_t *port, int c);
int    serial_getchar_nowait(Serial_t *fd);

int    serial_flush(Serial_t *fd);
void   serial_purge_rx(Serial_t *fd);
void   serial_purge_tx(Serial_t *fd);
void   serial_purge(Serial_t *fd);

int    serial_setbaudrate(struct Serial *fd, uint32_t baudrate);


#define serial_getstatus(serial)    ((serial)->status)
#define serial_setstatus(serial, new_status) ((serial)->status = (new_status))

#endif /* __SERIAL_H__ */
