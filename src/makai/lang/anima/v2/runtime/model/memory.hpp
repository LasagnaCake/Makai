#ifndef MAKAILIB_ANIMA_V2_RUNTIME_MODEL_ALLOCATOR_H
#define MAKAILIB_ANIMA_V2_RUNTIME_MODEL_ALLOCATOR_H

#include "../../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Memory {
		struct Lock {
			ScopeLock<Mutex> lock;
			constexpr operator bool() const {return true;}
		};

		void acquire();
		void release();
		Lock sync();

		Functor<void(pointer)> destructor;

	private:
		Mutex mutex;
		pointer data;
	};
}

#endif
