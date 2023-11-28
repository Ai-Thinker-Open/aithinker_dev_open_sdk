#ifndef __REG_CACHE_H__
#define __REG_CACHE_H__

#include "ln882h.h"

//reg_cache_en
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       cache_en :  1; /* 0: 0,           cache funtion enable*/
        uint32_t                     reserved_0 : 30; /*30: 1,                       RESERVED*/
        uint32_t                       cache_cs :  1; /*31:31, cache state. 0-----idle    1----filling*/
    } bit_field;
} T_CACHE_REG_CACHE_EN;

//reg_flash_base_addr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                flash_base_addr : 32; /*31: 0,                flash_base_addr*/
    } bit_field;
} T_CACHE_REG_FLASH_BASE_ADDR;

//reg_qspi_dr_addr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   qspi_dr_addr : 32; /*31: 0,       qspi DR register address*/
    } bit_field;
} T_CACHE_REG_QSPI_DR_ADDR;

//reg_flash_cmd
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   flash_rd_cmd :  8; /* 7: 0,         spi flash read command*/
        uint32_t           flash_rd_addr_format :  1; /* 8: 8, 24bit read address format. 0----{8'd0,addr}    1----- {addr,8'd0}*/
        uint32_t           flash_rd_data_format :  1; /* 9: 9, flash output data format.0---little endian.   1---big endian*/
        uint32_t            flash_rd_cmd_format :  1; /*10:10, 8bit read command format. 0---{24'd0,cmd}    1---- {cmd,24'd0}*/
        uint32_t                     reserved_1 :  5; /*15:11,                       RESERVED*/
        uint32_t                     tagram_ema :  3; /*18:16,             tag ram parameter */
        uint32_t                    tagram_emaw :  2; /*20:19,             tag ram parameter */
        uint32_t                     reserved_0 : 11; /*31:21,                       RESERVED*/
    } bit_field;
} T_CACHE_REG_FLASH_CMD;

//reg_flush_addr_l
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   flush_addr_l : 32; /*31: 0,        flush address low range*/
    } bit_field;
} T_CACHE_REG_FLUSH_ADDR_L;

//reg_flush_addr_h
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   flush_addr_h : 32; /*31: 0,         flush address hi range*/
    } bit_field;
} T_CACHE_REG_FLUSH_ADDR_H;

//reg_flush_all
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                      flush_all :  1; /* 0: 0,      flush all data in tag ram*/
        uint32_t                     reserved_0 : 31; /*31: 1,                       RESERVED*/
    } bit_field;
} T_CACHE_REG_FLUSH_ALL;

//reg_flush_en
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       flush_en :  1; /* 0: 0, write 1 to srart flush the range of cache data or all the cache data. when done, clear this bit*/
        uint32_t                     reserved_0 : 31; /*31: 1,                       RESERVED*/
    } bit_field;
} T_CACHE_REG_FLUSH_EN;

//Registers Mapping to the same address

typedef struct
{
    volatile            T_CACHE_REG_CACHE_EN                   reg_cache_en; /*  0x0,    RW, 0x00000000,           cache funtion enable*/
    volatile     T_CACHE_REG_FLASH_BASE_ADDR            reg_flash_base_addr; /*  0x4,    RW, 0x10000000,                  addr Register*/
    volatile        T_CACHE_REG_QSPI_DR_ADDR               reg_qspi_dr_addr; /*  0x8,    RW, 0x40200060,                  addr Register*/
    volatile           T_CACHE_REG_FLASH_CMD                  reg_flash_cmd; /*  0xc,    RW, 0x00020000,         flash command Register*/
    volatile        T_CACHE_REG_FLUSH_ADDR_L               reg_flush_addr_l; /* 0x10,    RW, 0x00000000,                 flush Register*/
    volatile        T_CACHE_REG_FLUSH_ADDR_H               reg_flush_addr_h; /* 0x14,    RW, 0x00000000,                 flush Register*/
    volatile           T_CACHE_REG_FLUSH_ALL                  reg_flush_all; /* 0x18,    RW, 0x00000000,                 flush Register*/
    volatile            T_CACHE_REG_FLUSH_EN                   reg_flush_en; /* 0x1c,    RW, 0x00000000,                 flush Register*/
} T_HWP_CACHE_T;

#define hwp_cache ((T_HWP_CACHE_T*)CACHE_BASE)


__STATIC_INLINE uint32_t cache_reg_cache_en_get(void)
{
    return hwp_cache->reg_cache_en.val;
}

__STATIC_INLINE void cache_reg_cache_en_set(uint32_t value)
{
    hwp_cache->reg_cache_en.val = value;
}

__STATIC_INLINE void cache_reg_cache_en_pack(uint8_t cache_en)
{
    hwp_cache->reg_cache_en.val = (((uint32_t)cache_en << 0));
}

__STATIC_INLINE void cache_reg_cache_en_unpack(uint8_t* cache_cs, uint8_t* cache_en)
{
    T_CACHE_REG_CACHE_EN localVal = hwp_cache->reg_cache_en;

    *cache_cs = localVal.bit_field.cache_cs;
    *cache_en = localVal.bit_field.cache_en;
}

__STATIC_INLINE uint8_t cache_cache_cs_getf(void)
{
    return hwp_cache->reg_cache_en.bit_field.cache_cs;
}

__STATIC_INLINE uint8_t cache_cache_en_getf(void)
{
    return hwp_cache->reg_cache_en.bit_field.cache_en;
}

__STATIC_INLINE void cache_cache_en_setf(uint8_t cache_en)
{
    hwp_cache->reg_cache_en.bit_field.cache_en = cache_en;
}

__STATIC_INLINE uint32_t cache_reg_flash_base_addr_get(void)
{
    return hwp_cache->reg_flash_base_addr.val;
}

__STATIC_INLINE void cache_reg_flash_base_addr_set(uint32_t value)
{
    hwp_cache->reg_flash_base_addr.val = value;
}

__STATIC_INLINE void cache_reg_flash_base_addr_pack(uint32_t flash_base_addr)
{
    hwp_cache->reg_flash_base_addr.val = (((uint32_t)flash_base_addr << 0));
}

__STATIC_INLINE void cache_reg_flash_base_addr_unpack(uint32_t* flash_base_addr)
{
    T_CACHE_REG_FLASH_BASE_ADDR localVal = hwp_cache->reg_flash_base_addr;

    *flash_base_addr = localVal.bit_field.flash_base_addr;
}

__STATIC_INLINE uint32_t cache_flash_base_addr_getf(void)
{
    return hwp_cache->reg_flash_base_addr.bit_field.flash_base_addr;
}

__STATIC_INLINE void cache_flash_base_addr_setf(uint32_t flash_base_addr)
{
    hwp_cache->reg_flash_base_addr.bit_field.flash_base_addr = flash_base_addr;
}

__STATIC_INLINE uint32_t cache_reg_qspi_dr_addr_get(void)
{
    return hwp_cache->reg_qspi_dr_addr.val;
}

__STATIC_INLINE void cache_reg_qspi_dr_addr_set(uint32_t value)
{
    hwp_cache->reg_qspi_dr_addr.val = value;
}

__STATIC_INLINE void cache_reg_qspi_dr_addr_pack(uint32_t qspi_dr_addr)
{
    hwp_cache->reg_qspi_dr_addr.val = (((uint32_t)qspi_dr_addr << 0));
}

__STATIC_INLINE void cache_reg_qspi_dr_addr_unpack(uint32_t* qspi_dr_addr)
{
    T_CACHE_REG_QSPI_DR_ADDR localVal = hwp_cache->reg_qspi_dr_addr;

    *qspi_dr_addr = localVal.bit_field.qspi_dr_addr;
}

__STATIC_INLINE uint32_t cache_qspi_dr_addr_getf(void)
{
    return hwp_cache->reg_qspi_dr_addr.bit_field.qspi_dr_addr;
}

__STATIC_INLINE void cache_qspi_dr_addr_setf(uint32_t qspi_dr_addr)
{
    hwp_cache->reg_qspi_dr_addr.bit_field.qspi_dr_addr = qspi_dr_addr;
}

__STATIC_INLINE uint32_t cache_reg_flash_cmd_get(void)
{
    return hwp_cache->reg_flash_cmd.val;
}

__STATIC_INLINE void cache_reg_flash_cmd_set(uint32_t value)
{
    hwp_cache->reg_flash_cmd.val = value;
}

__STATIC_INLINE void cache_reg_flash_cmd_pack(uint8_t tagram_emaw, uint8_t tagram_ema, uint8_t flash_rd_cmd_format, uint8_t flash_rd_data_format, uint8_t flash_rd_addr_format, uint8_t flash_rd_cmd)
{
    hwp_cache->reg_flash_cmd.val = (((uint32_t)tagram_emaw << 19) | ((uint32_t)tagram_ema << 16) | ((uint32_t)flash_rd_cmd_format << 10) | ((uint32_t)flash_rd_data_format << 9) | ((uint32_t)flash_rd_addr_format << 8) | ((uint32_t)flash_rd_cmd << 0));
}

__STATIC_INLINE void cache_reg_flash_cmd_unpack(uint8_t* tagram_emaw, uint8_t* tagram_ema, uint8_t* flash_rd_cmd_format, uint8_t* flash_rd_data_format, uint8_t* flash_rd_addr_format, uint8_t* flash_rd_cmd)
{
    T_CACHE_REG_FLASH_CMD localVal = hwp_cache->reg_flash_cmd;

    *tagram_emaw = localVal.bit_field.tagram_emaw;
    *tagram_ema = localVal.bit_field.tagram_ema;
    *flash_rd_cmd_format = localVal.bit_field.flash_rd_cmd_format;
    *flash_rd_data_format = localVal.bit_field.flash_rd_data_format;
    *flash_rd_addr_format = localVal.bit_field.flash_rd_addr_format;
    *flash_rd_cmd = localVal.bit_field.flash_rd_cmd;
}

__STATIC_INLINE uint8_t cache_tagram_emaw_getf(void)
{
    return hwp_cache->reg_flash_cmd.bit_field.tagram_emaw;
}

__STATIC_INLINE void cache_tagram_emaw_setf(uint8_t tagram_emaw)
{
    hwp_cache->reg_flash_cmd.bit_field.tagram_emaw = tagram_emaw;
}

__STATIC_INLINE uint8_t cache_tagram_ema_getf(void)
{
    return hwp_cache->reg_flash_cmd.bit_field.tagram_ema;
}

__STATIC_INLINE void cache_tagram_ema_setf(uint8_t tagram_ema)
{
    hwp_cache->reg_flash_cmd.bit_field.tagram_ema = tagram_ema;
}

__STATIC_INLINE uint8_t cache_flash_rd_cmd_format_getf(void)
{
    return hwp_cache->reg_flash_cmd.bit_field.flash_rd_cmd_format;
}

__STATIC_INLINE void cache_flash_rd_cmd_format_setf(uint8_t flash_rd_cmd_format)
{
    hwp_cache->reg_flash_cmd.bit_field.flash_rd_cmd_format = flash_rd_cmd_format;
}

__STATIC_INLINE uint8_t cache_flash_rd_data_format_getf(void)
{
    return hwp_cache->reg_flash_cmd.bit_field.flash_rd_data_format;
}

__STATIC_INLINE void cache_flash_rd_data_format_setf(uint8_t flash_rd_data_format)
{
    hwp_cache->reg_flash_cmd.bit_field.flash_rd_data_format = flash_rd_data_format;
}

__STATIC_INLINE uint8_t cache_flash_rd_addr_format_getf(void)
{
    return hwp_cache->reg_flash_cmd.bit_field.flash_rd_addr_format;
}

__STATIC_INLINE void cache_flash_rd_addr_format_setf(uint8_t flash_rd_addr_format)
{
    hwp_cache->reg_flash_cmd.bit_field.flash_rd_addr_format = flash_rd_addr_format;
}

__STATIC_INLINE uint8_t cache_flash_rd_cmd_getf(void)
{
    return hwp_cache->reg_flash_cmd.bit_field.flash_rd_cmd;
}

__STATIC_INLINE void cache_flash_rd_cmd_setf(uint8_t flash_rd_cmd)
{
    hwp_cache->reg_flash_cmd.bit_field.flash_rd_cmd = flash_rd_cmd;
}

__STATIC_INLINE uint32_t cache_reg_flush_addr_l_get(void)
{
    return hwp_cache->reg_flush_addr_l.val;
}

__STATIC_INLINE void cache_reg_flush_addr_l_set(uint32_t value)
{
    hwp_cache->reg_flush_addr_l.val = value;
}

__STATIC_INLINE void cache_reg_flush_addr_l_pack(uint32_t flush_addr_l)
{
    hwp_cache->reg_flush_addr_l.val = (((uint32_t)flush_addr_l << 0));
}

__STATIC_INLINE void cache_reg_flush_addr_l_unpack(uint32_t* flush_addr_l)
{
    T_CACHE_REG_FLUSH_ADDR_L localVal = hwp_cache->reg_flush_addr_l;

    *flush_addr_l = localVal.bit_field.flush_addr_l;
}

__STATIC_INLINE uint32_t cache_flush_addr_l_getf(void)
{
    return hwp_cache->reg_flush_addr_l.bit_field.flush_addr_l;
}

__STATIC_INLINE void cache_flush_addr_l_setf(uint32_t flush_addr_l)
{
    hwp_cache->reg_flush_addr_l.bit_field.flush_addr_l = flush_addr_l;
}

__STATIC_INLINE uint32_t cache_reg_flush_addr_h_get(void)
{
    return hwp_cache->reg_flush_addr_h.val;
}

__STATIC_INLINE void cache_reg_flush_addr_h_set(uint32_t value)
{
    hwp_cache->reg_flush_addr_h.val = value;
}

__STATIC_INLINE void cache_reg_flush_addr_h_pack(uint32_t flush_addr_h)
{
    hwp_cache->reg_flush_addr_h.val = (((uint32_t)flush_addr_h << 0));
}

__STATIC_INLINE void cache_reg_flush_addr_h_unpack(uint32_t* flush_addr_h)
{
    T_CACHE_REG_FLUSH_ADDR_H localVal = hwp_cache->reg_flush_addr_h;

    *flush_addr_h = localVal.bit_field.flush_addr_h;
}

__STATIC_INLINE uint32_t cache_flush_addr_h_getf(void)
{
    return hwp_cache->reg_flush_addr_h.bit_field.flush_addr_h;
}

__STATIC_INLINE void cache_flush_addr_h_setf(uint32_t flush_addr_h)
{
    hwp_cache->reg_flush_addr_h.bit_field.flush_addr_h = flush_addr_h;
}

__STATIC_INLINE uint32_t cache_reg_flush_all_get(void)
{
    return hwp_cache->reg_flush_all.val;
}

__STATIC_INLINE void cache_reg_flush_all_set(uint32_t value)
{
    hwp_cache->reg_flush_all.val = value;
}

__STATIC_INLINE void cache_reg_flush_all_pack(uint8_t flush_all)
{
    hwp_cache->reg_flush_all.val = (((uint32_t)flush_all << 0));
}

__STATIC_INLINE void cache_reg_flush_all_unpack(uint8_t* flush_all)
{
    T_CACHE_REG_FLUSH_ALL localVal = hwp_cache->reg_flush_all;

    *flush_all = localVal.bit_field.flush_all;
}

__STATIC_INLINE uint8_t cache_flush_all_getf(void)
{
    return hwp_cache->reg_flush_all.bit_field.flush_all;
}

__STATIC_INLINE void cache_flush_all_setf(uint8_t flush_all)
{
    hwp_cache->reg_flush_all.bit_field.flush_all = flush_all;
}

__STATIC_INLINE uint32_t cache_reg_flush_en_get(void)
{
    return hwp_cache->reg_flush_en.val;
}

__STATIC_INLINE void cache_reg_flush_en_set(uint32_t value)
{
    hwp_cache->reg_flush_en.val = value;
}

__STATIC_INLINE void cache_reg_flush_en_pack(uint8_t flush_en)
{
    hwp_cache->reg_flush_en.val = (((uint32_t)flush_en << 0));
}

__STATIC_INLINE void cache_reg_flush_en_unpack(uint8_t* flush_en)
{
    T_CACHE_REG_FLUSH_EN localVal = hwp_cache->reg_flush_en;

    *flush_en = localVal.bit_field.flush_en;
}

__STATIC_INLINE uint8_t cache_flush_en_getf(void)
{
    return hwp_cache->reg_flush_en.bit_field.flush_en;
}

__STATIC_INLINE void cache_flush_en_setf(uint8_t flush_en)
{
    hwp_cache->reg_flush_en.bit_field.flush_en = flush_en;
}
#endif
