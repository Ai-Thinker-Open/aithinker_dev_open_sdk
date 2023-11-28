#include "ota_agent.h"
#include "ota_image.h"
#include "ota_port.h"
#include "xz_decompress.h"
#include "flash_partition_mgr.h"
#include <string.h>

static uint8_t temp_buf[LEN_4KB];

static image_hdr_t         app_header;
static image_hdr_t         ota_header;
static partition_info_t    partition_info_app;
static partition_info_t    partition_info_ota;
static uint32_t            xz_already_read = 0;

static int verify_total_img_partition(partition_type_t type, \
                                      partition_info_t *info, image_hdr_t *hdr)
{
    int err;

    if (LN_TRUE != ln_fetch_partition_info(type, info)) {
        return OTA_ERR_PARTITION_TAB;
    }

    if (OTA_ERR_NONE != (err = image_header_read(type, hdr))) {
        return err;
    }

    if (OTA_ERR_NONE != (err = image_header_verify(hdr))) {
        return err;
    }

    if (OTA_ERR_NONE != (err = image_body_verify(info->start_addr, hdr))) {
        return err;
    }

    return OTA_ERR_NONE;
}

static int orig_image_do_restore(uint32_t app_body_offset, \
                                 uint32_t ota_body_offset, uint32_t size)
{
    uint32_t i = 0;
    uint32_t count_4k = size / LEN_4KB;
    uint32_t remainder_4k = size % LEN_4KB;
    ota_port_ctx_t * port = ota_get_port_ctx();

    for (i = 0; i < count_4k; i++) {
        port->flash_drv.read(ota_body_offset, temp_buf, LEN_4KB);
        port->flash_drv.write(app_body_offset, temp_buf, LEN_4KB);
        ota_body_offset += LEN_4KB;
        app_body_offset += LEN_4KB;
    }

    if (remainder_4k) {
        port->flash_drv.read(ota_body_offset, temp_buf, remainder_4k);
        port->flash_drv.write(app_body_offset, temp_buf, remainder_4k);
    }

    return OTA_ERR_NONE;
}

static int decompress_read_cb(uint8_t **buf, uint32_t *buf_len)
{
	int32_t tmp_len;
    uint32_t ota_xz_size = ota_header.img_size_orig_xz;
    uint32_t ota_img_body_offset = partition_info_ota.start_addr + sizeof(image_hdr_t);
    ota_port_ctx_t * port = ota_get_port_ctx();

	tmp_len = ota_xz_size - xz_already_read;
	if (tmp_len > LEN_4KB) {
		tmp_len = LEN_4KB;
	}

	*buf = temp_buf;

    port->flash_drv.read(ota_img_body_offset + xz_already_read, *buf, tmp_len);

	*buf_len = tmp_len;
	xz_already_read += tmp_len;

	return XZ_RET_OK;
}

static int decompress_write_cb(uint32_t offset, uint8_t *buf, uint32_t buf_len)
{
    uint32_t app_img_body_offset = partition_info_app.start_addr + sizeof(image_hdr_t);
    ota_port_ctx_t * port = ota_get_port_ctx();
    static uint32_t xz_already_write = 0;

    port->flash_drv.write(app_img_body_offset + xz_already_write, buf, buf_len);

	xz_already_write += buf_len;

	return XZ_RET_OK;
}

static int xz_image_do_restore(uint32_t ota_body_offset, uint32_t size)
{
	write_cb_t out;
	read_cb_t in;
    xz_header_t xz_header = {0,};
    ota_port_ctx_t * port = ota_get_port_ctx();

	out.Write = decompress_write_cb;
	in.Read   = decompress_read_cb;

    port->flash_drv.read(ota_body_offset, &xz_header, sizeof(xz_header_t));

    xz_already_read += sizeof(xz_header_t);

    if (XZ_RET_OK != xz_decompress_buf_to_buf(xz_header.header, &out, &in)) {
        return OTA_ERR_DECOMPRESS;
    }

    return OTA_ERR_NONE;
}

static int restore_image(jump_to_app_t jump_to_app)
{
    static image_hdr_t  img_hdr_temp;
    upg_state_t  upgrade_state;
    uint32_t app_body_offset = 0;
    uint32_t ota_body_offset = 0;
    uint32_t size = 0;
    ota_port_ctx_t * port = ota_get_port_ctx();
    int err = OTA_ERR_NONE;

    switch(ota_header.image_type)
    {
        case IMAGE_TYPE_ORIGINAL:
            // copy header
            port->flash_drv.write(partition_info_app.start_addr, &ota_header, sizeof(image_hdr_t));

            // copy image
            app_body_offset = partition_info_app.start_addr + sizeof(image_hdr_t);
            ota_body_offset = partition_info_ota.start_addr + sizeof(image_hdr_t);
            size = ota_header.img_size_orig;
            if (OTA_ERR_NONE != (err = orig_image_do_restore(app_body_offset, \
                                                             ota_body_offset, size))) {
                return err;
            }

            // verify app partition(new image)
            if (OTA_ERR_NONE != (err = verify_total_img_partition(PARTITION_TYPE_APP, \
                                                                &partition_info_app, &app_header))) {
                //TODO:HOOK
                return err;
            }

            // update OTA upg state
            upgrade_state = UPG_STATE_RESTORE_OK;

            if (LN_TRUE != port->upg_state_set(upgrade_state)) {
                //TODO:HOOK
                return OTA_ERR_NVDS_RW;
            }

            jump_to_app(partition_info_app.start_addr + sizeof(image_hdr_t));
            return OTA_ERR_NONE;

        case IMAGE_TYPE_ORIGINAL_XZ:
            // copy header
            memcpy(&img_hdr_temp, &ota_header, sizeof(image_hdr_t));
            img_hdr_temp.image_type = IMAGE_TYPE_ORIGINAL;
            img_hdr_temp.header_crc32 = ln_crc32_signle_cal((uint8_t *)&img_hdr_temp, \
                                                     sizeof(image_hdr_t) - sizeof(uint32_t));

            port->flash_drv.write(partition_info_app.start_addr, &img_hdr_temp, sizeof(image_hdr_t));

            // copy image
            ota_body_offset = partition_info_ota.start_addr + sizeof(image_hdr_t);
            size = ota_header.img_size_orig_xz;

            // it takes a lot of time.
            if (OTA_ERR_NONE != (err = xz_image_do_restore(ota_body_offset, size))) {
                return err;
            }

            // verify app partition(new image)
            if (OTA_ERR_NONE != (err = verify_total_img_partition(PARTITION_TYPE_APP, \
                                                             &partition_info_app, &app_header))) {
                //TODO:HOOK
                return err;
            }

            // update OTA upg state
            upgrade_state = UPG_STATE_RESTORE_OK;

            if (LN_TRUE != port->upg_state_set(upgrade_state)) {
                //TODO:HOOK
                return OTA_ERR_NVDS_RW;
            }

            jump_to_app(partition_info_app.start_addr + sizeof(image_hdr_t));
            return OTA_ERR_NONE;

        case IMAGE_TYPE_DIFF:
        case IMAGE_TYPE_DIFF_XZ:
            if ((app_header.ver.ver_major == ota_header.ver_diff_depend.ver_major) && \
                (app_header.ver.ver_minor == ota_header.ver_diff_depend.ver_minor))
            {
                // this feature is not supported yet.
                return OTA_ERR_NOT_SUPPORT;
            }
            /* why? it's impossible to go into this branch */
            else
            {
                // download ok! why? App should check it!
                return OTA_ERR_IMPOSSIBLE_VER;
            }

        default:
            return OTA_ERR_IMG_TYPE;
    }
}

int ota_boot_upgrade_agent(jump_to_app_t jump_to_app)
{
    ota_port_ctx_t * port = ota_get_port_ctx();
    upg_state_t  upgrade_state = (upg_state_t)((uint32_t)-1);
    int err = OTA_ERR_NONE;
    int ret = 0;

    if (LN_TRUE != ln_fetch_partition_info(PARTITION_TYPE_APP, &partition_info_app)) {
        return OTA_ERR_PARTITION_TAB;
    }

    ret = port->upg_state_get(&upgrade_state);
    if ((ret != LN_TRUE) || (upgrade_state == (upg_state_t)((uint32_t)-1)))
    {
        /* upg state parameter stored in the NVDS, if we have never set upgrade_state, 
           its original value is 0xFFFFFFFF((uint32_t)-1).  
           if reach here, may be first startup or NVDS damaged */

        /**
         * This part of the code is reserved for debug
        */
        // debug code start
        // LOG("upgrade_state = 0x%x", upgrade_state);
        // if (OTA_ERR_NONE != (err = verify_total_img_partition(PARTITION_TYPE_APP, \
        //                                               &partition_info_app, &app_header))) {
        //     //TODO:HOOK
        //     return err;
        // }
        // debug code end

        jump_to_app(partition_info_app.start_addr + sizeof(image_hdr_t));
        return OTA_ERR_NONE;
    }

    if (UPG_STATE_DOWNLOAD_OK != upgrade_state)
    {
        /**
         * This part of the code is reserved for debug
        */
       // debug code start
        // if (OTA_ERR_NONE != (err = verify_total_img_partition(PARTITION_TYPE_APP, \
        //                                               &partition_info_app, &app_header))) {
        //     //TODO:HOOK
        //     return err;
        // }
        // debug code end

        jump_to_app(partition_info_app.start_addr + sizeof(image_hdr_t));
        return OTA_ERR_NONE;
    }
    else
    {
        if (OTA_ERR_NONE != (err = verify_total_img_partition(PARTITION_TYPE_APP, \
                                                      &partition_info_app, &app_header)))
        {
            /* a power loss may have occurred during the last copy. */
            if (OTA_ERR_NONE != (err = verify_total_img_partition(PARTITION_TYPE_OTA, \
                                                          &partition_info_ota, &ota_header))) {
                //abnormal!!! app&ota image are damaged!
                //TODO:HOOK
                return err;
            }

            port->flash_drv.erase(partition_info_app.start_addr, partition_info_app.size);
            return restore_image(jump_to_app);
        }
        else
        {
            if (OTA_ERR_NONE != (err = verify_total_img_partition(PARTITION_TYPE_OTA, \
                                                          &partition_info_ota, &ota_header))) {
                //update OTA upg state [upg restory filed]

                //jump to app!
                jump_to_app(partition_info_app.start_addr + sizeof(image_hdr_t));

                return err;//ota image err but wo also jump to app
            }

            // version compare
            // 如果 APP 分区和 OTA 分区版本不一样，则升级
            // 如果 APP 分区和 OTA 分区版本一样：
            //   1. OTA 分区下载的固件是同一个固件，不升级。
            //   2. OTA 状态未更新的时候发生掉电，此时 ota img hdr 已经拷贝到 app 分区，这个时候需要校验 img crc
            //      如果 OTA 分区的 img crc 和 app 里面的 img crc 相同，则 ota 固件已经拷贝到 app 分区，只需要更新 ota 状态。
            //      如果 OTA 分区的 img crc 和 app 里面的 img crc 不同，则 ota 固件尚未完整拷贝到 app 分区，需要重新拷贝！！！
            if (((ota_header.ver.ver_major << 8) + ota_header.ver.ver_minor) == \
                ((app_header.ver.ver_major << 8) + app_header.ver.ver_minor))
            {
                /* a power loss may have occurred after the last restore before the UPG STATE update. */
                if ((app_header.img_size_orig == ota_header.img_size_orig) && \
                    (app_header.img_crc32_orig == ota_header.img_crc32_orig))
                {
                    //update OTA param
                    upgrade_state = UPG_STATE_RESTORE_OK;

                    if (LN_TRUE != port->upg_state_set(upgrade_state)) {
                        //TODO:HOOK
                        return OTA_ERR_NVDS_RW;
                    }

                    jump_to_app(partition_info_app.start_addr + sizeof(image_hdr_t));
                    return OTA_ERR_NONE;
                }
                /* same version, but new image! Normally this situation should not exist. */
                else
                {
                    port->flash_drv.erase(partition_info_app.start_addr, partition_info_app.size);
                    return restore_image(jump_to_app);
                }
            }
            else
            {
                port->flash_drv.erase(partition_info_app.start_addr, partition_info_app.size);
                return restore_image(jump_to_app);
            }
        }
    }

    return 0;
}
