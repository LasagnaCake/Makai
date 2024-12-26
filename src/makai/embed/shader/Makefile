ROOT := ../../../..
include $(ROOT)/make/core.make
#embed = @ld -r -b binary -o $(prefix).$@.$(strip $(1)).o **/*.vert **/*.frag

LEAN = $(if $(findstring debug,$@),,-static -s)

define embed
	@cd main
	@echo "Embedding main shaders..."
	@$(CXX) -o main.vert.shader.o -c shader.vert.s $(LEAN)
	@$(CXX) -o main.frag.shader.o -c shader.frag.s $(LEAN)
	@cd ..
	@cd buffer
	@echo "Embedding buffer shaders..."
	@$(CXX) -o buffer.vert.shader.o -c shader.vert.s $(LEAN)
	@$(CXX) -o buffer.frag.shader.o -c shader.frag.s $(LEAN)
	@cd ..
	@echo "Packing $(strip $(1))..."
	@$(LD) -r -o $(prefix).$@.$(strip $(1)).o **/*.shader.o $(LEAN)
	@echo "Done!"
endef

.ONESHELL:
.SHELLFLAGS = -ec

shader:
	$(call embed, debug)
	$(call embed, release)