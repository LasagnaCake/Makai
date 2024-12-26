ROOT := ../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), core)

INCLUDES := $(INC_SDL)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, app)
	$(call compile, display)
	$(call compile, dpi)
	$(call compile, popup)

	$(call submake, input)


release:
	$(call compile, app)
	$(call compile, display)
	$(call compile, dpi)
	$(call compile, popup)

	$(call submake, input)
