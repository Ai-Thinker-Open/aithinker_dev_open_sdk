/** @brief      Flash application interface.
 *
 *  @file       aiio_flash.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       SPI application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/04/18      <td>1.0.0       <td>linzy        <td>flash init
 *  <tr><td>2023/06/06      <td>1.0.0       <td>linzy        <td>update flash interface
 *  </table>
 *
 */
#include "aiio_flash.h"
#include "aiio_error.h"
#include "bl_flash.h"
#include "aiio_log.h"
#include "hosal_flash.h"
#include <bl_mtd.h>

#include "bl602_xip_sflash.h"
#include "bl602_xip_sflash_ext.h"

static uint32_t flash_id_date[3];
static uint16_t flash_device_id_date[2];
static uint8_t flash_manufacturer_id_date[2];
static uint8_t flash_unique_id_date[16];

static SPI_Flash_Cfg_Type g_flash_cfg = {
    .resetCreadCmd=0xff,
    .resetCreadCmdSize=3,
    .mid=0xc8,

    .deBurstWrapCmd=0x77,
    .deBurstWrapCmdDmyClk=0x3,
    .deBurstWrapDataMode=SF_CTRL_DATA_4_LINES,
    .deBurstWrapData=0xF0,

    /*reg*/
    .writeEnableCmd=0x06,
    .wrEnableIndex=0x00,
    .wrEnableBit=0x01,
    .wrEnableReadRegLen=0x01,

    .qeIndex=1,
    .qeBit=0x01,
    .qeWriteRegLen=0x02,
    .qeReadRegLen=0x1,

    .busyIndex=0,
    .busyBit=0x00,
    .busyReadRegLen=0x1,
    .releasePowerDown=0xab,

    .readRegCmd[0]=0x05,
    .readRegCmd[1]=0x35,
    .writeRegCmd[0]=0x01,
    .writeRegCmd[1]=0x01,

    .fastReadQioCmd=0xeb,
    .frQioDmyClk=16/8,
    .cReadSupport=1,
    .cReadMode=0xA0,

    .burstWrapCmd=0x77,
    .burstWrapCmdDmyClk=0x3,
    .burstWrapDataMode=SF_CTRL_DATA_4_LINES,
    .burstWrapData=0x40,
     /*erase*/
    .chipEraseCmd=0xc7,
    .sectorEraseCmd=0x20,
    .blk32EraseCmd=0x52,
    .blk64EraseCmd=0xd8,
    /*write*/
    .pageProgramCmd=0x02,
    .qpageProgramCmd=0x32,
    .qppAddrMode=SF_CTRL_ADDR_1_LINE,

    .ioMode=SF_CTRL_QIO_MODE,
    .clkDelay=1,
    .clkInvert=0x1,

    .resetEnCmd=0x66,
    .resetCmd=0x99,
    .cRExit=0xff,
    .wrEnableWriteRegLen=0x00,

    /*id*/
    .jedecIdCmd=0x9f,
    .jedecIdCmdDmyClk=0,
    .qpiJedecIdCmd=0x9f,
    .qpiJedecIdCmdDmyClk=0x00,
    .sectorSize=4,
    .pageSize=256,

    /*read*/
    .fastReadCmd=0x0b,
    .frDmyClk=8/8,
    .qpiFastReadCmd =0x0b,
    .qpiFrDmyClk=8/8,
    .fastReadDoCmd=0x3b,
    .frDoDmyClk=8/8,
    .fastReadDioCmd=0xbb,
    .frDioDmyClk=0,
    .fastReadQoCmd=0x6b,
    .frQoDmyClk=8/8,

    .qpiFastReadQioCmd=0xeb,
    .qpiFrQioDmyClk=16/8,
    .qpiPageProgramCmd=0x02,
    .writeVregEnableCmd=0x50,

    /* qpi mode */
    .enterQpi=0x38,
    .exitQpi=0xff,

     /*AC*/
    .timeEsector=300,
    .timeE32k=1200,
    .timeE64k=1200,
    .timePagePgm=5,
    .timeCe=20*1000,
    .pdDelay=20,
    .qeData=0,
};

int32_t aiio_flash_init(void)
{
    if(bl_flash_init() !=AIIO_OK)
    {
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

int32_t aiio_flash_deinit(void)
{

    return AIIO_OK;
}

int32_t aiio_flash_info_get(flash_dev_t *p_dev, logic_partition_t *partition)
{
    if(hosal_flash_info_get(p_dev,partition) !=AIIO_OK)
    {
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

int32_t aiio_flash_open(const char *name, unsigned int flags)
{
    flash_dev_t *p_dev;
    p_dev=hosal_flash_open(name,flags);
    return p_dev;
}

int32_t aiio_flash_close(flash_dev_t *p_dev)
{
    if(hosal_flash_close(p_dev) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

int32_t aiio_flash_write(uint32_t write_addr, uint8_t *write_src_data, uint32_t write_length)
{
    flash_dev_t *p_flash;
    p_flash = aiio_flash_open("DATA",0);
    if(p_flash == NULL)
    {
        aiio_log_e("No partition name DATA!\r\n");
        return p_flash;
    }
    if(write_src_data == NULL)
    {
        return AIIO_ERROR;
    }

    /*Write starts at address 0 in the partitioned table*/
    write_addr =0;
    if( bl_mtd_write(p_flash->flash_dev,write_addr, write_length,write_src_data) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    aiio_flash_close(p_flash);
    return AIIO_OK;
}

int32_t aiio_flash_read(uint32_t read_addr, uint8_t *read_dst_data, uint32_t read_length)
{
    flash_dev_t *p_flash;
    p_flash = aiio_flash_open("DATA",0);
    if(p_flash == NULL)
    {
        aiio_log_e("No partition name DATA!\r\n");
        return AIIO_ERROR;
    }
    if(read_dst_data ==NULL)
    {
        return AIIO_ERROR;
    }

    /*Read start at address 0 in the partitioned table*/
    read_addr = 0;
    if(bl_mtd_read(p_flash->flash_dev,read_addr, read_length, read_dst_data) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    aiio_flash_close(p_flash);
    return AIIO_OK;
}

int32_t aiio_flash_erase(uint32_t erase_addr, uint32_t erase_length)
{
    flash_dev_t *p_flash;
    p_flash = aiio_flash_open("DATA",0);
    if(p_flash == NULL)
    {
        aiio_log_e("No partition name DATA!\r\n");
        return p_flash;
    }

    /*Erase starts at address 0 in the partition table*/
    erase_addr =0;
    if(bl_mtd_erase(p_flash->flash_dev,erase_addr,erase_length) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    aiio_flash_close(p_flash);
    return AIIO_OK;
}

int32_t aiio_flash_write_erase(uint32_t addr, uint8_t *src_data, uint32_t length)
{
    flash_dev_t *p_flash;
    if(src_data == NULL)
    {
        return AIIO_ERROR;
    }
    p_flash = aiio_flash_open("DATA",0);
    if(p_flash == NULL)
    {
        aiio_log_e("No partition name DATA!\r\n");
        return p_flash;
    }

    /*Erase first and write starting at address 0 in the partitioned table*/
    addr = 0;
    if(bl_mtd_erase(p_flash->flash_dev,addr,length) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    if( bl_mtd_write(p_flash->flash_dev,addr, length,src_data) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    aiio_flash_close(p_flash);
    return AIIO_OK;
}

int32_t aiio_flash_id_read(void)
{
    uint32_t flash_id =0;
    if(XIP_SFlash_GetJedecId_Need_Lock_Ext(&g_flash_cfg,flash_id_date) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    flash_id = flash_id_date[0];
    return flash_id;
}

int32_t aiio_flash_device_id_read(void)
{
    uint16_t device_id=0;
    if(XIP_SFlash_GetDeviceId_Need_Lock_Ext(&g_flash_cfg,flash_device_id_date) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    // aiio_log_w("flash_device_date   :  %x  %x  %x\r\n",flash_device_date[0],flash_device_date[1],flash_device_date);
    device_id=flash_device_id_date[0];
    return device_id;
}

int32_t aiio_flash_manufacturer_id_read(void)
{
    uint8_t manufacturer_id=0;
    if( XIP_SFlash_GetDeviceId_Need_Lock_Ext(&g_flash_cfg,flash_manufacturer_id_date) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    // aiio_log_w("flash_device_date   :  %x  %x  %x\r\n",flash_device_date[0],flash_device_date[1],flash_device_date);
    manufacturer_id=flash_manufacturer_id_date[1];
    return manufacturer_id;
}

int32_t aiio_flash_unique_id_read(void)
{
    if( XIP_SFlash_GetUniqueId_Need_Lock(&g_flash_cfg,flash_unique_id_date,2) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    return flash_unique_id_date;
}

int32_t aiio_flash_status_read(void)
{
    uint32_t stat=0;
    if(SFlash_Read_Reg(&g_flash_cfg,0,(uint8_t *)&stat,1) != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    return stat;
}

