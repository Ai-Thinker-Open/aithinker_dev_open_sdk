# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += \
	src \
    src/application \
    . \

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

COMPONENT_SRCS := \
    src/fhost.c \
    src/fhost_config.c \
    src/fhost_cntrl.c \
    src/fhost_rx.c \
    src/fhost_tx.c \
    src/fhost_wpa.c \
    src/application/fhost_iperf.c \
    src/application/fhost_connect.c \
    src/application/wifi_mgmr_cli.c \
    src/application/wifi_mgmr_ext.c \
    src/application/wifi_mgmr.c \


COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(COMPONENT_OBJS))

COMPONENT_SRCDIRS := \
    src/application \
    src \
    . \

##
#CPPFLAGS +=
#CFLAGS += -DCONFIG_BLE_AT_CMD
CFLAGS += -DIOT_SDK_ADAPTER

ifeq ($(CONFIG_FHOST_PE_OPT),1)
CFLAGS += -msave-restore
endif
