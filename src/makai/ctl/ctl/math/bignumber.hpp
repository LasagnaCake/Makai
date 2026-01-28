#ifndef CTL_MATH_BIGNUMBER_H
#define CTL_MATH_BIGNUMBER_H

#include "core.hpp"

CTL_NAMESPACE_BEGIN

namespace Math {
	template <Type::Integer T, usize N, usize D = 0>
	struct BigNumber {
		using DataType					= T;
		constexpr static usize SIZE		= N;
		constexpr static usize DECIMALS	= D;
		using InternalType				= DataType[N];

		constexpr BigNumber() noexcept {
			for (auto& elem: content)
				elem = 0;
		}

		constexpr BigNumber(BigNumber const&)	= default;
		constexpr BigNumber(BigNumber&&)		= default;

		constexpr BigNumber& operator=(BigNumber const&)	= default;
		constexpr BigNumber& operator=(BigNumber&&)			= default;

		constexpr String toString() const {
			String result;
			// TODO: This
			return result;
		}

	private:
		InternalType content;
	};
}

CTL_NAMESPACE_END

#endif