include(ProjModuleCollect.cmake)

file(GLOB_RECURSE  TRANSPARENT_SRC  transparent/*.c)

set(PROJ_ALL_SRC
    app/main.c
    app/usr_app.c
    app/usr_ble_app.c
    bsp/serial_hw.c
    bsp/drv_adc_measure.c
    app/usr_ctrl.c
    app/wifi_driver_port.c
	app/sys_driver_port.c
	app/app_ble_pair.c
    startup/startup_${CHIP_SERIAL}_gcc.c
	${TRANSPARENT_SRC}
    ${MODULE_SRC}
    ${COMP_LWIP_DIR}/apps/mqtt/mqtt.c
    ${COMP_LWIP_DIR}/apps/sntp/sntp.c
	${COMP_SMARTLINK_DIR}/smartcfg/ln_smartcfg.c
	${COMP_SMARTLINK_DIR}/smartcfg/ln_smartcfg_port.c
)

set(COMP_DHCPD_SUPPORT          ON  PARENT_SCOPE)
set(COMP_LWIP_SUPPORT           ON  PARENT_SCOPE)

#----------------------------   project output elf   ---------------------------
set(TARGET_ELF_NAME                         ${USER_PROJECT})
set(pro_executable_target                   ${TARGET_ELF_NAME}.elf)
add_executable(${pro_executable_target}     ${PROJ_ALL_SRC})

target_link_libraries(${pro_executable_target}
    PUBLIC
    ${CHIP_SERIAL}_ble_full_stack
    ln::dhcpd
    lwip
    ${CHIP_SERIAL}_wifi
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
    transparent
)

include(gcc/gcc-custom-build-stage.cmake)
