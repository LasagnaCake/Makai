ROOT := ../../..
include $(ROOT)/make/core.make

INCLUDES := $(INC_STB) $(INC_CUTE) $(INC_MINIAUDIO)

override prefix := $(call concat, $(prefix), impl)

CONFIG_C			:= -fexpensive-optimizations -m64 -fms-extensions
DEBUG_CONFIG_C		:= $(CONFIG_C) -Wall -Og -ggdb3 -fno-omit-frame-pointer $(DEBUGMODE)
RELEASE_CONFIG_C	:= $(CONFIG_C) -O$(o) $(RELEASEMODE) $(MATHMODE)

MAKE_GLOADER_D	= @$(CPP_COMPILER) $(INC_OPENGL) $(DEBUG_CONFIG_C) -c $(gl-loader).c -o $(prefix).glapi.$@.o
MAKE_GLOADER	= @$(CPP_COMPILER) $(INC_OPENGL) $(RELEASE_CONFIG_C) -c $(gl-loader).c -o $(prefix).glapi.$@.o

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	$(call compile, stb)
	$(call compile, cute)
	$(call compile, miniaudio)
	$(MAKE_GLOADER_D)


release:
	$(call compile, stb)
	$(call compile, cute)
	$(call compile, miniaudio)
	$(MAKE_GLOADER)
