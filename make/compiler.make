include $(ROOT)/make/options.make

ifdef compiler
export compiler
else
ifeq ($(os),win)
compiler:=msys2-gcc
else
compiler:=gcc
endif
export compiler
endif

ifeq ($(compiler),msys2-gcc)
C_COMPILER		?=gcc
CPP_COMPILER	?=g++
endif
ifeq ($(compiler),msys2-clang)
C_COMPILER		?=clang
CPP_COMPILER	?=clang++
endif
ifeq ($(compiler),mingw-win)
C_COMPILER		?=mingw32-gcc
CPP_COMPILER	?=mingw32-g++
endif
ifeq ($(compiler),mingw-linux)
C_COMPILER		?=x86_64-w64-mingw32-gcc
CPP_COMPILER	?=x86_64-w64-mingw32-g++
endif
ifeq ($(compiler),gcc)
C_COMPILER		?=gcc
CPP_COMPILER	?=g++
endif
ifeq ($(compiler),clang)
C_COMPILER		?=clang
CPP_COMPILER	?=clang++
endif
ifeq ($(compiler),auto)
C_COMPILER		?=$(CC)
CPP_COMPILER	?=$(CXX)
compiler :=$(cc)
endif
ifndef compiler
C_COMPILER		?=$(CC)
CPP_COMPILER	?=$(CXX)
compiler :=$(CC)
endif
LINKER			?=ld

export C_COMPILER
export CPP_COMPILER
export LINKER
