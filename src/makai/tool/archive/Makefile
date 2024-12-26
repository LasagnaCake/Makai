ROOT := ../../../..
include $(ROOT)/make/core.make

INCLUDES := $(INC_CRYPTOPP) $(INC_CPPCODEC)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, archive)


release:
	$(call compile, archive)
