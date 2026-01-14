#ifndef CTL_META_UNPACK_H
#define CTL_META_UNPACK_H

#include "pack.hpp"
#include "../typetraits/decay.hpp"
#include "../container/tuple.hpp"

CTL_NAMESPACE_BEGIN	

namespace Meta::Unpack {
	namespace Impl {
		template <class T>
		struct Unpack;

		template <template <class...> class T, class... Types>
		struct Unpack<T<Types...>>: Decay::Pack<Types...> {
			template <usize N>
			using Type	= Select<N, Types...>;

			using First	= First<Types...>;
			using Last	= Last<Types...>;

			template <usize... N>
			using Columns	= Tuple<Type<N>...>;
			using All		= Tuple<Types...>;
		};
	}
	
	template <class T, usize N>
	using Type = typename Impl::Unpack<T>::template Type<N>;
	template <class T>
	using First = typename Impl::Unpack<T>::First;
	template <class T>
	using Last = typename Impl::Unpack<T>::Last;
	template <class T, usize... N>
	using AsTuple = typename Impl::Unpack<T>::All;
	template <class T, usize... N>
	using Columns = typename Impl::Unpack<T>::template Columns<N...>;
}

CTL_NAMESPACE_END

#endif