# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += \
    . \

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

COMPONENT_SRCS := \
    net_al.c \
    net_al_ext.c \
    net_tg_al.c \
    net_iperf_al.c \
    wifi_pkt_hooks.c \

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := \
    . \

ifeq ($(CONFIG_WIFI_PKT_INPUT_HOOK),1)
CPPFLAGS += -D PKT_INPUT_HOOK
endif
ifeq ($(CONFIG_WIFI_PKT_OUTPUT_HOOK),1)
CPPFLAGS += -D PKT_OUTPUT_HOOK
endif

##
#CPPFLAGS +=
#CFLAGS += -DCONFIG_BLE_AT_CMD
