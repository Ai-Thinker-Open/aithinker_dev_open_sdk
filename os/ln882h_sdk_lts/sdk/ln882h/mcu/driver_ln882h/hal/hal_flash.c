/**
 * @file     hal_flash.c
 * @author   BSP Team 
 * @brief    This file provides Flash function.
 * @version  0.0.0.1
 * @date     2021-10-19
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "hal/hal_flash.h"

#if (defined(OS_TICK_COMPENSATE))
#include "utils/runtime/runtime.h"
#endif

#if defined (__CC_ARM)
#pragma push
#pragma O2
#elif defined (__GNUC__)
#pragma GCC push_options
#pragma GCC optimize(2)
#endif


#if FLASH_UPDATE_READ_REG
#include "hal/hal_misc.h"
#include "hal/hal_gpio.h"
static void read_flash_reg(uint8_t *rd_ptr, uint32_t rd_len, uint8_t *wr_ptr, uint8_t wr_len, uint8_t read_r_dummy)
{
    hal_misc_cmp_set_spif_io_en(0);
    
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));
    gpio_init.dir = GPIO_OUTPUT;
    gpio_init.pin = GPIO_PIN_15;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(GPIOA_BASE,&gpio_init);
    hal_gpio_pin_set(GPIOA_BASE,GPIO_PIN_15);
    
    gpio_init.pin = GPIO_PIN_0;
    hal_gpio_init(GPIOB_BASE,&gpio_init);
    hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_0);
    
    gpio_init.pin = GPIO_PIN_2;
    hal_gpio_init(GPIOB_BASE,&gpio_init);
    hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_2);
    
    //START
    hal_gpio_pin_reset(GPIOA_BASE,GPIO_PIN_15);
    
    //WRITE DATA
    for(int i = 0; i < wr_len; i ++){
        for(int x = 0; x < 8; x++){
            if(((wr_ptr[i] >> (7 - x)) & 0x1) == 0x1){
                hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_2);
            }else{
                hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_2);
            }
            hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_0);
            hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_0);
        }
    }

    //1bit dummy
    if(read_r_dummy == 1){
        hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_0);
        hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_0);
    }
    
    //READ DATA
    hal_gpio_pin_direction_set(GPIOB_BASE,GPIO_PIN_2,GPIO_INPUT);
    for(int i = 0; i < rd_len; i ++){
        volatile uint8_t val = 0;
        for(int x = 0; x < 8; x++){
            hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_0);
            if(hal_gpio_pin_read(GPIOB_BASE,GPIO_PIN_2) == 1){
                val |= (1 << (7 - x));
            }
            hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_0);
        }
        rd_ptr[i] = val;
    }
    
    hal_gpio_pin_set(GPIOA_BASE,GPIO_PIN_15);
    
    hal_gpio_pin_direction_set(GPIOA_BASE,GPIO_PIN_15,GPIO_INPUT);
    hal_gpio_pin_direction_set(GPIOB_BASE,GPIO_PIN_0,GPIO_INPUT);
    hal_gpio_pin_direction_set(GPIOB_BASE,GPIO_PIN_2,GPIO_INPUT);
    
    hal_misc_cmp_set_spif_io_en(1);
}
#endif

/**
 * @brief Update the flash.
 * 
 *        The TH25Q-16HB flash needs to be upgraded to work properly. 
 *        And the volatile update should be running every time it's powered on.
 */
static uint8_t hal_flash_update_with_volatile(void)
{
    uint8_t  cmd;
    uint8_t  buf[10] = {0,};

    //1. read flash ID
    cmd = FLASH_READ_ID;
    buf[0] = 0; // Manufacturer ID
    buf[1] = 0; // Memory Type (device identification high 8 bit)
    buf[2] = 0; // Capacity    (device identification low 8 bit)
    hal_qspi_standard_read_byte(buf, 3, &cmd, sizeof(cmd));
    if(buf[0] != 0xEB){
        return 0;
    }

    //2-4. write cmd.
    cmd = 0x33;
    hal_qspi_standard_write(&cmd,1);
    cmd = 0xCC;
    hal_qspi_standard_write(&cmd,1);
    cmd = 0xAA;
    hal_qspi_standard_write(&cmd,1);

    //If the flash has upgraded,return 1.
    //write R reg.
    buf[0] = 0xFA;  //CMD
    buf[1] = 0x01;  //ADDR
    buf[2] = 0x20;  //ADDR
    buf[3] = 0x0D;  //ADDR
    buf[4] = 0x03;  //ADDR
    hal_qspi_standard_write(buf,5);
    
#if (FLASH_UPDATE_READ_REG)
    static uint8_t read_r_dummy = 0;
    //read R reg.
    buf[0] = 0xFA;
    buf[1] = 0x02;  //ADDR
    buf[2] = 0x20;  //ADDR
    buf[3] = 0x0D;  //ADDR
    buf[4] = 0x00;  //DATA
    read_r_dummy = 1;
    read_flash_reg(buf+4, 1, buf, 4, read_r_dummy);
    read_r_dummy = 0;
    //read back 0x03
#endif

    buf[0] = 0x48;  //CMD
    buf[1] = 0x00;  //ADDR
    buf[2] = 0x05;  //ADDR
    buf[3] = 0x0D;  //ADDR
    buf[4] = 0x00;  //DUMMY
    hal_qspi_standard_read_byte(buf+5, 1, buf, 5);
    
    if((buf[5] & 0x7F) == 0x7C){
        cmd = 0x55;
        hal_qspi_standard_write(&cmd,1);
        cmd = 0x88;
        hal_qspi_standard_write(&cmd,1);
        return 1;
    }

    //5. write enable
    hal_flash_write_enable();
    
    //6. write data
    buf[0] = 0x42;  //CMD
    buf[1] = 0x00;  //ADDR
    buf[2] = 0x01;  //ADDR
    buf[3] = 0xFE;  //ADDR
    buf[4] = 0x00;  //DATA
    buf[5] = 0x00;  //DATA
    hal_qspi_standard_write(buf,6);
    hal_flash_operation_wait();

    //7. write C reg.
    buf[0] = 0xFA;  //CMD
    buf[1] = 0x03;  //ADDR
    buf[2] = 0x04;  //ADDR
    buf[3] = 0x00;  //ADDR
    buf[4] = 0xFC;  //DATA
    buf[5] = 0xFA;  //DATA
    buf[6] = 0x00;  //DUMMY
    buf[7] = 0x00;  //DUMMY
    hal_qspi_standard_write(buf,8);

    //8. read C reg.
#if FLASH_UPDATE_READ_REG
    buf[0] = 0xFA;
    buf[1] = 0x04;  //ADDR
    buf[2] = 0x04;  //ADDR
    buf[3] = 0x00;  //ADDR
    buf[4] = 0x00;  //DATA
    buf[5] = 0x00;  //DATA
    read_flash_reg(buf+4, 2, buf, 4, read_r_dummy);
    //read back 0x0305
#endif

    //9. write C reg.
    buf[0] = 0xFA;  //CMD
    buf[1] = 0x03;  //ADDR
    buf[2] = 0x02;  //ADDR
    buf[3] = 0x00;  //ADDR
    buf[4] = 0x79;  //DATA
    buf[5] = 0xF3;  //DATA
    buf[6] = 0x00;  //DUMMY
    buf[7] = 0x00;  //DUMMY
    hal_qspi_standard_write(buf,8);

    //10. read C reg.
#if FLASH_UPDATE_READ_REG
    buf[0] = 0xFA;
    buf[1] = 0x04;  //ADDR
    buf[2] = 0x02;  //ADDR
    buf[3] = 0x00;  //ADDR
    buf[4] = 0x00;  //DATA
    buf[5] = 0x00;  //DATA
    read_flash_reg(buf+4, 2, buf, 4, read_r_dummy);
    //read back 0x860C
#endif

    //11-12. write cmd
    cmd = 0x55;
    hal_qspi_standard_write(&cmd,1);
    cmd = 0x88;
    hal_qspi_standard_write(&cmd,1);

    return 1;
}


/**
 * @brief Init FLASH.
 * @note Warning!!! If running on flash, do not init flash!
 */
void hal_flash_init(void)
{
    // "Do not call this funcation in your flash code,the funcation can run only in the ramcode."
    /*
        SSI Clock Divider. The LSB for this field is always set to 0 and is unaffected by a write
        operation, which ensures an even value is held in this register. If the value is 0, the
        serial output clock (sclk_out) is disabled. The frequency of the sclk_out is derived
        from the following equation:

        sclk_out = Fssi_clk/SCKDV
        where SCKDV is any even value between 2 and 65534. 
        For example:
            for Fssi_clk = 3.6864MHz and SCKDV =2
            Fsclk_out = 3.6864/2 = 1.8432MHz
        Reset Value: 0x0
    */
    
    /* QSPI_CLK  <= AHB_CLOCK / 2 */ 
    /* The first parameter is the frequency division value, and the second parameter is rx sample delay!*/
    hal_qspi_init(2, 1);
    
    hal_flash_update_with_volatile();
}

/**
 * @brief Deinit FLASH.
 * @note Warning!!! If running on flash, do not deinit flash!
 */
void hal_flash_deinit(void)
{
    hal_qspi_deinit();
}

/**
 * @brief Read a flash section.
 *
 * This function is used to read a part of the flash memory.
 *
 * @param offset Starting offset from the beginning of the flash device.
 * @param length Size of the portion of flash to read.
 * @param buffer Pointer on data to read.
 * @return uint8_t 0 if operation can start successfully.
 */
uint8_t hal_flash_read(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    uint8_t shift = (length < 4) ? length : (((uint32_t)buffer & 0x03) ? (4 - ((uint32_t)buffer & 0x03)) : 0);
    uint32_t length_in_words = (length - shift) / sizeof(uint32_t);
    uint8_t remainder = (length - shift) % sizeof(uint32_t);
    uint8_t read_cmd_buf[4];

    hal_assert( (offset+length) <= FALSH_SIZE_MAX );
#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    if(shift > 0)
    {
        read_cmd_buf[0]= FLASH_STANDARD_READ;
        read_cmd_buf[1]= (offset>>16)&0xff;
        read_cmd_buf[2]= (offset>>8)&0xff;
        read_cmd_buf[3]= (offset)&0xff;
        hal_qspi_standard_read_byte(buffer, shift, read_cmd_buf, sizeof(read_cmd_buf));
        offset += shift;
        buffer += shift;
    }

    if(length_in_words > 0){
        hal_qspi_standard_read_word((uint32_t *)buffer, length_in_words, FLASH_STANDARD_READ, offset);
        buffer += sizeof(uint32_t)*length_in_words;
        offset += sizeof(uint32_t)*length_in_words;
    }

    if(remainder)
    {
        read_cmd_buf[0]= FLASH_STANDARD_READ;
        read_cmd_buf[1]= (offset>>16)&0xff;
        read_cmd_buf[2]= (offset>>8)&0xff;
        read_cmd_buf[3]= (offset)&0xff;
        hal_qspi_standard_read_byte(buffer, remainder, read_cmd_buf, sizeof(read_cmd_buf));
    }

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif
    return 0;
}

uint8_t hal_flash_read_by_cache(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    uint32_t addr = 0;
    uint32_t i = 0;
    hal_assert( (offset+length) <= FALSH_SIZE_MAX );

    addr = CACHE_FLASH_BASE + offset;
    for(i = 0; i < length; i++){
        *buffer++ = *(uint8_t *)(addr++);
    }
    return 0;
}


/**
 * @brief Flash Chip Erase
 */
void hal_flash_chip_erase(void)
{
    uint8_t chip_erase_cmd = FLASH_CHIP_ERASE;

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    #if (defined(OS_TICK_COMPENSATE))
    uint32_t last;
    last = ln_os_tick_comp_pre_get_cycles();
    #endif
    flash_cache_disable();
#endif

    hal_flash_write_enable();
    hal_qspi_standard_write(&chip_erase_cmd, sizeof(chip_erase_cmd));
    hal_flash_operation_wait();

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    #if (defined(OS_TICK_COMPENSATE))
    ln_os_tick_comp_val_inc(last);
    #endif
    GLOBAL_INT_RESTORE();
#endif
}

static uint8_t hal_flash_erase_block(uint32_t offset, flash_erase_type_t type)
{
    uint8_t erase_cmd_addr[4];

    erase_cmd_addr[0]= FLASH_SECTOR_ERASE;//default:SECTOR ERASE
    erase_cmd_addr[1]= (offset >> 16)&0xff;
    erase_cmd_addr[2]= (offset >> 8 )&0xff;
    erase_cmd_addr[3]= (offset      )&0xff;
    switch(type)
    {
        case ERASE_SECTOR_4KB:
            erase_cmd_addr[0] = FLASH_SECTOR_ERASE;
            break;
        case ERASE_BLOCK_32KB:
            erase_cmd_addr[0] = FLASH_32KB_BLOCK_ERASE;
            break;
        case ERASE_BLOCK_64KB:
            erase_cmd_addr[0] = FLASH_64KB_BLOCK_ERASE;
            break;
        default:
            break;
    }

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    #if (defined(OS_TICK_COMPENSATE))
    uint32_t last;
    last = ln_os_tick_comp_pre_get_cycles();
    #endif
    flash_cache_disable();
#endif

    hal_flash_write_enable();
    hal_qspi_standard_write(erase_cmd_addr, sizeof(erase_cmd_addr));
    hal_flash_operation_wait();

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    #if (defined(OS_TICK_COMPENSATE))
    ln_os_tick_comp_val_inc(last);
    #endif
    GLOBAL_INT_RESTORE();
#endif

    return 0;
}

/**
 * @brief Erase a portion of flash, starts from offset.
 *
 * @param offset(Note:4K Aligned)
 * @param length(Note:4K Aligned)
 */
void hal_flash_erase(uint32_t offset, uint32_t length)
{
    hal_assert((offset%FALSH_SIZE_4K == 0));
    hal_assert((length%FALSH_SIZE_4K == 0));

    uint16_t num_32k = 0, num_4k = 0, i;

    uint32_t tmpoffset      = 0;
    uint32_t tailing_length = 0;
    uint8_t  tailing_4k_num = 0;
    uint32_t remainder_len  = 0;

    tmpoffset = offset;
    if(length >= FALSH_SIZE_BLOCK_32K){
        tailing_length = ((offset&(~(FALSH_SIZE_BLOCK_32K-1))) + FALSH_SIZE_BLOCK_32K) - offset;
        tailing_4k_num = tailing_length / FALSH_SIZE_4K;

        for(i = 0; i < tailing_4k_num; i++){
            hal_flash_erase_block(tmpoffset, ERASE_SECTOR_4KB);
            tmpoffset += FALSH_SIZE_4K;
        }

        remainder_len = length - tailing_length;
        num_32k       = remainder_len / FALSH_SIZE_BLOCK_32K;
        remainder_len = remainder_len % FALSH_SIZE_BLOCK_32K;
        num_4k        = remainder_len / FALSH_SIZE_4K + ((remainder_len % FALSH_SIZE_4K) ? 1:0);

        for(i = 0; i < num_32k; i++){
            hal_flash_erase_block(tmpoffset, ERASE_BLOCK_32KB);
            tmpoffset += FALSH_SIZE_BLOCK_32K;
        }

        for(i = 0; i < num_4k; i++){
            hal_flash_erase_block(tmpoffset, ERASE_SECTOR_4KB);
            tmpoffset += FALSH_SIZE_4K;
        }
    }else{
        num_4k        = length / FALSH_SIZE_4K + ((length % FALSH_SIZE_4K) ? 1:0);
        for(i = 0; i < num_4k; i++){
            hal_flash_erase_block(tmpoffset, ERASE_SECTOR_4KB);
            tmpoffset += FALSH_SIZE_4K;
        }
    }
}

static void hal_flash_page_program_general(flash_area_t type, uint32_t offset, uint32_t length, uint8_t *buffer)
{
    hal_assert(length <= FLASH_PAGE_SIZE);

    uint8_t page_program_buf[4 + FLASH_PAGE_SIZE];

    if(SECURITY_AREA == type){
        page_program_buf[0]= FLASH_SECURITY_PROGRAM;
    } else {
        page_program_buf[0]= FLASH_PAGE_PROGRAM;
    }
    page_program_buf[1]= (offset >> 16)&0xff;
    page_program_buf[2]= (offset >> 8 )&0xff;
    page_program_buf[3]= (offset      )&0xff;

    memcpy(&page_program_buf[4],buffer, length);

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    #if (defined(OS_TICK_COMPENSATE))
    uint32_t last;
    last = ln_os_tick_comp_pre_get_cycles();
    #endif
    flash_cache_disable();
#endif

    hal_flash_write_enable();
    hal_qspi_standard_write(page_program_buf, length+4);
    hal_flash_operation_wait();

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    #if (defined(OS_TICK_COMPENSATE))
    ln_os_tick_comp_val_inc(last);
    #endif
    GLOBAL_INT_RESTORE();
#endif
}

static uint8_t hal_flash_program_general(flash_area_t type, uint32_t offset, uint32_t length, uint8_t *buffer)
{
    hal_assert(offset < FALSH_SIZE_MAX);

    uint32_t page_base = offset&(~(FLASH_PAGE_SIZE-1));
    uint16_t cycles = 0;
    uint16_t remainder = 0;
    uint16_t i;

    if(page_base != offset){
        uint32_t tailing_length = (page_base + FLASH_PAGE_SIZE) - offset;
        uint32_t prog_length = (length < tailing_length) ? length : tailing_length;
        hal_flash_page_program_general(type, offset, prog_length, buffer);
        buffer += prog_length;
        offset += prog_length;
        length -= prog_length;
    }

    cycles = length / FLASH_PAGE_SIZE;
    for(i = 0; i < cycles; i++){
        hal_flash_page_program_general(type, offset, FLASH_PAGE_SIZE, buffer);
        offset += FLASH_PAGE_SIZE;
        buffer += FLASH_PAGE_SIZE;
    }

    remainder = length % FLASH_PAGE_SIZE;
    if(remainder) {
        hal_flash_page_program_general(type, offset, remainder, buffer);
    }

    return 0;
}

uint8_t hal_flash_program(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    return hal_flash_program_general(NORMAL_AREA, offset, length, buffer);
}


/**
 * @brief The Read Identification (RDID) command provides read access to
 * manufacturer identification, device identification, and Common Flash Interface
 * (CFI) information.The device identification indicates the memory type in the first byte,
 *and the memory capacity of the device in the second  byte.
 * @return uint32_t a value Manufacturer ID Device identification(Memory Type,Capacity)
 */
uint32_t hal_flash_read_id(void)
{
    uint8_t cmd;
    uint8_t read_back[3];
    uint32_t value = 0;

    cmd = FLASH_READ_ID;
    read_back[0] = 0; // Manufacturer ID
    read_back[1] = 0; // Memory Type (device identification high 8 bit)
    read_back[2] = 0; // Capacity    (device identification low 8 bit)

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_read_byte(read_back, sizeof(read_back), &cmd, sizeof(cmd));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif

    value = (read_back[0] << 16) | (read_back[1] << 8) | (read_back[2]);
    return value;
}

/**
 * @brief The READ_ID command identifies the Device Manufacturer ID and the
 * Device ID. The command is also referred to as Read Electronic Manufacturer
 * and device Signature (REMS).
 *
 * @return uint32_t a value contains Manufacturer ID and Device ID.
 */
uint16_t hal_flash_read_device_id(void)
{
    uint8_t cmd_buf[4];
    uint8_t read_back[2]; // Manufacturer ID, Device ID
    uint16_t value = 0;

    cmd_buf[0] = FLASH_READ_DEVICE_ID;
    cmd_buf[1] = 0;//dumy data
    cmd_buf[2] = 0;//dumy data
    cmd_buf[3] = 0;//dumy data

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_read_byte(read_back, sizeof(read_back), cmd_buf, sizeof(cmd_buf));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif

    value = (read_back[0] << 8) | (read_back[1]);
    return value;
}

/**
 * @brief The Read Status Register-1 (RDSR1) command allows the Status Register-1
 * contents to be read from SO. The Status Register-1 contents may be read at any
 * time, even while a program, erase, or write operation is in progress.
 *
 * @return uint8_t status value
 */
uint8_t hal_flash_read_sr1(void)
{
    uint8_t cmd = FLASH_READ_STATUS_REG_1;
    uint8_t status = 0;

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_read_byte(&status, sizeof(status), &cmd, sizeof(cmd));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif

    return status;
}

/**
 * @brief The Read Status Register-2 (RDSR2) command allows the Status Register-2
 * contents to be read from SO. The Status Register-2 contents may be read at any
 * time, even while a program, erase, or write operation is in progress.
 *
 * @return uint8_t status value
 */
uint8_t hal_flash_read_sr2(void)
{
    uint8_t cmd = FLASH_READ_STATUS_REG_2;
    uint8_t status = 0;

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_read_byte( &status, sizeof(status), &cmd, sizeof(cmd));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif

    return status;
}

/**
 * @brief Read FLASH status 16bits.
 * @return uint32_t status 16bits.
 */
uint16_t hal_flash_read_status(void)
{
    uint8_t low = 0, high = 0;
    uint16_t value = 0;

    low  = hal_flash_read_sr1();
    high = hal_flash_read_sr2();

    value = ( high << 8 ) | (low & 0xFF);
    return value;
}

/**
 * @brief Read FLASH Unique ID 128bits.
 */
void hal_flash_read_unique_id(uint8_t *unique_id)
{
    uint8_t cmd_buf[5];
    uint8_t read_back[16]; 

    cmd_buf[0] = FLASH_READ_UNIQUE_ID;
    cmd_buf[1] = 0;//dumy data
    cmd_buf[2] = 0;//dumy data
    cmd_buf[3] = 0;//dumy data
    cmd_buf[4] = 0;//dumy data

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_read_byte(read_back, sizeof(read_back), cmd_buf, sizeof(cmd_buf));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif

    memcpy(unique_id,read_back,sizeof(read_back));
}


/**
 * @brief Send command Program/Erase Suspend
 */
void hal_flash_program_erase_suspend(void)
{
    uint8_t cmd = FLASH_SUSPEND;
#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_write(&cmd, sizeof(cmd));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif
}

/**
 * @brief Send command Program/Erase Resume
 */
void hal_flash_program_erase_resume(void)
{
    uint8_t cmd = FLASH_RESUME;
#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_write(&cmd, sizeof(cmd));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif
}

/**
 * @brief Flash Write Enable
 */
void hal_flash_write_enable(void)
{
    uint8_t cmd = 0;
    cmd = FLASH_WRITE_ENABLE;

    hal_qspi_standard_write(&cmd, sizeof(cmd));
}

/**
 * @brief Flash Write Disable
 */
void hal_flash_write_disable(void)
{
    uint8_t cmd = 0;
    cmd = FLASH_WRITE_DISABLE;

    hal_qspi_standard_write(&cmd, sizeof(cmd));
}

/**
 * @brief Flash One Time Program(OTP)
 * @note Warning!!! There is only one chance to lock the OTP!
 */
void hal_flash_lock_otp(void)
{
    uint8_t read_sr1_cmd = FLASH_READ_STATUS_REG_1;
    uint8_t read_sr2_cmd = FLASH_READ_STATUS_REG_2;
    uint8_t write_sr_cmd[3];
    flash_status_reg1_t status1;
    flash_status_reg2_t status2;

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_read_byte(&status1.reg1_data, sizeof(uint8_t), &read_sr1_cmd, sizeof(read_sr1_cmd));
    hal_qspi_standard_read_byte(&status2.reg2_data, sizeof(uint8_t), &read_sr2_cmd, sizeof(read_sr2_cmd));

    status2.bits.LB = 1;

    write_sr_cmd[0] = FLASH_WRITE_NON_VOLATILE_SR;
    write_sr_cmd[1] = status1.reg1_data;
    write_sr_cmd[2] = status2.reg2_data;

    hal_flash_write_enable();
    hal_qspi_standard_write(write_sr_cmd, sizeof(write_sr_cmd));
    hal_flash_operation_wait();

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif
}

/**
 * @brief Get Flash OTP State
 * @return 1:locked  0:unlock
 */
uint8_t hal_flash_get_otp_state(void)
{
    uint8_t read_sr2_cmd = FLASH_READ_STATUS_REG_2;
    flash_status_reg2_t status2;

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_qspi_standard_read_byte( &status2.reg2_data, sizeof(uint8_t), &read_sr2_cmd, sizeof(read_sr2_cmd));

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif

    return (uint8_t)status2.bits.LB;
}

/**
 * @brief Flash Erase Security Area
 * @note Warning!!!(If the OTP Lock Bit is already set to 1, erase operation does not take effect.)
 */
void hal_flash_security_area_erase(uint32_t offset)
{
    uint8_t erase_cmd_addr[4];
    hal_assert(offset <= FLASH_SECURITY_SIZE_MAX);

    erase_cmd_addr[0]= FLASH_SECURITY_ERASE;
    erase_cmd_addr[1]= (offset >> 16)&0xff;
    erase_cmd_addr[2]= (offset >> 8 )&0xff;
    erase_cmd_addr[3]= (offset      )&0xff;

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    hal_flash_write_enable();
    hal_qspi_standard_write(erase_cmd_addr, sizeof(erase_cmd_addr));
    hal_flash_operation_wait();

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif
}

/**
 * @brief Flash Program Security Area
 * @note Warning!!!(If the OTP Lock Bit is already set to 1, program operation does not take effect.)
 */
uint8_t hal_flash_security_area_program(uint32_t offset, uint32_t len, uint8_t * buf)
{
    hal_assert(offset <= FLASH_SECURITY_SIZE_MAX);
    hal_assert((offset+len) <= FLASH_SECURITY_SIZE_MAX);

    return hal_flash_program_general(SECURITY_AREA, offset, len, buf);
}


/**
 * @brief Flash Read Security Area
 */
void hal_flash_security_area_read(uint32_t offset, uint32_t len, uint8_t * buf)
{
    hal_assert((offset+len) <= FLASH_SECURITY_SIZE_MAX);

    uint8_t read_cmd[5];
    uint8_t page_num   = len / FLASH_SECURITY_PAGE_SIZE;
    uint16_t remainder = len % FLASH_SECURITY_PAGE_SIZE;
    uint16_t i = 0;

#if (FLASH_XIP == 1)
    GLOBAL_INT_DISABLE();
    flash_cache_disable();
#endif

    for(i = 0; i < page_num; i++){
        read_cmd[0]= FLASH_SECURITY_READ;
        read_cmd[1]= (offset>>16)&0xff;
        read_cmd[2]= (offset>>8 )&0xff;
        read_cmd[3]= (offset    )&0xff;
        read_cmd[4]= 0;//dumy data @XTX Flash Datasheet.
        hal_qspi_standard_read_byte(buf, FLASH_SECURITY_PAGE_SIZE, read_cmd, sizeof(read_cmd));
        buf    += FLASH_SECURITY_PAGE_SIZE;
        offset += FLASH_SECURITY_PAGE_SIZE;
    }

    if(remainder){
        read_cmd[0]= FLASH_SECURITY_READ;
        read_cmd[1]= (offset>>16)&0xff;
        read_cmd[2]= (offset>>8 )&0xff;
        read_cmd[3]= (offset    )&0xff;
        read_cmd[4]= 0;//dumy data @XTX Flash Datasheet.
        hal_qspi_standard_read_byte(buf, remainder, read_cmd, sizeof(read_cmd));
    }

#if (FLASH_XIP == 1)
    flash_cache_init(0);
    GLOBAL_INT_RESTORE();
#endif
}

/**
 * @brief Set Quad mode.
 * That is, WP# becomes IO2 and HOLD# becoms IO3. The WP# and HOLD# inputs are not monitored
 * for their normal functions and are internally set to high (inactive).
 *
 * @return uint8_t
 */
void hal_flash_quad_mode_enable(uint8_t enable)
{
    uint8_t read_sr1_cmd = FLASH_READ_STATUS_REG_1;
    uint8_t read_sr2_cmd = FLASH_READ_STATUS_REG_2;
    uint8_t write_sr_cmd[3];
    flash_status_reg1_t status1={0};
    flash_status_reg2_t status2={0};

    hal_qspi_standard_read_byte(&status1.reg1_data, sizeof(uint8_t), &read_sr1_cmd, sizeof(read_sr1_cmd));
    hal_qspi_standard_read_byte(&status2.reg2_data, sizeof(uint8_t), &read_sr2_cmd, sizeof(read_sr2_cmd));
    
    enable = (enable ? 1 : 0);
    if(((status2.bits.CMP != 0)||(status1.bits.BP0_4 != 0)||(status1.bits.SRP != 0)||(status2.bits.QE != enable)))
    {
        status2.bits.CMP = 0;
        status1.bits.BP0_4 = 0;
        status1.bits.SRP = 0;
        status2.bits.QE = enable;
    }
    
    write_sr_cmd[0] = FLASH_WRITE_NON_VOLATILE_SR;
    write_sr_cmd[1] = status1.reg1_data;
    write_sr_cmd[2] = status2.reg2_data;

    hal_flash_write_enable();
    hal_qspi_standard_write(write_sr_cmd, sizeof(write_sr_cmd));
    hal_flash_operation_wait();
}

void hal_flash_operation_wait(void)
{
    uint8_t flash_status,read_stat_cmd = FLASH_READ_STATUS_REG_1;
    do
    {
        hal_qspi_standard_read_byte(&flash_status, sizeof(flash_status), &read_stat_cmd, sizeof(read_stat_cmd));
    } while( flash_status & 0x1);
}

#if defined (__CC_ARM)
#pragma pop
#elif defined (__GNUC__)
#pragma GCC pop_options
#endif
