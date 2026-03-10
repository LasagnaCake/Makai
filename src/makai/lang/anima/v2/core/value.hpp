#ifndef MAKAILIB_ANIMA_V2_CORE_VALUE_H
#define MAKAILIB_ANIMA_V2_CORE_VALUE_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct Value {
		MemorySlice<byte>	content;
		uint64				type;

		template <class T>
		consteval static bool sized() {
			return (sizeof(T) >= sizeof(byte));
		}

		constexpr Value() noexcept {}

		template <Makai::Type::Different<Value> T>
		constexpr Value(T const& v)
		requires (sized<T>() && Makai::Type::CopyConstructible<T>) {
			operator=(v);
		}

		template <Makai::Type::Different<Value> T>
		constexpr Value(T&& v)
		requires (sized<T>() && Makai::Type::MoveConstructible<T>) {
			operator=(move(v));
		}

		constexpr Value(Value const& other) {
			operator=(other);
		}

		constexpr Value(Value&& other) = default;

		template <class T> constexpr Span<T>		span() requires (sized<T>())		{return Span<T>(data<T>(), size<T>());			}
		template <class T> constexpr Span<T const>	span() const requires (sized<T>())	{return Span<T const>(data<T>(), size<T>());	}

		template <class T> constexpr ref<T>			data() requires (sized<T>())		{return reinterpret_cast<ref<T>>(content.data());		}
		template <class T> constexpr ref<T const>	data() const requires (sized<T>())	{return reinterpret_cast<ref<T const>>(content.data());	}
		template <class T> constexpr usize			size() const requires (sized<T>())	{return content.size() / sizeof(T);						}

		template <class T> constexpr T&			as() requires (sized<T>())			{return *data<T>();	}
		template <class T> constexpr T const&	as() const requires (sized<T>())	{return *data<T>();	}

		template <Makai::Type::Different<Value> T>
		constexpr Value& operator=(T const& value)
		requires (sized<T>() && Makai::Type::CopyConstructible<T>) {
			prepare<T>();
			as<T>() = value;
			return *this;
		}

		template <Makai::Type::Different<Value> T>
		constexpr Value& operator=(T&& value)
		requires (sized<T>() && Makai::Type::MoveConstructible<T>) {
			prepare<T>();
			as<T>() = move(value);
			return *this;
		}

		constexpr Value& operator=(Value const& other) {
			if (other.clone)
				operator=(other.clone.value().invoke(*this));
			return *this;
		}
		constexpr Value& operator=(Value&&) = default;

		template <class T> constexpr bool fits() const requires (sized<T>()) {return sizeof(T) == content.size();}

	private:
		friend class Object;

		template <Makai::Type::Different<Value> T>
		void prepare() {
			destruct(*this);
			content.free();
			content.create(sizeof(T));
			destruct = [] (Value& self) {
				MX::destruct<T>(self.template data<T>());
			};
			if constexpr (Makai::Type::CopyConstructible<T>) {
				clone = [] (Value& self) {
					return Value(self.template as<T>());
				};
			} else clone = null;
		}

		Functor<void(Value&)>				destruct;
		Nullable<Function<Value(Value&)>>	clone;
	};

	struct Object {
		using Storage = Instance<Object>;
		List<Storage>			fields;
		Instance<Definition>	type;
		Instance<Definition>	origin;
		Value					value;
		Map<uint64, uint64>		vtable;

		~Object();

		uint64 resolveMethod(uint64 const id);

		Storage getAtIndex(uint64 const index) const;
		bool setAtIndex(uint64 const index, Storage const& value);
		Storage field(uint64 const id) const;

		Storage clone();
	};
}

#endif
