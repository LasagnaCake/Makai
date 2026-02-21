#CC	:= gcc
#CXX	:= g++

LEAN := -static -s

ROOT := ../..

COMPILER_CONFIG	:= -m64 -std=gnu++20 -fms-extensions -fcoroutines -I$(ROOT)/output/include
DEBUGMODE := -DMAKAILIB_DEBUG -fconcepts-diagnostics-depth=4 -Wall -Wpedantic

LIBREQS		:= -static-libstdc++ -static-libgcc -lcryptopp -lcurl -lSDL2 -lSDL2_net
GRAPHREQS	:= -lopengl32

export os ?=win

sinclude $(ROOT)/make/os.make

o?=3

testcmd = @$(CXX) -O$(o) -o output/$(1)/test.$(2).$(1)$(EXEC_TYPE) test.$(2).cpp $(COMPILER_CONFIG) $(ROOT)/output/lib/$(3)

NOCMD := -mwindows

ifdef debug-release
RELEASEMODE := $(DEBUGMODE)
else
RELEASEMODE := $(NOCMD) $(LEAN)
endif

dotest = \
	$(if $(findstring debug,$(1)),\
		$(call testcmd,$(strip $(1)),$(strip $(2)),libmakai.$(strip $(1)).a) $(DEBUGMODE),\
		$(call testcmd,$(strip $(1)),$(strip $(2)),libmakai.a) $(RELEASEMODE)\
	)

case = $(if $(findstring $(strip $(2)),$(strip $(1))),$(3))

params = $(OS_LIBS) $(LIBREQS) \
	$(call case, $(1), 01,)\
	$(call case, $(1), 02,$(GRAPHREQS))\
	$(call case, $(1), 03,$(GRAPHREQS))\
	$(call case, $(1), 04,$(GRAPHREQS))\
	$(call case, $(1), 05,$(GRAPHREQS))\
	$(call case, $(1), 06,$(GRAPHREQS))\
	$(call case, $(1), 07,$(GRAPHREQS))\
	$(call case, $(1), 08,$(GRAPHREQS))\
	$(call case, $(1), 09,$(GRAPHREQS))\

define mktest
	@echo "$(strip $(2))"
	$(call dotest, $(1), $(2)) $(call params, $(2))
endef

ifndef t
define tests
	$(call mktest, $(1), 01.compilation)
	$(call mktest, $(1), 02.app)
	$(call mktest, $(1), 03.graphics)
	$(call mktest, $(1), 04.input)
	$(call mktest, $(1), 05.text)
	$(call mktest, $(1), 06.armature)
	$(call mktest, $(1), 07.sound)
	$(call mktest, $(1), 08.json)
	$(call mktest, $(1), 09.flow)
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
