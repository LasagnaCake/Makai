#CC 	?= gcc
#CXX ?= g++

ROOT	:= ../../..

LEAN := -s

debug := 0

ADDCONF := -static-libstdc++ -static-libgcc

COMPILER_CONFIG	:= -fexpensive-optimizations -m64 -std=gnu++20 -fcoroutines -fms-extensions $(LQQ)
LINKER_CONFIG	:= $(ADDCONF) -m64 -fms-extensions

ifdef openmp
USE_OPENMP := -fopenmp -openmp -ftree-parallelize-loops=128
export openmp
endif

export o?=3

OPTIMIZATIONS	:= $(USE_OPENMP) -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize $(LEAN)

export os ?=win

sinclude $(ROOT)/make/os.make

INCLUDES	:= -I$(ROOT)/output/include
ifeq ($(from-lite),1)
LIBRARIES	:= $(OS_LIBS) $(ADDLIBS) $(LITE_BUILD_REQS)
else
LIBRARIES	:= $(OS_LIBS) $(ADDLIBS)
endif

DEBUGMODE		:= -DMAKAILIB_DEBUG
DEBUGCOMPCFG	:= $(COMPILER_CONFIG) -Wall -Wpedantic -Og -ggdb3 -fno-omit-frame-pointer $(DEBUGMODE)
DEBUGLINKCFG	:= $(LINKER_CONFIG) -Og $(LIBRARIES)

COMPCFG	:= $(COMPILER_CONFIG) -O$(o) $(OPTIMIZATIONS)
LINKCFG	:= $(LINKER_CONFIG) -s -O$(o) $(LEAN) $(LIBRARIES)

COMPILER	:= $(CXX) $(INCLUDES)
ifeq ($(os),linux)
LINKER		:= $(CXX)
else
LINKER		:= $(CXX) -static
endif

DEBUGEXTRAOBJS := $(ROOT)/output/lib/libmakai.debug.a
EXTRAOBJS := $(ROOT)/output/lib/libmakai.a

LIBNAME	:= archive

define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

ifeq ($(debug),1)
CFG := $(DEBUGCOMPCFG)
LK := $(DEBUGLINKCFG)
OBJ := $(DEBUGEXTRAOBJS)
else
CFG := $(COMPCFG)
LK := $(LINKCFG)
OBJ := $(EXTRAOBJS)
endif

ifeq ($(os),win)
WINFLAG := -mwindows
endif

.PHONY: debug release all tooling packer concerto minimac brevec arte finale
.ONESHELL:
.SHELLFLAGS = -ec

tooling: prologue av2 finale

all: tooling

define compile
	@echo "[$(strip $(1))]"
	$(COMPILER) $(CFG) -c av2$(strip $(1)).cpp -o av2$(strip $(1)).o
	$(LINKER) -o av2$(strip $(1)).dll av2$(strip $(1)).o $(OBJ) $(LK)
endef

prologue:
	@echo ""
	@echo "libraries: $(LIBRARIES)"
	@echo "debug? $(debug)"

av2:
	$(call compile, io)
	$(call compile, tostring)

finale:
	@echo 
