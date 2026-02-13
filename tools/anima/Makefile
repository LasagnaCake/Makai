#CC 	?= gcc
#CXX ?= g++

ROOT	:= ../..

LEAN := -static -s

debug := 0

COMPILER_CONFIG	:= -fexpensive-optimizations -m64 -std=gnu++20 -fcoroutines -fms-extensions
LINKER_CONFIG	:= -static-libstdc++ -static-libgcc -m64 -fms-extensions

ifdef openmp
USE_OPENMP := -fopenmp -openmp -ftree-parallelize-loops=128
export openmp
endif

export o?=3

OPTIMIZATIONS	:= $(USE_OPENMP) -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize $(LEAN)

sinclude $(ROOT)/make/os-windows.make

INCLUDES	:= -I$(ROOT)/output/include
LIBRARIES	:= $(OS_LIBS)

DEBUGMODE		:= -DMAKAILIB_DEBUG
DEBUGCOMPCFG	:= $(COMPILER_CONFIG) -Wall -Wpedantic -Og -ggdb3 -fno-omit-frame-pointer $(DEBUGMODE)
DEBUGLINKCFG	:= $(LINKER_CONFIG) -Og $(LIBRARIES)

COMPCFG	:= $(COMPILER_CONFIG) -O$(o) $(OPTIMIZATIONS) $(DEBUGMODE)
LINKCFG	:= $(LINKER_CONFIG) -s -O$(o) $(LIBRARIES) $(LEAN)

COMPILER	:= @$(CXX) $(INCLUDES)
LINKER		:= @$(CXX)

DEBUGEXTRAOBJS := $(ROOT)/output/lib/libmakai.debug.a
EXTRAOBJS := $(ROOT)/output/lib/libmakai.a

LIBNAME	:= archive

define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

ifdef debug
CFG := $(DEBUGCOMPCFG)
LK := $(DEBUGLINKCFG)
OBJ := $(DEBUGEXTRAOBJS)
else
CFG := $(COMPCFG)
LK := $(LINKCFG)
OBJ := $(EXTRAOBJS)
endif

.PHONY: debug release all tooling packer concerto minimac finale
.ONESHELL:
.SHELLFLAGS = -ec

tooling: prologue packer concerto minimac arte anima finale

all: tooling

prologue:
	@echo ""

packer:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c packer.cpp -o packer.o
	$(LINKER) -o packer.exe packer.o $(OBJ) $(LK)

concerto:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c concerto.cpp -o concerto.o
	$(LINKER) -o concerto.exe concerto.o $(OBJ) $(LK)

minimac:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c minimac.cpp -o minimac.o
	$(LINKER) -o minimac.exe minimac.o $(OBJ) $(LK)

arte:
	@echo "[$@]"
	$(COMPILER) $(COMPCFG) -c arte.cpp -o carte.o -DARTE_CLI -mwindows
	$(COMPILER) $(COMPCFG) -c arte.cpp -o warte.o -DARTE_NO_CLI
	$(LINKER) -o carte.exe carte.o $(OBJ) $(LINKCFG) -mwindows
	$(LINKER) -o warte.exe warte.o $(OBJ) $(LINKCFG)

anima:
	$(COMPILER) $(COMPCFG) -c anima.cpp -o anima.o
	$(LINKER) -o anima.exe anima.o $(OBJ) $(LINKCFG)

finale:
	@echo 
