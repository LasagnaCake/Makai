ROOT := ../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), v2)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call submake-all, runtime toolchain)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
