ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), audio)

INCLUDES := $(INC_SDL)

define MAKE_SUB
	$(call compile-all, core source music sound)
endef

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
