define GET_TIME
@printf "\nTime: "
@date +\"%H:%M:%S\"
@echo ""
endef

prefix:=lib

#WINGARBAGE := -lole32 -loleaut32 -limm32 -lwinmm -lversion -lpowrprof -lcomdlg32 -lsetupapi -lgdi32

SDL			:= lib/SDL2-2.0.10/lib/libSDL2.dll.a
SDLNET		:= lib/SDL2-2.0.10/lib/libSDL2_net.a
SDLMIXER	:= lib/SDL2-2.0.10/lib/libSDL2_mixer.dll.a
CRYPTOPP	:= lib/cryptopp/lib/libcryptopp.a
GLEW		:= lib/OpenGL/GLEW/lib/libglew32.dll.a

ifeq (,$(wildcard obj/extern/extern.3p.a))
CREATE_LIB_3P := link-extern
endif

CONFIG := -static #--static-libstdc++ --static-libgcc

include make/options.make

.PHONY: clear-output package-lib build-debug build-release up-debug up-release link-debug link-release build-all up-all link-all debug release copy-headers copy-ex-headers copy-o-debug copy-o-release all help link-extern
.ONESHELL:
.SHELLFLAGS = -ec

export HELP_MESSAGE

help:
	@echo ""
	@echo "$$HELP_MESSAGE"
	@echo ""

clear-output:
	@rm -rf output/*
	@rm -rf obj/debug/*
	@rm -rf obj/release/*

package-lib:
	@cd output
	@7z a -tzip mingw64.zip lib include -r -mem=AES256
	@cd ..

ship-it: clear-output all package-lib

all: build-all up-all link-all

build-all: build-debug build-release

up-all: copy-headers copy-ex-headers copy-o-debug copy-o-release

link-all: $(CREATE_LIB_3P) link-debug link-release

debug: build-debug up-debug $(CREATE_LIB_3P) link-debug

release: build-release up-release $(CREATE_LIB_3P) link-release

build-debug:
	@make -C"$(MAKAISRC)" debug prefix="$(prefix)"

build-release:
	@make -C"$(MAKAISRC)" release prefix="$(prefix)"

copy-headers:
	@echo "Copying headers..."
	@mkdir -p output/include/makai
	@cd $(MAKAISRC)
	@cp -r --parents *.hpp ../../output/include/makai/
	@cp -r --parents **/*.hpp ../../output/include/makai/
	@cp -r --parents **/**/*.hpp ../../output/include/makai/
	@cp -r --parents **/**/**/*.hpp ../../output/include/makai/
	#@cp -r --parents **/**/**/**/*.hpp ../../output/include/makai/
	@cp -r --parents ctl/* ../../output/include/makai/
	@cd ../..

copy-ex-headers:
	@echo "Copying extensions..."
	@mkdir -p output/include/makai-ex
	@cd $(MAKAIEXSRC)
	@cp -r --parents *.hpp ../../output/include/makai-ex/
	@cp -r --parents **/*.hpp ../../output/include/makai-ex/
	@cp -r --parents **/**/*.hpp ../../output/include/makai-ex/
	@cp -r --parents **/**/**/*.hpp ../../output/include/makai-ex/
	#@cp -r --parents **/**/**/**/*.hpp ../../output/include/makai-ex/
	@cp -r --parents ctl/* ../../output/include/makai-ex/
	@cd ../..

copy-o-debug:
	@echo "Copying objects..."
	@mkdir -p obj/debug
	#@cp -r $(MAKAISRC)/*.debug.o obj/debug/
	@cp -r $(MAKAISRC)/**/*.debug.o obj/debug/
	@cp -r $(MAKAISRC)/**/**/*.debug.o obj/debug/
	@cp -r $(MAKAISRC)/**/**/**/*.debug.o obj/debug/
	#@cp -r $(MAKAISRC)/**/**/**/**/*.debug.o obj/debug/

copy-o-release:
	@echo "Copying objects..."
	@mkdir -p obj/release
	#@cp -r $(MAKAISRC)/*.release.o obj/release/
	@cp -r $(MAKAISRC)/**/*.release.o obj/release/
	@cp -r $(MAKAISRC)/**/**/*.release.o obj/release/
	@cp -r $(MAKAISRC)/**/**/**/*.release.o obj/release/
	#@cp -r $(MAKAISRC)/**/**/**/**/*.release.o obj/release/

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
	@for file in *.o; do mv $$file $(THIRD_PARTY_PREFIX).$(strip $(1)).$$file; done
endef

define repack
	@echo "Repacking [$(strip $(1))]..."
	@cd $(strip $(1))
	@ar rcvs ../$(THIRD_PARTY_PREFIX).$(strip $(1)).a *.o
	@cd ..
endef

extract-extern:
	@echo "Creating lib folder..."
	@rm -rf obj/extern
	@mkdir -p obj/extern/sdl
	@mkdir -p obj/extern/sdl-net
	@mkdir -p obj/extern/sdl-mixer
	@mkdir -p obj/extern/cryptopp
	@echo "Extracting objects..."
	@ar x $(SDL) --output "obj/extern/sdl"
	@ar x $(SDLNET) --output "obj/extern/sdl-net"
	@ar x $(SDLMIXER) --output "obj/extern/sdl-mixer"
	@ar x $(CRYPTOPP) --output "obj/extern/cryptopp"

rename-sdl-mixer:
	@cd obj/extern/sdl-mixer
	@echo "Renaming [sdl-mixer]..."
	@for file in SDL2_mixer_dll_*.o; do mv -- "$$file" "$(THIRD_PARTY_PREFIX).sdl-mixer.$${file##*_}"; done

rename-extern:
	@echo "Renaming objects..."
	@cd obj/extern
	@cd sdl
	$(call addname, sdl)
	@cd ../sdl-net
	$(call addname, sdl-net)
	@cd ../sdl-mixer
	@echo "Renaming [sdl-mixer]..."
	@for file in SDL2_mixer_dll_*.o; do mv -- "$$file" "$(THIRD_PARTY_PREFIX).sdl-mixer.$${file##*_}"; done
	@cd ../cryptopp
	$(call addname, cryptopp)
	@cd ../../..

repack-extern:
	@cd obj/extern
	@echo "Re-combining libraries..."
	$(call repack, sdl)
	$(call repack, sdl-net)
	$(call repack, sdl-mixer)
	$(call repack, cryptopp)
	@cd ../..

combine-extern:
	@echo "Combining files..."
	@ar -M <makelib.extern.mri
	@ranlib obj/extern/extern.3p.a
	@rm -rf obj/extern/st*

link-extern: extract-extern rename-extern repack-extern combine-extern
	@echo "Done!"