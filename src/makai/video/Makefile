ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), video)

INCLUDES := $(INC_PCRE2)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, mvsx)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
