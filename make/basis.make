sinclude options.make

os ?=win

export apply = $(foreach arg,$(2),$(call $(1),$(arg)))

ifeq ($(os),win)
define MAKAILIB_EXTERN_MRI
create obj/extern/extern.3p.a
addlib obj/extern/$(THIRD_PARTY_PREFIX).sdl.a
addlib obj/extern/$(THIRD_PARTY_PREFIX).sdl-net.a
addlib obj/extern/$(THIRD_PARTY_PREFIX).cryptopp.a
addlib obj/extern/$(THIRD_PARTY_PREFIX).curl.a
save
end
endef
endif

ifeq ($(os),linux)
define MAKAILIB_EXTERN_MRI
create obj/extern/extern.3p.a
addlib obj/extern/$(THIRD_PARTY_PREFIX).cryptopp.a
addlib obj/extern/$(THIRD_PARTY_PREFIX).sdl-net.a
save
end
endef
endif

export MAKAILIB_EXTERN_MRI

THIRD_PARTY_PREFIX := lib.3p

define combine-extern
	@echo "Combining files..."
	echo "$$MAKAILIB_EXTERN_MRI" | ar -M
	@echo "Finishing touches..."
	@ranlib obj/extern/extern.3p.a
	@rm -rf obj/extern/st*

endef

define addname
	@echo "Renaming [$(strip $(1))]..."
	@for file in *.o*; do mv $$file $(THIRD_PARTY_PREFIX).$(strip $(1)).$$file; done

endef

define repack
	@echo "Repacking [$(strip $(1))]..."
	@ar rcvs ../$(THIRD_PARTY_PREFIX).$(strip $(1)).a *.o*

endef

define pack-lib
	@mkdir -p obj/extern/$(strip $(1))
    @echo "Extracting $(strip $(1))"
	@ar x $(2) --output "obj/extern/$(strip $(1))"
	@cd obj/extern/$(strip $(1))
	$(call addname, $(strip $(1)))
	$(call repack, $(strip $(1)))
    @echo "Finalizing..."
	@cd ../../..
	@echo "Done!"

endef

lib-name =$(firstword $(subst :, ,$(1)))
lib-path =$(subst $(call lib-name,$(1)):,,$(1))

lib-expand = $(call pack-lib, $(call lib-name,$(1)), $(call lib-path,$(1)))
lib-debug = $(info "$(call lib-name,$(1)) @ $(call lib-path,$(1))")

#pack-extern = $(call apply,lib-debug,$(1))
pack-extern = $(call apply,lib-expand,$(1))

export combine-extern
export pack-extern
