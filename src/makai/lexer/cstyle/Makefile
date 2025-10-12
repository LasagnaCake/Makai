ROOT := ../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), cstyle)

INCLUDES := $(INC_STB)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, tokenstream)
endef

all: debug release

debug:
	$(MAKE_SUB)


release:
	$(MAKE_SUB)
