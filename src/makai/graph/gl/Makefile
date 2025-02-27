ROOT := ../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), graph.gl)

INCLUDES := $(INC_SDL) $(INC_STB) $(INC_OPENGL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, core)
	$(call compile, global)
	$(call compile, vertex)
	$(call compile, blend)
	$(call compile, color)
	$(call compile, image)
	$(call compile, texture)

	$(call submake, shader)
	$(call submake, renderer)
	$(call submake, material)

release:
	$(call compile, core)
	$(call compile, global)
	$(call compile, vertex)
	$(call compile, blend)
	$(call compile, color)
	$(call compile, image)
	$(call compile, texture)

	$(call submake, shader)
	$(call submake, renderer)
	$(call submake, material)
