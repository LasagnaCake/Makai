ROOT := ../../..
include $(ROOT)/make/core.make

INCLUDES := $(INC_STB) $(INC_CUTE) $(INC_SDL) $(INC_MINIAUDIO)

override prefix := $(call concat, $(prefix), impl)

ifndef gl-loader
gl-loader :=glad
endif

CONFIG_C			:= -fexpensive-optimizations -m64 -fms-extensions
DEBUG_CONFIG_C		:= $(CONFIG_C) -Wall -Og -ggdb3 -fno-omit-frame-pointer $(DEBUGMODE)
RELEASE_CONFIG_C	:= $(CONFIG_C) -O$(o) $(RELEASEMODE) $(MATHMODE)

MAKE_GLOADER_D	= $(CPP_COMPILER) $(INC_OPENGL) $(DEBUG_CONFIG_C) -c $(gl-loader).c -o $(prefix).glapi.$@.o
MAKE_GLOADER	= $(CPP_COMPILER) $(INC_OPENGL) $(RELEASE_CONFIG_C) -c $(gl-loader).c -o $(prefix).glapi.$@.o

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	@$(MAKE_GLOADER_D)
	$(call compile, stb)
	$(call compile, cute)
	$(call compile, miniaudio)


release:
	@$(MAKE_GLOADER)
	$(call compile, stb)
	$(call compile, cute)
	$(call compile, miniaudio)
