#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "utils/debug/log.h"

#include "transport.h"
#include "transport_utils.h"



/**
 * This list will hold all transport available
 */
STAILQ_HEAD(ln_transport_list_t, ln_transport_item_t);

static ln_transport_handle_t ln_transport_get_default_parent(ln_transport_handle_t t)
{
    /*
    * By default, the underlying transport layer handle is the handle itself
    */
    return t;
}

ln_transport_list_handle_t ln_transport_list_init()
{
    ln_transport_list_handle_t list = OS_Malloc(sizeof(struct ln_transport_list_t));
    LN_TRANSPORT_MEM_CHECK(list, return NULL);
    STAILQ_INIT(list);
    return list;
}

ln_err_t ln_transport_list_add(ln_transport_list_handle_t list, ln_transport_handle_t t, const char *scheme)
{
    if (list == NULL || t == NULL) {
        return LN_ERR_INVALID_ARG;
    }
    t->scheme = OS_Malloc(strlen(scheme) + 1);
    LN_TRANSPORT_MEM_CHECK(t->scheme, return LN_ERR_NO_MEM);
    strcpy(t->scheme, scheme);
    STAILQ_INSERT_TAIL(list, t, next);
    return LN_OK;
}

ln_transport_handle_t ln_transport_list_get_transport(ln_transport_list_handle_t list, const char *scheme)
{
    if (!list) {
        return NULL;
    }
    if (scheme == NULL) {
        return STAILQ_FIRST(list);
    }
    ln_transport_handle_t item;
    STAILQ_FOREACH(item, list, next) {
        if (strcasecmp(item->scheme, scheme) == 0) {
            return item;
        }
    }
    return NULL;
}

ln_err_t ln_transport_list_destroy(ln_transport_list_handle_t list)
{
    ln_transport_list_clean(list);
    OS_Free(list);
    return LN_OK;
}

ln_err_t ln_transport_list_clean(ln_transport_list_handle_t list)
{
    ln_transport_handle_t item = STAILQ_FIRST(list);
    ln_transport_handle_t tmp;
    while (item != NULL) {
        tmp = STAILQ_NEXT(item, next);
        if (item->_destroy) {
            item->_destroy(item);
        }
        ln_transport_destroy(item);
        item = tmp;
    }
    STAILQ_INIT(list);
    return LN_OK;
}




ln_transport_handle_t ln_transport_init()
{
    ln_transport_handle_t t = OS_Malloc(sizeof(struct ln_transport_item_t));
    LN_TRANSPORT_MEM_CHECK(t, return NULL);
    return t;
}

ln_transport_handle_t ln_transport_get_payload_transport_handle(ln_transport_handle_t t)
{
    if (t && t->_read) {
        return t->_parent_transfer(t);
    }
    return NULL;
}

ln_err_t ln_transport_destroy(ln_transport_handle_t t)
{
    // if (t->scheme) {
    //     OS_Free(t->scheme);
    // }
    OS_Free(t);
    return LN_OK;
}

int ln_transport_connect(ln_transport_handle_t t, const char *host, int port, int timeout_ms)
{
    int ret = -1;
    if (t && t->_connect) {
        return t->_connect(t, host, port, timeout_ms);
    }
    return ret;
}

int ln_transport_connect_async(ln_transport_handle_t t, const char *host, int port, int timeout_ms)
{
    int ret = -1;
    if (t && t->_connect_async) {
        return t->_connect_async(t, host, port, timeout_ms);
    }
    return ret;
}

int ln_transport_read(ln_transport_handle_t t, char *buffer, int len, int timeout_ms)
{
    if (t && t->_read) {
        return t->_read(t, buffer, len, timeout_ms);
    }
    return -1;
}

int ln_transport_write(ln_transport_handle_t t, const char *buffer, int len, int timeout_ms)
{
    if (t && t->_write) {
        return t->_write(t, buffer, len, timeout_ms);
    }
    return -1;
}

int ln_transport_poll_read(ln_transport_handle_t t, int timeout_ms)
{
    if (t && t->_poll_read) {
        return t->_poll_read(t, timeout_ms);
    }
    return -1;
}

int ln_transport_poll_write(ln_transport_handle_t t, int timeout_ms)
{
    if (t && t->_poll_write) {
        return t->_poll_write(t, timeout_ms);
    }
    return -1;
}

int ln_transport_close(ln_transport_handle_t t)
{
    if (t && t->_close) {
        return t->_close(t);
    }
    return 0;
}

void *ln_transport_get_context_data(ln_transport_handle_t t)
{
    if (t) {
        return t->data;
    }
    return NULL;
}

ln_err_t ln_transport_set_context_data(ln_transport_handle_t t, void *data)
{
    if (t) {
        t->data = data;
        return LN_OK;
    }
    return LN_FAIL;
}

ln_err_t ln_transport_set_func(ln_transport_handle_t t,
                             connect_func _connect,
                             io_read_func _read,
                             io_func _write,
                             trans_func _close,
                             poll_func _poll_read,
                             poll_func _poll_write,
                             trans_func _destroy)
{
    if (t == NULL) {
        return LN_FAIL;
    }
    t->_connect = _connect;
    t->_read = _read;
    t->_write = _write;
    t->_close = _close;
    t->_poll_read = _poll_read;
    t->_poll_write = _poll_write;
    t->_destroy = _destroy;
    t->_connect_async = NULL;
    t->_parent_transfer = ln_transport_get_default_parent;
    return LN_OK;
}

int ln_transport_get_default_port(ln_transport_handle_t t)
{
    if (t == NULL) {
        return -1;
    }
    return t->port;
}

ln_err_t ln_transport_set_default_port(ln_transport_handle_t t, int port)
{
    if (t == NULL) {
        return LN_FAIL;
    }
    t->port = port;
    return LN_OK;
}

ln_err_t ln_transport_set_async_connect_func(ln_transport_handle_t t, connect_async_func _connect_async_func)
{
    if (t == NULL) {
        return LN_FAIL;
    }
    t->_connect_async = _connect_async_func;
    return LN_OK;
}

ln_err_t ln_transport_set_parent_transport_func(ln_transport_handle_t t, payload_transfer_func _parent_transport)
{
    if (t == NULL) {
        return LN_FAIL;
    }
    t->_parent_transfer = _parent_transport;
    return LN_OK;
}
