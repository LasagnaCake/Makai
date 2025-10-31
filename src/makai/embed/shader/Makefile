ROOT := ../../../..
include $(ROOT)/make/core.make
#embed = @ld -r -b binary -o $(prefix).$@.$(strip $(1)).o **/*.vert **/*.frag

LEAN = $(if $(findstring debug,$@),,-static -s)

define embed
	@cd main
	@echo "Removing old main shaders..."
	@rm -f *.shader.o
	@echo "Embedding main shaders..."
	@$(CPP_COMPILER) -o main.vert.shader.o -c shader.vert.s $(LEAN)
	@$(CPP_COMPILER) -o main.frag.shader.o -c shader.frag.s $(LEAN)
	@cd ..
	@cd buffer
	@echo "Removing old buffer shaders..."
	@rm -f *.shader.o
	@echo "Embedding buffer shaders..."
	@$(CPP_COMPILER) -o buffer.vert.shader.o -c shader.vert.s $(LEAN)
	@$(CPP_COMPILER) -o buffer.frag.shader.o -c shader.frag.s $(LEAN)
	@cd ..
	@echo "Removing old shader packs..."
	@rm -f *.$@.$(strip $(1)).o
	@echo "Packing $(strip $(1))..."
	@$(LINKER) -r -o $(prefix).$@.$(strip $(1)).o **/*.shader.o $(LEAN)
	@echo "Done!"
endef

.ONESHELL:
.SHELLFLAGS = -ec

shader:
	$(call embed, debug)
	$(call embed, release)
