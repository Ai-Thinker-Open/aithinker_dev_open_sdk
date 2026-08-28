#include "aiio_adapter_include.h"
#include "aiio_smartliving_demo.h"

static iotx_conn_info_t     iotx_conn_info = {0, 0, NULL, NULL, NULL, NULL, NULL};

iotx_conn_info_pt iotx_conn_info_get(void)
{
    return &iotx_conn_info;
}

void iotx_conn_info_release(void)
{
    if (iotx_conn_info.host_name != NULL) {
        free(iotx_conn_info.host_name);
    }
    if (iotx_conn_info.username != NULL) {
        free(iotx_conn_info.username);
    }
    if (iotx_conn_info.password != NULL) {
        free(iotx_conn_info.password);
    }
    if (iotx_conn_info.client_id != NULL) {
        free(iotx_conn_info.client_id);
    }
    memset(&iotx_conn_info, 0, sizeof(iotx_conn_info));
}

iotx_conn_info_pt iotx_conn_info_reload(void (*cb)(void *), void* client)
{
    iotx_conn_info_release();
    if (iotx_guider_authenticate(&iotx_conn_info, cb, client) < 0  || iotx_conn_info.init == 0) {
        return NULL;
    }
    return &iotx_conn_info;
}

int IOT_SetupConnInfo(void **info_ptr)
{
    if (NULL == info_ptr) {
        return -1;
    }
    *info_ptr = iotx_conn_info_reload(NULL, NULL);
    if (*info_ptr == NULL) {
        return -1;
    }
    return 0;
}