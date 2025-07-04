ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), tool)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call submake-all, archive)
endef

all: debug release

debug:
	$(call submake, archive)


release:
	$(call submake, archive)
