ROOT := ../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), rd)

INCLUDES := $(INC_SDL) $(INC_STB) $(INC_OPENGL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, mode core drawable reference renderable bar text scene framebuffer)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
