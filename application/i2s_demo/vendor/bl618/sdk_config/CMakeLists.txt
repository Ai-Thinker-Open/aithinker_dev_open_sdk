cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

add_definitions(-DCONFIG_AIIO_I2S=1)
sdk_add_include_directories(.)
sdk_add_include_directories(src)

aux_source_directory(./src SRC_FILES)
list(REMOVE_ITEM SRC_FILES "./src/aiio_main_init.c")


target_sources(app PRIVATE
    ${SRC_FILES}
)

sdk_set_main_file(src/aiio_main_init.c)

project(dev_project)
