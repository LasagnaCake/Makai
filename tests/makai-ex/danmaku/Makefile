#CC	:= gcc
#CXX	:= g++

export os ?=win

o?=3

LEAN := -static -s

ROOT := ../../..

COMPILER_CONFIG	:= -m64 -std=gnu++20 -fms-extensions -fcoroutines -I$(ROOT)/output/include
DEBUGMODE := -DMAKAILIB_DEBUG -fconcepts-diagnostics-depth=4 -Wall -Wpedantic -Og

ifeq($(from-lite),1)
LITEREQS := -lcryptopp -lcurl -lSDL2 -lSDL2_net
endif

LIBREQS		:= -static-libstdc++ -static-libgcc $(LITEREQS)
ifeq($((os),win)
GRAPHREQS	:= -lopengl32
endif

sinclude $(ROOT)/make/os.make

testcmd = @$(CXX) -o output/$(1)/test.$(2).$(1)$(EXEC_TYPE) test.$(2).cpp $(COMPILER_CONFIG) $(ROOT)/output/lib/$(3)

NOCMD := -mwindows

ifdef debug-release
RELEASEMODE := $(DEBUGMODE)
else
RELEASEMODE := $(NOCMD) $(LEAN) -O$(o)
endif

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
	$(call mktest-ex, $(1), 01.compilation)
	$(call mktest-ex, $(1), 02.player)
	$(call mktest-ex, $(1), 03.bullet)
	$(call mktest-ex, $(1), 04.laser)
	$(call mktest-ex, $(1), 05.item)
	$(call mktest-ex, $(1), 06.enemy)
	$(call mktest-ex, $(1), 07.boss)
endef
else
define tests
	$(call mktest-ex, $(1), $(t))
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
