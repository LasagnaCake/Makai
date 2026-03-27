ROOT := ../../../../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), bv)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, node resolver parser transformer intermediate)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
