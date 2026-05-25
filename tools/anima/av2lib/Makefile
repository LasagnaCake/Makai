#CC 	?= gcc
#CXX ?= g++

ROOT	:= ../../..

debug ?=0

LEAN := -s

ADDCONF := -static-libstdc++ -static-libgcc

COMPILER_CONFIG	:= -fexpensive-optimizations -m64 -std=gnu++20 -fcoroutines -fms-extensions #-DCTL_DEVMODE_DEBUG
LINKER_CONFIG	:= $(ADDCONF) -m64 -fms-extensions

ifdef openmp
USE_OPENMP := -fopenmp -openmp -ftree-parallelize-loops=128
export openmp
endif

export o?=3

OPTIMIZATIONS	:= $(USE_OPENMP) -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize

export os ?=win

include $(ROOT)/make/os.make
include $(ROOT)/make/compiler.make

INCLUDES	:= -I$(ROOT)/output/include
ifeq ($(from-lite),1)
LIBRARIES	:= $(OS_LIBS) $(LITE_BUILD_REQS)
else
LIBRARIES	:= $(OS_LIBS) $(ADDLIBS)
endif

ifeq ($(debug),1)
COMPCFG	:= $(COMPILER_CONFIG) -Og
LINKCFG	:= $(LINKER_CONFIG) -Og
INC_MAKAI:=$(ROOT)/output/lib/libmakai.debug.a
EXTRAOBJS := -L$(ROOT)/output/lib -lmakai.debug
else
COMPCFG	:= $(COMPILER_CONFIG) -O$(o) $(OPTIMIZATIONS)
LINKCFG	:= $(LINKER_CONFIG) -O$(o) $(LEAN)
INC_MAKAI:=$(ROOT)/output/lib/libmakai.a
EXTRAOBJS := -L$(ROOT)/output/lib -lmakai
endif

COMPILER	:= $(CPP_COMPILER) $(INCLUDES) -fPIC -fPIE
LINKER		:= $(CPP_COMPILER)

LIBNAME	:= archive

define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

CFG := $(COMPCFG)
LK := $(LINKCFG)
OBJ := $(EXTRAOBJS)

COMMA :=,
SPACE :=

ifeq ($(os),win)
WINFLAG := -mwindows
makelib = -Wl,--subsystem,windows
else
makelib = -Wl,-soname,$(strip $(1))
endif

.PHONY:prologue av2 libs finale
.ONESHELL:
.SHELLFLAGS = -ec

libs: prologue av2 finale

all: libs

define compile
	@echo "compiling [$(strip $(1))]"
	$(COMPILER) $(CFG) -c av2$(strip $(1)).cpp -o av2$(strip $(1)).o
	@echo "linking [$(strip $(1))]"
	$(LINKER) $(LK) -o av2$(strip $(1)).andl -shared $(makelib) av2$(strip $(1)).o $(OBJ) $(LIBRARIES)
endef

prologue:
	@echo ""
	@echo "libraries: $(LIBRARIES)"
	@echo "debug? $(debug)"

av2:
	$(call compile, console)
	$(call compile, tostring)
	$(call compile, convert)
	$(call compile, string)
	$(call compile, meta)

finale:
	@echo 
