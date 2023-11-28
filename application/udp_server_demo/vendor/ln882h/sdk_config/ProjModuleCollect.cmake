##################################  kernel  ####################################
set(KERNEL_SRC
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/event_groups.c
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/list.c
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/queue.c
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/tasks.c
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/timers.c
    ${COMP_KERNEL_DIR}/FreeRTOS/hooks.c
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/MemMang/heap_5.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_common.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_cpuusage.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_debug.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_mutex.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_queue.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_semaphore.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_thread.c
    ${COMP_KERNEL_DIR}/FreeRTOS_Adapter/freertos_timer.c
)
include_directories(${COMP_KERNEL_DIR})
include_directories(${COMP_KERNEL_DIR}/osal)
include_directories(${COMP_KERNEL_DIR}/FreeRTOS_Adapter)
include_directories(${COMP_KERNEL_DIR}/FreeRTOS/Source/include)
include_directories(${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F)
list(APPEND MODULE_SRC ${KERNEL_SRC})

##################################  LN_AT  #####################################
file(GLOB_RECURSE  LNAT_SRC  ${COMP_LNAT_DIR}/*.c)
include_directories(${COMP_LNAT_DIR})
include_directories(${COMP_LNAT_DIR}/adapter)
include_directories(${COMP_LNAT_DIR}/cmd)
include_directories(${COMP_LNAT_DIR}/parser)
include_directories(${COMP_LNAT_DIR}/transfer)
list(APPEND MODULE_SRC ${LNAT_SRC})

##################################  LN_AT_CMD  #####################################
file(GLOB_RECURSE  LNAT_CMD_SRC  ${COMP_LNAT_CMD_DIR}/*.c)
include_directories(${COMP_LNAT_CMD_DIR})
include_directories(${COMP_LNAT_CMD_DIR}/base)
include_directories(${COMP_LNAT_CMD_DIR}/mqtt)
include_directories(${COMP_LNAT_CMD_DIR}/tcpip)
include_directories(${COMP_LNAT_CMD_DIR}/http)
include_directories(${COMP_LNAT_CMD_DIR}/bluetooth)
list(APPEND MODULE_SRC ${LNAT_CMD_SRC})

###################################  ping  #####################################
file(GLOB_RECURSE  PING_SRC  ${COMP_PING_DIR}/*.c)
include_directories(${COMP_PING_DIR})
list(APPEND MODULE_SRC ${PING_SRC})

###################################  iperf  ####################################
file(GLOB_RECURSE  IPERF_SRC  ${COMP_IPERF_DIR}/*.c)
include_directories(${COMP_IPERF_DIR})
list(APPEND MODULE_SRC ${IPERF_SRC})

###################################  mbedtls  ####################################
file(GLOB_RECURSE  MBEDTLS_SRC  ${COMP_MBEDTLS_DIR}/library/*.c)
include_directories(${COMP_MBEDTLS_DIR}/include)
list(APPEND MODULE_SRC ${MBEDTLS_SRC})

###################################  mbedtls port ####################################
file(GLOB_RECURSE  MBEDTLS_PORT_SRC  ${COMP_MBEDTLS_DIR}/port_ln/library/*.c)
include_directories(${COMP_MBEDTLS_DIR}/port_ln/include)
list(APPEND MODULE_SRC ${MBEDTLS_PORT_SRC})

###################################  httpsclinet  ####################################
file(GLOB_RECURSE  HTTPSCLIENT_SRC  ${COMP_HTTPSCLIENT_DIR}/src/*.c)
include_directories(${COMP_HTTPSCLIENT_DIR}/inc)
list(APPEND MODULE_SRC ${HTTPSCLIENT_SRC})

###################################  ota  ####################################
file(GLOB_RECURSE  FOTA_SRC  ${COMP_FOTA_DIR}/ota_agent/*.c)
include_directories(${COMP_FOTA_DIR}/ota_agent)
list(APPEND MODULE_SRC ${FOTA_SRC})

###################################  partition_mgr  ####################################
file(GLOB_RECURSE  PARTITION_SRC  ${COMP_PARTMGR_DIR}/*.c)
include_directories(${COMP_PARTMGR_DIR})
list(APPEND MODULE_SRC ${PARTITION_SRC})

###################################   kv   #####################################
file(GLOB_RECURSE  KV_SRC  ${COMP_KV_DIR}/*.c)
include_directories(${COMP_KV_DIR}/kv)
include_directories(${COMP_KV_DIR}/kv_port)
list(APPEND MODULE_SRC ${KV_SRC})

###################################  nvds  #####################################
file(GLOB_RECURSE  NVDS_SRC  ${COMP_NVDS_DIR}/*.c)
include_directories(${COMP_NVDS_DIR})
list(APPEND MODULE_SRC ${NVDS_SRC})

###################################  utils  ####################################
set(UTILS_SRC
    ${COMP_UTILS_DIR}/debug/CmBacktrace/cm_backtrace.c
    ${COMP_UTILS_DIR}/debug/ln_assert.c
    ${COMP_UTILS_DIR}/debug/log.c
    ${COMP_UTILS_DIR}/fifo/fifobuf.c
    ${COMP_UTILS_DIR}/reboot_trace/reboot_trace.c
    ${COMP_UTILS_DIR}/runtime/runtime.c
    ${COMP_UTILS_DIR}/power_mgmt/ln_pm.c

    ${COMP_UTILS_DIR}/ln_psk_calc.c
    ${COMP_UTILS_DIR}/ln_sha1.c
    ${COMP_UTILS_DIR}/ln_aes.c
    ${COMP_UTILS_DIR}/ln_misc.c
    ${COMP_UTILS_DIR}/crc16.c
    ${COMP_UTILS_DIR}/crc32.c
    ${COMP_UTILS_DIR}/system_parameter.c
    ${COMP_UTILS_DIR}/wrap_stdio.c
    ${COMP_UTILS_DIR}/linux_compat/linux_compat_time.c
)

include_directories(${COMP_UTILS_DIR})
include_directories(${COMP_UTILS_DIR}/runtime)
include_directories(${COMP_UTILS_DIR}/reboot_trace)
include_directories(${COMP_UTILS_DIR}/fifo)
include_directories(${COMP_UTILS_DIR}/debug)
include_directories(${COMP_UTILS_DIR}/debug/CmBacktrace)
include_directories(${COMP_UTILS_DIR}/linux_compat)
list(APPEND MODULE_SRC ${UTILS_SRC})

###################################  serial  ###################################
file(GLOB_RECURSE  SERIAL_SRC  ${COMP_SERIAL_DIR}/*.c)
include_directories(${COMP_SERIAL_DIR})
list(APPEND MODULE_SRC ${SERIAL_SRC})

###################################   MCU   ####################################
set(MCU_SRC
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_adc.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_cache.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_common.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_flash.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_qspi.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_uart.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_wdt.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_trng.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_gpio.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_interrupt.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_clock.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_misc.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_ext.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_timer.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_efuse.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_i2c.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_spi.c
    ${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/hal/hal_adv_timer.c
    ${MCU_LN882X_DIR}/${CHIP_SERIAL}/system_${CHIP_SERIAL}.c
)

include_directories(${MCU_LN882X_DIR}/${CHIP_SERIAL})
include_directories(${MCU_LN882X_DIR}/CMSIS_5.3.0)
include_directories(${MCU_LN882X_DIR}/driver_${CHIP_SERIAL})
include_directories(${MCU_LN882X_DIR}/driver_${CHIP_SERIAL}/reg)
list(APPEND MODULE_SRC ${MCU_SRC})

###############################   BLE (export) #################################
file(GLOB_RECURSE  BLE_LIB_IMPORT_SRC   ${COMP_BLE_DIR}/ble_lib_import/*.c)
file(GLOB_RECURSE  BLE_ARCH             ${COMP_BLE_DIR}/ble_arch/*.c)
file(GLOB_RECURSE  BLE_PROFILE_COMMON   ${COMP_BLE_DIR}/ble_profiles/prf_common/*.c)
file(GLOB_RECURSE  BLE_APP_COMP         ${COMP_BLE_DIR}/ble_app/*.c)
include_directories(${COMP_BLE_DIR})
include_directories(${COMP_BLE_DIR}/ble_arch)
include_directories(${COMP_BLE_DIR}/ble_profiles/prf_common)
include_directories(${COMP_BLE_DIR}/ble_lib_import)

include_directories(${COMP_BLE_DIR}/ble_app/ble_common)
include_directories(${COMP_BLE_DIR}/ble_app/ble_connection_manager)
include_directories(${COMP_BLE_DIR}/ble_app/ble_device_manager)
include_directories(${COMP_BLE_DIR}/ble_app/ble_event)
include_directories(${COMP_BLE_DIR}/ble_app/ble_gap/gap_advertising)
include_directories(${COMP_BLE_DIR}/ble_app/ble_gap/gap_misc)
include_directories(${COMP_BLE_DIR}/ble_app/ble_gap/gap_scan)
include_directories(${COMP_BLE_DIR}/ble_app/ble_gatt/gatt_client)
include_directories(${COMP_BLE_DIR}/ble_app/ble_gatt/gatt_common)
include_directories(${COMP_BLE_DIR}/ble_app/ble_gatt/gatt_server)
include_directories(${COMP_BLE_DIR}/ble_app/ble_import)
include_directories(${COMP_BLE_DIR}/ble_app/ble_smp)
include_directories(${COMP_BLE_DIR}/ble_app/ble_store)
include_directories(${COMP_BLE_DIR}/ble_app/ble_test)

include_directories(${COMP_BLE_DIR}/mac/ble/hl/api)
include_directories(${COMP_BLE_DIR}/mac/ble/hl/inc)
include_directories(${COMP_BLE_DIR}/mac/ble/ll/api)
include_directories(${COMP_BLE_DIR}/mac/ble/ll/import)
include_directories(${COMP_BLE_DIR}/mac/ble/ll/src)
include_directories(${COMP_BLE_DIR}/mac/ble/ll/src/llm)
include_directories(${COMP_BLE_DIR}/mac/em/api)
include_directories(${COMP_BLE_DIR}/mac/hci/api)
include_directories(${COMP_BLE_DIR}/mac/sch/api)
include_directories(${COMP_BLE_DIR}/mac/sch/import)
include_directories(${COMP_BLE_DIR}/modules/aes/api)
include_directories(${COMP_BLE_DIR}/modules/aes/api)
include_directories(${COMP_BLE_DIR}/modules/common/api)
include_directories(${COMP_BLE_DIR}/modules/dbg/api)
include_directories(${COMP_BLE_DIR}/modules/ecc_p256/api)
include_directories(${COMP_BLE_DIR}/modules/h4tl/api)
include_directories(${COMP_BLE_DIR}/modules/ke/api)
include_directories(${COMP_BLE_DIR}/modules/lib_ver/api)
include_directories(${COMP_BLE_DIR}/modules/nvds/api)
include_directories(${COMP_BLE_DIR}/modules/rf/api)
include_directories(${COMP_BLE_DIR}/modules/rwip/api)


list(APPEND MODULE_SRC ${BLE_LIB_IMPORT_SRC})
list(APPEND MODULE_SRC ${BLE_ARCH})
list(APPEND MODULE_SRC ${BLE_PROFILE_COMMON})
list(APPEND MODULE_SRC ${BLE_APP_COMP})
###################################   MISC  ####################################
set(MISC_SRC
    ${COMP_WIFI_DIR}/wifi_manager/wifi_manager.c
    ${COMP_WIFI_DIR}/wifi_lib_import/wifi_port.c
    ${COMP_LIBC_STUB_DIR}/newlib_stub.c
)
include_directories(${COMP_WIFI_DIR}/wifi_manager)
include_directories(${COMP_WIFI_DIR}/wifi_lib_import)
include_directories(${COMP_WIFI_DIR}/wifi_lib_export)
list(APPEND MODULE_SRC ${MISC_SRC})
