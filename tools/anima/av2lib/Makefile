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

COMPCFG	:= $(COMPILER_CONFIG) -O$(o) $(OPTIMIZATIONS)
LINKCFG	:= $(LINKER_CONFIG) -s -O$(o) $(LEAN) $(LIBRARIES) -fPIC -shared

COMPILER	:= $(CXX) $(INCLUDES)

EXTRAOBJS := $(ROOT)/output/lib/libmakai.a

LIBNAME	:= archive

define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

CFG := $(COMPCFG)
LK := $(LINKCFG)
OBJ := $(EXTRAOBJS)

ifeq ($(os),win)
WINFLAG := -mwindows
dynlib ?=dll
else
dynlib ?=so
endif

.PHONY: debug release all libs packer concerto minimac brevec arte finale
.ONESHELL:
.SHELLFLAGS = -ec

libs: prologue av2 finale

all: libs

define compile
	@echo "[$(strip $(1))]"
	$(COMPILER) $(CFG) -c av2$(strip $(1)).cpp -o av2$(strip $(1)).$(dynlib)
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

finale:
	@echo 
