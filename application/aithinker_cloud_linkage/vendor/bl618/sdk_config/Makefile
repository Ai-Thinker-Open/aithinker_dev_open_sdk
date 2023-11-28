SDK_DEMO_PATH ?= .
BL_SDK_BASE ?= $(SDK_DEMO_PATH)/../..

NOW_REAL_PATH:= $(realpath ./)
CP_CUR_DIR := $(shell cp ./board $(NOW_REAL_PATH) -rf)

export BL_SDK_BASE


$(CP_CUR_DIR)

CHIP ?= bl616
# BOARD ?= bl616dk
BOARD ?= board
BOARD_DIR ?= $(NOW_REAL_PATH)
CROSS_COMPILE ?= riscv64-unknown-elf-

makefile_dirs += ./bflb_block_pool
makefile_dirs += ./bflog/barebone_sync
makefile_dirs += ./bflog/freertos_async
makefile_dirs += ./coremark_v1.01
makefile_dirs += ./freertos
makefile_dirs += ./helloworld
makefile_dirs += ./mbedtls
makefile_dirs += ./memheap
makefile_dirs += ./partition
makefile_dirs += ./posix/freertos
makefile_dirs += ./printf
makefile_dirs += ./shell/shell_no_os
makefile_dirs += ./shell/shell_os

ifeq ($(CHIP),bl602)
filter_out_dirs += ./peripherals/csi/%
filter_out_dirs += ./peripherals/emac/%
filter_out_dirs += ./peripherals/pwm_v2/%
filter_out_dirs += ./peripherals/usbdev/%

endif

ifeq ($(CHIP),bl702)
filter_out_dirs += ./peripherals/csi/%
filter_out_dirs += ./peripherals/emac/%
filter_out_dirs += ./peripherals/pwm_v2/%
endif

ifeq ($(CHIP),bl702L)
filter_out_dirs += ./peripherals/csi/%
filter_out_dirs += ./peripherals/emac/%
filter_out_dirs += ./peripherals/pwm_v2/%
filter_out_dirs += ./peripherals/usbdev/%
endif

ifeq ($(CHIP),bl616)
filter_out_dirs += ./peripherals/csi/%
filter_out_dirs += ./peripherals/pwm_v1/%
# makefile_dirs += ./fatfs
# makefile_dirs += ./littlefs/kv_test
makefile_dirs += ./lvgl
makefile_dirs += ./lua
# makefile_dirs += ./btble/ble_mesh_cli
makefile_dirs += ./btble/peripheral
endif

ifeq ($(CHIP),bl808)
filter_out_dirs += ./peripherals/pwm_v1/%
makefile_dirs += ./fatfs
makefile_dirs += ./littlefs/kv_test
makefile_dirs += ./lvgl
makefile_dirs += ./lua
endif


# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(BL_SDK_BASE)/project.build
