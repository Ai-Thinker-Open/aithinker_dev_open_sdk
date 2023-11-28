file(GLOB_RECURSE  AIRKISS_SRC  ${COMP_AIRKISS_DIR}  "*.c")

set(airkiss_static_target   "airkiss")
add_library(${airkiss_static_target}  STATIC  ${AIRKISS_SRC})
add_library(components::smartlink::airkiss  ALIAS  ${airkiss_static_target})

# TODO: adapt for different cloud server, components::qcloud may be changed.

target_include_directories(${airkiss_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${CMAKE_SOURCE_DIR}/components/
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${COMP_WIFI_DIR}
    ${COMP_LWIP_DIR}/include
    ${COMP_LWIP_DIR}/port
    ${COMP_UTILS_DIR}
    ${COMP_QCLOUD_SDK_DIR}/include
    ${COMP_QCLOUD_SDK_DIR}/include/exports
    ${EXTRA_INCLUDE_PATH}
)
