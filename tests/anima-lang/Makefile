ROOT := ../..

COMPILER := $(ROOT)/tools/ex/anima-compiler/animac.exe
COMPILER_DEBUG := $(ROOT)/tools/ex/anima-compiler/animac.debug.exe

LEAN := -static -s

LIBREQS		:= -static-libstdc++ -static-libgcc

sinclude $(ROOT)/make/os-windows.make

COMPILER_CONFIG	:= -m64 -std=gnu++20 -fms-extensions -fcoroutines -I$(ROOT)/output/include
DEBUGMODE := -DMAKAILIB_DEBUG -fconcepts-diagnostics-depth=4 -Wall -Wpedantic

o?=g

enginecmd = @$(CXX) -O$(o) -o output/test.engine.exe test.engine.cpp $(COMPILER_CONFIG) $(ROOT)/output/lib/$(strip $(1))

make-test-engine = $(call enginecmd, libmakai.debug.a) $(DEBUGMODE) $(OS_LIBS) $(LIBREQS)

define compile
	@echo "$(strip $(1))"
	$(COMPILER) $(strip $(1)).an output/$(strip $(1)).anb
endef

define compile-debug
	@echo "$(strip $(1))"
	$(COMPILER_DEBUG) $(strip $(1)).an output/$(strip $(1)).anb
endef

engine:
	@echo "Making test engine..."
	$(make-test-engine)

tests:
	$(call compile, test.01.compilation)
	$(call compile, test.02.blocks)
	$(call compile, test.03.choice)
	$(call compile, test.04.functions)
	$(call compile, test.05.macro-append)

tests-with-compiler-debug:
	$(call compile-debug, test.01.compilation)
	$(call compile-debug, test.02.blocks)
	$(call compile-debug, test.03.choice)
	$(call compile-debug, test.04.functions)
	$(call compile-debug, test.05.macro-append)