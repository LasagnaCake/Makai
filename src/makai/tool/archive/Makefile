ROOT := ../../../..
include $(ROOT)/make/core.make

INCLUDES := $(INC_CRYPTOPP) $(INC_CPPCODEC)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, archive)
endef

all: debug release

debug:
	$(call compile, archive)


release:
	$(call compile, archive)
