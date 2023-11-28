#ifndef  __LN_KV_ERR_H__
#define  __LN_KV_ERR_H__

typedef enum kv_err_en {
    KV_ERR_NONE                 = 0u,

    KV_ERR_BUF_TOO_SHORT        = 10u,
    KV_ERR_BLK_STATUS_ERROR,

    KV_ERR_DATA_FETCH_FAILED    = 20u,

    KV_ERR_FLASH_ERASE_FAILED   = 30u,
    KV_ERR_FLASH_READ_FAILED,
    KV_ERR_FLASH_WRITE_FAILED,

    KV_ERR_GC_NOTHING           = 40u,

    KV_ERR_INTERNAL_ERROR       = 50u,
    KV_ERR_INSUFFICIENT_SPACE,
    KV_ERR_INVALID_PARAM,
    KV_ERR_INVALID_ITEM,

    KV_ERR_NEXT_LOOP            = 60u,
    KV_ERR_NOT_EXIST,
    KV_ERR_NO_WRITEABLE_BLK,

    KV_ERR_OUT_OF_MEMORY        = 70u,

    KV_ERR_POS_FIX_FAILED       = 80u,

    KV_ERR_SIZE_EXCEEDED        = 90u,
} kv_err_t;


#endif /* __LN_KV_ERR_H__ */

