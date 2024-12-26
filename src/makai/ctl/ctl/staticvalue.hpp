#ifndef CTL_STATICVALUE_H
#define CTL_STATICVALUE_H

#include "namespace.hpp"

CTL_NAMESPACE_BEGIN

namespace Base {
	template <class P, class T, T V>
	struct StaticValue {
		constexpr static T INITIAL_VALUE = V;

		#ifdef CTL_HEADER_ONLY
		inline static T value = INITIAL_VALUE;
		#else
		static T value;
		#endif

		constexpr T& operator=(T const& v)	{value = v; return *this;	}
		constexpr operator T&()				{return value;				}
		constexpr operator T() const		{return value;				}
	};
}

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_STATICVALUE_H
