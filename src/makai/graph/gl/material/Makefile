ROOT := ../../../../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), mat)

INCLUDES := $(INC_SDL) $(INC_OPENGL)

define MAKE_SUB
	$(call compile-all, effect core)
endef

all: debug release

debug:
	$(MAKE_SUB)

release:
	$(MAKE_SUB)
