cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

sdk_add_include_directories(.)
sdk_add_include_directories(src)

aux_source_directory(./src SRC_FILES)
list(REMOVE_ITEM SRC_FILES "./src/aiio_main_init.c")

file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/demos/*")
file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/demos/rtos/*")

target_sources(app PRIVATE ${sources})

target_sources(app PRIVATE
${SRC_FILES}
)

# User
sdk_add_compile_definitions(-DCONFIG_CLI_CMD_ENABLE)
#sdk_add_compile_definitions(-DBL616_DHCP_DEBUG)
# sdk_add_compile_definitions(-DSIMU_DATA_TEST)
sdk_add_compile_definitions(-DCONFIG_PSRAM_COPY_CODE)
sdk_add_compile_definitions(-DCONFIG_USB_EHCI_ISO)

# target_sources(app PRIVATE rtos/bl_cam_frame.c video/jpeg_head.c video/mjpeg.c video/dbi_disp.c video/pbuff_dvp.c web/mlwip_https.c)

if(CONFIG_CHERRYUSB)
# add_subdirectory(${BL_SDK_BASE}/components/usb/usbh_uvc_uac usbh_uvc_uac)
# sdk_add_static_library(libusbh_uvc_uac.a libhibooster.a)
sdk_add_static_library(libusbh_uvc_uac.a)
file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/demos/video/*")
endif()

sdk_set_linker_script(bl616_psram.ld)
sdk_set_main_file(src/aiio_main_init.c)

project(dev_project)
