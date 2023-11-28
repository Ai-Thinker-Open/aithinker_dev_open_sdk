#include "serial.h"
#include "serial_hw.h"
#include "utils/fifo/fifobuf.h"
#include "utils/debug/ln_assert.h"
#include "ln_types.h"

Serial_t *serial_handles[SER_PORT_NUM];

/**
 * Insert \a c in tx FIFO buffer.
 * \note This function will switch out the calling process
 * if the tx buffer is full. If the buffer is full
 * and \a port->txtimeout is 0 return EOF immediatly.
 *
 * \return EOF on error or timeout, \a c otherwise.
 */
int serial_putchar(Serial_t *port, int c)
{
    if (fifo_isfull(&port->txfifo))
    {
        /* (re)trigger tx interrupt */
        port->hw->table->txStart(port->hw);
    }

    fifo_push(&port->txfifo, (unsigned char)c);

    /* (re)trigger tx interrupt */
    port->hw->table->txStart(port->hw);

    /* Avoid returning signed extended char */
    return (int)((unsigned char)c);
}

/**
 * Fetch a character from the rx FIFO buffer.
 * If the buffer is empty, ser_getchar_nowait() returns
 * EOF immediatly.
 * \note Deprecated, use ser_getchar with rx_timeout set to 0.
 */
int serial_getchar_nowait(Serial_t *fd)
{
    if (fifo_isempty(&fd->rxfifo))
        return EOF;

    /* NOTE: the double cast prevents unwanted sign extension */
    return (int)(unsigned char)fifo_pop(&fd->rxfifo);
}

/**
 * Read at most \a size bytes from \a port and put them in \a buf
 *
 * \return number of bytes actually read.
 */
size_t serial_read(Serial_t *fd, void *_buf, size_t size)
{

    size_t i = 0;
    char *buf = (char *)_buf;
    int c;

    while (i < size)
    {
        if ((c = serial_getchar_nowait(fd)) == EOF)
            break;
        buf[i++] = c;
    }

    return i;
}

/**
 * \brief Write a buffer to serial.
 *
 * \return 0 if OK, EOF in case of error.
 *
 * \todo Optimize with fifo_pushblock()
 */
size_t serial_write(Serial_t *fd, const void *_buf, size_t size)
{
    const char *buf = (const char *)_buf;
    size_t i = 0;

    while (size--)
    {
        if (serial_putchar(fd, *buf++) == EOF)
            break;
        i++;
    }
    return i;
}

/**
 * Flush both the RX and TX buffers.
 */
void serial_purge(Serial_t *fd)
{
    serial_purge_rx(fd);
    serial_purge_tx(fd);
}

/**
 * Flush RX buffer.
 */
void serial_purge_rx(Serial_t *fd)
{
    fifo_flush(&fd->rxfifo);
}

/**
 * Flush TX buffer.
 */
void serial_purge_tx(Serial_t *fd)
{
    fifo_flush(&fd->txfifo);
}

/**
 * Wait until all pending output is completely
 * transmitted to the other end.
 *
 * \note The current implementation only checks the
 *       software transmission queue. Any hardware
 *       FIFOs are ignored.
 */
int serial_flush(Serial_t *fd)
{
    /*
     * Wait until the FIFO becomes empty, and then until the byte currently in
     * the hardware register gets shifted out.
     */
    while (!fifo_isempty(&fd->txfifo)
            || fd->hw->table->txSending(fd->hw)){
        //cpu_relax();
    }
    return 0;
}

static void serial_rx_default_cb(void)
{
}

/**
 * Initialize a serial port.
 *
 * \param fd KFile Serial struct interface.
 * \param unit  Serial unit to open. Possible values are architecture dependant.
 */
static Serial_t *serial_open(Serial_t *fd, serial_port_id_t port_id, uint32_t baudrate, serial_rx_callbcak cb)
{
    LN_ASSERT(port_id < SER_PORT_NUM);
    LN_ASSERT(!fd->is_open);

    serial_handles[port_id] = fd;
    fd->is_open = LN_TRUE;
    fd->port_id = port_id;
    fd->hw      = serial_hw_getdesc(port_id);
    
    if(cb){
        fd->rx_callback = cb;
    } else {
        fd->rx_callback = serial_rx_default_cb;
    }

    /* Initialize circular buffers */
    LN_ASSERT(fd->hw->txbuffer);
    LN_ASSERT(fd->hw->rxbuffer);
    fifo_init(&fd->txfifo, fd->hw->txbuffer, fd->hw->txbuffer_size);
    fifo_init(&fd->rxfifo, fd->hw->rxbuffer, fd->hw->rxbuffer_size);
    fd->hw->table->init(fd->hw, fd, baudrate);

    /* Set default values */
#if 0//CONFIG_SER_RXTIMEOUT != -1 || CONFIG_SER_TXTIMEOUT != -1
    ser_settimeouts(fd, CONFIG_SER_RXTIMEOUT, CONFIG_SER_TXTIMEOUT);
#endif

    /* Clear error flags */
    serial_setstatus(fd, 0);

    return fd;
}

/**
 * Clean up serial port, disabling the associated hardware.
 */
void serial_deinit(Serial_t *fd)
{
    int i;
    // Wait until we finish sending everything
    serial_flush(fd);

    fd->hw->table->cleanup(fd->hw);
    /*
     * We purge the FIFO buffer only after the low-level cleanup, so that
     * we are sure that there are no more interrupts.
     */
    serial_purge(fd);

    for(i = 0; i< SER_PORT_NUM; i++) {
        if(fd == serial_handles[i]) {
            serial_handles[i] = NULL;
            break;
        }
    }
}

/**
 * Init serial driver for \a unit.
 *
 * Use values SER_UARTn as values for \a unit.
 */
void serial_init(Serial_t *fd, serial_port_id_t port_id, uint32_t baudrate, serial_rx_callbcak cb)
{
    memset(fd, 0, sizeof(*fd));
    serial_open(fd, port_id, baudrate, cb);
}

/**
 * Set the baudrate for the serial port
 */
int serial_setbaudrate(struct Serial *fd, uint32_t baudrate)
{
    LN_ASSERT(baudrate != 0);

    return fd->hw->table->setBaudrate(fd->hw, baudrate);
}

