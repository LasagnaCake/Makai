ROOT := ../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), in)

INCLUDES := $(INC_SDL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, manager)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
