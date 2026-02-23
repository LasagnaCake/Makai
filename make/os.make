ifeq ($(os),win)
export OS_LIBS := -lole32 -loleaut32 -limm32 -lwinmm -lversion -lpowrprof -lcomdlg32 -lsetupapi -lgdi32 -ldwmapi -lbcrypt -ldbghelp
export EXEC_TYPE :=.exe
endif

export LITE_BUILD_REQS := -lcryptopp -lSDL2 -lSDL2_net -Bdynamic -lcurl $(pkg-config --libs curl) -lgssapi
