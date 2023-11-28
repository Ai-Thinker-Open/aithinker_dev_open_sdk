cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

sdk_add_include_directories(.)
sdk_add_include_directories(src)

add_definitions(-DCONFIG_EASYFLASH4=1)
add_definitions(-DCONFIG_BFLB_MTD=1)
add_definitions(-DCONFIG_PARTITION=1)

aux_source_directory(./src SRC_FILES)
list(REMOVE_ITEM SRC_FILES "./src/aiio_main_init.c")


target_sources(app PRIVATE
    ${SRC_FILES}
)

sdk_set_main_file(src/aiio_main_init.c)

project(dev_project)
