#ifndef CTL_TYPETRAITS_NAMEOF_H
#define CTL_TYPETRAITS_NAMEOF_H

#include "../namespace.hpp"
#include "basictraits.hpp"
#include "../container/span.hpp"
#include "decay.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Type name extraction implementations.
namespace Impl::TypeName {
	template <class _>
	constexpr cstring fname() {
		return __PRETTY_FUNCTION__;
	}

	struct Format {
		usize lead	= 0;
		usize total	= 0;
	};

	constexpr usize csize(cstring const str) {
		usize end = 0;
		while (str[end] != '\0') ++end;
		return end;
	}

	constexpr cstring basename() {
		return fname<int>();
	}

	constexpr Format base() {
		Format fmt;
		cstring const fname = basename();
		usize const end = csize(fname);
		Span<char const> str = Span<char const>(fname, end);
		fmt.lead = str.rfind('_') + 4;
		fmt.total = str.size() - 3;
		return fmt;
	}

	constexpr Format fmt = base();

	template <class T>
	struct Solver {
		constexpr static usize NAME_SIZE = csize(fname<T>()) - fmt.total + 1;

		using ReturnType = Span<char const, NAME_SIZE>;

		constexpr static ReturnType name() {
			cstring const fname = fname<T>();
			usize end = 0;
			while (fname[end] != '\0') ++end;
			return ReturnType str = ReturnType(fname + fmt.lead);
		}
	};
}

template <class T>
constexpr typename Impl::TypeName::Solver<T>::ReturnType nameof() {
	return Impl::TypeName::Solver<T>::name();
}

CTL_NAMESPACE_END

#endif