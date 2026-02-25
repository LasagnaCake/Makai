ifeq ($(os),win)
export OS_LIBS := -lole32 -loleaut32 -limm32 -lwinmm -lversion -lpowrprof -lcomdlg32 -lsetupapi -lgdi32 -ldwmapi -lbcrypt -ldbghelp
export EXEC_TYPE :=.exe
endif
ifeq ($(os),linux)
export OS_LIBS := -L:libcurl.so -L:libsdl2.so
endif

export lite-static = $(foreach lib,$(1), $(shell pkg-config --libs $(lib)))
export lite-shared = $(foreach lib,$(1), -l:$(lib).so)
export LITE_BUILD_REQS := -shared $(call lite-static, libcrypto++ SDL2_net) $(call ) $(call lite-shared, libcurl, libsdl2)
