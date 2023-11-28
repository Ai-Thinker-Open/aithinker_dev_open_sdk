###################################  utils  ####################################
set(UTILS_SRC
    ${COMP_UTILS_DIR}/debug/ln_assert.c
    ${COMP_UTILS_DIR}/debug/log.c
    ${COMP_UTILS_DIR}/wrap_stdio.c
    ${COMP_UTILS_DIR}/fifo/fifobuf.c
)

include_directories(${COMP_UTILS_DIR})
include_directories(${COMP_UTILS_DIR}/fifo)
list(APPEND MODULE_SRC ${UTILS_SRC})

###################################  serial  ###################################
file(GLOB_RECURSE  SERIAL_SRC  ${COMP_SERIAL_DIR}/*.c)
include_directories(${COMP_SERIAL_DIR})
list(APPEND MODULE_SRC ${SERIAL_SRC})

###################################   MCU   ####################################
set(MCU_SRC
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_adc.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_aes.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_cache.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_common.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_dma.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_ext.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_flash.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_gpio.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_i2s.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_it.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_i2c.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_qspi.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_rtc.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_spi.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_timer.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_trng.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_uart.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_wdt.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_ws2811.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_misc.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_clock.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_adv_timer.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_sdio_device.c

    ${MCU_LN882X_DIR}/${CHIP_SERIAL}/system_${CHIP_SERIAL}.c
)

include_directories(${MCU_LN882X_DIR}/${CHIP_SERIAL})
include_directories(${MCU_LN882X_DIR}/CMSIS_5.3.0)
include_directories(${MCU_LN882X_DIR}/driver_${CHIP_SERIAL})
include_directories(${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal)
include_directories(${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/reg)
list(APPEND MODULE_SRC ${MCU_SRC})

###################################   MISC  ####################################
set(MISC_SRC
    ${COMP_LIBC_STUB_DIR}/newlib_noos.c
    ${COMP_LIBC_STUB_DIR}/stub_malloc.c
)
include_directories(${LN_SDK_ROOT}/components)
list(APPEND MODULE_SRC ${MISC_SRC})
