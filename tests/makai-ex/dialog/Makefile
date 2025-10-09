#CC	:= gcc
#CXX	:= g++

LEAN := -static -s

ROOT := ../../..

COMPILER_CONFIG	:= -m64 -std=gnu++20 -fms-extensions -fcoroutines -I$(ROOT)/output/include
DEBUGMODE := -DMAKAILIB_DEBUG -fconcepts-diagnostics-depth=4 -Wall -Wpedantic -Og

LIBREQS		:= -static-libstdc++ -static-libgcc
GRAPHREQS	:= -lopengl32

sinclude $(ROOT)/make/os-windows.make

NOCMD := -mwindows

o?=2

ifdef debug-release
RELEASEMODE := $(DEBUGMODE)
else
RELEASEMODE := $(NOCMD) $(LEAN) -O$(o)
endif

enginecmd = @$(CXX) -o output/test.diag.engine.$(strip $(2)).exe test.diag.engine.cpp $(COMPILER_CONFIG) $(ROOT)/output/lib/$(strip $(1))

make-test-engine = $(if $(findstring debug,$(1)),\
		$(call enginecmd, libmakai.debug.a, $(1)) $(OS_LIBS) $(LIBREQS) $(DEBUGMODE),\
		$(call enginecmd, libmakai.a, $(1)) $(OS_LIBS) $(LIBREQS) $(RELEASEMODE)\
	)

define animac
	@echo "$(strip $(1))"
	$(COMPILER) $(strip $(1)).an output/$(strip $(1)).anb
endef

define animac-debug
	@echo "$(strip $(1))"
	$(COMPILER_DEBUG) $(strip $(1)).an output/$(strip $(1)).anb
endef

dotest = \
	$(if $(findstring debug,$(1)),\
		$(call testcmd,$(strip $(1)),$(strip $(2)),libmakai.$(strip $(1)).a) $(DEBUGMODE),\
		$(call testcmd,$(strip $(1)),$(strip $(2)),libmakai.a) $(RELEASEMODE)\
	)

case = $(if $(findstring $(strip $(2)),$(strip $(1))),$(3))

params = $(OS_LIBS) $(LIBREQS) $(GRAPHREQS) \
	$(call case, $(1), 01,)\
	$(call case, $(1), 02,)\

define mktest
	@echo "$(strip $(2))"
	$(call dotest, $(1), $(2)) $(call params, $(2))
endef

define mktest-ex
	$(call mktest, $(1), ex.dmk.$(strip $(2)))
endef

ifndef t
define tests
	$(call animac, 01.base)
endef
else
define tests
	$(call animac, $(t))
endef
endif

all: engine

dialog:
	@echo "Building tests..."
	@mkdir -p output
	$(call tests)
	@echo "Done!"
	@echo 
	
engine: engine-debug engine-release

engine-debug:
	@echo "Building engine [debug]..."
	@mkdir -p output
	$(call make-test-engine, debug)
	@echo "Done!"
	@echo 
	
engine-release:
	@echo "Building engine [release]..."
	@mkdir -p output
	$(call make-test-engine, release)
	@echo "Done!"
	@echo 
