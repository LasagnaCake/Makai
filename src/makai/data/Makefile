ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), data)

INCLUDES := $(INC_CRYPTOPP)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, encdec hash hmac totp)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
