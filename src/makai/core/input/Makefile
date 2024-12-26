ROOT := ../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), in)

INCLUDES := $(INC_SDL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, manager)

release:
	$(call compile, manager)
