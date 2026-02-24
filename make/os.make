ifeq ($(os),win)
export OS_LIBS := -lole32 -loleaut32 -limm32 -lwinmm -lversion -lpowrprof -lcomdlg32 -lsetupapi -lgdi32 -ldwmapi -lbcrypt -ldbghelp
export EXEC_TYPE :=.exe
endif

export lite-libs = $(foreach lib,$(1), $(shell pkg-config --libs --shared $(lib)))
export LITE_BUILD_REQS := -shared $(call lite-libs, libcrypto++ sdl2 SDL2_net libcurl)
