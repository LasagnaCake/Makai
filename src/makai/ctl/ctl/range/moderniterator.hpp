#ifndef CTL_RANGE_ITERATABLE_H
#define CTL_RANGE_ITERATABLE_H

#include "iterate.hpp"
#include "../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Range-related facilites.
namespace Range {
	/// @brief Implementation details.
	namespace Impl {
		/// @brief Modern iterator wrapper.
		/// @tparam T iteratable object type.
		template <class T>
		requires requires (T it) {
			{it.next()};
			{it.finished()}	-> Type::Equal<bool>;
		}
		struct ModernIterator {
			/// @brief Iterator wrapper.
			struct Wrapper {
				/// @brief Pre-increment operator.
				constexpr Wrapper& operator++(int)					{it.next(); return *this;	}
				/// @brief Post-increment operator.
				constexpr Wrapper& operator++()						{it.next(); return *this;	}
				/// @brief Dereference operator.
				constexpr T operator*() const
				requires (requires (T it) {{it.value()} -> Type::Different<void>;	})
				{return it.value();		}
				/// @brief Dereference operator.
				constexpr T operator*() const
				requires (requires (T it) {{it.current()} -> Type::Different<void>;	})
				{return it.current();	}
				/// @brief Equality operator.
				constexpr bool operator==(Wrapper const&) const		{return !it.finished();		}
				/// @brief Inequality operator.
				constexpr bool operator!=(Wrapper const&) const		{return it.finished();		}
			private:
				/// @brief Underlying iterator.
				T it;
			};

			/// @brief Iterator begin.
			constexpr Wrapper begin()	{return {it};	}
			/// @brief Iterator end.
			constexpr Wrapper end()		{return {};		}

		private:
			/// @brief Underlying iterator.
			T it;
		};
	}

	template<class T>
	constexpr Impl::ModernIterator<T> iterate(T const& it) {
		return {copy(it)};
	}
	
	template<class T>
	constexpr Impl::ModernIterator<T> iterate(T&& it) {
		return {move(it)};
	}
}

CTL_NAMESPACE_END

#endif