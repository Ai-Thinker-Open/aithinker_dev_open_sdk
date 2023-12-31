/****************************************************************************************
* @file flash.ld
*
* @brief This file is the link script file (gnuarm or armgcc).
*
* Copyright (C) BouffaloLab 2021
*
****************************************************************************************
*/

/* configure the CPU type */
OUTPUT_ARCH( "riscv" )
/* link with the standard c library */
INPUT(-lc)
/* link with the standard GCC library */
INPUT(-lgcc)
/* configure the entry point */
ENTRY(__start)

StackSize    = 0x0400; /*  1KB */
HeapMinSize  = 0x1000; /*  4KB */

MEMORY
{
    fw_header_memory  (rx)  : ORIGIN = 0x58000000 - 0x1000, LENGTH = 4K
    xip_memory  (rx)  : ORIGIN = 0x58000000, LENGTH = 32M
    itcm_memory (rx)  : ORIGIN = 0x62020000, LENGTH = 20K
    dtcm_memory (rx)  : ORIGIN = 0x62025000, LENGTH = 4K
    nocache_ram_memory (!rx) : ORIGIN = 0x22026000, LENGTH = 40K
    ram_memory  (!rx) : ORIGIN = 0x62030000, LENGTH = 160K + 64K - 20K - 4K - 40K
    xram_memory  (!rx) : ORIGIN = 0x40000000, LENGTH = 16K
}

SECTIONS
{
    .fw_header :
    {
        KEEP(*(.fw_header))
    } > fw_header_memory

    .text :
    {
        . = ALIGN(4);
        __text_code_start__ = .;

        KEEP (*(SORT_NONE(.init)))
        KEEP (*(SORT_NONE(.vector)))

        *(.text)
        *(.text.*)

        /* section information for shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;

        /* section information for usb usbh_class_info */
        . = ALIGN(4);
        __usbh_class_info_start__ = .;
        KEEP(*(.usbh_class_info))
        . = ALIGN(4);
        __usbh_class_info_end__ = .;

        /*put .rodata**/
        *(EXCLUDE_FILE( *bl606p_glb*.o* \
                        *bl606p_glb_gpio*.o* \
                        *bl606p_pds*.o* \
                        *bl606p_aon*.o* \
                        *bl606p_hbn*.o* \
                        *bl606p_l1c*.o* \
                        *bl606p_common*.o* \
                        *bl606p_clock*.o* \
                        *bl606p_ef_ctrl*.o* \
                        *bl606p_sf_cfg*.o* \
                        *bl606p_sf_ctrl*.o* \
                        *bl606p_sflash*.o* \
                        *bl606p_xip_sflash*.o* \
                        *bl606p_romapi_patch*.o* ) .rodata*)
        *(.srodata)
        *(.srodata.*)

        /* _dump_sections for bflb_coredump.c*/
        . = ALIGN(4);
        PROVIDE( _dump_sections = ABSOLUTE(.));
        LONG(ORIGIN(itcm_memory));
        LONG(LENGTH(itcm_memory));
        LONG(ORIGIN(dtcm_memory));
        LONG(LENGTH(dtcm_memory));
        LONG(ORIGIN(ram_memory));
        LONG(LENGTH(ram_memory));
        LONG(ORIGIN(nocache_ram_memory));
        LONG(LENGTH(nocache_ram_memory));
        LONG(ORIGIN(ram_wifi));
        LONG(LENGTH(ram_wifi));
        LONG(ORIGIN(xram_memory));
        LONG(LENGTH(xram_memory));
        LONG(0xffffffff);
        LONG(0xffffffff);
        /* _dump_sections for bl_coredump.c*/

        . = ALIGN(4);
        __text_code_end__ = .;
    } > xip_memory

    . = ALIGN(4);
    __itcm_load_addr = .;

    .itcm_region : AT (__itcm_load_addr)
    {
        . = ALIGN(4);
        __tcm_code_start__ = .;

        *(.tcm_code.*)
        *(.tcm_const.*)
        *(.sclock_rlt_code.*)
        *(.sclock_rlt_const.*)

        *bl606p_glb*.o*(.rodata*)
        *bl606p_glb_gpio*.o*(.rodata*)
        *bl606p_pds*.o*(.rodata*)
        *bl606p_aon*.o*(.rodata*)
        *bl606p_hbn*.o*(.rodata*)
        *bl606p_l1c*.o*(.rodata*)
        *bl606p_common*.o*(.rodata*)
        *bl606p_clock*.o*(.rodata*)
        *bl606p_ef_ctrl*.o*(.rodata*)
        *bl606p_sf_cfg*.o*(.rodata*)
        *bl606p_sf_ctrl*.o*(.rodata*)
        *bl606p_sflash*.o*(.rodata*)
        *bl606p_xip_sflash*.o*(.rodata*)
        *bl606p_romapi_patch*.o*(.rodata*)

        . = ALIGN(4);
        __tcm_code_end__ = .;
    } > itcm_memory

    __dtcm_load_addr = __itcm_load_addr + SIZEOF(.itcm_region);

    .dtcm_region : AT (__dtcm_load_addr)
    {
        . = ALIGN(4);
        __tcm_data_start__ = .;

        *(.tcm_data)
        /* *finger_print.o(.data*) */

        . = ALIGN(4);
        __tcm_data_end__ = .;
    } > dtcm_memory

    /*************************************************************************/
    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (NOLOAD):
    {
        . = ALIGN(0x4);
        . = . + StackSize;
        . = ALIGN(0x4);
    } > dtcm_memory

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(dtcm_memory) + LENGTH(dtcm_memory);
    PROVIDE( __freertos_irq_stack_top = __StackTop);
    PROVIDE( _sp_main = __StackTop);
    PROVIDE( _sp_base = __StackTop - StackSize);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __tcm_data_end__, "region RAM overflowed with stack")
    /*************************************************************************/
    __nocache_ram_load_addr = __dtcm_load_addr + SIZEOF(.dtcm_region);

    .nocache_ram_region  : AT (__nocache_ram_load_addr)
    {
        . = ALIGN(4);
        __nocache_ram_data_start__ = .;

        *(.nocache_ram)

        . = ALIGN(4);
        __nocache_ram_data_end__ = .;
    } > nocache_ram_memory

    __ram_load_addr = __nocache_ram_load_addr + SIZEOF(.nocache_ram_region);

    /* Data section */
    RAM_DATA : AT (__ram_load_addr)
    {
        . = ALIGN(4);
        __ram_data_start__ = .;

        PROVIDE( __global_pointer$ = . + 0x800 );

        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
        *(.sdata2)
        *(.sdata2.*)

        . = ALIGN(4);
        __bflog_tags_start__ = .;
        *(.bflog_tags_array)
        . = ALIGN(4);
        __bflog_tags_end__ = .;
        __ram_data_end__ = .;
    } > ram_memory

    __etext_final = (__ram_load_addr + SIZEOF (RAM_DATA));
    ASSERT(__etext_final <= ORIGIN(xip_memory) + LENGTH(xip_memory), "code memory overflow")

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start__ = .;

        *(.bss*)
        *(.sbss*)
        *(COMMON)

        . = ALIGN(4);
        __bss_end__ = .;
    } > ram_memory

    .noinit_data (NOLOAD) :
    {
        . = ALIGN(4);
        __noinit_data_start__ = .;

        *(.noinit_data*)

        . = ALIGN(4);
        __noinit_data_end__ = .;
    } > ram_memory

    .nocache_noinit_ram_region (NOLOAD) :
    {
        . = ALIGN(4);
        __nocache_noinit_ram_data_start__ = .;

        *(.nocache_noinit_ram)
        *(.noncacheable)

        . = ALIGN(4);
        __nocache_noinit_ram_data_end__ = .;
    } > nocache_ram_memory

    .heap (NOLOAD):
    {
        . = ALIGN(4);
        __HeapBase = .;

        /*__end__ = .;*/
        /*end = __end__;*/
        KEEP(*(.heap*))

        . = ALIGN(4);
        __HeapLimit = .;
    } > ram_memory

    __HeapLimit = ORIGIN(ram_memory) + LENGTH(ram_memory);
    ASSERT(__HeapLimit - __HeapBase >= HeapMinSize, "heap region overflow")

}

