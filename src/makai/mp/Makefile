ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), mp)

INCLUDES := $(INC_OPENCL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call submake-all, ocl)
endef

all: debug release

debug:
	$(MAKE_SUB)


release:
	$(MAKE_SUB)
