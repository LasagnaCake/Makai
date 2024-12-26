ROOT :=../..
include $(ROOT)/make/core.make

override prefix := $(call concat, $(prefix), lib.makai.ex)

.PHONY: debug release all
.ONESHELL:
.SHELLFLAGS = -ec

all: debug release

debug:
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
	@echo "<extensions>"
	@echo ""

	$(call submake, game)

	@echo ""
	@echo "</extensions>"
	@echo ""

	@echo "Done!"
	
	@echo "[--- END ---]"
	@echo ""
	@echo "TYPE: $@"
	@echo ""
	$(GET_TIME)
	@echo ""


release:
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
	@echo "<extensions>"
	@echo ""

	$(call submake, game)

	@echo ""
	@echo "</extensions>"
	@echo ""

	@echo "Done!"
	
	
	@echo "[--- END ---]"
	@echo ""
	@echo "TYPE: $@"
	@echo ""
	$(GET_TIME)
	@echo ""
