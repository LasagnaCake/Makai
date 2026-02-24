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
LINKCFG	:= $(LINKER_CONFIG) -s -O$(o) $(LIBRARIES) $(LEAN)

COMPILER	:= $(CXX) $(INCLUDES)
LINKER		:= $(CXX)

DEBUGEXTRAOBJS := -L$(ROOT)/output/lib/libmakai.debug.a
EXTRAOBJS := -L$(ROOT)/output/lib/libmakai.a

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

.PHONY: debug release all tooling packer concerto minimac finale
.ONESHELL:
.SHELLFLAGS = -ec

tooling: prologue packer concerto minimac arte anima finale

all: tooling

prologue:
	@echo ""
	@echo "libraries: $(LIBRARIES)"

packer:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c packer.cpp -o packer.o
	$(LINKER) $(LK) $(OBJ) -o packer$(EXEC_TYPE) packer.o

concerto:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c concerto.cpp -o concerto.o
	$(LINKER) $(LK) $(OBJ) -o concerto$(EXEC_TYPE) concerto.o

minimac:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c minimac.cpp -o minimac.o
	$(LINKER) $(LK) $(OBJ) -o minimac$(EXEC_TYPE) minimac.o

arte:
	@echo "[$@]"
	$(COMPILER) $(CFG) -c arte.cpp -o carte.o -DARTE_CLI -mwindows
	$(COMPILER) $(CFG) -c arte.cpp -o warte.o -DARTE_NO_CLI
	$(LINKER) $(LK) $(OBJ) -mwindows -o carte$(EXEC_TYPE) carte.o
	$(LINKER) $(LK) $(OBJ) -o warte$(EXEC_TYPE) warte.o

anima:
	$(COMPILER) $(CFG) -c anima.cpp -o anima.o
	$(LINKER) $(LK) $(OBJ) -o anima$(EXEC_TYPE) anima.o

mk-push:
	cp carte ../../output/bin
	cp warte ../../output/bin
	cp minimac ../../output/bin
	cp concerto ../../output/bin
	cp anima ../../output/bin
	cp packer ../../output/bin

finale:
	@echo 
