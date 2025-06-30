#ifndef CTL_RANGE_ITERATE_H
#define CTL_RANGE_ITERATE_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

namespace Range {
	namespace Impl {
		template <class T>
		struct IteratorWrapper {
			constexpr IteratorWrapper(T const& begin, T const& end): from(begin), to(end) {}
			constexpr T begin() const	{return from;	}
			constexpr T end() const		{return to;		}
		private:
			T const from;
			T const to;
		};
	}

	template<class T>
	constexpr Impl::IteratorWrapper<T> iterate(T const& begin, T const& end) {
		return {begin, end};
	}
}

CTL_NAMESPACE_END

#endif