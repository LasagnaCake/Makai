ROOT := ../../..
include $(ROOT)/make/core.make

pfx := $(prefix)
override prefix = $(call concat, $(pfx), $@)

embed:
	$(call submake-any, shader)