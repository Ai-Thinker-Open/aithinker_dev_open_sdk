cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

sdk_set_linker_script(bl616_flash.ld)
sdk_set_main_file(main.c)

project(sec_eng_aes_sw_key_costtime)
