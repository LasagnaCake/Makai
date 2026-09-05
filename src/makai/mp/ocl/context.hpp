#ifndef MAKAILIB_MP_OCL_CONTEXT_H
#define MAKAILIB_MP_OCL_CONTEXT_H

#include "component.hpp"
#include "clonable.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	struct Context: Component {
		struct Program;
		struct Image;
		struct Buffer;
		struct MemorySlice;
		struct Pipe;
		struct Sampler;
		struct Event;
		struct CommandQueue;

		friend struct Context::Program;
		friend struct Context::Image;
		friend struct Context::Buffer;
		friend struct Context::MemorySlice;
		friend struct Context::Pipe;
		friend struct Context::Sampler;
		friend struct Context::Event;
		friend struct Context::CommandQueue;

		struct Impl;

		Context();

		template <class T> struct Contextual: Self<T> {
			Context context() const {
				self().context();
			}
		};

		template<Type::Constructible<Context const&> T>
		T create() const {
			return T(*this);
		}
	};
}

#endif
