file(GLOB_RECURSE  SOFTAPCFG_SRC  ${COMP_SOFTAPCFG_DIR}  "*.c")

set(softapcfg_static_target   "softapcfg")
add_library(${softapcfg_static_target}  STATIC  ${SOFTAPCFG_SRC})
add_library(components::smartlink::softapcfg  ALIAS  ${softapcfg_static_target})

target_include_directories(${softapcfg_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${CMAKE_SOURCE_DIR}/components
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${COMP_WIFI_DIR}
    ${COMP_LWIP_DIR}/include
    ${COMP_LWIP_DIR}/port
    ${COMP_DHCPD_DIR}
    ${COMP_UTILS_DIR}
    ${EXTRA_INCLUDE_PATH}
)
