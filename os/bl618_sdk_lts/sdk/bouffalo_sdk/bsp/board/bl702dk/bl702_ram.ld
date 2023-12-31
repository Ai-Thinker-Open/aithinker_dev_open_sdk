/****************************************************************************************
* @file bl702_flash.ld
*
* @brief This file is the map file (gnuarm or armgcc).
*
* Copyright (C) BouffaloLab 2021
*
****************************************************************************************
*/

/* configure the CPU type */
OUTPUT_ARCH( "riscv" )

/* configure the entry point */
ENTRY(__start)

StackSize    = 0x0400; /*  1KB */
HeapMinSize  = 0x1000; /*  4KB */

MEMORY
{
    fw_header_memory  (rx)  : ORIGIN = 0x22010000 - (176+16), LENGTH = (176+16)
    code_memory  (rx)  : ORIGIN = 0x22010000, LENGTH = 64K
    itcm_memory (rx)  : ORIGIN = 0x22020000, LENGTH = 0K
    dtcm_memory (rx)  : ORIGIN = 0x42020000, LENGTH = 4K
    ram_memory  (!rx) : ORIGIN = 0x42021000, LENGTH = 112K - 64K - 4K
    hbn_memory  (rx)  : ORIGIN = 0x40010000, LENGTH = 0xE00     /* hbn ram 4K used 3.5K*/
}

SECTIONS
{
    PROVIDE(__metal_chicken_bit = 0);

    .fw_header :
    {
        KEEP(*(.fw_header))
        LONG(ORIGIN(code_memory))
        LONG(0x48474553)
    } > fw_header_memory

    .init :
    {
        KEEP (*(SORT_NONE(.init)))
        KEEP (*(SORT_NONE(.vector)))

    } > code_memory

    .text :
    {
        . = ALIGN(4);
        __text_code_start__ = .;

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

        *(.srodata)
        *(.srodata.*)

        . = ALIGN(4);
        __text_code_end__ = .;
    } > code_memory

    . = ALIGN(4);
    __itcm_load_addr = .;

    .itcm_region : AT (__itcm_load_addr)
    {
        . = ALIGN(4);
        __tcm_code_start__ = .;
        . = ALIGN(4);
        __tcm_code_end__ = .;
    } > itcm_memory

    __hbn_load_addr = __itcm_load_addr + SIZEOF(.itcm_region);

    .hbn_ram_region : AT (__hbn_load_addr)
    {
        . = ALIGN(4);
        __hbn_ram_start__ = .;
        *bl702_hbn_wakeup*.o*(.rodata*)
        *(.hbn_ram_code*)
        *(.hbn_ram_data)
        . = ALIGN(4);
        __hbn_ram_end__ = .;
    } > hbn_memory

    __dtcm_load_addr = __hbn_load_addr + SIZEOF(.hbn_ram_region);

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
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __tcm_data_end__, "region RAM overflowed with stack")
    /*************************************************************************/

    __ram_load_addr = __dtcm_load_addr + SIZEOF(.dtcm_region);

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

        *(.nocache_ram)
        
        . = ALIGN(4);
        __bflog_tags_start__ = .;
        *(.bflog_tags_array)
        . = ALIGN(4);
        __bflog_tags_end__ = .;
        __ram_data_end__ = .;
    } > ram_memory

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
        *(.nocache_noinit_ram)

        . = ALIGN(4);
        __noinit_data_end__ = .;
    } > ram_memory

    .heap (NOLOAD):
    {
        . = ALIGN(4);
        __HeapBase = .;

        KEEP(*(.heap*))

        . = ALIGN(4);
        __HeapLimit = .;
    } > ram_memory

    __HeapLimit = ORIGIN(ram_memory) + LENGTH(ram_memory);

    ASSERT((__HeapLimit -  __HeapBase ) >= HeapMinSize, "heap size is too short.")

}

