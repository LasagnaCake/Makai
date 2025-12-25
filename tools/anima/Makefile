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

DEBUGMODE		:= -DMAKAILIB_DEBUG -DCTL_CONSOLE_OUT -DNDEBUG
DEBUGCOMPCFG	:= $(COMPILER_CONFIG) -Wall -Wpedantic -Og -ggdb3 -fno-omit-frame-pointer $(DEBUGMODE)
DEBUGLINKCFG	:= $(LINKER_CONFIG) -Og $(LIBRARIES)

COMPCFG	:= $(COMPILER_CONFIG) -O$(o) $(OPTIMIZATIONS) $(DEBUGMODE)
LINKCFG	:= $(LINKER_CONFIG) -s -O$(o) $(LIBRARIES) $(LEAN)

COMPILER	:= @$(CXX) $(INCLUDES)
LINKER		:= @$(CXX)

DEBUGEXTRAOBJS := $(ROOT)/output/lib/libmakai.debug.a
EXTRAOBJS := $(ROOT)/output/lib/libmakai.debug.a

LIBNAME	:= archive

define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: compiler

compiler:
	@echo ""
	@echo "[--- START ---]"
	$(GET_TIME)
	
	@echo "[1/2] compiling [$@]..."
	$(COMPILER) $(COMPCFG) -c packer.cpp -o packer.o
	$(COMPILER) $(COMPCFG) -c concerto.cpp -o concerto.o
	$(COMPILER) $(COMPCFG) -c minimac.cpp -o minimac.o

	$(LINKER) -o packer.exe packer.o $(DEBUGEXTRAOBJS) $(LINKCFG)
	$(LINKER) -o concerto.exe concerto.o $(DEBUGEXTRAOBJS) $(LINKCFG)
	$(LINKER) -o minimac.exe minimac.o $(DEBUGEXTRAOBJS) $(LINKCFG)

	@echo "Done!"
	
	$(GET_TIME)
	@echo "[--- END ---]"
	@echo 