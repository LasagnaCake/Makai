ROOT := ../../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), ref)

INCLUDES := $(INC_SDL) $(INC_STB) $(INC_OPENGL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, arrow core line plane triangle referend patch)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
