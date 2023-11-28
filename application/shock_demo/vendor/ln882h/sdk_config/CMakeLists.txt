include(ProjModuleCollect.cmake)

file(GLOB_RECURSE  APP_SRC  app/*.c)
file(GLOB_RECURSE  BSP_SRC  bsp/*.c)
file(GLOB_RECURSE  PROJECT_SRC  src/*.c)
file(GLOB_RECURSE  TRANSPARENT_SRC  transparent/*.c)


set(PROJ_ALL_SRC
    startup/startup_${CHIP_SERIAL}_gcc.c
    ${MODULE_SRC}
    ${TRANSPARENT_SRC}
    ${COMP_LWIP_DIR}/apps/mqtt/mqtt.c
    ${COMP_LWIP_DIR}/apps/sntp/sntp.c
    ${COMP_SMARTLINK_DIR}/smartcfg/ln_smartcfg.c
    ${COMP_SMARTLINK_DIR}/smartcfg/ln_smartcfg_port.c
    ${APP_SRC}
    ${BSP_SRC}
    ${PROJECT_SRC}
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
    src
    transparent
)

include(gcc/gcc-custom-build-stage.cmake)
