#ifndef CTL_ALGORITHM_CSTROP_H
#define CTL_ALGORITHM_CSTROP_H

#include "../namespace.hpp"
#include "../ctypes.hpp"

CTL_NAMESPACE_BEGIN

template <class T>
constexpr ref<T> subsub(ref<T> const str, usize const sz, ref<T> const sub, usize const subsz) {
	if (!(str && sub)) return null;
	if (!(sz && subsz)) return null;
	for (usize i = 0; i < sz; ++i) {
		if (str[i] != sub[i]) continue;
		usize j = 1;
		for (; j < subsz; ++j) {
			if ((i + j) >= sz) return null;
			else if (str[i+j] != sub[j]) break;
		}
		if (j == subsz)
			return (str + i);
	}
	return null;
}

constexpr auto const& strstr	= subsub<char const>;
constexpr auto const& wstrwstr	= subsub<wchar const>;

CTL_NAMESPACE_END

#endif
