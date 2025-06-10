ROOT := ../../..
include $(ROOT)/make/core.make

pfx := $(prefix)
override prefix = $(call concat, $(pfx), $@)

all: embed
debug: embed
release: embed

embed:
	$(call submake-any, shader)