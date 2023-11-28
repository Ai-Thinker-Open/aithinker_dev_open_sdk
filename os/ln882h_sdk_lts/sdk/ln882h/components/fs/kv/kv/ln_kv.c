#include "ln_kv.h"
#include "ln_kv_api.h"
#include "ln_utils.h"
#include <string.h>


kv_ctl_t kv_ctl;
static kv_err_t kv_gc(void);
static kv_err_t kv_try_gc(void);

static uint8_t kv_checksum_crc8(uint8_t *buf, int nbyte)
{
    int i;
    uint8_t crc = 0;

#define POLY            0x31
#define WIDTH           8
#define TOP_BIT         0x80

    while (nbyte--) {
        crc ^= *buf++;
        for (i = 0; i < WIDTH; ++i) {
            if (crc & TOP_BIT) {
                crc = (crc << 1) ^ POLY;
            } else {
                crc <<= 1;
            }
        }
        crc &= 0xFF;
    }

    return crc;
}

__STATIC_INLINE__ void kv_lock(void)
{
#ifndef KV_CONFIG_OS_NONE
    if (ln_kv_is_os_running()) {
        ln_kv_mutex_lock(KV_MGR_LOCK);
    }
#endif
}

__STATIC_INLINE__ void kv_unlock(void)
{
#ifndef KV_CONFIG_OS_NONE
    if (ln_kv_is_os_running()) {
        ln_kv_mutex_unlock(KV_MGR_LOCK);
    }
#endif
}

static kv_err_t kv_flash_read(uint32_t addr, void *buf, size_t buf_size)
{
    if (KV_FLASH_READ(addr, buf, buf_size) < 0) {
        return KV_ERR_FLASH_READ_FAILED;
    }
    return KV_ERR_NONE;
}

static kv_err_t kv_flash_write(uint32_t addr, void *buf, size_t buf_size)
{
    if (KV_FLASH_WRITE(addr, buf, buf_size) < 0) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }
    return KV_ERR_NONE;
}

static kv_err_t kv_flash_erase(uint32_t start, size_t len)
{
    if (KV_FLASH_ERASE(start, len) < 0) {
        return KV_ERR_FLASH_ERASE_FAILED;
    }
    return KV_ERR_NONE;
}

static kv_err_t kv_flash_wunit_modify(uint32_t addr, kv_wunit_t data)
{
    return kv_flash_write(addr, &data, sizeof(kv_wunit_t));
}

static kv_err_t  kv_flash_blk_erase(uint32_t blk_start)
{
    return kv_flash_erase(blk_start, KV_BLK_SIZE);
}

static void kv_flash_ctl_init(uint32_t flash_start, uint32_t flash_end, kv_flash_drv_t *flash_drv, kv_flash_prop_t *flash_prop)
{
    memcpy(&kv_ctl.flash_ctl.flash_drv, flash_drv, sizeof(kv_flash_drv_t));

    KV_FLASH_START              = flash_start;
    KV_FLASH_END                = flash_end;
    KV_FLASH_SECTOR_SIZE_LOG2   = flash_prop->sector_size_log2;

    KV_FLASH_WRITE_ALIGN        = sizeof(kv_byte_t);
}

static kv_err_t kv_blk_hdr_read(uint32_t blk_start, kv_blk_hdr_t *blk_hdr)
{
    kv_err_t err = kv_flash_read(blk_start, blk_hdr, sizeof(kv_blk_hdr_t));

    //log_i("blk info @ 0x%08x:\n", blk_start);
    //log_i("magic: 0x%08llx, gc_src: 0x%08llx --> gc_dst: 0x%08llx, gc_done: 0x%08llx\n\n",
    //       blk_hdr->magic, blk_hdr->gc_src, blk_hdr->gc_dst, blk_hdr->gc_done);

    return err;
}

static kv_err_t kv_blk_format(uint32_t blk_start)
{
    if (kv_flash_blk_erase(blk_start) != KV_ERR_NONE) {
        return KV_ERR_FLASH_ERASE_FAILED;
    }

    if (kv_flash_wunit_modify(KV_ADDR_OF_FIELD(blk_start, kv_blk_hdr_t, magic),
                                    KV_BLK_HDR_MAGIC) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    kv_blk_set_fresh(blk_start);

    return KV_ERR_NONE;
}

static uint32_t kv_blk_next_fresh(void)
{
    uint32_t cur_blk;

    // we try to find a fresh block behind current block(take all the blocks as a ring)
    KV_BLK_FOR_EACH_FROM(cur_blk, KV_MGR_WORKSPACE) {
        if (kv_blk_is_fresh(cur_blk)) {
            return cur_blk;
        }
    }

    return KV_BLK_INVALID;
}

static uint32_t kv_blk_get_a_fresh(void)
{
    uint32_t cur_blk;

    KV_BLK_FOR_EACH(cur_blk) {
        if (kv_blk_is_fresh(cur_blk)) {
            return cur_blk;
        }
    }

    return KV_BLK_INVALID;
}

static uint32_t kv_blk_search_inuse(uint32_t item_size)
{
    uint32_t cur_blk;

    KV_BLK_FOR_EACH_FROM(cur_blk, KV_MGR_WORKSPACE) {
        if (kv_blk_is_inuse(cur_blk) &&
            kv_blk_freesz_get(cur_blk) >= item_size) {
            return cur_blk;
        }
    }

    return KV_BLK_INVALID;
}

static uint32_t kv_blk_search_suitable(uint32_t item_size)
{
    uint32_t the_blk;

    // no more writeable block, no need to do the next logic
    if (KV_NO_WRITEABLE_BLK()) {
        return KV_BLK_INVALID;
    }

    // no more blocks with sufficient space, searched before
    if (KV_MGR_WORKSPACE == KV_BLK_INVALID) {
        return KV_BLK_INVALID;
    }

    do {
        // current workspace is just ok(workspace is kinda cache)
        if (kv_blk_freesz_get(KV_MGR_WORKSPACE) >= item_size) {
            return KV_MGR_WORKSPACE;
        }

        // first we should search in inuse blocks
        the_blk = kv_blk_search_inuse(item_size);
        if (the_blk != KV_BLK_INVALID) {
            KV_MGR_WORKSPACE = the_blk;
            return KV_MGR_WORKSPACE;
        }

        // no more fresh block, cannot allocate new block and cannot do gc neither.
        if (KV_MGR_BLK_NUM_FRESH < 1) {
            return KV_BLK_INVALID;
        }

        // if we have more than one fresh block, just give it out
        if (KV_MGR_BLK_NUM_FRESH > 1) {
            KV_MGR_WORKSPACE = kv_blk_next_fresh();
            return KV_MGR_WORKSPACE;
        }

        // no more block inuse has sufficient space, and only one fresh block left, we should try gc
        if (KV_MGR_BLK_NUM_FRESH == 1) {
            if (kv_gc() != KV_ERR_NONE) {
                // no more dirty block to gc, we give out the last precious fresh block(no more gc any more)
                KV_MGR_WORKSPACE = kv_blk_next_fresh();
                return KV_MGR_WORKSPACE;
            }

            /* if reach here, means we free some discarded space by gc,
               we should try to search in inuse block again(the only fresh block left is so precious that we should
               give it out as a last resort).
             */
        }
    } while (LN_TRUE);
}

static kv_err_t kv_blk_mark_gc_src(uint32_t blk_start)
{
    if (kv_flash_wunit_modify(KV_ADDR_OF_FIELD(blk_start, kv_blk_hdr_t, gc_src),
                                    KV_BLK_HDR_GC_SRC) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    return KV_ERR_NONE;
}

static kv_err_t kv_blk_mark_gc_dst(uint32_t blk_start)
{
    if (kv_flash_wunit_modify(KV_ADDR_OF_FIELD(blk_start, kv_blk_hdr_t, gc_dst),
                                    KV_BLK_HDR_GC_DST) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    return KV_ERR_NONE;
}

static kv_err_t kv_blk_mark_gc_done(uint32_t blk_start)
{
    if (kv_flash_wunit_modify(KV_ADDR_OF_FIELD(blk_start, kv_blk_hdr_t, gc_done),
                                    KV_BLK_HDR_GC_DONE) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    return KV_ERR_NONE;
}

static kv_err_t kv_item_hdr_write(uint32_t item_start, kv_item_hdr_t *item_hdr)
{
    if (kv_flash_write(KV_ADDR_OF_FIELD(item_start, kv_item_hdr_t, checksum),
                            &item_hdr->checksum,
                            KV_ITEM_HDR_SIZE - sizeof(kv_wunit_t) * 2) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    return kv_flash_wunit_modify(KV_ADDR_OF_FIELD(item_start, kv_item_hdr_t, magic),
                                    item_hdr->magic);
}

static kv_err_t kv_item_write(uint32_t item_start, kv_item_hdr_t *item_hdr, const uint8_t *item_body, uint32_t item_body_len)
{
    /*
       write item header first, then the kv buffer(item body).
       if meat a power down while item writing, we wanna the consequence is predictable.

       *** for some norflash on-chip, no support for multi-times-write on the same write unit,
       that means the 0xFF -> 0xEE -> 0xCC -> 0x44 trick actually does not work.
       only write once on one single write unit.
       so we do the header write except the kv_wunit_t domain(donnot waste the chance to modify
       essential member, like discarded_flag, otherwise we have to "erase" before "write" again).
     */

    // write header, if a power down happen here, we can know it by header magic verify.
    if (kv_item_hdr_write(item_start, item_hdr) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    // if a power down happen here, we can know it by checksum verify.

    // write body(key & value)
    return kv_flash_write(item_start + KV_ITEM_HDR_SIZE, (void *)item_body, item_body_len);
}

static kv_err_t kv_item_hdr_read(uint32_t item_start, kv_item_hdr_t *item_hdr)
{
    return kv_flash_read(item_start, item_hdr, sizeof(kv_item_hdr_t));
}

static kv_err_t kv_item_body_read(kv_item_t *item)
{
    uint8_t *kv_buf = (uint8_t *)ln_kv_malloc(KV_ITEM_SIZE_OF_BODY(item));

    if (!kv_buf) {
        return KV_ERR_OUT_OF_MEMORY;
    }

    if (kv_flash_read(KV_ITEM_ADDR_OF_BODY(item), kv_buf,
                        KV_ITEM_SIZE_OF_BODY(item)) != KV_ERR_NONE) {
        ln_kv_free(kv_buf);
        return KV_ERR_FLASH_READ_FAILED;
    }

    item->body = kv_buf;

    return KV_ERR_NONE;
}

static kv_err_t kv_item_do_delete(uint32_t item_start)
{
    // set the discarded_flag on the flash
    return kv_flash_wunit_modify(KV_ADDR_OF_FIELD(item_start, kv_item_hdr_t, discarded_flag), KV_ITEM_DISCARDED);
}

static kv_err_t kv_item_delete_aux(uint32_t item_start)
{
    if (kv_item_do_delete(item_start) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    kv_blk_set_dirty(KV_ITEM_ADDR2BLK(item_start));

    return KV_ERR_NONE;
}

static kv_err_t kv_item_delete(kv_item_t *item)
{
    return kv_item_delete_aux(item->pos);
}

static kv_err_t kv_item_try_delete(kv_item_t *item)
{
    kv_item_hdr_t prev_item_hdr;

    uint32_t prev_pos = item->hdr.prev_pos;

    if (kv_item_hdr_read(prev_pos, &prev_item_hdr) != KV_ERR_NONE) {
        return KV_ERR_FLASH_READ_FAILED;
    }

    if (!KV_ITEM_IS_LEGAL(&prev_item_hdr) ||
        KV_ITEM_IS_DISCARDED(&prev_item_hdr)) {
        /* situation 1 in kv_item_update, the prev_pos is in a fresh block now(for gc sake),
           no need to delete the previous one.
         */
        return KV_ERR_NONE;
    }

    // test if situation 2 in kv_item_update happened

    uint8_t k_len       = item->hdr.k_len;
    uint8_t prev_k_len  = prev_item_hdr.k_len;

    if (k_len != prev_k_len) {
        return KV_ERR_NONE;
    }

    uint8_t * prev_key = (uint8_t *)ln_kv_malloc(k_len);
    if (!prev_key) {
        return KV_ERR_OUT_OF_MEMORY;
    }

    if (kv_flash_read(prev_pos + KV_ITEM_HDR_SIZE, prev_key, k_len) != KV_ERR_NONE) {
        ln_kv_free(prev_key);
        return KV_ERR_FLASH_READ_FAILED;
    }

    // key changes, means another turn of gc happened, the previous block is filled with new item.
    if (memcmp(prev_key, (void *)item->body, k_len) != 0) {
        ln_kv_free(prev_key);
        return KV_ERR_NONE;
    }

    ln_kv_free(prev_key);

    // the previous item is still there, delete it.
    return kv_item_delete_aux(prev_pos);
}

__STATIC_INLINE__ void kv_item_free(kv_item_t *item)
{
    if (item->body) {
        ln_kv_free(item->body);
    }
    ln_kv_free(item);
}

static int kv_item_hdr_verify(kv_item_hdr_t *item_hdr, uint32_t item_start, uint32_t blk_start)
{
    uint8_t  k_len = item_hdr->k_len;
    uint16_t v_len = item_hdr->v_len;

    // 1. test key/value size
    if (k_len == 0 || \
        v_len == 0 || \
        k_len == (uint8_t)-1 || \
        v_len == (uint16_t)-1) {
        return LN_FALSE;
    }

    // 2. test the flash address range
    if (item_start + KV_ITEM_SIZE(k_len, v_len) > KV_BLK_END(blk_start)) {
        return LN_FALSE;
    }

    if (item_start + KV_ITEM_SIZE(k_len, v_len) > KV_FLASH_END) {
        return LN_FALSE;
    }

    return LN_TRUE;
}

__STATIC_INLINE__ int kv_item_body_verify(kv_item_t *item)
{
    return item->hdr.checksum == kv_checksum_crc8(item->body, item->hdr.k_len + item->hdr.v_len);
}

__STATIC_INLINE__ int kv_item_is_updated_one(kv_item_t *item)
{
    /* attention:
       there's a possibility that item->pos equals to item->hdr.prev_pos,
       if this happen, means the previous one is gc-ed, and the "item" is just
       moved to the prev_pos by coincidence.
       that means the previous one is not in prev_pos any more. no need to delete.
      */
    return item->hdr.prev_pos != 0 && item->pos != item->hdr.prev_pos;
}

static int kv_item_is_moved(kv_item_t *item)
{
    kv_err_t err;
    int is_moved = LN_FALSE;
    kv_item_hdr_t item_hdr;
    uint8_t *body_backup = NULL;

    // drag the header out of the flash
    err = kv_item_hdr_read(item->pos, &item_hdr);
    if (err != KV_ERR_NONE) {
        return LN_FALSE;
    }

    // compare the header
    if (memcmp(&item_hdr, &item->hdr, sizeof(kv_item_hdr_t)) != 0) {
        return LN_TRUE;
    }

    // backup the body
    body_backup = item->body;

    // drag the body out of the flash
    err = kv_item_body_read(item);
    if (err != KV_ERR_NONE) {
        return LN_FALSE;
    }

    // compare the body
    if (memcmp(body_backup, item->body, item->hdr.k_len + item->hdr.v_len) != 0) {
        is_moved = LN_TRUE;
    }

    ln_kv_free(item->body);
    item->body = body_backup;

    return is_moved;
}

static kv_err_t kv_item_do_gc(kv_item_t *item, const void *p_blk_dst)
{
    kv_err_t err;
	uint32_t blk_dst = *(uint32_t *)p_blk_dst;

    err = kv_item_body_read(item);
    if (err != KV_ERR_NONE) {
        return err;
    }

    if (kv_item_write(KV_BLK_USABLE_ADDR(blk_dst),
                        &item->hdr, item->body,
                        KV_ITEM_SIZE_OF_BODY(item)) != KV_ERR_NONE) {
        return KV_ERR_FLASH_WRITE_FAILED;
    }

    // reduce the free_size
    kv_blk_freesz_reduce(blk_dst, KV_ITEM_SIZE_OF_ITEM(item));

    return KV_ERR_NEXT_LOOP;
}

static kv_err_t kv_item_do_fetch_new_copy(kv_item_t *item, const void *the_item)
{
    kv_err_t err;
    kv_item_t * dst_item = (kv_item_t *)the_item;
    uint8_t  dst_k_len   = dst_item->hdr.k_len;
    uint16_t dst_v_len   = dst_item->hdr.v_len;
    uint8_t  k_len       = item->hdr.k_len;
    uint16_t v_len       = item->hdr.v_len;

    if (k_len != dst_k_len ||
        v_len != dst_v_len) {
        return KV_ERR_NEXT_LOOP;
    }

    err = kv_item_body_read(item);
    if (err != KV_ERR_NONE) {
        return err;
    }

    if (memcmp(item->body, dst_item->body, k_len + v_len) == 0) {
        return KV_ERR_NONE;
    }

    return KV_ERR_NEXT_LOOP;
}

static kv_err_t kv_item_do_fetch(kv_item_t *item, const void *key)
{
    kv_err_t err;
    uint8_t k_len = item->hdr.k_len;

    if (strlen((const char *)key) != k_len) {
        return KV_ERR_NEXT_LOOP;
    }

    err = kv_item_body_read(item);
    if (err != KV_ERR_NONE) {
        return err;
    }

    if (memcmp(item->body, key, k_len) == 0) {
        return KV_ERR_NONE;
    }

    return KV_ERR_NEXT_LOOP;
}

static kv_err_t kv_item_do_recovery(kv_item_t *item, const void *dummy)
{
    LN_UNUSED(dummy);

    kv_err_t err;

    err = kv_item_body_read(item);
    if (err != KV_ERR_NONE) {
        return err;
    }

    if (!kv_item_body_verify(item)) {
        err = kv_item_delete(item);
    } else if (kv_item_is_updated_one(item)) {
        /* we need to take a look inside into the previous item,
           to check whether the previous one is correctly deleted.
           maybe a power down happened during the update
        */

        /*
          if current item's pos is not as same as its prev_pos, two situation:
            1. the item is be gc-ed to another block(just the same).
            2. the item is updated, the new data is saved but the old one may be not deleted.
         */
        err = kv_item_try_delete(item);
    }

    if (err != KV_ERR_NONE) {
        return err;
    }

    return KV_ERR_NEXT_LOOP;
}

/*
   aos's logic framework is nice here, so I learn from it.
   although I have my own idea to write this logic, but the more other kv logic I process,
   the more reasonable of this framework I find. thanks.
*/
static kv_err_t kv_item_walkthru(uint32_t blk_start,
                                                    kv_item_walker_t walker,
                                                    const void *arg,
                                                    kv_item_t **item_out)
{
    kv_err_t err;
    uint32_t cur_item;
    kv_item_t *item;
    kv_item_hdr_t *item_hdr;
    int is_item_integral = LN_TRUE;

    if (item_out) {
        *item_out = NULL;
    }

    cur_item = KV_BLK_FIRST_ITEM(blk_start);

    do {
        is_item_integral = LN_TRUE;

        item = (kv_item_t *)ln_kv_malloc(sizeof(kv_item_t));
        if (!item) {
            return KV_ERR_OUT_OF_MEMORY;
        }

        item->body  = NULL;
        item_hdr    = &item->hdr;

        // drag the item header out of the flash to see whether is a legal item
        err = kv_item_hdr_read(cur_item, item_hdr);
        if (err != KV_ERR_NONE) {
            kv_item_free(item);
            return err;
        }

        /* meet a magic none-set item
           if magic is not set:
             1. it's a fresh item slot(not used)
             2. meet a power down before item magic write done
         */
        if (!KV_ITEM_IS_LEGAL(item_hdr)) {
            if (KV_ITEM_IS_FRESH(item_hdr)) {
                // situation 1, it's a fresh item slot, meet the ending
                kv_item_free(item);
                break;
            }

            // situation 2, meet power down, probably broken, just mark the item as discarded
            is_item_integral = LN_FALSE;
        }

        if (!is_item_integral ||
            !kv_item_hdr_verify(item_hdr, cur_item, blk_start)) {
            // it's no integral item, or header verify illegal
            if (kv_item_do_delete(cur_item) == KV_ERR_NONE) {
                kv_blk_set_dirty(blk_start);
            }

            cur_item += KV_ITEM_HDR_SIZE;
            kv_item_free(item);

            continue;
        }

        if (!KV_ITEM_IS_DISCARDED(item_hdr)) {
            // tell the item where he is, he does not know yet.
            item->pos = cur_item;

            err = walker(item, arg);
            if (err == KV_ERR_NONE) {
                if (item_out) {
                    *item_out = item;
                } else {
                    kv_item_free(item);
                }
                return KV_ERR_NONE;
            } else if (err != KV_ERR_NEXT_LOOP) {
                kv_item_free(item);
                return err;
            }
        } else {
            // it's a discarded item, deleted before
            kv_blk_set_dirty(blk_start);
        }

        cur_item += KV_ITEM_SIZE(item_hdr->k_len, item_hdr->v_len);
        kv_item_free(item);
    } while (cur_item + KV_ITEM_HDR_SIZE < KV_BLK_END(blk_start));

    // have walked through all the items, get enough information to refresh the index(blk_info).
    if (cur_item == KV_BLK_FIRST_ITEM(blk_start)) {
        // come from fresh block break
        kv_blk_set_fresh(blk_start);
    } else if (cur_item + KV_ITEM_HDR_SIZE < KV_BLK_END(blk_start)) {
        // come from while ending
        kv_blk_freesz_set(blk_start, KV_BLK_END(blk_start) - cur_item);
        kv_blk_set_inuse(blk_start);
    } else {
        // go across the boarder, means the space left is insufficient
        kv_blk_freesz_set(blk_start, 0);
        kv_blk_reset_inuse(blk_start);
    }

    return KV_ERR_NONE;
}

static kv_item_t *kv_item_do_find(uint32_t blk_start, const char *key)
{
    kv_item_t *the_item;

    if (kv_item_walkthru(blk_start, kv_item_do_fetch, key, &the_item) == KV_ERR_NONE) {
        return the_item;
    }

    return NULL;
}

static kv_item_t *kv_item_find(const char *key)
{
    uint32_t cur_blk;
    kv_item_t *item;

    KV_BLK_FOR_EACH(cur_blk) {
        if (kv_blk_is_bad(cur_blk) ||
            kv_blk_is_fresh(cur_blk)) {
            continue;
        }

        item = kv_item_do_find(cur_blk, key);
        if (item) {
            return item;
        }
    }
    return NULL;
}

static kv_item_t *kv_item_do_find_new_copy(uint32_t blk_start, kv_item_t *item)
{
    kv_item_t *the_item;

    if (kv_item_walkthru(blk_start, kv_item_do_fetch_new_copy,
                            item, &the_item) == KV_ERR_NONE) {
        return the_item;
    }

    return NULL;
}

static kv_item_t *kv_item_find_new_copy(kv_item_t *the_item)
{
    uint32_t cur_blk;
    kv_item_t *item;

    KV_BLK_FOR_EACH(cur_blk) {
        if (kv_blk_is_bad(cur_blk) ||
            kv_blk_is_fresh(cur_blk)) {
            continue;
        }

        item = kv_item_do_find_new_copy(cur_blk, the_item);
        if (item) {
            return item;
        }
    }
    return NULL;
}

static kv_err_t kv_item_do_save(uint32_t item_start, const char *k, const void *v, size_t v_len, uint32_t prev_pos)
{
    kv_err_t err;
    kv_item_hdr_t item_hdr = {0};

    uint8_t  k_len  = strlen(k);
    uint32_t kv_len = KV_ITEM_BODY_SIZE(k_len, v_len);

    uint8_t *kv_buf = (uint8_t *)ln_kv_malloc(kv_len);
    if (!kv_buf) {
        return KV_ERR_OUT_OF_MEMORY;
    }

    memset(kv_buf,          0,  kv_len);
    memcpy(kv_buf,          k,  k_len);
    memcpy(kv_buf + k_len,  v,  v_len);

    item_hdr.magic      = KV_ITEM_HDR_MAGIC;
    item_hdr.checksum   = kv_checksum_crc8(kv_buf, k_len + v_len);

    item_hdr.k_len      = k_len;
    item_hdr.v_len      = v_len;
    item_hdr.prev_pos   = prev_pos;

    // we donnot deal with the discarded_flag here
    err = kv_item_write(item_start, &item_hdr, kv_buf, kv_len);

    ln_kv_free(kv_buf);

    return err;
}

static int kv_item_value_is_match(kv_item_t *item, const void *value, size_t value_len)
{
    uint8_t  k_len = item->hdr.k_len;
    uint16_t v_len = item->hdr.v_len;

    if (value_len != v_len) {
        return LN_FALSE;
    }

    if (memcmp(item->body + k_len, value, v_len) != 0) {
        return LN_FALSE;
    }

    return LN_TRUE;
}

static kv_err_t kv_item_fix(kv_item_t *item)
{
    kv_item_t *moved_item = kv_item_find_new_copy(item);
    if (!moved_item) {
        return KV_ERR_POS_FIX_FAILED;
    }

    item->pos = moved_item->pos;

    kv_item_free(moved_item);

    return KV_ERR_NONE;
}

static kv_err_t kv_item_save(const char *k, const void *v, size_t v_len, kv_item_t *prev_item)
{
    kv_err_t err;
    uint32_t blk_start, k_len, item_size;

    k_len       = strlen(k);
    item_size   = KV_ITEM_SIZE(k_len, v_len);

    blk_start   = kv_blk_search_suitable(item_size);
    if (blk_start == KV_BLK_INVALID) {
        return KV_ERR_INSUFFICIENT_SPACE;
    }

    /* gc may have been triggered in kv_blk_search_suitable, the "item" has been gc-ed from its original block to another,
       so there is a possibility that neither the item->pos nor item->hdr.prev_pos is valid any more(actually the
       prev_pos(is the "item->pos") of the new item is not valid either).
       and there is no way to know where the new position of the "item" is (except to search again).
       this may cause seriously bug.
     */

    /* gc maybe has been triggered in kv_blk_search_suitable, and the prev_item maybe moved from its
       original block to another, we should identify this situation, and do kv_item_fix
     */
    if (prev_item && kv_item_is_moved(prev_item)) {
        /* we quite sure that the item is moved to another block because of gc.
           find the new position of the item
         */
        err = kv_item_fix(prev_item);
        if (err != KV_ERR_NONE) {
            return err;
        }
    }

    err = kv_item_do_save(KV_BLK_USABLE_ADDR(blk_start), k, v, v_len, prev_item ? prev_item->pos : 0);
    if (err != KV_ERR_NONE) {
        return err;
    }

    // reduce the free_size
    kv_blk_freesz_reduce(blk_start, item_size);

    if (kv_blk_is_fresh(blk_start)) {
        kv_blk_reset_fresh(blk_start);

        if (!kv_blk_is_full(blk_start)) {
            kv_blk_set_inuse(blk_start);
        }
    } else if (kv_blk_is_full(blk_start)) {
        kv_blk_reset_inuse(blk_start);
    }

    return KV_ERR_NONE;
}

static kv_err_t kv_item_update(kv_item_t *item, const char *key, const void *value, size_t value_len)
{
    kv_err_t err;

    if (kv_item_value_is_match(item, value, value_len)) {
        return KV_ERR_NONE;
    }

    /* save first, then delete, is trying best to keep data integrality. */

    err = kv_item_save(key, value, value_len, item);
    if (err != KV_ERR_NONE) {
        return err;
    }

    /*
       if gc triggered int kv_item_save, and the "item" has been gc-ed from its original block to another,
       AND worse than that, a power down just happen here.
        1. the previous block is fresh(after gc)
        2. the previous block is filled with other items(another round of gc happen)
       no matter 1 or 2 happen, when recovery, should test whether the item in prev_pos is legal(if not, means
       block of prev_pos is fresh now), or with the same key as the "item"(the item in prev_pos may be one with
       new key for gc's sake).
       a lot of things should be done in kv_item_try_delete.
     */

    return kv_item_delete(item);
}

static kv_err_t kv_param_verify(uint32_t flash_start, uint32_t flash_end, kv_flash_drv_t *flash_drv, kv_flash_prop_t *flash_prop)
{
    if (!flash_drv || !flash_prop) {
        return KV_ERR_INVALID_PARAM;
    }

    if (!KV_IS_ALINGED_LOG2(flash_start, flash_prop->sector_size_log2)) {
        return KV_ERR_INVALID_PARAM;
    }

    if (!KV_IS_ALINGED_LOG2(flash_end, flash_prop->sector_size_log2)) {
        return KV_ERR_INVALID_PARAM;
    }

    return KV_ERR_NONE;
}

__STATIC_INLINE__ kv_err_t kv_mgr_index_build(uint32_t blk_start)
{
    return kv_item_walkthru(blk_start, kv_item_do_recovery, NULL, NULL);
}

static int kv_mgr_blk_index_rebuild(void)
{
    uint32_t cur_blk;
    int is_rebuild_done = LN_FALSE;

    KV_BLK_FOR_EACH(cur_blk) {
        if (kv_blk_is_hanging(cur_blk)) {
            if (kv_mgr_index_build(cur_blk) == KV_ERR_NONE) {
                kv_blk_reset_hanging(cur_blk);
                is_rebuild_done = LN_TRUE;
            }
        }
    }

    return is_rebuild_done;
}

static kv_err_t kv_mgr_workspace_locate(void)
{
    uint32_t cur_blk;

    /* we give blocks with KV_BLK_FLAG_HANGING a chance to rebuild index */
    if (KV_MGR_BLK_NUM_HANGING > 0) {
        kv_mgr_blk_index_rebuild();
    }

    if (KV_MGR_BLK_NUM_INUSE == 0 && KV_MGR_BLK_NUM_FRESH == 1) {
        /* if here, we cannot just give out the last fresh block, otherwise the kv will get into
            KV_ERR_NO_WRITEABLE_BLK next time, try a gc here to get a "rescue" */

        kv_try_gc();
    }

    if (KV_NO_WRITEABLE_BLK()) {
        return KV_ERR_NO_WRITEABLE_BLK;
    }

    /* if no block inuse, we locate workspace in first fresh block
       else locate in first block inuse
    */
    KV_BLK_FOR_EACH(cur_blk) {
        if (KV_MGR_BLK_NUM_INUSE == 0) {
            if (kv_blk_is_fresh(cur_blk)) {
                KV_MGR_WORKSPACE = cur_blk;
                return KV_ERR_NONE;
            }
        } else if (kv_blk_is_inuse(cur_blk)) {
            KV_MGR_WORKSPACE = cur_blk;
            return KV_ERR_NONE;
        }
    }

    // actually unreachable here, but have to make compiler happy
    return KV_ERR_NONE;
}

/*
 *          src                              dst
 * 1. mark gc_src tag
 *                                      2. mark gc_dst tag
 * 3. copy data to dst
 * 4. format src block
 *                                      5. mark gc_done tag
 */
static kv_err_t kv_do_gc(uint32_t blk_src, uint32_t blk_dst, int is_handle_incomplete)
{
    kv_err_t err;

    // step 1
    if (!is_handle_incomplete) {
        // in handle incomplete case, gc_src already been marked before power down
        err = kv_blk_mark_gc_src(blk_src);
        if (err != KV_ERR_NONE) {
            return err;
        }
    }

    // step 2
    err = kv_blk_mark_gc_dst(blk_dst);
    if (err != KV_ERR_NONE) {
        return err;
    }

    // step 3
    err = kv_item_walkthru(blk_src, kv_item_do_gc, (const void *)&blk_dst, NULL);
    if (err != KV_ERR_NONE) {
        return err;
    }

    // step 4
    kv_blk_reset_inuse(blk_src);

    if (kv_blk_format(blk_src) != KV_ERR_NONE) {
        kv_blk_set_bad(blk_src);
    }

    // step 5
    err = kv_blk_mark_gc_done(blk_dst);
    if (err != KV_ERR_NONE) {
        return err;
    }

    kv_blk_reset_fresh(blk_dst);

    if (!kv_blk_is_full(blk_dst)) {
        kv_blk_set_inuse(blk_dst);
    }

    return KV_ERR_NONE;
}

struct blk_info {
    int nr;
#define NR_ABNORMAL_BLK_MAX     3
    uint32_t blks[NR_ABNORMAL_BLK_MAX];
};

/*
 * a power down may happen during any process of a gc action, we should fix
 * all incomplete gc when kv bootup.
 */
static kv_err_t kv_handle_incomplete_gc(struct blk_info gc_src_blk, struct blk_info gc_dst_not_done_blk, uint32_t fresh_blk)
{
    kv_err_t err;
    uint32_t src_blk, dst_blk;

    if (gc_src_blk.nr > 1 || gc_dst_not_done_blk.nr > 1) {
        log_e("blk check error: more than one gc_src[%d] or gc_dst_not_done[%d] block\n", gc_src_blk.nr, gc_dst_not_done_blk.nr);
        return KV_ERR_BLK_STATUS_ERROR;
    }

    if (gc_src_blk.nr == 0) {
        return KV_ERR_NONE;
    }

    src_blk = gc_src_blk.blks[0];

    if (gc_dst_not_done_blk.nr == 0) {
        if (fresh_blk == (uint32_t)-1) {
            log_e("no gc_dst_not_done block, neither fresh block\n");
            return KV_ERR_BLK_STATUS_ERROR;
        } else {
            // log_i("info: no gc_dst_not_done block, choose a fresh block[0x%x]\n", fresh_blk);
            dst_blk = fresh_blk;
        }
    } else {
        dst_blk = gc_dst_not_done_blk.blks[0];

        /* format dst_blk first(make it fresh), for an in-complete gc happends in a very small
           probability, for a clean code, we just do a totally retry here. */
        err = kv_blk_format(dst_blk);
        if (err != KV_ERR_NONE) {
            kv_blk_set_bad(dst_blk);
            return err;
        }
    }

    return kv_do_gc(src_blk, dst_blk, LN_TRUE);
}

static kv_err_t kv_mgr_ctl_build(void)
{
    uint32_t cur_blk;
    kv_blk_hdr_t blk_hdr;

    /* theoretically, should at least only one gc_src/gc_dst_not_done block */
    uint32_t fresh_blk = (uint32_t)-1;
    struct blk_info gc_src_blk = { 0 }, gc_dst_not_done_blk = { 0 };

    KV_BLK_FOR_EACH(cur_blk) {
        if (kv_blk_hdr_read(cur_blk, &blk_hdr) != KV_ERR_NONE) {
            // sth must be wrong seriously with this block
            kv_blk_set_bad(cur_blk);
            continue;
        }

        if (!KV_BLK_IS_LEGAL(&blk_hdr)) {
            if (kv_blk_format(cur_blk) != KV_ERR_NONE) {
                // sth must be wrong seriously with this block
                kv_blk_set_bad(cur_blk);
            } else {
                fresh_blk = cur_blk;
            }
            // we get a fresh block
            continue;
        }

        if (KV_BLK_IS_GC_SRC(&blk_hdr)) {
            gc_src_blk.blks[gc_src_blk.nr++] = cur_blk;
            continue;
        }

        if (KV_BLK_IS_GC_DST_NOT_DONE(&blk_hdr)) {
            gc_dst_not_done_blk.blks[gc_dst_not_done_blk.nr++] = cur_blk;
            continue;
        }

        // do index building
        if (kv_mgr_index_build(cur_blk) != KV_ERR_NONE) {
            // sth goes wrong while index building, we give it a mark
            kv_blk_set_hanging(cur_blk);
            continue;
        }

        if (kv_blk_is_fresh(cur_blk)) {
            fresh_blk = cur_blk;
        }
    }

    return kv_handle_incomplete_gc(gc_src_blk, gc_dst_not_done_blk, fresh_blk);
}

static kv_err_t kv_mgr_ctl_init(void)
{
    kv_blk_detail_t *blk_detail;

    KV_MGR_BLK_NUM_TOTAL = KV_FLASH_SIZE / KV_FLASH_SECTOR_SIZE;
#if defined(KV_MGR_STATIC_MEM)
    blk_detail = KV_MGR_BLK_DETAIL;
    memset(blk_detail, 0, sizeof(kv_blk_detail_t) * KV_MGR_BLK_NUM_TOTAL);
#else
    blk_detail = (kv_blk_detail_t *)ln_kv_malloc(sizeof(kv_blk_detail_t) * KV_MGR_BLK_NUM_TOTAL);
    if (!blk_detail) {
        return KV_ERR_OUT_OF_MEMORY;
    }

    memset(blk_detail, 0, sizeof(kv_blk_detail_t) * KV_MGR_BLK_NUM_TOTAL);
    KV_MGR_BLK_DETAIL = blk_detail;
#endif

#ifndef KV_CONFIG_OS_NONE
    if (ln_kv_mutex_create(KV_MGR_LOCK) != LN_TRUE) {
        return KV_ERR_INTERNAL_ERROR;
    }
#endif

    return KV_ERR_NONE;
}

static void kv_mgr_ctl_deinit(void)
{
#ifndef KV_CONFIG_OS_NONE
    ln_kv_mutex_delete(KV_MGR_LOCK);
#endif

#if !defined(KV_MGR_STATIC_MEM)
    ln_kv_free(KV_MGR_BLK_DETAIL);
    KV_MGR_BLK_DETAIL = NULL;
#endif

    memset(&kv_ctl, 0, sizeof(kv_ctl));
}

/* on each turn of gc, we free some discarded items in the dirty block.
   so we get more space to save the new item.
   gc should be done only when necessary, if there is no sitiation of an item too big to save,
   should not do gc(gc cause erase/write of the flash).
 */
static kv_err_t kv_gc(void)
{
    uint32_t cur_blk, blk_dst;
    int is_gc_done = LN_FALSE, is_rebuild_done = LN_FALSE;

    /* we give blocks with KV_BLK_FLAG_HANGING a chance to rebuild index */
    if (KV_MGR_BLK_NUM_HANGING > 0) {
        is_rebuild_done = kv_mgr_blk_index_rebuild();
    }

    // there must be at least one fresh block left, make workspace pointer to the fresh one
    blk_dst = kv_blk_next_fresh();
    if (blk_dst == KV_BLK_INVALID) {
        /* kinda a bug here, KV_MGR_BLK_NUM_FRESH == 1 */
        return KV_ERR_GC_NOTHING;
    }

    KV_BLK_FOR_EACH(cur_blk) {
        if (kv_blk_is_dirty(cur_blk)) {
            log_i("try to do gc at blk = 0x%08x\n", cur_blk);
            if (kv_do_gc(cur_blk, blk_dst, LN_FALSE) == KV_ERR_NONE) {
                is_gc_done = LN_TRUE;
                break;
            }
        }
    }

    if (is_gc_done) {
        // if do nothing, should restore the workspace;
        KV_MGR_WORKSPACE = blk_dst;
    }

    return (is_gc_done || is_rebuild_done) ? KV_ERR_NONE : KV_ERR_GC_NOTHING;
}

static kv_err_t kv_try_gc(void)
{
    uint32_t cur_blk, blk_dst;

    blk_dst = kv_blk_get_a_fresh();
    if (blk_dst == KV_BLK_INVALID) {
        return KV_ERR_GC_NOTHING;
    }

    KV_BLK_FOR_EACH(cur_blk) {
        if (kv_blk_is_dirty(cur_blk)) {
            if (kv_do_gc(cur_blk, blk_dst, LN_FALSE) == KV_ERR_NONE) {
                return KV_ERR_NONE;
            }
        }
    }

    return KV_ERR_GC_NOTHING;
}

#if 0
static void kv_u8_disp(uint8_t *buf, size_t buf_len)
{
    // int i = 0;

    // elog_raw("\"");
    // for (i = 0; i < buf_len; ++i) {
    //     elog_raw("%c", buf[i]);
    // }
    // elog_raw("\"\n");
}

static kv_err_t kv_block_walkthru(uint32_t blk_start)
{
    // int i = 0;
    kv_err_t err;
    uint32_t cur_item;
    kv_item_t *item;
    kv_item_hdr_t *item_hdr;
    int is_item_integral = LN_TRUE;

    cur_item = KV_BLK_FIRST_ITEM(blk_start);

    do {
        // log_i("\n - [%d] item\n", i++);
        log_i(" - addr: 0x%x\n", cur_item);

        is_item_integral = LN_TRUE;

        item = (kv_item_t *)ln_kv_malloc(sizeof(kv_item_t));
        if (!item) {
            log_e("malloc failed for KV item\n");
            return KV_ERR_OUT_OF_MEMORY;
        }

        item->body  = NULL;
        item_hdr    = &item->hdr;

        err = kv_item_hdr_read(cur_item, item_hdr);
        if (err != KV_ERR_NONE) {
            log_e(" - kv header read failed\n");
            kv_item_free(item);
            return err;
        }

        if (!KV_ITEM_IS_LEGAL(item_hdr)) {
            if (KV_ITEM_IS_FRESH(item_hdr)) {
                kv_item_free(item);
                break;
            }

            is_item_integral = LN_FALSE;
        }

        if (!is_item_integral) {
            // log_i(" - not integral\n");
        }

        if (!kv_item_hdr_verify(item_hdr, cur_item, blk_start)) {
            // log_i(" - header verify failed\n");
        }

        if (KV_ITEM_IS_DISCARDED(item_hdr)) {
            // log_i(" - discarded\n");
        }

        if (!is_item_integral || \
            !kv_item_hdr_verify(item_hdr, cur_item, blk_start)) {

            cur_item += KV_ITEM_HDR_SIZE;
            kv_item_free(item);

            continue;
        }

        item->pos = cur_item;

        err = kv_item_body_read(item);
        if (err == KV_ERR_NONE) {
            if (!kv_item_body_verify(item)) {
                log_e(" - kv body verify failed\n");
            } else {
                // log_i(" - prev_pos: 0x%x [%d block]\n", item->hdr.prev_pos, KV_BLK_ADDR2IDX(KV_ITEM_ADDR2BLK(item->hdr.prev_pos)));
                // log_i(" - curr_pos: 0x%x [%d block]\n", item->pos, KV_BLK_ADDR2IDX(KV_ITEM_ADDR2BLK(item->pos)));
                // log_i(" - k_len: %d\n", item->hdr.k_len);
                // log_i(" - v_len: %d\n", item->hdr.v_len);
                // log_i(" - k: ");
                kv_u8_disp(item->body, item->hdr.k_len);
                // log_i(" - v: ");
                kv_u8_disp(item->body + item->hdr.k_len, item->hdr.v_len);
            }
        }

        cur_item += KV_ITEM_SIZE(item_hdr->k_len, item_hdr->v_len);
        kv_item_free(item);
    } while (cur_item + KV_ITEM_HDR_SIZE < KV_BLK_END(blk_start));

    // have walked through all the items, get enough information to refresh the index(blk_info).
    if (cur_item == KV_BLK_FIRST_ITEM(blk_start)) {
        // log_i("block fresh\n");
    } else if (cur_item + KV_ITEM_HDR_SIZE < KV_BLK_END(blk_start)) {
        // log_i("block inuse, freesize: 0x%x\n", KV_BLK_END(blk_start) - cur_item);
    } else {
        log_w("block full\n");
    }

    return KV_ERR_NONE;
}

kv_err_t tos_kv_walkthru(void)
{
    // int i = 0;
    uint32_t cur_blk;
    kv_blk_hdr_t blk_hdr;

    // log_i("workspace: 0x%x [%d block]\n", KV_MGR_WORKSPACE, KV_BLK_ADDR2IDX(KV_MGR_WORKSPACE));

    KV_BLK_FOR_EACH(cur_blk) {
        // log_i("[%d] block\n", i++);
        // log_i("addr: 0x%x\n", cur_blk);

        if (kv_blk_hdr_read(cur_blk, &blk_hdr) != KV_ERR_NONE) {
            // log_i("block header read failed\n");
            continue;
        }

        if (!KV_BLK_IS_LEGAL(&blk_hdr)) {
            // log_i("block not formatted\n");
            continue;
        }

        if (KV_BLK_IS_GC_SRC(&blk_hdr)) {
            // log_i("block is gc-src\n");
        }

        if (KV_BLK_IS_GC_DST(&blk_hdr)) {
            // log_i("block is gc-dst\n");
        }

        if (KV_BLK_IS_GC_DONE(&blk_hdr)) {
            // log_i("block is gc-done\n");
        }

        if (KV_BLK_IS_GC_DST_NOT_DONE(&blk_hdr)) {
            // log_i("block is gc-dst but not done\n");
        }

        if (kv_block_walkthru(cur_blk) != KV_ERR_NONE) {
            // log_i("block diagnosis failed\n");
            continue;
        }
    }

    return KV_ERR_NONE;
}
#endif

kv_err_t ln_kv_init(uint32_t flash_start, uint32_t flash_end, kv_flash_drv_t *flash_drv, kv_flash_prop_t *flash_prop)
{
    kv_err_t err;

    if (kv_param_verify(flash_start, flash_end, flash_drv, flash_prop) != KV_ERR_NONE) {
        return KV_ERR_INVALID_PARAM;
    }

    memset(&kv_ctl, 0, sizeof(kv_ctl));

    kv_flash_ctl_init(flash_start, flash_end, flash_drv, flash_prop);

    err = kv_mgr_ctl_init();
    if (err != KV_ERR_NONE) {
        return err;
    }

    err = kv_mgr_ctl_build();
    if (err != KV_ERR_NONE) {
        return err;
    }

    return kv_mgr_workspace_locate();
}

kv_err_t ln_kv_deinit(void)
{
    kv_mgr_ctl_deinit();
    return KV_ERR_NONE;
}

kv_err_t ln_kv_set(const char *key, const void *value, size_t value_len)
{
    kv_err_t err;
    kv_item_t *item;

    if (!key || !value) {
        return KV_ERR_INVALID_PARAM;
    }

    if (strlen(key) >= (uint8_t)-1 ||
        value_len >= (uint16_t)-1) {
        return KV_ERR_SIZE_EXCEEDED;
    }

    kv_lock();

    item = kv_item_find(key);
    if (item) { // already exist
        err = kv_item_update(item, key, value, value_len);
        kv_item_free(item);
    } else {
        err = kv_item_save(key, value, value_len, NULL);
    }

    kv_unlock();

    return err;
}

kv_err_t ln_kv_get(const char *key, void *value_buf, size_t value_buf_size, size_t *value_len)
{
    uint8_t k_len;
    uint16_t v_len;
    kv_item_t *item;

    if (!key || !value_buf || !value_len) {
        return KV_ERR_INVALID_PARAM;
    }

    if (strlen(key) >= (uint8_t)-1) {
        return KV_ERR_SIZE_EXCEEDED;
    }

    kv_lock();

    item = kv_item_find(key);
    if (!item) {
        kv_unlock();
        return KV_ERR_NOT_EXIST;
    }

    kv_unlock();

    k_len       = item->hdr.k_len;
    v_len       = item->hdr.v_len;
    *value_len  = v_len;

    if (value_buf_size < v_len) {
        kv_item_free(item);
        return KV_ERR_BUF_TOO_SHORT;
    }

    memcpy(value_buf, item->body + k_len, v_len);
    kv_item_free(item);

    return KV_ERR_NONE;
}

int ln_kv_has_key(const char *key)
{
    kv_item_t *item;
    int has_key = LN_FALSE;

    if (!key) {
        return KV_ERR_INVALID_PARAM;
    }

    if (strlen(key) >= (uint8_t)-1) {
        return KV_ERR_SIZE_EXCEEDED;
    }

    kv_lock();
    item = kv_item_find(key);
    if (item) {
        has_key = LN_TRUE;
        kv_item_free(item);
    }
    kv_unlock();

    return has_key;
}

kv_err_t ln_kv_del(const char *key)
{
    kv_err_t err;
    kv_item_t *item;

    if (!key) {
        return KV_ERR_INVALID_PARAM;
    }

    if (strlen(key) >= (uint8_t)-1) {
        return KV_ERR_SIZE_EXCEEDED;
    }

    kv_lock();

    item = kv_item_find(key);
    if (!item) {
        kv_unlock();
        return KV_ERR_NOT_EXIST;
    }

    err = kv_item_delete(item);
    kv_item_free(item);

    kv_unlock();

    return err;
}

