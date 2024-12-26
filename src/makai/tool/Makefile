ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), tool)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call submake, archive)


release:
	$(call submake, archive)
