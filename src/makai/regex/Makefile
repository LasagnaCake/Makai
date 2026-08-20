ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), re)

INCLUDES := $(INC_CURL) $(INC_SDL_NET)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, engine handler)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
