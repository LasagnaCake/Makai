ROOT := ../../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), tc)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call submake-all, assembler)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
