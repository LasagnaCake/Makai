#CC	:= gcc
#CXX	:= g++

dev ?= 1

LEAN := -static -s

ROOT := ../..

ifeq ($(dev), 1)
DEVMODE := -DCTL_DEVMODE_DEBUG
endif

COMPILER_CONFIG	:= -m64 -std=gnu++20 -fms-extensions -fcoroutines -I$(ROOT)/src/makai/ctl
DEBUGMODE := -DCTL_CONSOLE_OUT $(DEVMODE) -Wall -Wpedantic

LIBREQS		:= -static-libstdc++ -static-libgcc
GRAPHREQS	:= -lopengl32

export os ?=win

sinclude $(ROOT)/make/os.make

o?=3

testcmd = @$(CXX) -O$(o) -o output/$(1)/test.$(2).$(1)$(EXEC_TYPE) test.$(2).cpp $(COMPILER_CONFIG)

#NOCMD := -mwindows

ifdef debug-release
RELEASEMODE := $(DEBUGMODE)
else
RELEASEMODE := $(NOCMD) $(LEAN)
endif

dotest = \
	$(if $(findstring debug,$(1)),\
		$(call testcmd,$(strip $(1)),$(strip $(2))) $(DEBUGMODE),\
		$(call testcmd,$(strip $(1)),$(strip $(2))) $(RELEASEMODE)\
	)

case = $(if $(findstring $(strip $(2)),$(strip $(1))),$(3))

params = $(LIBREQS) $(OS_LIBS)

define mktest
	@echo "$(strip $(2))"
	$(call dotest, $(1), $(2)) $(call params, $(2))
endef

ifndef t
define tests
	$(call mktest, $(1), 01.compilation)
	$(call mktest, $(1), 02.list)
	$(call mktest, $(1), 03.map)
	$(call mktest, $(1), 04.string)
	$(call mktest, $(1), 05.function)
	$(call mktest, $(1), 06.pointer)
	$(call mktest, $(1), 07.nullable)
	$(call mktest, $(1), 08.format)
	$(call mktest, $(1), 09.span)
	$(call mktest, $(1), 10.view)
	$(call mktest, $(1), 11.filesystem)
	$(call mktest, $(1), 12.regex)
	$(call mktest, $(1), 13.random)
	$(call mktest, $(1), 14.error)
	$(call mktest, $(1), 15.co)
	$(call mktest, $(1), 16.bind)
	$(call mktest, $(1), ex.01.obfuscator)
	$(call mktest, $(1), ex.02.collision)
	$(call mktest, $(1), ex.03.value)
endef
else
define tests
	$(call mktest, $(1), $(t))
endef
endif

all: debug release

debug:
	@echo "Building tests [$@]..."
	@mkdir -p output/$@
	$(call tests, $@)
	@echo "Done!"
	@echo 

release:
	@echo "Building tests [$@]..."
	@mkdir -p output/$@
	$(call tests, $@)
	@echo "Done!"
	@echo 
