#ifndef MAKAILIB_MP_OCL_COMPONENT_H
#define MAKAILIB_MP_OCL_COMPONENT_H

#include "../../compat/ctl.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	struct Component {
	protected:
		struct IResource {
			virtual ~IResource() {}

			virtual pointer resource() const = 0;
		};

		template <Type::Subclass<Component> TComponent>
		static auto& impl(TComponent& component) {
			return *Cast::morph<ref<typename TComponent::Impl>>(component.wrapper->resource);
		}

		pointer resource() const {
			return wrapper->resource->resource();
		}

		template <Type::Subclass<IResource> TResource>
		Component(owner<TResource> const resource): wrapper(wrapper.create()) {
			wrapper->resource = resource;
		}

	private:
		struct Wrapper {
			owner<IResource> resource;
			~Wrapper() {delete resource;}
		};

		AtomicCell<Wrapper> wrapper;
	};
}

#endif
