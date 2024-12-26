ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), audio)

INCLUDES := $(INC_SDL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, core)
	$(call compile, source)
	$(call compile, music)
	$(call compile, sound)

release:
	$(call compile, core)
	$(call compile, source)
	$(call compile, music)
	$(call compile, sound)
