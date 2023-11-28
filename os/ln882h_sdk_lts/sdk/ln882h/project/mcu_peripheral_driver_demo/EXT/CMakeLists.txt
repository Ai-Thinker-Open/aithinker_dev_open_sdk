include(ProjModuleCollect.cmake)

set(PROJ_ALL_SRC
    app/main.c

    bsp/serial_hw.c

    bsp/ln_show_reg.c
    bsp/ln_test_common.c
    bsp/ln_uart_test.c
    bsp/ln_spi_master_test.c
    bsp/ln_spi_slave_test.c
    bsp/ln_i2c_test.c
    bsp/ln_mpu9250_test.c
    bsp/ln_oled_test.c
    bsp/ln_timer_test.c
    bsp/ln_rtc_test.c
    bsp/ln_adv_timer_test.c
    bsp/ln_wdt_test.c
    bsp/ln_i2s_test.c
    bsp/ln_ws2811_test.c
    bsp/ln_dma_test.c
    bsp/ln_gpio_test.c
    bsp/ln_ext_test.c
    bsp/ln_flash_test.c
    bsp/ln_sdio_test.c
    bsp/ln_aes_test.c
    bsp/ln_ds18b20_test.c
    bsp/ln_clk_test.c
    bsp/ln_trng_test.c
    bsp/ln_adc_test.c
    bsp/ln_spi_lcd_test.c
    
    startup/startup_${CHIP_SERIAL}_gcc.c
    ${MODULE_SRC}
)


#----------------------------   project output elf   ---------------------------
set(TARGET_ELF_NAME                         ${USER_PROJECT})
set(pro_executable_target                   ${TARGET_ELF_NAME}.elf)
add_executable(${pro_executable_target}     ${PROJ_ALL_SRC})

target_link_libraries(${pro_executable_target}
    PUBLIC
    -lc -lm -lnosys

    PRIVATE
    -T${LINKER_SCRIPT}
    ${EXTRA_LINK_FLAGS}
)

target_link_directories(${pro_executable_target}
    PRIVATE
    ${LN_SDK_ROOT}/lib/gcclib
)

target_include_directories(${pro_executable_target}
    PRIVATE
    app
    bsp
    cfg
)

include(gcc/gcc-custom-build-stage.cmake)
