#ifndef CTL_EX_DATA_BANK_H
#define CTL_EX_DATA_BANK_H

#include "../../ctl/container/lists/lists.hpp"
#include "../../ctl/container/pointer/pointer.hpp"
#include "../../ctl/container/tuple.hpp"
#include "../../ctl/container/id/id.hpp"
#include "../../ctl/meta/meta.hpp"
#include "../../ctl/exnamespace.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Type::Data {
	template <class T>
	concept Queryable = requires (T t) {
		Type::Equal<typename T::RowType, Meta::Unpack::AsTuple<typename T::RowType>>;
		Type::OneOf<
			AsNormal<Meta::Unpack::First<typename T::RowType>>,
			uint8, uint16, uint32, uint64,
			ID::LUID, ID::VLUID, ID::ELUID
		>;
		Type::Constructible<T, Meta::Unpack::AsTuple<typename T::RowType>>;
		{t.toTuple()} -> Type::Equal<Meta::Unpack::AsTuple<typename T::RowType>>;
	};
}

namespace Data {
	template <Type::Data::Queryable T>
	struct Bank {
		using DataType	= T;
		using RowType	= typename T::RowType;

		template <usize... N>
		using Columns = Meta::Unpack::Columns<RowType, N...>;

		template <usize... N>
		struct Query {
			using ResultType = List<Meta::If<sizeof...(N), Columns<N...>, RowType>>;

			template <usize... N2>
			constexpr Query<N2...> reduced() const {
				return {
					.content = content.template toList<Columns<N2...>>(
						[] (auto const& elem) {
							return elem.template reduced<N2...>();
						}
					)
				};
			}

		private:
			ResultType content;
		};
		
		template <usize... N>
		constexpr Query<> query()
		requires (sizeof...(N) == 0) {
			Query<> result;
			for (auto& elem: content)
				result.pushBack(elem.toTuple());
			return result;
		}
		
		template <usize... N>
		constexpr Query<N...> query() 
		requires (sizeof...(N) > 0) {
			Query<N...> result;
			for (auto& elem: content)
				result.pushBack(elem.toTuple().template reduced<N...>());
			return result;
		}

	private:
		LinkedList<DataType> content;
	};
}

CTL_EX_NAMESPACE_END

#endif