define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

prefix:=lib

SDL			:= lib/SDL2-2.0.10/lib/libSDL2.dll.a
SDLNET		:= lib/SDL2-2.0.10/lib/libSDL2_net.a
CRYPTOPP	:= lib/cryptopp/lib/libcryptopp.a
CURL		:= lib/curl/lib/libcurl.dll.a
#OPENSSL		:= lib/openssl/lib/openssl.dll.a

ifeq (,$(wildcard obj/extern/extern.3p.a))
CREATE_LIB_3P := link-extern
endif

CONFIG := -static #--static-libstdc++ --static-libgcc

include make/options.make

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

package-lib:
	@cd output
	@7z a -tzip mingw64.zip bin lib include -r -mem=AES256
	@cd ..

it: clear-output link-extern all tooling

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
	@ar -M <makelib.debug.mri
	@echo "Finalizing..."
	@ranlib output/lib/libmakai.debug.a
	@rm -rf output/lib/st*
	@echo "Done!"
	@echo 

link-release:
	@echo "Creating lib folder..."
	@rm -rf output/lib/libmakai.a
	@mkdir -p output/lib
	@echo "Building library..."
	@ar rcvs output/lib/libmakai.a obj/release/*.release.o
	@echo "Adding externals..."
	@ar -M <makelib.release.mri
	@echo "Finalizing..."
	@ranlib output/lib/libmakai.a
	@rm -rf output/lib/st*
	@echo "Done!"
	@echo 

THIRD_PARTY_PREFIX := lib.3p

define addname
	@echo "Renaming [$(strip $(1))]..."
	@for file in *.o*; do mv $$file $(THIRD_PARTY_PREFIX).$(strip $(1)).$$file; done
endef

define repack
	@echo "Repacking [$(strip $(1))]..."
	@cd $(strip $(1))
	@ar rcvs ../$(THIRD_PARTY_PREFIX).$(strip $(1)).a *.o*
	@cd ..
endef

extract-extern:
	@echo "Creating lib folder..."
	@rm -rf obj/extern
	@mkdir -p obj/extern/sdl
	@mkdir -p obj/extern/sdl-net
	@mkdir -p obj/extern/cryptopp
	@mkdir -p obj/extern/curl
	@echo "Extracting SDL..."
	@ar x $(SDL) --output "obj/extern/sdl"
	@echo "Extracting SDL-Net..."
	@ar x $(SDLNET) --output "obj/extern/sdl-net"
	@echo "Extracting CryptoPP..."
	@ar x $(CRYPTOPP) --output "obj/extern/cryptopp"
	@echo "Extracting cURL..."
	@ar x $(CURL) --output "obj/extern/curl"
#	@echo "Extracting OpenSSL..."
#	@ar x $(OPENSSL) --output "obj/extern/openssl"

rename-extern:
	@echo "Renaming objects..."
	@cd obj/extern
	@cd sdl
	$(call addname, sdl)
	@cd ../sdl-net
	$(call addname, sdl-net)
	@cd ../cryptopp
	$(call addname, cryptopp)
	@cd ../curl
	$(call addname, curl)
#	@cd ../openssl
#	$(call addname, openssl)
	@cd ../../..

repack-extern:
	@cd obj/extern
	@echo "Re-combining libraries..."
	$(call repack, sdl)
	$(call repack, sdl-net)
	$(call repack, cryptopp)
	$(call repack, curl)
#	$(call repack, openssl)
	@cd ../..

combine-extern:
	@echo "Combining files..."
	@ar -M <makelib.extern.mri
	@ranlib obj/extern/extern.3p.a
	@rm -rf obj/extern/st*

link-extern: extract-extern rename-extern repack-extern combine-extern
	@echo "Done!"

tooling: build-tooling copy-tooling

build-tooling:
	@cd tools/anima
	@make
	@cd ../..

copy-tooling:
	@echo "Copying tooling..."
	@mkdir -p output/bin/anima/breve/lib
	@cd tools/anima
	$(call refmove, *.exe, ../../output/bin)
	@cd stdlib
	$(call refcopy, *.bv, ../../../output/bin/anima/breve/lib)
	@cd ../../../dll/network
	$(call refmove, libcurl-4.dll, ../../output/bin)
	@cd ../security
	$(call refmove, *.dll, ../../output/bin)
	@cd ../..
