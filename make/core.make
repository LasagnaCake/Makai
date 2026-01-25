sinclude options.make

export space := 

define newline

endef

ifeq ($(compiler),msys2-gcc)
C_COMPILER		?=gcc
CPP_COMPILER	?=g++
endif
ifeq ($(compiler),msys2-clang)
C_COMPILER		?=clang
CPP_COMPILER	?=clang++
endif
ifeq ($(compiler),mingw)
C_COMPILER		?=mingw32-gcc
CPP_COMPILER	?=mingw32-g++
endif
ifeq ($(compiler),auto)
C_COMPILER		?=$(CC)
CPP_COMPILER	?=$(CXX)
endif
ifndef compiler
C_COMPILER		?=$(CC)
CPP_COMPILER	?=$(CXX)
endif
LINKER			?=ld

ifndef gl-loader
gl-loader:=glad
export gl-loader
endif

export C_COMPILER
export CPP_COMPILER
export LINKER

export lower =$(shell echo $(1) | tr A-Z a-z)
export upper =$(shell echo $(1) | tr a-z A-Z)

export concat =$(strip $(1)).$(strip $(2))

export LEAN := -static -s

COMPILER_CONFIG	:= -m64 -std=gnu++20 -fconcepts-diagnostics-depth=4 -fcoroutines -fms-extensions

ifdef openmp
export USE_OPENMP := -fopenmp -openmp -ftree-parallelize-loops=$(omp-threads)
endif

ifdef no-buffers
export NO_BUFFERS := -DMAKAILIB_DO_NOT_USE_BUFFERS
endif

ifeq ($(math),fast)
	export MATHMODE := -ffast-math -fsingle-precision-constant
endif
ifeq ($(math),safe)
	export MATHMODE := -frounding-math -fsignaling-nans
endif

OPTIMIZATIONS	:= $(USE_OPENMP) $(MATHMODE) -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize -fexpensive-optimizations

export DEBUGMODE	:= -DMAKAILIB_DEBUG -DCTL_CONSOLE_OUT -DNDEBUG

ifdef debug-release
	export RELEASEMODE := $(DEBUGMODE)
else
	export RELEASEMODE := $(LEAN)
endif

#export ROOT := $(CURDIR)/..

export path ="$(strip $(1))"

GL_LOADER_FLAG := -DMAKAILIB_GL_LOADER=MAKAILIB_USE_$(call upper,$(gl-loader))

export libpath	= -I$(call path, $(ROOT)/lib/$(strip $(1)))
export corepath	= -I$(call path, $(ROOT)/output/$(strip $(1)))

export INC_SDL			= $(call libpath, SDL2-2.0.10/include)
export INC_OPENGL		= $(call libpath, OpenGL/$(call upper,$(gl-loader))/include) $(GL_LOADER_FLAG)
export INC_STB			= $(call libpath, stb)
export INC_CUTE			= $(call libpath, cute_headers)
export INC_CURL			= $(call libpath, curl/include)
export INC_CRYPTOPP		= $(call libpath, cryptopp/include)
export INC_XML2JSON		= $(call libpath, xml2json/include)
export INC_JSON2XML		= $(call libpath, json2xml)
export INC_MINIAUDIO	= $(call libpath, miniaudio) $(call libpath, minivorbis)
export INC_WEBGPU		= $(call libpath, wgpu-native/windows-x86_64/include)

export INC_MAKAI		= $(call corepath, include)

FRAME_PTR := -fno-omit-frame-pointer

DEBUG_CONFIG_BASE	= $(COMPILER_CONFIG) -Wall -Wpedantic -Og -ggdb3 $(FRAME_PTR) $(DEBUGMODE)
DEBUG_CONFIG		= $(DEBUG_CONFIG_BASE)
RELEASE_CONFIG_BASE	= $(COMPILER_CONFIG) $(OPTIMIZATIONS) $(FRAME_PTR) $(RELEASEMODE)
RELEASE_CONFIG		= $(RELEASE_CONFIG_BASE) -O$(o)

COMPILER = $(CPP_COMPILER) $(INCLUDES)

export NO_OP := @:

compile-debug	= $(COMPILER) $(DEBUG_CONFIG_BASE) -c $(strip $(1)).cpp -o $(prefix).$(strip $(1)).$@.o
compile-release	= $(COMPILER) $(RELEASE_CONFIG_BASE) -O$(strip $(2)) -c $(strip $(1)).cpp -o $(prefix).$(strip $(1)).$@.o

export compile-with-o = \
	$(if $(findstring debug,$@),\
		$(call compile-debug, $(1), $(2)),\
		$(call compile-release, $(1), $(2))\
	)

export compile	= @$(call compile-with-o, $(1), $(o))
compile-chain	= $(call compile-with-o, $(1), $(o))

define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef
export GET_TIME

export leave = $(subst $(space),,$(filter ../,$(subst /, ../ ,$(strip $(1)))))

submake-impl = $(GNU_MAKE) -C$(call path, $(1)) $@ prefix="$(strip $(2))"

submake-chain = $(call submake-impl, $(1), $(prefix))

export submake = @$(call submake-impl, $(1), $(prefix))

submake-any-impl = $(GNU_MAKE) -C$(call path, $(1)) prefix="$(strip $(2))"

export submake-any = @$(call submake-any-impl, $(1), $(prefix))

submake-any-chain = $(call submake-any-impl, $(1), $(prefix))

ifdef subsystem
	export SUBSYSTEM := subsystem="$(strip $(subsystem))"
endif

ifdef subsystem
	override SUBSYSTEM_PATH			:= $(subst $(SPACE),,$(strip $(subsystem)))
	override SUBSYSTEM_BASE			:= $(strip $(firstword $(subst ., ,$(SUBSYSTEM_PATH))))
	override SUBSYSTEM_SUBPATH		:= $(subst $(SUBSYSTEM_BASE).,,$(SUBSYSTEM_PATH))
	override SUBSYSTEM_PROPAGATE	:= $(if $(findstring $(SUBSYSTEM_BASE),$(SUBSYSTEM_SUBPATH)),,"subsystem=$(SUBSYSTEM_SUBPATH)")
else
	override SUBSYSTEM_PROPAGATE:=
endif

compile-splice = $(call compile-chain,$(1));$(space)

ifndef SUBSYSTEM
	compile-splice = $(call compile-chain,$(1));$(space)
	compile-all-impl = @$(foreach file,$(1),$(call compile-splice,$(file)))
else
	ifneq ($(SUBSYSTEM_PROPAGATE),)
		compile-all-impl = $(NO_OP);
	else ifeq ($(SUBSYSTEM_BASE),*)
		compile-all-impl = @$(foreach file,$(1),$(call compile-splice,$(file)))
	else
		compile-all-impl = @$(call compile-splice, $(SUBSYSTEM_BASE))
	endif
endif

submake-splice = $(call submake-chain,$(1)) $(SUBSYSTEM_PROPAGATE);$(space)

ifndef SUBSYSTEM
	submake-all-impl = @$(foreach subsys,$(1),$(call submake-splice,$(subsys)))
else
	ifneq ($(SUBSYSTEM_PROPAGATE),)
		submake-all-impl = @$(call submake-splice, $(SUBSYSTEM_BASE))
	else ifeq ($(SUBSYSTEM_BASE),*)
		submake-all-impl = @$(foreach subsys,$(1),$(call submake-splice,$(subsys)))
	else
		submake-all-impl = $(NO_OP);
	endif
endif

ifdef debug-makefile
	define SUBMAKE_DEBUG_OUTPUT
		@echo "Path: [$(SUBSYSTEM_PATH)]"
		@echo "Base: [$(SUBSYSTEM_BASE)]"
		@echo "Subpath: [$(SUBSYSTEM_SUBPATH)]"
		@echo "Propagate: [$(SUBSYSTEM_PROPAGATE)]"
	endef
endif

define compile-all
	$(SUBMAKE_DEBUG_OUTPUT)
	$(call compile-all-impl,$(strip $(1)))
endef
export compile-all

define submake-all
	$(SUBMAKE_DEBUG_OUTPUT)
	$(call submake-all-impl,$(strip $(1)))
endef
export submake-all

#export compile-all = $(call compile-all-impl,$(strip $(1)))

#export submake-all = $(call submake-all-impl,$(strip $(1)))
