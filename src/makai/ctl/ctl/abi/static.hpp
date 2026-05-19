#ifndef CTL_ABI_STATIC_H
#define CTL_ABI_STATIC_H

#include "../namespace.hpp"
#include "../ctypes.hpp"

CTL_NAMESPACE_BEGIN

namespace ABI {
	template <class T>
	struct Static {
		#ifdef CTL_SEPARATE_ABI
		static T value;
		#else
		inline static T value;
		#endif

		ref<T> operator->() const {return &value;}
	};
}

CTL_NAMESPACE_END

#endif // CTL_ABI_STATIC_H
