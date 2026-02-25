link-static = $(foreach lib,$(1), $(shell pkg-config --libs $(lib)))

ifeq ($(os),win)
link-shared = $(foreach lib,$(1), $(shell pkg-config --libs "$(strip $(lib)).dll"))
export OS_LIBS := -lole32 -loleaut32 -limm32 -lwinmm -lversion -lpowrprof -lcomdlg32 -lsetupapi -lgdi32 -ldwmapi -lbcrypt -ldbghelp
export EXEC_TYPE :=.exe
endif
ifeq ($(os),linux)
link-shared = $(foreach lib,$(1), -l:$(strip $(lib)).so)
export OS_LIBS := $(call link-shared, libcurl libSDL2)
endif
export LITE_BUILD_REQS := -shared $(call link-static, libcrypto++ SDL2_net) $(call link-shared, libcurl libSDL2)
