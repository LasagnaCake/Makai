ROOT := ../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), rd)

INCLUDES := $(INC_SDL) $(INC_STB) $(INC_OPENGL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, mode)
	$(call compile, core)
	$(call compile, drawable)
	$(call compile, reference)
	$(call compile, renderable)
	$(call compile, bar)
	$(call compile, text)
	$(call compile, scene)
	$(call compile, framebuffer)

release:
	$(call compile, mode)
	$(call compile, core)
	$(call compile, drawable)
	$(call compile, reference)
	$(call compile, renderable)
	$(call compile, bar)
	$(call compile, text)
	$(call compile, scene)
	$(call compile, framebuffer)
