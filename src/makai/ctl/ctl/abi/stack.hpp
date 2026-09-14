#ifndef CTL_ABI_STACK_HPP
#define CTL_ABI_STACK_HPP

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../typetraits/cast.hpp"

CTL_NAMESPACE_BEGIN

namespace ABI::Stack {
	template <class T>
	inline void push(T const value) requires (
		Type::Primitive<T>
	or	Type::Enumerator<T>
	or	Type::Pointer<T>
	) {
		if constexpr (sizeof(T) < sizeof(uint16))
			asm ("push %0" :: "r" ((uint16)value))
		;
		else if constexpr (sizeof(T) < sizeof(uint32))
			asm ("push %0" :: "r" (bitcast<uint16>(value)))
		;
		else if constexpr (sizeof(T) < sizeof(uint64))
			//asm ("push %0" :: "r" (bitcast<uint32>(value)))
		;
		else if constexpr (sizeof(T) < sizeof(uint128))
			asm ("push %0" :: "r" (bitcast<uint64>(value)))
		;
		else {
		}
	}

	template <class T>
	inline T pop() requires (
		Type::Primitive<T>
	or	Type::Enumerator<T>
	or	Type::Pointer<T>
	) {
		T into;
		if constexpr (sizeof(T) < sizeof(uint16)) {
			uint16 buf;
			asm ("pop %0" : "=r" (buf));
			into = (T)buf;
		} else if constexpr (sizeof(T) < sizeof(uint32)) {
			uint16 buf;
			asm ("pop %0" : "=r" (buf));
			into = bitcast<T>(buf);
		} else if constexpr (sizeof(T) < sizeof(uint64)) {
			uint32 buf;
			//asm ("pop %0" : "=r" (buf));
			into = bitcast<T>(buf);
		} else if constexpr (sizeof(T) < sizeof(uint128)) {
			uint64 buf;
			asm ("pop %0" : "=r" (buf));
			into = bitcast<T>(buf);
		} else {
		}
		return into;
	}
}

CTL_NAMESPACE_END

#endif
