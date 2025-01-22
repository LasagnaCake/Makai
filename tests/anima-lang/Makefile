COMPILER := ../../tools/ex/anima-compiler/animac.exe
COMPILER_DEBUG := ../../tools/ex/anima-compiler/animac.debug.exe

define compile
	@echo "$(strip $(1))"
	$(COMPILER) $(strip $(1)).an output/$(strip $(1)).anb
endef

define compile-debug
	@echo "$(strip $(1))"
	$(COMPILER_DEBUG) $(strip $(1)).an output/$(strip $(1)).anb
endef

tests:
	$(call compile, test.01.compilation)
	$(call compile, test.02.blocks)

tests-with-absolute-debug:
	$(call compile-debug, test.01.compilation)
	$(call compile-debug, test.02.blocks)