ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), audio)

INCLUDES := $(INC_MINIAUDIO)

define MAKE_SUB
	$(call compile-all, engine global)
endef

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
