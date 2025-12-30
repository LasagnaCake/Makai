#CC 	?= gcc
#CXX ?= g++

ROOT	:= ../..

LEAN := -static -s

COMPILER_CONFIG	:= -fexpensive-optimizations -m64 -std=gnu++20 -fcoroutines -fms-extensions
LINKER_CONFIG	:= -static-libstdc++ -static-libgcc -m64 -fms-extensions

ifdef openmp
USE_OPENMP := -fopenmp -openmp -ftree-parallelize-loops=128
export openmp
endif

export o?=3

OPTIMIZATIONS	:= $(USE_OPENMP) -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize $(LEAN)

sinclude $(ROOT)/make/os-windows.make

INCLUDES	:= -I$(ROOT)/lib/cryptopp/include -I$(ROOT)/lib/cppcodec-0.2 -I$(ROOT)/output/include
LIBRARIES	:= $(OS_LIBS)

DEBUGMODE		:= 
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

.PHONY: debug release all tooling packer concerto minimac finale
.ONESHELL:
.SHELLFLAGS = -ec

tooling: prologue packer concerto minimac finale

all: tooling

prologue:
	@echo ""

packer:
	@echo "[$@]"
	$(COMPILER) $(COMPCFG) -c packer.cpp -o packer.o
	$(LINKER) -o packer.exe packer.o $(DEBUGEXTRAOBJS) $(LINKCFG)

concerto:
	@echo "[$@]"
	$(COMPILER) $(COMPCFG) -c concerto.cpp -o concerto.o
	$(LINKER) -o concerto.exe concerto.o $(DEBUGEXTRAOBJS) $(LINKCFG)

minimac:
	@echo "[$@]"
	$(COMPILER) $(COMPCFG) -c minimac.cpp -o minimac.o
	$(LINKER) -o minimac.exe minimac.o $(DEBUGEXTRAOBJS) $(LINKCFG)

finale:
	@echo 