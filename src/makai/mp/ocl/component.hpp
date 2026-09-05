#ifndef MAKAILIB_MP_OCL_COMPONENT_H
#define MAKAILIB_MP_OCL_COMPONENT_H

#include "../../compat/ctl.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	template <Type::Class T>
	struct Component {
	private:
		friend T;

		struct Impl;

		using Deleter = Functor<void(Impl*)>;

		template <Type::Equal<Impl> TImpl>
		static Deleter deleterFor() {return [] (owner<TImpl> impl) {delete impl;};}

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
