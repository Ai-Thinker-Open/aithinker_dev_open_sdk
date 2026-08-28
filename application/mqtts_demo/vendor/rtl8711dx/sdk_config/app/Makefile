
include $(MAKE_INCLUDE_GEN)

.PHONY: all clean

#*****************************************************************************#
#                               VARIABLES	                              #
#*****************************************************************************#
#DIR = $(SRCDIR)
DIR = ../src

MODULE_IFLAGS += -I$(DIR)/
MODULE_IFLAGS += -I$(BASEDIR)/component/os/freertos/freertos_v10.4.3/Source/include/
MODULE_IFLAGS += -I$(BASEDIR)/component/os/freertos/freertos_v10.4.3/Source/portable/GCC/AmebaDplus_KM4/non_secure/
MODULE_IFLAGS += -I$(BASEDIR)/component/lwip/lwip_v2.1.2/src/include/lwip/
MODULE_IFLAGS += -I$(BASEDIR)/component/network/cJSON/

vpath %.c $(DIR)
#*****************************************************************************#
#                               Source FILE LIST                              #
#*****************************************************************************#
#CSRC = $(DIR)/main.c
CSRC := $(wildcard $(DIR)/*.c)

#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
OBJS = $(notdir $(CSRC:.c=.o))

#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#
CORE_TARGETS:
COPY_RAM_OBJS: CORE_TARGETS
all: COPY_RAM_OBJS
	
#*****************************************************************************#
#              GENERATE OBJECT FILE 
#*****************************************************************************#
CORE_TARGETS: $(OBJS)

#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean:
	$(call CLEAN_FUNC)

-include $(DEPS)	
