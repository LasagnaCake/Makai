link-static = $(foreach lib,$(1), $(shell pkg-config --libs $(lib)))

SHARELIB := libcurl libSDL2

ifeq ($(os),win)
link-shared = $(foreach lib,$(1), $(shell pkg-config --libs "$(strip $(lib)).dll"))
export OS_LIBS := -lole32 -loleaut32 -limm32 -lwinmm -lversion -lpowrprof -lcomdlg32 -lsetupapi -lgdi32 -ldwmapi -lbcrypt -ldbghelp
export EXEC_TYPE :=.exe
export ADDLIBS :=
endif
ifeq ($(os),linux)
link-shared = $(foreach lib,$(1), -l:$(strip $(lib)).so)
export OS_LIBS :=
export ADDLIBS := $(call link-shared, $(SHARELIB))
endif
export LITE_BUILD_REQS := -shared $(call link-static, libcrypto++ SDL2_net) $(call link-shared, $(SHARELIB))
