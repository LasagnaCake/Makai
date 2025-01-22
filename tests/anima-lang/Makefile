COMPILER := ../../tools/ex/anima-compiler/animac.exe

define compile
	@echo "$(strip $(1))"
	$(COMPILER) $(strip $(1)).an output/$(strip $(1)).anb
endef

tests:
	$(call compile, test.01.compilation)