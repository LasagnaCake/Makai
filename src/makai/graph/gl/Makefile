ROOT := ../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), gl)

INCLUDES := $(INC_SDL) $(INC_STB) $(INC_OPENGL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, core global vertex blend image texture)
	$(call submake-all, shader renderer material)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
