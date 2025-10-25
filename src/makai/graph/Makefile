ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), graph)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, graph)
	$(call submake-all, gl color)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
