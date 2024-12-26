ROOT :=../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), makai)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
	@echo ""
	@echo "[-------------------------------------]"
	@echo "[-------------------------------------]"
	@echo "[---!!!                         !!!---]"
	@echo "[--!!!  Full compilation start!  !!!--]"
	@echo "[---!!!                         !!!---]"
	@echo "[-------------------------------------]"
	@echo "[-------------------------------------]"
	@echo ""
	@$(CXX) --version
	@echo ""
	$(GET_TIME)
	@echo ""
	@echo "TYPE: $@"
	@echo ""
	@echo "[--- START ---]"
	
	@echo "[1/2] compiling [$@]..."

	@echo ""
	@echo "<systems>"
	@echo ""

	$(call submake-any, embed)
	$(call submake, impl)
	$(call submake, tool)
	$(call submake, file)
	$(call submake, data)
	$(call submake, audio)
	$(call submake, graph/gl)
	$(call submake, core)

	@echo ""
	@echo "</systems>"
	@echo ""

	@echo "Done!"
	
	@echo "[--- END ---]"
	@echo ""
	@echo "TYPE: $@"
	@echo ""
	$(GET_TIME)
	@echo ""
	@echo "[------------------------------------]"
	@echo "[------------------------------------]"
	@echo "[---!!!                        !!!---]"
	@echo "[--!!!  Full compilation done!  !!!--]"
	@echo "[---!!!                        !!!---]"
	@echo "[------------------------------------]"
	@echo "[------------------------------------]"
	@echo ""


release:
	@echo ""
	@echo "[-------------------------------------]"
	@echo "[-------------------------------------]"
	@echo "[---!!!                         !!!---]"
	@echo "[--!!!  Full compilation start!  !!!--]"
	@echo "[---!!!                         !!!---]"
	@echo "[-------------------------------------]"
	@echo "[-------------------------------------]"
	@echo ""
	@$(CXX) --version
	@echo ""
	$(GET_TIME)
	@echo ""
	@echo "TYPE: $@"
	@echo ""
	@echo "[--- START ---]"
	
	@echo "[1/2] compiling [$@]..."

	@echo ""
	@echo "<systems>"
	@echo ""

	$(call submake-any, embed)
	$(call submake, impl)
	$(call submake, tool)
	$(call submake, file)
	$(call submake, data)
	$(call submake, audio)
	$(call submake, graph/gl)
	$(call submake, core)

	@echo ""
	@echo "</systems>"
	@echo ""

	@echo "Done!"
	
	
	@echo "[--- END ---]"
	@echo ""
	@echo "TYPE: $@"
	@echo ""
	$(GET_TIME)
	@echo ""
	@echo "[------------------------------------]"
	@echo "[------------------------------------]"
	@echo "[---!!!                        !!!---]"
	@echo "[--!!!  Full compilation done!  !!!--]"
	@echo "[---!!!                        !!!---]"
	@echo "[------------------------------------]"
	@echo "[------------------------------------]"
	@echo ""
