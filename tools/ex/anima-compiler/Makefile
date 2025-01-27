#CC 	?= gcc
#CXX ?= g++

LEAN := -static -s

ROOT := ../../..

DEBUGMODE := -DMAKAILIB_DEBUG -fconcepts-diagnostics-depth=4 -Wall -Wpedantic
COMPILER_CONFIG	:= $(DEBUGMODE) -m64 -std=gnu++20 -fms-extensions -fcoroutines -I$(ROOT)/output/include

LIBREQS		:= -static-libstdc++ -static-libgcc
WINREQS		:= -lole32 -loleaut32 -limm32 -lwinmm -lversion -lpowrprof -lcomdlg32 -lsetupapi -lgdi32 -lbcrypt

o?=3

lean-bean-machine:=1

ifdef lean-bean-machine
COMPILER_CONFIG += $(LEAN)
endif

PRGNAME	:= animac
SRC_DST	= -o $(PRGNAME)$(strip $(1)).exe $(PRGNAME).cpp

compile = @$(CXX) -O$(o) $(call SRC_DST, $(1)) $(COMPILER_CONFIG) $(ROOT)/output/lib/libmakai.debug.a $(LIBREQS) $(WINREQS)
compile = @$(CXX) -O$(o) $(call SRC_DST, $(1)) $(COMPILER_CONFIG) $(ROOT)/output/lib/libmakai.debug.a $(LIBREQS) $(WINREQS)

NOCMD := -mwindows

ifdef debug-release
RELEASEMODE := $(DEBUGMODE)
else
RELEASEMODE := $(NOCMD) $(LEAN)
endif

define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

.PHONY: compiler all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	@echo "Making debug..."
	$(call compile, .debug) -DMAKAILIB_EX_ANIMA_COMPILER_DEBUG_ABSOLUTELY_EVERYTHING

release:
	@echo "Making release..."
	$(call compile,)