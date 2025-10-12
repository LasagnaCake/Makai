ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), lex)

INCLUDES := $(INC_SDL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call submake-all, cstyle)
endef

all: debug release

debug:
	$(MAKE_SUB)


release:
	$(MAKE_SUB)
