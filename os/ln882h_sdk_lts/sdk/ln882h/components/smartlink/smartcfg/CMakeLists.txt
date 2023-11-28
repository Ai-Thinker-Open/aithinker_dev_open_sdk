file(GLOB_RECURSE  SMARTCFG_SRC  ${COMP_SMARTCFG_DIR}  "*.c")

set(smartcfg_static_target   "smartcfg")
add_library(${smartcfg_static_target}  STATIC  ${SMARTCFG_SRC})
add_library(components::smartlink::smartcfg  ALIAS  ${smartcfg_static_target})

target_include_directories(${smartcfg_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${MCU_LN882X_DIR}/driver_ln882x/inc
    ${MCU_LN882X_DIR}/driver_ln882x/inc/ll
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${COMP_WIFI_DIR}
    ${CMAKE_SOURCE_DIR}/components
    ${COMP_LWIP_DIR}/include
    ${COMP_LWIP_DIR}/port
    ${COMP_UTILS_DIR}
    ${EXTRA_INCLUDE_PATH}
)
