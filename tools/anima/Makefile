#CC 	?= gcc
#CXX ?= g++

ROOT	:= ../..

LEAN := -s

debug ?=0

ADDCONF := -static-libstdc++ -static-libgcc

COMPILER_CONFIG	:= -fexpensive-optimizations -m64 -std=gnu++20 -fcoroutines -fms-extensions $(LQQ)
LINKER_CONFIG	:= $(ADDCONF) -m64 -fms-extensions

ifdef openmp
USE_OPENMP := -fopenmp -openmp -ftree-parallelize-loops=128
export openmp
endif

export o?=3

OPTIMIZATIONS	:= $(USE_OPENMP) -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize

export os ?=win

include $(ROOT)/make/options.make
include $(ROOT)/make/core.make
include $(ROOT)/make/os.make

INCLUDES	:= -I$(ROOT)/output/include
ifeq ($(from-lite),1)
LIBRARIES	:= $(OS_LIBS) $(LITE_BUILD_REQS)
else
LIBRARIES	:= $(OS_LIBS) $(ADDLIBS)
endif

DEBUGMODE		:= -DMAKAILIB_DEBUG
DEBUGCOMPCFG	:= $(COMPILER_CONFIG) -Wall -Wpedantic -Og -ggdb3 -fno-omit-frame-pointer $(DEBUGMODE)
DEBUGLINKCFG	:= $(LINKER_CONFIG) -Og $(LIBRARIES)

COMPCFG	:= $(COMPILER_CONFIG) -O$(o) $(OPTIMIZATIONS)

COMPILER	:= $(CPP_COMPILER) $(INCLUDES)
ifeq ($(os),linux)
LINKER	:= $(CPP_COMPILER)
LINKCFG	:= $(LINKER_CONFIG) -O$(o) $(LIBRARIES) $(LEAN)
else
LINKER	:= $(CPP_COMPILER)
LINKCFG	:= $(LINKER_CONFIG) $(LEAN) -O$(o) $(LIBRARIES) $(LEAN)
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
dynlib ?=dll
else
dynlib ?=so
endif

ifeq ($(do-dynlibs),1)
DYNLIB_ACTION :=av2lib
DYNLIB_COPY :=cp av2lib/*.andl ../../output/bin/av2
endif

.PHONY: debug release all tooling packer concerto minimac brevec arte av2lib mkpg finale
.ONESHELL:
.SHELLFLAGS = -ec

tooling: prologue packer brevec minimac arte mkpg $(DYNLIB_ACTION) finale # concerto

all: tooling

define compile
	@echo "[$(strip $(1))]"
	$(COMPILER) $(CFG) -c $(strip $(1)).cpp -o $(strip $(1)).o
	$(LINKER) -o $(strip $(1))$(EXEC_TYPE) $(strip $(1)).o $(OBJ) $(LK)
endef

prologue:
	@echo ""
	@echo "libraries: $(LIBRARIES)"
	@echo "debug? $(debug)"

packer:
	$(call compile, $@)

concerto:
	$(call compile, $@)

minimac:
	$(call compile, $@)

brevec:
	$(call compile, $@)

mkpg:
	$(call compile, $@)

arte:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c art.cpp -o art.o
	$(LINKER) -o art$(EXEC_TYPE) art.o $(OBJ) $(LK)

av2lib:
	@echo "[$@]"
	@cd $@
	$(GNU_MAKE) from-lite=$(lite) dynlib=$(dynlib) debug=$(debug) o=$(o)
	@cd ../..

mk-push:
	cp art$(EXEC_TYPE) ../../output/bin
	cp minimac$(EXEC_TYPE) ../../output/bin
	cp brevec$(EXEC_TYPE) ../../output/bin
#	cp concerto$(EXEC_TYPE) ../../output/bin
	cp packer$(EXEC_TYPE) ../../output/bin
	cp mkpg$(EXEC_TYPE) ../../output/bin
	mkdir -p ../../output/bin/av2
	$(DYNLIB_COPY)

finale:
	@echo 
