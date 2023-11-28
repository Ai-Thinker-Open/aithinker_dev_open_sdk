#include "proj_config.h"
#include "ln882h.h"

#include "hal/hal_flash.h"
#include "hal/hal_cache.h"
#include "hal/hal_misc.h"
#include "hal/hal_gpio.h"

#include "utils/debug/log.h"

#include "flash_partition_mgr.h"
#include "ln_nvds.h"
#include "ota_agent.h"

typedef void (*jump_func_t)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
extern uint32_t boot_header_info_init(void);

static void set_interrupt_priority(void)
{
    __NVIC_SetPriorityGrouping(4);

    NVIC_SetPriority(SysTick_IRQn,   1);
    NVIC_SetPriority(UART0_IRQn,     4);
    NVIC_SetPriority(UART1_IRQn,     4);
}

static void jump_to_application(uint32_t app_offset)
{
    //Prepare for jump
    {
        log_deinit();                 //clear log uart isr
        hal_flash_quad_mode_enable(1);//Enable QSPI 4bit mode
        flash_cache_init(0);          //Init Flash cache
    }

    //Jump to user's Reset_Handler
    {
        uint32_t *vec_int_base = (uint32_t *)(CACHE_FLASH_BASE + app_offset);
        jump_func_t *jump_func = (jump_func_t *)(vec_int_base + 1);
        
        __set_MSP(*vec_int_base);
        (*jump_func)(CACHE_FLASH_BASE + app_offset, 1, 2, 3);
    }
}

int main (int argc, char* argv[])
{
    partition_info_t nvds_part_info;
    
    SetSysClock();

    /**
     * Remove uart pin initialization in the rom stage
    */
    {
        hal_gpio_pin_afio_en(GPIOA_BASE, GPIO_PIN_2, HAL_DISABLE);
        hal_gpio_pin_afio_en(GPIOA_BASE, GPIO_PIN_3, HAL_DISABLE);
    }

    set_interrupt_priority();
    __enable_irq();

    log_init();
    hal_flash_init();
    
    uint32_t flash_id = hal_flash_read_id();
    
    flash_cache_disable();

    if (LN_TRUE != ln_verify_partition_table()) {
        // partition tab error!
    }
    
    if (LN_TRUE != ln_fetch_partition_info(PARTITION_TYPE_NVDS, &nvds_part_info)) {
        // no NVDS partition!    
    }

    if (NVDS_ERR_OK != ln_nvds_init(nvds_part_info.start_addr)) {
        // NVDS init filed.
    }

    ota_port_init();

    do {
        if (0xaeaebebe == hal_misc_awo_get_r_idle_reg()) {
            partition_info_t part_ota_info;
            if (LN_TRUE != ln_fetch_partition_info(PARTITION_TYPE_OTA, &part_ota_info)) {
                break;
            }
            jump_to_application(part_ota_info.start_addr + sizeof(image_hdr_t));
            while(1);
        }
    } while(0);
    if(OTA_ERR_NONE != ota_boot_upgrade_agent(jump_to_application))
    {
        // TODO:process error code
    }
    while(1);
}



void MemManage_Handler (void)
{

}

void BusFault_Handler (void)
{

}

void UsageFault_Handler (void)
{

}

