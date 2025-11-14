ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), file)

INCLUDES := $(INC_XML2JSON) $(INC_JSON2XML)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call compile-all, json flow xml slf get)
endef

all: debug release

# compile-get = $(call compile-with-o, get, 1)

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
