ROOT := ../../../..
include $(ROOT)/make/core.make
#embed = @ld -r -b binary -o $(prefix).$@.$(strip $(1)).o **/*.vert **/*.frag

LEAN = $(if $(findstring debug,$@),,-static -s)

define embed
	@echo "Removing old ocl programs..."
	@rm -f *.ocl.o
	@echo "Embedding MVSX codec utilities..."
	@cd mvsx
	@rm -f *.ocl.o
	@$(CPP_COMPILER) -o mvsx.decoder.v1.ocl.o -c decoder.v1.s $(LEAN)
	@cd ..
	@echo "Removing old ocl packs..."
	@rm -f *.$@.$(strip $(1)).o
	@echo "Packing $(strip $(1))..."
	@$(LINKER) -r -o $(prefix).$@.$(strip $(1)).o **/*.ocl.o $(LEAN)
	@echo "Done!"
endef

.ONESHELL:
.SHELLFLAGS = -ec

ocl:
	$(call embed, debug)
	$(call embed, release)
