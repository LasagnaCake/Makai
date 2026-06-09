#ifndef CTL_ALGORITHM_TRANSFER_H
#define CTL_ALGORITHM_TRANSFER_H

#include "../namespace.hpp"
#include "../typetraits/basictraits.hpp"
#include "../typetraits/verify.hpp"
#include "../memory/memory.hpp"

CTL_NAMESPACE_BEGIN

template <class T>
struct Transfer {
	using DataType = T;

	struct Range {
		usize start = 0;
		usize count = 0;

		constexpr Range() noexcept {}
		constexpr Range(usize const count) noexcept: count(count) {}
		constexpr Range(usize const start, usize const count) noexcept: start(start), count(count) {}
	};

	ref<DataType const>	from				= nullptr;
	ref<DataType>		to					= nullptr;
	usize				count				= 0;
	Range				remakeInDestination	= 0;
	Range				clearInSource		= 0;

	constexpr void perform() const {
		CTL_DEVMODE_FN_DECL;
		if (!(count and from and to)) return;
		if (from == to) return;
		if (!(clearInSource.count + 1))			unreachable();
		if (!(remakeInDestination.count + 1))	unreachable();
		if (!(count + 1))						unreachable();
		if (Type::Standard<T> && inRunTime())
			MX::memmove<T>(to, from, count);
		else {
			if (!remakeInDestination.count)
				MX::objcopy<T>(to, from, count);
			else if (!remakeInDestination.start) {
				auto const remakeCount = remakeInDestination.count;
				MX::objremake(to, from, remakeCount);
				MX::objcopy(to + remakeCount, from + remakeCount, count - remakeCount);
			} else {
				auto const remakeCount = remakeInDestination.count;
				auto const remakeStart = remakeInDestination.start;
				auto const remakeEnd = (remakeStart + remakeCount);
				MX::objcopy(to, from, remakeStart);
				MX::objremake(to + remakeStart, from, remakeCount);
				if (auto const remain = count - remakeEnd)
					MX::objcopy(to + remakeEnd, from + remakeEnd, remain);
			}
			if (clearInSource.count)
				MX::objclear(from + clearInSource.start, clearInSource.count);
		}
	}
};

CTL_NAMESPACE_END

#endif
