#ifndef MAKAILIB_ANIMA_V2_CORE_VALUE_H
#define MAKAILIB_ANIMA_V2_CORE_VALUE_H

#include "instruction.hpp"

namespace Makai::Anima::V2::Core {
	struct Value {
		MemorySlice<byte>	content;
		uint64				type;

		template <class T>
		consteval static bool sized() {
			return (sizeof(T) >= sizeof(byte));
		}

		template <class T> constexpr Span<T>	span() const requires (sized<T>())	{return Span<T>(data<T>(), size<T>());				}

		template <class T> constexpr ref<T>		data() const requires (sized<T>())	{return reinterpret_cast<ref<T>>(content.data());	}
		template <class T> constexpr usize		size() const requires (sized<T>())	{return content.size() / sizeof(T);					}

		template <class T> constexpr T&			as() const requires (sized<T>())	{return *data<T>();									}

		template <Type::Different<Value> T>
		constexpr Value& operator=(T const& value)
		requires (sized<T>() && Type::CopyConstructible<T>) {
			prepare<T>();
			as<T>() = value;
			return *this;
		}

		template <Type::Different<Value> T>
		constexpr Value& operator=(T&& value)
		requires (sized<T>() && Type::MoveConstructible<T>) {
			prepare<T>();
			as<T>() = move(value);
			return *this;
		}

		template <class T> constexpr bool fits() const requires (sized<T>()) {return sizeof(T) == content.size();}

	private:
		template <Type::Different<Value> T>
		void prepare() {
			destruct();
			content.free();
			content.create(sizeof(T));
			destruct = [&] () {
				MX::destruct<T>(data<T>());
			};
		}

		Functor<void()> destruct;
	};

	struct Object {
		using Reference = Instance<Value>;
		List<Reference> fields;
	};
}

#endif
