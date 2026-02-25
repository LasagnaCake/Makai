#CC 	?= gcc
#CXX ?= g++

ROOT	:= ../..

LEAN := -s

debug := 0

ADDCONF := -static -static-libstdc++ -static-libgcc

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
LIBRARIES	:= $(OS_LIBS) $(LITE_BUILD_REQS)
else
LIBRARIES	:= $(OS_LIBS)
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
LINKER		:= $(CXX)
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

.PHONY: debug release all tooling packer concerto minimac arte finale
.ONESHELL:
.SHELLFLAGS = -ec

tooling: prologue packer concerto arte minimac finale

all: tooling

prologue:
	@echo ""
	@echo "libraries: $(LIBRARIES)"

packer:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c packer.cpp -o packer.o
	$(LINKER) -o packer$(EXEC_TYPE) packer.o $(OBJ) $(LK)

concerto:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c concerto.cpp -o concerto.o
	$(LINKER) -o concerto$(EXEC_TYPE) concerto.o $(OBJ) $(LK)

minimac:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c minimac.cpp -o minimac.o
	$(LINKER) -o minimac$(EXEC_TYPE) minimac.o $(OBJ) $(LK)

arte:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c arte.cpp -o art.o -DARTE_CLI
	$(COMPILER) $(CFG) -c arte.cpp -o artw.o -DARTE_NO_CLI $(WINFLAG)
	$(LINKER) -o art$(EXEC_TYPE) art.o $(OBJ) $(LK)
	$(LINKER) -o artw$(EXEC_TYPE) artw.o $(OBJ)$(LK) $(WINFLAG)

mk-push:
	cp art$(EXEC_TYPE) ../../output/bin
	cp artw$(EXEC_TYPE) ../../output/bin
	cp minimac$(EXEC_TYPE) ../../output/bin
	cp concerto$(EXEC_TYPE) ../../output/bin
	cp packer$(EXEC_TYPE) ../../output/bin

finale:
	@echo 
