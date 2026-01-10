ROOT := ../../../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), asm)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, minima semibreve breve)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
