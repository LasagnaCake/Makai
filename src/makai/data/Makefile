ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), data)

INCLUDES := $(INC_CPPCODEC) $(INC_CRYPTOPP)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, encdec)
	$(call compile, hash)

release:
	$(call compile, encdec)
	$(call compile, hash)
