define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

prefix:=lib

ifeq ($(os),linux)
LIBFILE_TYPE :=.a
else
LIBFILE_TYPE :=.dll.a
endif

ifndef os
LIBFILE_SRC ?=win64
else
LIBFILE_SRC ?=$(os)64
endif

SDL			= lib/SDL2-2.0.10/lib/$(LIBFILE_SRC)/libSDL2$(LIBFILE_TYPE)
SDLNET		= lib/SDL2-2.0.10/lib/$(LIBFILE_SRC)/libSDL2_net.a
CRYPTOPP	= lib/cryptopp/lib/$(LIBFILE_SRC)/libcryptopp.a
CURL		= lib/curl/lib/$(LIBFILE_SRC)/libcurl$(LIBFILE_TYPE)
#OPENSSL		= lib/openssl/lib/$(LIBFILE_SRC)/openssl$(LIBFILE_TYPE)

ifeq ($(lite),1)
LINK_EXTERN :=
EXTERN_AR_STEP :=:
else
LINK_EXTERN :=link-extern
EXTERN_AR_STEP :=ar
endif

ifeq ($(os),win)
define MOVE_DLL
	@cd res
	$(call refmove, *.dll, ../output/lib)
	@cd ..
endef
LINUX_FULL_PRE := :
OS_DEPENDENT_LIBS := cryptopp:$(CRYPTOPP) sdl:$(SDL) sdl-net:$(SDLNET) curl:$(CURL)
else
ifeq ($(os),linux)
LINUX_FULL_PRE := @unzip -o lib/cryptopp/lib/$(os)64/libcryptopp.a.zip -d lib/cryptopp/lib/$(os)64/
OS_DEPENDENT_LIBS := cryptopp:$(CRYPTOPP) sdl-net:$(SDLNET)
endif
endif

ifeq (,$(wildcard obj/extern/extern.3p.a))
CREATE_LIB_3P :=$(LINK_EXTERN)
endif

CONFIG := -static #--static-libstdc++ --static-libgcc

include make/options.make
include make/basis.make

.PHONY: clear-output package-lib build-debug build-release up-debug up-release link-debug link-release build-all up-all link-all debug release copy-headers copy-ex-headers copy-o-debug copy-o-release all help link-extern
.ONESHELL:
.SHELLFLAGS = -ec

export HELP_MESSAGE

define refcopy
	@find . -name '$(strip $(1))' -exec cp --parents \{\} $(strip $(2)) \;
endef

define refmove
	@find . -name '$(strip $(1))' -exec cp \{\} $(strip $(2)) \;
endef

define clear-old-shaders
	@cd obj/$(strip $(1))
	@rm -f *.embed.shader.$(strip $(1)).o
	@cd ../..
endef

help:
	@echo ""
	@echo "$$HELP_MESSAGE"
	@echo ""

clear-output:
	@rm -rf output/*
	@rm -rf obj/debug/*
	@rm -rf obj/extern/*
	@rm -rf obj/release/*

ifeq ($(lite),1)
BUILDTYPE:=lite
else
BUILDTYPE:=full
endif

package-lib:
	@cd output
	@7z a -tzip $(os)-$(compiler)-$(BUILDTYPE).zip bin lib include -r -mem=AES256
	@cd ..

it: clear-output $(LINK_EXTERN) all tooling

ship-it: it package-lib

all: build-all up-all link-all

build-all: build-debug build-release

up-all: copy-headers copy-ex-headers copy-o-debug copy-o-release

link-all: $(CREATE_LIB_3P) link-debug link-release

debug: build-debug up-debug $(CREATE_LIB_3P) link-debug

release: build-release up-release $(CREATE_LIB_3P) link-release

build-debug:
	@$(GNU_MAKE) -C"$(MAKAISRC)" debug prefix="$(prefix)" $(SUBSYSTEM)

build-release:
	@$(GNU_MAKE) -C"$(MAKAISRC)" release prefix="$(prefix)" $(SUBSYSTEM)

copy-headers:
	@echo "Copying headers..."
	@mkdir -p output/include/makai
	@cd $(MAKAISRC)
	$(call refcopy, *.hpp, ../../output/include/makai/)
	@cp -r --parents ctl/* ../../output/include/makai/
	@cd ../..

copy-ex-headers:
	@echo "Copying extensions..."
	@mkdir -p output/include/makai-ex
	@cd $(MAKAIEXSRC)
	$(call refcopy, *.hpp, ../../output/include/makai-ex/)
	@cd ../..

copy-o-debug:
	@echo "Copying objects..."
	@mkdir -p obj/debug
	$(call clear-old-shaders, debug)
	@cd $(MAKAISRC)
	$(call refmove, *.debug.o, ../../obj/debug/)
	@cd ../..

copy-o-release:
	@echo "Copying objects..."
	@mkdir -p obj/release
	$(call clear-old-shaders, release)
	@cd $(MAKAISRC)
	$(call refmove, *.release.o, ../../obj/release/)
	@cd ../..

up-debug: copy-headers copy-o-debug
	@echo "File copy done!"

up-release: copy-headers copy-o-release
	@echo "File copy done!"

link-debug:
	@echo "Creating lib folder..."
	@rm -rf output/lib/libmakai.debug.a
	@mkdir -p output/lib
	@echo "Building library..."
	@ar rcvs output/lib/libmakai.debug.a obj/debug/*.debug.o
	@echo "Adding externals..."
	@$(EXTERN_AR_STEP) -M <makelib.debug.mri
	@echo "Finalizing..."
	@ranlib output/lib/libmakai.debug.a
	@rm -rf output/lib/st*
	$(MOVE_DLL)
	@echo "Done!"
	@echo 

link-release:
	@echo "Creating lib folder..."
	@rm -rf output/lib/libmakai.a
	@mkdir -p output/lib
	@echo "Building library..."
	@ar rcvs output/lib/libmakai.a obj/release/*.release.o
	@echo "Adding externals..."
	@$(EXTERN_AR_STEP) -M <makelib.release.mri
	@echo "Finalizing..."
	@ranlib output/lib/libmakai.a
	@rm -rf output/lib/st*
	$(MOVE_DLL)
	@echo "Done!"
	@echo 

combine-extern:
	@echo "Combining files..."
	@ar -M <$(MRI)
	@ranlib obj/extern/extern.3p.a
	@rm -rf obj/extern/st*

link-extern:
	@echo "Packing libraries..."
	$(LINUX_FULL_PRE)
	@echo "Libraries: $(OS_DEPENDENT_LIBS)";
	$(call pack-extern, $(OS_DEPENDENT_LIBS))
	$(call combine-extern)
	@echo "Done!"

tooling: build-tooling copy-tooling

build-tooling:
	@cd tools/anima
	$(GNU_MAKE) debug=$(debug-tooling) from-lite=$(lite)
	@cd ../..

ifeq ($(os),win)
define MOVE_DLL_TOOLS
	@cd ../../../dll/current
	$(call refmove, *.dll, ../../output/bin)
	@cd ../..
endef
endif

copy-tooling:
	@echo "Copying tooling..."
	@mkdir -p output/bin/anima/breve/lib
	@cd tools/anima
	$(GNU_MAKE) mk-push
	@cd stdlib
	$(call refcopy, *.bv, ../../../output/bin/anima/breve/lib)
	$(MOVE_DLL_TOOLS)

#export clean-libname = $(subst dec,,$(subst _krb5,,$(subst lber,ldap2,$(subst api,,$(1)))))
#export lite-solver-pass1 = $(foreach lib,$(foreach lib,$(1), $(patsubst -l%,%,$(shell pkg-config --libs-only-l --static $(lib)))), lib$(call clean-libname,$(lib))-dev)
#export lite-solver-pass2 = $(subst libz-dev,zlib1g-dev,$(subst libcurl-dev,,$(subst nettle,nettle*,$(subst gnutls,gnutls*,$(call lite-solver-pass1,$(1))))))
#export lite-solver = $(call lite-solver-pass2,$(1))
configure-ubuntu:
	sudo apt update
	sudo apt install --yes libgl1-mesa-dev
