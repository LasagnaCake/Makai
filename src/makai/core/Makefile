ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), core)

INCLUDES := $(INC_SDL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, app display dpi popup)
	$(call submake-all, input)
endef

all: debug release

debug:
	$(MAKE_SUB)


release:
	$(MAKE_SUB)
