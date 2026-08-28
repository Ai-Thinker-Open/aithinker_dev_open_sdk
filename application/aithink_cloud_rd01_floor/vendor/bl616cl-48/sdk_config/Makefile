SDK_DEMO_PATH ?= $(abspath .)
BL_SDK_BASE ?= $(SDK_DEMO_PATH)/../..

export BL_SDK_BASE

CHIP ?= bl616cl
BOARD ?= bl616cldk
CROSS_COMPILE ?= riscv64-unknown-elf-

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(BL_SDK_BASE)/project.build
