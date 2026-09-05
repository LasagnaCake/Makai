ROOT := ../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), ocl)

INCLUDES := $(INC_OPENCL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, context program image)
endef

all: debug release

debug:
	$(MAKE_SUB)


release:
	$(MAKE_SUB)
