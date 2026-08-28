#ifndef _AXK_OTA_FLASH_H_
#define _AXK_OTA_FLASH_H_

#include <stdint.h>




int partition_erase(int start_addr, int size);
int partition_write_ota_farmware(int dst_offset, const void *src, int size);
void set_boot_partition(void);
void set_reboot(_Bool ota_result);

#endif //_AXK_OTA_FLASH_H_
