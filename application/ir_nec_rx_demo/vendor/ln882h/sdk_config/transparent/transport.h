#ifndef _LN_TRANSPORT_H_
#define _LN_TRANSPORT_H_

//#include <ln_err.h>
#include "transport_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ln_transport_list_t* ln_transport_list_handle_t;
typedef struct ln_transport_item_t* ln_transport_handle_t;

typedef int (*connect_func)(ln_transport_handle_t t, const char *host, int port, int timeout_ms);
typedef int (*io_func)(ln_transport_handle_t t, const char *buffer, int len, int timeout_ms);
typedef int (*io_read_func)(ln_transport_handle_t t, char *buffer, int len, int timeout_ms);
typedef int (*trans_func)(ln_transport_handle_t t);
typedef int (*poll_func)(ln_transport_handle_t t, int timeout_ms);
typedef int (*connect_async_func)(ln_transport_handle_t t, const char *host, int port, int timeout_ms);
typedef ln_transport_handle_t (*payload_transfer_func)(ln_transport_handle_t);

/**
 * Transport layer structure, which will provide functions, basic properties for transport types
 */
struct ln_transport_item_t {
    int             port;
    int             socket;         /*!< Socket to use in this transport */
    char            *scheme;        /*!< Tag name */
    void            *context;       /*!< Context data */
    void            *data;          /*!< Additional transport data */
    connect_func    _connect;       /*!< Connect function of this transport */
    io_read_func    _read;          /*!< Read */
    io_func         _write;         /*!< Write */
    trans_func      _close;         /*!< Close */
    poll_func       _poll_read;     /*!< Poll and read */
    poll_func       _poll_write;    /*!< Poll and write */
    trans_func      _destroy;       /*!< Destroy and free transport */
    connect_async_func _connect_async;      /*!< non-blocking connect function of this transport */
    payload_transfer_func  _parent_transfer;       /*!< Function returning underlying transport layer */

    STAILQ_ENTRY(ln_transport_item_t) next;
};

/**
 * @brief      Create transport list
 *
 * @return     A handle can hold all transports
 */
ln_transport_list_handle_t ln_transport_list_init(void);

/**
 * @brief      Cleanup and free all transports, include itself,
 *             this function will invoke ln_transport_destroy of every transport have added this the list
 *
 * @param[in]  list  The list
 *
 * @return
 *     - LN_OK
 *     - LN_FAIL
 */
ln_err_t ln_transport_list_destroy(ln_transport_list_handle_t list);

/**
 * @brief      Add a transport to the list, and define a scheme to indentify this transport in the list
 *
 * @param[in]  list    The list
 * @param[in]  t       The Transport
 * @param[in]  scheme  The scheme
 *
 * @return
 *     - LN_OK
 */
ln_err_t ln_transport_list_add(ln_transport_list_handle_t list, ln_transport_handle_t t, const char *scheme);

/**
 * @brief      This function will remove all transport from the list,
 *             invoke ln_transport_destroy of every transport have added this the list
 *
 * @param[in]  list  The list
 *
 * @return
 *     - LN_OK
 *     - LN_ERR_INVALID_ARG
 */
ln_err_t ln_transport_list_clean(ln_transport_list_handle_t list);

/**
 * @brief      Get the transport by scheme, which has been defined when calling function `ln_transport_list_add`
 *
 * @param[in]  list  The list
 * @param[in]  tag   The tag
 *
 * @return     The transport handle
 */
ln_transport_handle_t ln_transport_list_get_transport(ln_transport_list_handle_t list, const char *scheme);

/**
 * @brief      Initialize a transport handle object
 *
 * @return     The transport handle
 */
ln_transport_handle_t ln_transport_init(void);

/**
 * @brief      Cleanup and free memory the transport
 *
 * @param[in]  t     The transport handle
 *
 * @return
 *     - LN_OK
 *     - LN_FAIL
 */
ln_err_t ln_transport_destroy(ln_transport_handle_t t);

/**
 * @brief      Get default port number used by this transport
 *
 * @param[in]  t     The transport handle
 *
 * @return     the port number
 */
int ln_transport_get_default_port(ln_transport_handle_t t);

/**
 * @brief      Set default port number that can be used by this transport
 *
 * @param[in]  t     The transport handle
 * @param[in]  port  The port number
 *
 * @return
 *     - LN_OK
 *     - LN_FAIL
 */
ln_err_t ln_transport_set_default_port(ln_transport_handle_t t, int port);

/**
 * @brief      Transport connection function, to make a connection to server
 *
 * @param      t           The transport handle
 * @param[in]  host        Hostname
 * @param[in]  port        Port
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 * - socket for will use by this transport
 * - (-1) if there are any errors, should check errno
 */
int ln_transport_connect(ln_transport_handle_t t, const char *host, int port, int timeout_ms);

/**
 * @brief      Non-blocking transport connection function, to make a connection to server
 *
 * @param      t           The transport handle
 * @param[in]  host        Hostname
 * @param[in]  port        Port
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 * - socket for will use by this transport
 * - (-1) if there are any errors, should check errno
 */
int ln_transport_connect_async(ln_transport_handle_t t, const char *host, int port, int timeout_ms);

/**
 * @brief      Transport read function
 *
 * @param      t           The transport handle
 * @param      buffer      The buffer
 * @param[in]  len         The length
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *  - Number of bytes was read
 *  - (-1) if there are any errors, should check errno
 */
int ln_transport_read(ln_transport_handle_t t, char *buffer, int len, int timeout_ms);

/**
 * @brief      Poll the transport until readable or timeout
 *
 * @param[in]  t           The transport handle
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *     - 0      Timeout
 *     - (-1)   If there are any errors, should check errno
 *     - other  The transport can read
 */
int ln_transport_poll_read(ln_transport_handle_t t, int timeout_ms);

/**
 * @brief      Transport write function
 *
 * @param      t           The transport handle
 * @param      buffer      The buffer
 * @param[in]  len         The length
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *  - Number of bytes was written
 *  - (-1) if there are any errors, should check errno
 */
int ln_transport_write(ln_transport_handle_t t, const char *buffer, int len, int timeout_ms);

/**
 * @brief      Poll the transport until writeable or timeout
 *
 * @param[in]  t           The transport handle
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *     - 0      Timeout
 *     - (-1)   If there are any errors, should check errno
 *     - other  The transport can write
 */
int ln_transport_poll_write(ln_transport_handle_t t, int timeout_ms);

/**
 * @brief      Transport close
 *
 * @param      t     The transport handle
 *
 * @return
 * - 0 if ok
 * - (-1) if there are any errors, should check errno
 */
int ln_transport_close(ln_transport_handle_t t);

/**
 * @brief      Get user data context of this transport
 *
 * @param[in]  t        The transport handle
 *
 * @return     The user data context
 */
void *ln_transport_get_context_data(ln_transport_handle_t t);

/**
 * @brief      Get transport handle of underlying protocol
 *             which can access this protocol payload directly
 *             (used for receiving longer msg multiple times)
 *
 * @param[in]  t        The transport handle
 *
 * @return     Payload transport handle
 */
ln_transport_handle_t ln_transport_get_payload_transport_handle(ln_transport_handle_t t);

/**
 * @brief      Set the user context data for this transport
 *
 * @param[in]  t        The transport handle
 * @param      data     The user data context
 *
 * @return
 *     - LN_OK
 */
ln_err_t ln_transport_set_context_data(ln_transport_handle_t t, void *data);

/**
 * @brief      Set transport functions for the transport handle
 *
 * @param[in]  t            The transport handle
 * @param[in]  _connect     The connect function pointer
 * @param[in]  _read        The read function pointer
 * @param[in]  _write       The write function pointer
 * @param[in]  _close       The close function pointer
 * @param[in]  _poll_read   The poll read function pointer
 * @param[in]  _poll_write  The poll write function pointer
 * @param[in]  _destroy     The destroy function pointer
 *
 * @return
 *     - LN_OK
 */
ln_err_t ln_transport_set_func(ln_transport_handle_t t,
                             connect_func _connect,
                             io_read_func _read,
                             io_func _write,
                             trans_func _close,
                             poll_func _poll_read,
                             poll_func _poll_write,
                             trans_func _destroy);


/**
 * @brief      Set transport functions for the transport handle
 *
 * @param[in]  t                    The transport handle
 * @param[in]  _connect_async_func  The connect_async function pointer
 *
 * @return
 *     - LN_OK
 *     - LN_FAIL
 */
ln_err_t ln_transport_set_async_connect_func(ln_transport_handle_t t, connect_async_func _connect_async_func);

/**
 * @brief      Set parent transport function to the handle
 *
 * @param[in]  t                    The transport handle
 * @param[in]  _parent_transport    The underlying transport getter pointer
 *
 * @return
 *     - LN_OK
 *     - LN_FAIL
 */
ln_err_t ln_transport_set_parent_transport_func(ln_transport_handle_t t, payload_transfer_func _parent_transport);

#ifdef __cplusplus
}
#endif
#endif /* _LN_TRANSPORT_ */
