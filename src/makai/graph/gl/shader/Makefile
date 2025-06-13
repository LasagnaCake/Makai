ROOT := ../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), sh)

INCLUDES := $(INC_SDL) $(INC_STB) $(INC_OPENGL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, core uniform)
endef

all: debug release

debug:
	$(call compile, core)
	$(call compile, uniform)

release:
	$(call compile, core)
	$(call compile, uniform)
