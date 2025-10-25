ROOT :=../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), makai)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

define MAKE_SUB
	$(call submake-all,embed impl tool lexer file data audio graph core)
endef

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

	$(MAKE_SUB)

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

	$(MAKE_SUB)

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
