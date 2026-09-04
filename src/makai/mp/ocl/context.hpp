#ifndef MAKAILIB_MP_OCL_CONTEXT_H
#define MAKAILIB_MP_OCL_CONTEXT_H

#include "component.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	struct Context: Component<Context> {
		struct Impl;

		struct Program;
		struct Image;
		struct Buffer;
		struct MemorySlice;

		friend struct Context::Program;
		friend struct Context::Image;
		friend struct Context::Buffer;
		friend struct Context::MemorySlice;

		Context();

		template<Type::Constructible<Context const&> T>
		T create() const {
			return T(*this);
		}
	};
}

#endif
