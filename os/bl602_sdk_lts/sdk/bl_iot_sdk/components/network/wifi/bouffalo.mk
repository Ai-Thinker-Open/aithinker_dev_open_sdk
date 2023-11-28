COMPONENT_LIB_ONLY := 1
COMPONENT_ADD_INCLUDEDIRS += include modules/bugkiller/inc plf/refip/src/driver/phy/bl602_phy_rf
LIBS ?= wifi at_602 cloud_602
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib $(addprefix -l,$(LIBS))
ALL_LIB_FILES := $(patsubst %,$(COMPONENT_PATH)/lib/lib%.a,$(LIBS))
COMPONENT_ADD_LINKER_DEPS := $(ALL_LIB_FILES)
