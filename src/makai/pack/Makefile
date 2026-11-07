ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), pack)

INCLUDES := $(INC_CRYPTOPP)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, core file)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
