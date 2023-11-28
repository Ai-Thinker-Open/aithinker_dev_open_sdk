cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

sdk_set_main_file(test/test_runners/main.c)

add_subdirectory(${BL_SDK_BASE}/tests tests)
add_subdirectory(test)

project(gpio)
