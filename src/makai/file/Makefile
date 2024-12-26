ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), file)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

# compile-get = $(call compile-with-o, get, 1)

debug:
	$(call compile, json)
	$(call compile, slf)
	$(call compile, get)

release:
	$(call compile, json)
	$(call compile, slf)
	$(call compile, get)
