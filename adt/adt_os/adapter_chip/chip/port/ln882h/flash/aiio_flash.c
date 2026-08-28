/** @brief      Flash application interface.
 *
 *  @file       aiio_flash.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       SPI application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/04/25      <td>1.0.0       <td>linzy        <td>flash init
 *  </table>
 *
 */
#include "aiio_flash.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include <hal/hal_flash.h>

#include "reg_qspi.h"
#include "reg_cache.h"

#define FALSH_SIZE_MAX		    (4 *1024*1024)
#define FALSH_SIZE_4K           (4 *1024)
#define FALSH_SIZE_BLOCK_32K    (32*1024)
#define CACHE_DISABLE       0
#define FLASH_XIP                             1

#define AIIO_GLOBAL_INT_START()      CRITICAL_SECT_START()
#define AIIO_GLOBAL_INT_RESTORE()    CRITICAL_SECT_END()

typedef enum
{
    FLASH_REG_STATUS_READ_1       = 0x05,  //Read status register1
    FLASH_REG_STATUS_READ_2       = 0x35,  //Read status register2

    FLASH_WRITE_DISABLE_CMD       = 0x04,  //Write Disable
    FLASH_WRITE_ENABLE_CMD        = 0x06,  //Write enable

    FLASH_STANDARD_READ_CMD       = 0x03,  //Read date
    FLASH_SECTOR_ERASE_CMD        = 0x20,  //Sentor Erase 
    FLASH_32KB_BLOCK_ERASE_CMD    = 0x52,  //32K Block Erase
    FLASH_64KB_BLOCK_ERASE_CMD    = 0xd8,  //64K Block Erase
    FLASH_ID_READ_CMD             = 0x9F,  //Read Jedec ID    
    FLASH_DEVICE_ID_READ_CMD      = 0x90,  //Read Manufacture ID/Device ID
    FLASH_READ_UNIQUE_ID_CMD      = 0x4B,//Read Unique ID       

}aiio_flash_cmd_cfg_t;

typedef enum
{
    FLASH_ERASE_SECTOR_4KB,
    FLASH_ERASE_BLOCK_32KB,
    FLASH_ERASE_BLOCK_64KB,
}aiio_flash_erase_type_t;

int32_t aiio_flash_init(void)
{
    // aiio_log_d("Input LN882h aiio_flash_init");
    // hal_flash_init();
    // aiio_log_d("Exit LN882h aiio_flash_init");
    return AIIO_OK;
}

int32_t aiio_flash_deinit(void)
{
    hal_flash_deinit();
    return AIIO_OK;
}

int32_t aiio_flash_write_erase(uint32_t addr, uint8_t *src_data, uint32_t length)
{
    /*Erase Flash before write*/
    // aiio_log_d("Enter LN882h aiio_flash_write_erase");
    aiio_flash_erase(addr ,length);
    if(src_data == NULL)
    {
        return AIIO_ERROR;
    }
    if(aiio_flash_write(addr,src_data,length) !=AIIO_OK)
    {
        aiio_log_e("Write Flash failed!");
        return AIIO_ERROR;
    }
    // aiio_log_d("Exit LN882h aiio_flash_write_erase");
    return AIIO_OK;
}

int32_t aiio_flash_write(uint32_t write_addr, uint8_t *write_src_data, uint32_t write_length)
{
    if(write_src_data == NULL)
    {
        return AIIO_ERROR;
    }

    if(hal_flash_program( write_addr,write_length,write_src_data) !=AIIO_OK)
    {
        aiio_log_e("Write Flash failed!");
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

static uint8_t flash_cache_cache_cs_getf(void)
{
    return hwp_cache->reg_cache_en.bit_field.cache_cs;
}

static void aiio_flash_cache_disable(void)
{
    while( flash_cache_cache_cs_getf() == 1);
    hwp_cache->reg_cache_en.bit_field.cache_en = CACHE_DISABLE;//CACHE_DISABLE
    hwp_qspi->ssienr.bit_field.ssi_en = QSPI_DISABLE;//QSPI_DISABLE_L
}

int32_t aiio_flash_read(uint32_t read_addr, uint8_t *read_dst_data, uint32_t read_length)
{
    uint8_t shift = (read_length < 4) ? read_length : (((uint32_t)read_dst_data & 0x03) ? (4 - ((uint32_t)read_dst_data & 0x03)) : 0);
    uint32_t length_in_words = (read_length - shift) / sizeof(uint32_t);
    uint8_t remainder = (read_length - shift) % sizeof(uint32_t);
    uint8_t read_cmd_buf[4];

    hal_assert( (read_addr+read_length) <= FALSH_SIZE_MAX );
    AIIO_GLOBAL_INT_START();
    aiio_flash_cache_disable();

    if(shift > 0)
    {
        read_cmd_buf[0]= FLASH_STANDARD_READ_CMD;
        read_cmd_buf[1]= (read_addr>>16)&0xff;
        read_cmd_buf[2]= (read_addr>>8)&0xff;
        read_cmd_buf[3]= (read_addr)&0xff;
        hal_qspi_standard_read_byte(read_dst_data, shift, read_cmd_buf, sizeof(read_cmd_buf));
        read_addr += shift;
        read_dst_data += shift;
    }

    if(length_in_words > 0){
        hal_qspi_standard_read_word((uint32_t *)read_dst_data, length_in_words, FLASH_STANDARD_READ_CMD, read_addr);
        read_dst_data += sizeof(uint32_t)*length_in_words;
        read_addr += sizeof(uint32_t)*length_in_words;
    }

    if(remainder)
    {
        read_cmd_buf[0]= FLASH_STANDARD_READ_CMD;//read flash cmd
        read_cmd_buf[1]= (read_addr>>16)&0xff;
        read_cmd_buf[2]= (read_addr>>8)&0xff;
        read_cmd_buf[3]= (read_addr)&0xff;
        hal_qspi_standard_read_byte(read_dst_data, remainder, read_cmd_buf, sizeof(read_cmd_buf));
    }

    flash_cache_init(0);
    AIIO_GLOBAL_INT_RESTORE();

    return AIIO_OK;
}

static void aiio_flash_write_enable(void)
{
    uint8_t write_enable_cmd = 0;
    write_enable_cmd = FLASH_WRITE_ENABLE_CMD;

    hal_qspi_standard_write(&write_enable_cmd, sizeof(write_enable_cmd));
}

static void aiio_flash_operation_wait(void)
{
    uint8_t flash_status;
    uint8_t read_stat_cmd = FLASH_REG_STATUS_READ_1;
    do
    {
        hal_qspi_standard_read_byte(&flash_status, sizeof(flash_status), &read_stat_cmd, sizeof(read_stat_cmd));
    } while( flash_status & 0x1);
}

static uint8_t aiio_flash_erase_block(uint32_t addr, aiio_flash_erase_type_t type)
{
    uint8_t erase_cmd[4];

    erase_cmd[0]= FLASH_SECTOR_ERASE_CMD;//default:SECTOR ERASE
    erase_cmd[1]= (addr >> 16)&0xff;
    erase_cmd[2]= (addr >> 8 )&0xff;
    erase_cmd[3]= (addr)&0xff;
    switch(type)
    {
        case FLASH_ERASE_SECTOR_4KB:
            erase_cmd[0] = FLASH_SECTOR_ERASE_CMD;
            break;
        case FLASH_ERASE_BLOCK_32KB:
            erase_cmd[0] = FLASH_32KB_BLOCK_ERASE_CMD;
            break;
        case FLASH_ERASE_BLOCK_64KB:
            erase_cmd[0] = FLASH_64KB_BLOCK_ERASE_CMD;
            break;
        default:
            break;
    }

    AIIO_GLOBAL_INT_START();
    #if (defined(OS_TICK_COMPENSATE))
    uint32_t last;
    last = ln_os_tick_comp_pre_get_cycles();
    #endif
    aiio_flash_cache_disable();

    aiio_flash_write_enable();
    hal_qspi_standard_write(erase_cmd, sizeof(erase_cmd));
    aiio_flash_operation_wait();

    flash_cache_init(0);
    #if (defined(OS_TICK_COMPENSATE))
    ln_os_tick_comp_val_inc(last);
    #endif
    AIIO_GLOBAL_INT_RESTORE();

    return 0;
}

int32_t aiio_flash_erase(uint32_t erase_addr, uint32_t erase_length)
{
    /*Erase need 4K aligned*/
    hal_assert((erase_addr%FALSH_SIZE_4K == 0));
    hal_assert((erase_length%FALSH_SIZE_4K == 0));

    uint16_t num_32k = 0, num_4k = 0, i;

    uint32_t tmpoffset      = 0;
    uint32_t tailing_length = 0;
    uint8_t  tailing_4k_num = 0;
    uint32_t remainder_len  = 0;

    tmpoffset = erase_addr;
    // aiio_log_d("erase_length : %d\r\n",erase_length);
    if(erase_length >= FALSH_SIZE_BLOCK_32K){
        tailing_length = ((erase_addr&(~(FALSH_SIZE_BLOCK_32K-1))) + FALSH_SIZE_BLOCK_32K) - erase_addr;
        tailing_4k_num = tailing_length / FALSH_SIZE_4K;

        for(i = 0; i < tailing_4k_num; i++){
            aiio_flash_erase_block(tmpoffset, FLASH_ERASE_SECTOR_4KB);
            tmpoffset += FALSH_SIZE_4K;
        }

        remainder_len = erase_length - tailing_length;
        num_32k       = remainder_len / FALSH_SIZE_BLOCK_32K;
        remainder_len = remainder_len % FALSH_SIZE_BLOCK_32K;
        num_4k        = remainder_len / FALSH_SIZE_4K + ((remainder_len % FALSH_SIZE_4K) ? 1:0);

        for(i = 0; i < num_32k; i++){
            aiio_flash_erase_block(tmpoffset, FLASH_ERASE_BLOCK_32KB);
            tmpoffset += FALSH_SIZE_BLOCK_32K;
        }

        for(i = 0; i < num_4k; i++){
            aiio_flash_erase_block(tmpoffset, FLASH_ERASE_SECTOR_4KB);
            tmpoffset += FALSH_SIZE_4K;
        }
    }else{
        num_4k = erase_length / FALSH_SIZE_4K + ((erase_length % FALSH_SIZE_4K) ? 1:0);
        // aiio_log_d("num_4k : %x\r\n",num_4k);
        for(i = 0; i < num_4k; i++){
            aiio_flash_erase_block(tmpoffset, FLASH_ERASE_SECTOR_4KB);
            tmpoffset += FALSH_SIZE_4K;
        }
    }

    return AIIO_OK;
}

int32_t aiio_flash_id_read(void)
{
    uint8_t cmd;
    uint8_t read_back[3];
    uint32_t value = 0;

    cmd = FLASH_ID_READ_CMD;  //cmd
    read_back[0] = 0; // Manufacturer ID
    read_back[1] = 0; // Memory Type (device identification high 8 bit)
    read_back[2] = 0; // Capacity    (device identification low 8 bit)

    AIIO_GLOBAL_INT_START();
    aiio_flash_cache_disable();

    hal_qspi_standard_read_byte(read_back, sizeof(read_back), &cmd, sizeof(cmd));

    flash_cache_init(0);
    AIIO_GLOBAL_INT_RESTORE();

    value = (read_back[0] << 16) | (read_back[1] << 8) | (read_back[2]);
    return value;
}

int32_t aiio_flash_device_id_read(void)
{
    uint8_t cmd_buf[4];
    uint8_t read_back[2]; // Manufacturer ID, Device ID
    uint16_t value =0;

    cmd_buf[0] = FLASH_DEVICE_ID_READ_CMD;
    cmd_buf[1] = 0;//dumy data
    cmd_buf[2] = 0;//dumy data
    cmd_buf[3] = 0;//dumy data

    AIIO_GLOBAL_INT_START();
    aiio_flash_cache_disable();

    hal_qspi_standard_read_byte(read_back, sizeof(read_back), cmd_buf, sizeof(cmd_buf));

    flash_cache_init(0);
    AIIO_GLOBAL_INT_RESTORE();

    value = (read_back[0] << 8) | (read_back[1]);
    return value;
}

int32_t aiio_flash_manufacturer_id_read(void)
{
    uint8_t cmd_buf[4];
    uint8_t read_back[2]; // Manufacturer ID, Device ID
    uint8_t value =0;

    cmd_buf[0] = FLASH_DEVICE_ID_READ_CMD;
    cmd_buf[1] = 0;//dumy data
    cmd_buf[2] = 0;//dumy data
    cmd_buf[3] = 0;//dumy data

    AIIO_GLOBAL_INT_START();
    aiio_flash_cache_disable();

    hal_qspi_standard_read_byte(read_back, sizeof(read_back), cmd_buf, sizeof(cmd_buf));
    
    flash_cache_init(0);
    AIIO_GLOBAL_INT_RESTORE();
    value = read_back[0];
    return value;
}

int32_t aiio_flash_unique_id_read(void)
{
    uint8_t unique_id[2];
    uint8_t cmd_buf[5];
    uint8_t read_back[16]; 

    cmd_buf[0] = FLASH_READ_UNIQUE_ID_CMD;
    cmd_buf[1] = 0;//dumy data
    cmd_buf[2] = 0;//dumy data
    cmd_buf[3] = 0;//dumy data
    cmd_buf[4] = 0;//dumy data

    AIIO_GLOBAL_INT_START();
    aiio_flash_cache_disable();

    hal_qspi_standard_read_byte(read_back, sizeof(read_back), cmd_buf, sizeof(cmd_buf));

    flash_cache_init(0);
    AIIO_GLOBAL_INT_RESTORE();

    memcpy(unique_id,read_back,sizeof(read_back));
    aiio_log_d("   %x   %x  %x \r\n",read_back[0],read_back[1],read_back[2]);
    aiio_log_d("   %x\r\n",unique_id);
    return AIIO_OK;
}

int32_t aiio_flash_status_register1_read(void)
{
    uint8_t cmd = FLASH_REG_STATUS_READ_1;
    uint8_t status = 0;

    aiio_flash_cache_disable();
    hal_qspi_standard_read_byte(&status, sizeof(status), &cmd, sizeof(cmd));
    flash_cache_init(0);
    return status;

}

int32_t aiio_flash_status_register2_read(void)
{
    uint8_t cmd = FLASH_REG_STATUS_READ_2;
    uint8_t status = 0;

    aiio_flash_cache_disable();
    hal_qspi_standard_read_byte( &status, sizeof(status), &cmd, sizeof(cmd));
    flash_cache_init(0);
    return status;
}

int32_t aiio_flash_status_read(void)
{
    uint8_t low = 0, high = 0;
    uint16_t value = 0;
    low  = aiio_flash_status_register1_read();
    high = aiio_flash_status_register2_read();
    value = ( high << 8 ) | (low & 0xFF);
    return value;
}


