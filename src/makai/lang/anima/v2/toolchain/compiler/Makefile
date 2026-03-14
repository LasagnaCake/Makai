ROOT := ../../../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), comp)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, core breve parser resolver)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
