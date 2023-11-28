SDK_DEMO_PATH ?= .
BL_SDK_BASE ?= $(SDK_DEMO_PATH)/../..

export BL_SDK_BASE

NOW_REAL_PATH:= $(realpath ./)
CP_CUR_DIR := $(shell cp ./bl616vtb $(NOW_REAL_PATH) -rf)


CHIP ?= bl616
BOARD ?= bl616vtb
CROSS_COMPILE ?= riscv64-unknown-elf-
BOARD_DIR = ${NOW_REAL_PATH}

# add custom cmake definition
#cmake_definition+=-Dxxx=sss
include $(BL_SDK_BASE)/project.build
