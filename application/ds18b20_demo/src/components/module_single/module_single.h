#ifndef __MODULE_SINGLE_H_
#define __MODULE_SINGLE_H_
#include "aiio_adapter_include.h"
#ifdef __MODULE_SINGLE_C_
#define MODULE_SINGLE_EXT
#else
#define MODULE_SINGLE_EXT extern
#endif

#ifndef null
#define null (void *)0
#endif

// 配置
typedef struct
{
    void (*module_single_config_write)(uint8_t status);
    uint8_t (*module_single_config_read)(void);
    void (*module_single_config_delay)(void); // 5us 的延迟函数
} module_single_config_t;

// 数据类型
typedef struct
{
    uint8_t *buf;
    uint16_t buf_len;
} module_single_dat_t;

// 功能
typedef struct module_single_type
{
    module_single_config_t *module_single_config;

    uint8_t (*module_single_reset)(struct module_single_type *this);
    void (*module_single_write)(struct module_single_type *this, module_single_dat_t *module_single_dat);
    void (*module_single_write_byte)(struct module_single_type *this, module_single_dat_t *module_single_dat);
    void (*module_single_read)(struct module_single_type *this, module_single_dat_t *module_single_dat);
    void (*module_single_read_byte)(struct module_single_type *this, module_single_dat_t *module_single_dat);
} module_single_type_t;

MODULE_SINGLE_EXT uint8_t module_single_create(module_single_type_t *this, module_single_config_t *module_single_config);

#endif
