cmake_minimum_required(VERSION  3.10)

if (DEFINED LN_SDK_ROOT)
    message(STATUS      "<GET> LN_SDK_ROOT = ${LN_SDK_ROOT}")
    if (NOT  ${LN_SDK_ROOT}  STREQUAL  ${CMAKE_CURRENT_SOURCE_DIR})
        set(ERR_MSG "--------------------------------------------------------\n"
                    "LN_SDK_ROOT expects to be current directory:            \n"
                    "LN_SDK_ROOT = ${LN_SDK_ROOT}                            \n"
                    "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}  \n"
                    "--------------------------------------------------------\n")
        message(FATAL_ERROR "${ERR_MSG}")
    endif()
else()
    set(LN_SDK_ROOT     ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS      "<SET> LN_SDK_ROOT = ${LN_SDK_ROOT}")
endif()

if(DEFINED CHIP_SERIAL)
    message(STATUS      "<GET> CHIP_SERIAL = ${CHIP_SERIAL}")
else()
    set(CHIP_SERIAL     ln882h)
    message(STATUS      "<SET> CHIP_SERIAL = ${CHIP_SERIAL}")
endif()

include_directories(
	project/combo_mcu_at_example/app
	project/combo_mcu_at_example/bsp
	components/ln_at_cmd/tcpip
	mcu/driver_ln882h
	mcu/driver_ln882h/reg
	components/serial
	components/ln_at/transfer
	components/ln_at
	components/ln_at/adapter
	components/ln_at/cmd
	components/ln_at/parser
	components/wifi/wifi_lib_import
	components/utils
	components/fs/nvds
	mcu/CMSIS_5.3.0
	mcu/ln882h
	components/utils/debug
	project/combo_mcu_at_example/startup
)


################################################################################
## NOTE: no need to setup toolchain if LN SDK is used as git submodule.
##       It's the git main module's duty to setup toolchain.
################################################################################
if(NOT DEFINED VENDOR_SDK_ADAPTED)
    ############################################################################
    #######################   NOTE: select user project here  ##################
    ############################################################################
    if(NOT DEFINED USER_PROJECT)
        #set(USER_PROJECT  wifi_mcu_basic_example)
        #set(USER_PROJECT  ble_mcu_data_trans)
        #set(USER_PROJECT  ble_mcu_prf_hid_mouse)
        #set(USER_PROJECT  ble_mcu_prf_battery)
        #set(USER_PROJECT  ble_mcu_scan)
        set(USER_PROJECT   dev_project)
        #set(USER_PROJECT  mcu_peripheral_driver_test)
        message(STATUS "<SET> USER_PROJECT = ${USER_PROJECT}")
    else()
        message(STATUS "<GET> USER_PROJECT= ${USER_PROJECT}")
    endif()

    ############################################################################
    ############################  Debug/Release  ###############################
    ############################################################################
    set(CMAKE_BUILD_TYPE  Release CACHE  STRING  "build for release"    FORCE)
    # set(CMAKE_BUILD_TYPE  Debug   CACHE  STRING  "build for debug"      FORCE)

    set(allowableBuildTypes  Debug  Release)
    if (NOT CMAKE_BUILD_TYPE IN_LIST  allowableBuildTypes)
        set(CMAKE_BUILD_TYPE  Debug   CACHE  STRING  "build for debug"      FORCE)
    endif()
    message(STATUS  "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

    ############################################################################
    #######################  set up cross compiler toolchain  ##################
    ############################################################################
    include(${LN_SDK_ROOT}/project/${USER_PROJECT}/gcc/gcc-toolchain-setup.cmake)
    include(${LN_SDK_ROOT}/project/${USER_PROJECT}/gcc/gcc-compiler-flags.cmake)

    ############################################################################
    #########################  project language selection   ####################
    ############################################################################
    project(LN-Combo-SDK
        VERSION    1.0.0
        LANGUAGES  C  CXX  ASM
    )

    ############################################################################
    ###########################   misc settings   ##############################
    ############################################################################

    set(CMAKE_VERBOSE_MAKEFILE    ON)

    #-----------------------------   output path   -----------------------------
    set(EXECUTABLE_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/bin)
    set(LIBRARY_OUTPUT_PATH       ${CMAKE_BINARY_DIR}/lib)
else()
    message(STATUS "----------------------------------------------------------")
    message(STATUS "-----------  LN SDK Configured by third_party  -----------")
    message(STATUS "----------------------------------------------------------")
    set(USER_PROJECT  main_entry_for_tuya)
endif()

################################################################################
##########################   default module selection   ########################
#-------------------------- library built within LN software team  -------------
option(COMP_WIFI_SUPPORT        "LN WiFi firmware."                         OFF)
option(COMP_BLE_SUPPORT         "LN BLE  firmware."                         OFF)

#----------------------------  library  choosen by customer  -------------------
option(COMP_DHCPD_SUPPORT       "DHCP client."                              OFF)
option(COMP_LWIP_SUPPORT        "Light weight TCP/IP stack."                OFF)

################################################################################
###########################   define module top dir   ##########################
################################################################################
set(MCU_LN882X_DIR              ${LN_SDK_ROOT}/mcu)
set(COMP_BLE_DIR                ${LN_SDK_ROOT}/components/ble)
set(COMP_FOTA_DIR               ${LN_SDK_ROOT}/components/fota)
set(COMP_KV_DIR                 ${LN_SDK_ROOT}/components/fs/kv)
set(COMP_NVDS_DIR               ${LN_SDK_ROOT}/components/fs/nvds)
set(COMP_PARTMGR_DIR            ${LN_SDK_ROOT}/components/fs/partition_mgr)
set(COMP_KERNEL_DIR             ${LN_SDK_ROOT}/components/kernel)
set(COMP_LNAT_DIR               ${LN_SDK_ROOT}/components/ln_at)
set(COMP_LNAT_CMD_DIR           ${LN_SDK_ROOT}/components/ln_at_cmd)
set(COMP_DHCPD_DIR              ${LN_SDK_ROOT}/components/net/dhcpd)
set(COMP_IPERF_DIR              ${LN_SDK_ROOT}/components/net/iperf)
set(COMP_PING_DIR               ${LN_SDK_ROOT}/components/net/ping)
set(COMP_HTTPSCLIENT_DIR        ${LN_SDK_ROOT}/components/net/httpsclient)
set(COMP_MBEDTLS_DIR            ${LN_SDK_ROOT}/components/net/mbedtls)
set(COMP_SERIAL_DIR             ${LN_SDK_ROOT}/components/serial)
set(COMP_UTILS_DIR              ${LN_SDK_ROOT}/components/utils)
set(COMP_LIBC_STUB_DIR          ${LN_SDK_ROOT}/components/libc)
set(COMP_WIFI_DIR               ${LN_SDK_ROOT}/components/wifi)
set(COMP_SMARTLINK_DIR          ${LN_SDK_ROOT}/components/smartlink)

if(DEFINED THIRD_PARTY_LWIP AND (THIRD_PARTY_LWIP))
    message(STATUS "LwIP: third-party provided.")
else()
    message(STATUS "LwIP: LN SDK provided.")
    set(COMP_LWIP_DIR           ${LN_SDK_ROOT}/components/net/lwip-2.1.3/src)
endif()

################################################################################
#############################    user project    ###############################
################################################################################
add_subdirectory(${LN_SDK_ROOT}/project/${USER_PROJECT})
add_subdirectory(components)
