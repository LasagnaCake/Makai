#ifndef MAKAILIB_MP_OCL_COMPONENT_H
#define MAKAILIB_MP_OCL_COMPONENT_H

#include "../../compat/ctl.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	template <Type::Class T>
	struct Component: T {
		friend struct T;

		using Impl = typename T::Impl;
		using Deleter = Functor<void(owner<Impl>)>;

		template <class TImpl>
		static Deleter deleterFor() {return [] (owner<TImpl> impl) {delete impl;};}

	private:
		friend struct Component<T>::Wrapper;

		struct Wrapper {
			owner<Impl> impl;
			~Wrapper() {Component::deleter(impl);}
		};

		Impl& impl() const {
			return *wrapper->impl;
		}

		static Deleter deleter;
		AtomicCell<Wrapper> wrapper;
	};
}

#endif
