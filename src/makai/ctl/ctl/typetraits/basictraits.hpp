#ifndef CTL_TYPETRAITS_BASICTRAITS_H
#define CTL_TYPETRAITS_BASICTRAITS_H

#include <typeinfo>

#include "../ctypes.hpp"
#include "containers.hpp"
#include "converter.hpp"
#include "../meta/logic.hpp"
#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Type constraints.
namespace Type {
	/// @brief Implementation of type constraints.
	namespace Impl {
		/// @brief Pieces used in implementations.
		namespace Partial {
			template<typename T>	struct IsPointer:			FalseType	{};
			template<typename T>	struct IsPointer<T*>:		TrueType	{};

			template<typename T>	struct IsLeftReference:		FalseType	{};
			template<typename T>	struct IsLeftReference<T&>:	TrueType	{};

			template<typename T>	struct IsTemporary:			FalseType	{};
			template<typename T>	struct IsTemporary<T&&>:	TrueType	{};

			// cppreference
			template<typename T>			struct IsMemberPointer:			FalseType	{};
			template<typename T, class C>	struct IsMemberPointer<T C::*>:	TrueType	{};

			// Hooray... it's intrinsics...
			template<typename T>	struct IsUnion:			BooleanConstant<__is_union(T)>	{};
			template<typename T>	struct IsEnumerator:	BooleanConstant<__is_enum(T)>	{};

			// cppreference
			template<class T>
			BooleanConstant<!IsUnion<AsNonCV<T>>::value> isClass(int T::*);
			template<class>
			FalseType isClass(...);

			// cppreference
			template<typename Base>
			TrueType isPointerConvertible(const volatile Base*);
			template<typename>
			FalseType isPointerConvertible(const volatile void*);

			// cppreference
			template<typename T, typename Base>
			auto isDerived(int) -> decltype(isPointerConvertible<Base>(static_cast<T*>(nullptr)));
			template<typename, typename>
			auto isDerived(...) -> TrueType;

			// cppreference
			template<class T>
			auto isReturnable(int) -> decltype(
				void(static_cast<T(*)()>(nullptr)), TrueType {}
			);
			template<class>
			auto isReturnable(...) -> FalseType;

			// cppreference
			template<class From, class To>
			auto isImplicit(int) -> decltype(
				void(declval<void(&)(To)>()(declval<From>())), TrueType {}
			);
			template<class, class>
			auto isImplicit(...) -> FalseType;

			// Based off of: https://stackoverflow.com/a/38181716
			template <class, class T, class... Args>	struct IsConstructible: FalseType {};
			template <class T, class... Args>			struct IsConstructible<
				VoidType<decltype(::new T(declval<Args>()...))
			>, T, Args...> : TrueType {};
			
			template<class Lambda, int=(Lambda{}(), 0)>
			constexpr bool isConstexpr(Lambda)	{return true;	}
			constexpr bool isConstexpr(...)		{return false;	}

			template<typename T, typename = void>
			constexpr bool isDefined = false;

			template<typename T>
			constexpr bool isDefined<T, decltype(typeid(T), void())> = true;

			// cppreference
			template <class T>
			TrueType isPoly(decltype(dynamic_cast<const volatile void*>(static_cast<T*>(nullptr))));
			
			// cppreference
			template <class T>
			FalseType isPoly();
		}

		template<typename A, typename B>	struct IsEqual:			FalseType	{};
		template<typename T>				struct IsEqual<T, T>:	TrueType	{};

		template<typename A, typename B>
		concept Equal = IsEqual<A, B>::value;

		template<typename T>	struct IsConstant:			FalseType 	{};
		template<typename T>	struct IsConstant<T const>:	TrueType	{};

		template<typename T>	struct IsVolatile:				FalseType 	{};
		template<typename T>	struct IsVolatile<T volatile>:	TrueType	{};

		template<typename T>	struct IsInteger:	BooleanConstant<
			requires (T t, T* p, void (*f)(T)) {
				reinterpret_cast<T>(t);
				f(0);
				p + t;
			}
		> {};

		template<typename T>	struct IsReal:		TrueType::Or<
			IsEqual<float,			AsNonCV<T>>
		,	IsEqual<double,			AsNonCV<T>>
		,	IsEqual<long double,	AsNonCV<T>>
		> {};

		template<typename T>	struct IsNumber:	TrueType::Or<IsInteger<T>, IsReal<T>> {};

		template<typename T>	struct IsUnion:			Partial::IsUnion<AsNonCV<T>>		{};
		template<typename T>	struct IsEnumerator:	Partial::IsEnumerator<AsNonCV<T>>	{};

		template<typename T>	struct IsVoid:						FalseType	{};
		template<>				struct IsVoid<void>:				TrueType	{};
		template<>				struct IsVoid<void const>:			TrueType	{};
		template<>				struct IsVoid<void volatile>:		TrueType	{};
		template<>				struct IsVoid<void volatile const>:	TrueType	{};

		template<class T>			struct IsAutoArray:			FalseType	{};
		template<class T>			struct IsAutoArray<T[]>:	TrueType	{};

		template<class T>			struct IsFixedArray:		FalseType	{};
		template<class T, usize N>	struct IsFixedArray<T[N]>:	TrueType	{};

		template<typename T>		struct IsArray:				TrueType::Or<IsAutoArray<T>, IsFixedArray<T>>	{};

		template<typename T>	struct IsNullPointer:			FalseType	{};
		template<>				struct IsNullPointer<nulltype>:	TrueType	{};

		template<typename T>	struct IsPointer:		Partial::IsPointer<AsNonCV<T>>			{};
		template<typename T>	struct IsMemberPointer:	Partial::IsMemberPointer<AsNonCV<T>>	{};
		template<typename T>	struct IsLeftReference:	Partial::IsLeftReference<AsNonCV<T>>	{};
		template<typename T>	struct IsTemporary:		Partial::IsTemporary<AsNonCV<T>>		{};
		template<typename T>	struct IsReference:		TrueType::Or<IsLeftReference<T>, IsTemporary<T>>	{};

		template<class T>						struct IsFunction:				FalseType	{};
		template<typename R, typename... Args>	struct IsFunction<R(Args...)>:	TrueType	{};

		template<typename T> requires (IsNumber<T>::value)	struct IsUnsigned:	BooleanConstant<(T(-1) > T(0))>				{};
		template<typename T> requires (IsNumber<T>::value)	struct IsSigned:	BooleanConstant<(!IsUnsigned<T>::value)>	{};

		// cppreference
		template<typename T>	struct IsClass:			decltype(Partial::isClass<T>(nullptr))	{};

		// cppreference
		template<typename T, typename Base>	struct IsDerived:	BooleanConstant<
			IsClass<Base>::value && IsClass<T>::value
		&&	decltype(Partial::isDerived<T, Base>(0))::value
		>	{};

		// cppreference
		template<class T, class To> struct IsConvertible:	BooleanConstant<
		#ifdef CTL_NO_BUILTINS
			(
				decltype(Partial::isReturnable<To>(0))::value
			&&	decltype(Partial::isImplicit<T, To>(0))::value
			)
		||	TrueType::And<IsVoid<T>, IsVoid<To>>::value
		#else
			__is_convertible(T, To)
		#endif
		> {};

		// cppreference
		template<class T, class To>
		struct IsNothrowConvertible: TrueType::And<IsVoid<T>, IsVoid<To>> {};
		template<class T, class To>
			requires requires {
				static_cast<To(*)()>(nullptr);
				{ declval<void(&)(To) noexcept>()(declval<T>()) } noexcept;
			}
		struct IsNothrowConvertible<T, To>: TrueType {};

		#ifdef CTL_NO_BUILTINS
		template<typename T, typename... Args>	struct IsConstructible:	Partial::IsConstructible<VoidType<>, T, Args...>	{};
		#else
		template<typename T, typename... Args> struct IsConstructible: BooleanConstant<__is_constructible(T, Args...)> {};
		#endif

		template<class TFunction, typename F>
		struct IsFunctional;

		template<class TFunction, typename TReturn, typename... TArgs>
		struct IsFunctional<TFunction, TReturn(TArgs...)>: BooleanConstant<
			requires (TFunction func, TArgs... args) {
				{func(args...)} -> Equal<TReturn>;
			}
		> {};

		template <class T> struct IsCompileTimeable: BooleanConstant<Partial::isConstexpr([]{return T();})>	{};

		template <class T> struct IsDefined: BooleanConstant<Partial::isDefined<T>>	{};

		template <template <class...> class T, class... Types>
		struct IsValid: T<Types...> {};

		template <class T> struct IsPolymorphic: decltype(Partial::isPoly<T>()) {};

		template <class T> struct IsVirtual: BooleanConstant<__has_virtual_destructor(T)> {};
	}

	/// @brief Both types must be equal.
	template<typename A, typename B>
	concept Equal = Impl::Equal<A, B>;

	/// @brief Both types must NOT be equal.
	template<typename A, typename B>
	concept Different = !Equal<A, B>;

	/// @brief `T` must be able to be (implicitly) convertible to `To`.
	template<typename T, typename To>
	concept Convertible = Impl::IsConvertible<T, To>::value;

	/// @brief `T` must be able to be (implicitly) convertible to `To`, and must not throw. 
	template<typename T, typename To>
	concept NothrowConvertible = Impl::IsNothrowConvertible<T, To>::value;

	/// @brief Type must be constant.
	template<typename T>
	concept Constant = Impl::IsConstant<T>::value;

	/// @brief Type must be volatile.
	template<typename T>
	concept Volatile = Impl::IsVolatile<T>::value;

	/// @brief Type must be an integer.
	template<typename T>
	concept Integer = Impl::IsInteger<T>::value;

	/// @brief Type must be a floating-point number.
	template<typename T>
	concept Real = Impl::IsReal<T>::value;

	/// @brief Type must be a floating-point number.
	template<typename T>
	concept Float = Real<T>;

	/// @brief Type must be a number.
	template<typename T>
	concept Number = Integer<T> || Real<T>;

	/// @brief Type must be a C-style array of deduced size.
	template<typename T>
	concept AutoArray = Impl::IsAutoArray<T>::value;

	/// @brief Type must be a C-style array of fixed size.
	template<typename T>
	concept FixedArray = Impl::IsFixedArray<T>::value;

	/// @brief Type must be a C-style array.
	template<typename T>
	concept Array = Impl::IsArray<T>::value;

	/// @brief Type must be void.
	template<typename T>
	concept Void = Impl::IsVoid<T>::value;

	/// @brief Type must not be void.
	template <typename T>
	concept NonVoid = !Void<T>;

	/// @brief Type must be `nulltype`.
	template<typename T>
	concept Null = Impl::IsNullPointer<T>::value;

	/// @brief Type must not be `nulltype`.
	template<typename T>
	concept NonNull = !Null<T>;

	/// @brief Type must be unsigned.
	template<typename T>
	concept Unsigned = Impl::IsUnsigned<T>::value;

	/// @brief Type must be signed.
	template<typename T>
	concept Signed = Impl::IsSigned<T>::value;

	/// @brief Type must be a signed integer.
	template<typename T>
	concept SignedInteger = Signed<T> && Integer<T>;

	/// @brief Type must be an unsigned integer.
	template<typename T>
	concept UnsignedInteger = Unsigned<T> && Integer<T>;

	/// @brief Type must be a function.
	template<typename T>
	concept Function = Impl::IsFunction<T>::value;

	/// @brief Type must be a pointer.
	template<typename T>
	concept Pointer = Impl::IsPointer<T>::value;

	/// @brief Type must be a pointer to a member of a `class` or `struct`.
	template<typename T>
	concept MemberPointer = Impl::IsMemberPointer<T>::value;

	/// @brief Type must be an lvalue reference (common reference - `&`).
	template<typename T>
	concept LeftReference = Impl::IsLeftReference<T>::value;

	/// @brief Type must be an rvalue reference (temporary reference - `&&`)
	template<typename T>
	concept Temporary = Impl::IsTemporary<T>::value;

	/// @brief Type must be a reference of some kind.
	template<typename T>
	concept Reference = LeftReference<T> || Temporary<T>;

	/// @brief Type must be convertible from a number type.
	template <typename T>
	concept Numerable = Convertible<usize, T>;

	/// @brief Type must be an `enum`.
	template<typename T>
	concept Enumerator = Impl::IsEnumerator<T>::value;

	/// @brief Type must be an `union`.
	template<typename T>
	concept Union = Impl::IsUnion<T>::value;

	/// @brief Type must be a `class` or `struct`.
	template<typename T>
	concept Class = Impl::IsClass<T>::value;

	/// @brief Type must be `Base` or a subclass of it.
	template<typename T, typename Base>
	concept Derived	= Impl::IsDerived<T, Base>::value;

	/// @brief Type must be a subclass of (or is) `Base`.
	template<typename T, typename Base>
	concept Subclass = Derived<T, Base> && Different<T, Base>;

	/// @brief Type must be a standard type (Every type but `class`es, `struct`s or `union`s).
	template <typename T>
	concept Standard = !(Class<T> || Union<T>);

	/// @brief Type must be a "primitive" type (Standard types, excluding `enum`s).
	template <typename T>
	concept Primitive = Standard<T> && !Enumerator<T>;

	/// @brief Type must be a fundamental type (Number, `void` or `nulltype`).
	template <typename T>
	concept Fundamental = Number<T> || Void<T> || Null<T>;

	/// @brief Type must not be a standard type.
	template <typename T>
	concept Derivable = !Standard<T>;

	/// @brief Type must be a scalar type (Number, pointer, member pointer or `nulltype`).
	template<typename T>
	concept Scalar = Number<T> || Pointer<T> || MemberPointer<T> || Null<T>;

	/// @brief Type must be constructible from `Args`.
	template <class T, typename... Args>
	concept Constructible = Impl::IsConstructible<T, Args...>::value;

	/// @brief `A + B` must be a valid operation.
	template <typename A, typename B>
	concept Addable = requires (A a, B b) {a + b;};

	/// @brief `A - B` must be a valid operation.
	template <typename A, typename B>
	concept Subtractable = requires (A a, B b) {a - b;};

	/// @brief `A * B` must be a valid operation.
	template <typename A, typename B>
	concept Multipliable = requires (A a, B b) {a * b;};

	/// @brief `A / B` must be a valid operation.
	template <typename A, typename B>
	concept Dividable = requires (A a, B b) {a / b;};

	/// @brief `A % B` must be a valid operation.
	template <typename A, typename B>
	concept Modulable = requires (A a, B b) {a % b;};

	/// @brief `A += B` must be a valid operation.
	template <typename A, typename B>
	concept AddAssignable = requires (A a, B b) {a += b;};

	/// @brief `A -= B` must be a valid operation.
	template <typename A, typename B>
	concept SubAssignable = requires (A a, B b) {a -= b;};

	/// @brief `A *= B` must be a valid operation.
	template <typename A, typename B>
	concept MulAssignable = requires (A a, B b) {a *= b;};

	/// @brief `A /= B` must be a valid operation.
	template <typename A, typename B>
	concept DivAssignable = requires (A a, B b) {a /= b;};

	/// @brief `A %= B` must be a valid operation.
	template <typename A, typename B>
	concept ModAssignable = requires (A a, B b) {a %= b;};

	/// @brief `T++` must be a valid operation.
	template <typename T>
	concept PostIncrementable = requires (T t) {t++;};

	/// @brief `T--` must be a valid operation.
	template <typename T>
	concept PostDecrementable = requires (T t) {t--;};

	/// @brief `++T` must be a valid operation.
	template <typename T>
	concept PreIncrementable = requires (T t) {++t;};

	/// @brief `--T` must be a valid operation.
	template <typename T>
	concept PreDecrementable = requires (T t) {--t;};

	/// @brief Bitwise operations type constraints.
	namespace Bitwise {
		/// @brief `A & B` must be a valid operation.
		template <typename A, typename B>
		concept Andable = requires (A a, B b) {a & b;};

		/// @brief `A | B` must be a valid operation.
		template <typename A, typename B>
		concept Orable = requires (A a, B b) {a | b;};

		/// @brief `A ^ B` must be a valid operation.
		template <typename A, typename B>
		concept Xorable = requires (A a, B b) {a ^ b;};
		
		/// @brief `A & B`, `A | B`, and `A ^ B` must be valid operations.
		template <typename A, typename B>
		concept Expressionable =
			Andable<A, B>
		&&	Orable<A, B>
		&&	Xorable<A, B>
		;
		
		/// @brief `A &= B` must be a valid operation.
		template <typename A, typename B>
		concept AndAssignable = requires (A a, B b) {a &= b;};

		/// @brief `A |= B` must be a valid operation.
		template <typename A, typename B>
		concept OrAssignable = requires (A a, B b) {a |= b;};

		/// @brief `A ^= B` must be a valid operation.
		template <typename A, typename B>
		concept XorAssignable = requires (A a, B b) {a ^= b;};

		/// @brief `A &= B`, `A |= B` and `A ^= B` must be valid operations.
		template <typename A, typename B>
		concept Assignable =
			AndAssignable<A, B>
		&&	OrAssignable<A, B>
		&&	XorAssignable<A, B>
		;

		/// @brief `~T` must be a valid operation.
		template <typename T>
		concept Negatable = requires (T t) {~t;};

		/// @brief
		///		All bitwise operations (with `A` in the left-hand side)
		///		must be valid, and both must be negatable.
		/// @note Operations: `A & B`, `A | B`, `A ^ B`, `A &= B`, `A |= B`, `A ^= B`, `~A` and `~B`.
		template<typename A, typename B>
		concept All =
			Expressionable<A, B>
		&&	Assignable<A, B>
		&&	Negatable<A>
		&&	Negatable<B>
		;
	}

	/// @brief Logical operations type constraints.
	namespace Logical {
		/// @brief `A && B` must be a valid operation.
		template <typename A, typename B>
		concept Andable = requires (A a, B b) {a && b;};

		/// @brief `A || B` must be a valid operation.
		template <typename A, typename B>
		concept Orable = requires (A a, B b) {a || b;};

		/// @brief `!T` must be a valid operation.
		template <typename T>
		concept Negatable = requires (T t) {!t;};
		
		/// @brief
		///		All logical operations (with `A` in the left-hand side)
		///		must be valid, and both must be negatable.
		/// @note Operations: `A && B`, `A || B`, `!A` and `!B`.
		template <typename A, typename B>
		concept All =
			Andable<A, B>
		&&	Orable<A, B>
		&&	Negatable<A>
		&&	Negatable<B>
		;
	}

	/// @brief Comparison operators type constraints.
	namespace Comparable {
		/// @brief `A == B` must be a valid operation.
		template <typename A, typename B>
		concept Equals = requires (A a, B b) {a == b;};

		/// @brief `A != B` must be a valid operation.
		template <typename A, typename B>
		concept NotEquals = requires (A a, B b) {a != b;};

		/// @brief `A < B` must be a valid operation.
		template <typename A, typename B>
		concept Lesser = requires (A a, B b) {a < b;};

		/// @brief `A > B` must be a valid operation.
		template <typename A, typename B>
		concept Greater = requires (A a, B b) {a > b;};

		/// @brief `A <= B` must be a valid operation.
		template <typename A, typename B>
		concept LesserEquals = requires (A a, B b) {a <= b;};

		/// @brief `A >= B` must be a valid operation.
		template <typename A, typename B>
		concept GreaterEquals = requires (A a, B b) {a >= b;};

		/// @brief `A <=> B` must be a valid operation.
		template <typename A, typename B>
		concept Threeway = requires (A a, B b) {a <=> b;};

		/// @brief
		///		All comparison operations (with `A` in the left-hand side),
		///		except threeway comparison, must be valid.
		/// @note
		///		Operations: `A == B`, `A != B`, `A < B`, `A > B`, `A <= B` and `A >= B`.
		///		If threeway comparison is implemented, all (except `==` and `!=`)
		///		are implicitly valid.
		template <typename A, typename B>
		concept All =
			Equals<A, B>
		&&	NotEquals<A, B>
		&&	Lesser<A, B>
		&&	Greater<A, B>
		&&	LesserEquals<A, B>
		&&	GreaterEquals<A, B>
		;
	}

	/// @brief Stream operations type constraints.
	namespace Stream {
		/// @brief `A << B` must be a valid operation.
		template <typename A, typename B>
		concept Insertible = requires (A a, B b) {a << b;};

		/// @brief `A >> B` must be a valid operation.
		template <typename A, typename B>
		concept Extractible = requires (A a, B b) {a >> b;};

		/// @brief `A << B` and `A >> B` must be valid operations.
		template <typename A, typename B>
		concept Expressionable = Insertible<A, B> && Extractible<A, B>;
		
		/// @brief `A <<= B` must be a valid operation.
		template <typename A, typename B>
		concept InsAssignable = requires (A a, B b) {a <<= b;};

		/// @brief `A >>= B` must be a valid operation.
		template <typename A, typename B>
		concept ExtAssignable = requires (A a, B b) {a >>= b;};
		
		/// @brief `A <<= B` and `A >>= B` must be valid operations.
		template <typename A, typename B>
		concept Assignable = InsAssignable<A, B> && ExtAssignable<A, B>;

		/// @brief All stream operations (with `A` in the left-hand side) must be valid.
		/// @note Operations: `A << B`, `A >> B`, `A <<= B` and `A >>= B`.
		template <typename A, typename B>
		concept All = Expressionable<A, B> && Assignable<A, B>;
	}

	/// @brief `A + B`, `A - B`, `A * B` and `A / B` must be valid operations.
	template <typename A, typename B>
	concept Calculable =
			Addable<A, B>
		&&	Subtractable<A, B>
		&&	Multipliable<A, B>
		&&	Dividable<A, B>
	;

	/// @brief `A += B`, `A -= B`, `A *= B` and `A /= B` must be valid operations.
	template <typename A, typename B>
	concept CalcAssignable =
			AddAssignable<A, B>
		&&	SubAssignable<A, B>
		&&	MulAssignable<A, B>
		&&	DivAssignable<A, B>
	;

	/// @brief
	///		All basic arithmetic operations (with `A` in the left-hand side),
	///		except modulo, must be valid.
	template <typename A, typename B>
	concept Arithmetic = Calculable<A, B> && CalcAssignable<A, B>;

	/// @brief `A % B` and `A %= B` must be valid operations.
	template <typename A, typename B>
	concept HasModulo = Modulable<A, B> && ModAssignable<A, B>;

	/**
	* @brief Type must be a "safe" type.
	*
	* A "safe" type must not be:
	*
	*	1. A pointer (null or non-null).
	*
	*	2. A reference.
	*
	*	3. A function.
	*
	*	4. `void`.
	*
	* Ergo, a "raw value" type.
	*/
	template <typename T>
	concept Safe = !(
			Pointer<T>
		||	Reference<T>
		||  Function<T>
		||	Void<T>
		||	Null<T>
	);

	/// @brief Type must be a safe type, and a C-style array.
	template <typename T>
	concept SafeArray = Array<T> && Safe<T>;

	/// @brief Type must be a safe type, and not be an array, or constant.
	template <typename T>
	concept Mutable = Safe<T> && !(Array<T> || Constant<T>);

	/// @brief Type must be usable in a compile-time expression.
	template <typename T>
	concept CompileTimeable = Impl::IsCompileTimeable<T>::value;

	/// @brief Type must be complete.
	template <class T>
	concept Defined = Impl::IsDefined<T>::value;

	/// @brief Type must represent a character of text.
	template<class T>
	concept Character =
		Type::Equal<T, char>
	||	Type::Equal<T, wchar_t>
	||	Type::Equal<T, char16_t>
	||	Type::Equal<T, char32_t>
	||	Type::Equal<T, char8_t>
	;

	/// @brief Type must represent an ASCII character.
	template<class T>
	concept ASCII =
		Type::Equal<T, char>
	||	Type::Equal<T, wchar_t>
	;

	/// @brief Type must be callable, and must behave like a function `F`.
	template<class T, typename F>
	concept Functional = Impl::IsFunctional<T, F>::value;

	/// @brief Type must be one in a series of `Types`.
	template<class T, typename... Types>
	concept OneOf = (... || Equal<T, Types>);

	/// @brief Type must NOT be one in a series of `Types`.
	template<class T, typename... Types>
	concept NoneOf = !OneOf<T, Types...>;

	/// @brief Type must be equal to `T2` or `T2 const`.
	template<class T, typename T2>
	concept EqualOrConst = OneOf<T, AsNonConst<T2>, AsNonConst<T2> const>;

	/// @brief Constraint class `T` must be true when given a series of `Types`.
	template<template <class...> class T, class... Types>
	concept Valid = Impl::IsValid<T, Types...>::value;

	/// @brief Type must be an iteratable type.
	template<class T>
	concept Iteratable = requires (T t) {
		{t.begin()};
		{t.end()};
	};

	/// @brief Type must be able to be "C-style" iterated.
	template<class T>
	concept CIteratable = requires (T t) {
		{t.data()};
		{t.size()};
	};

	/// @brief Type must be a polymorphic type.
	template <class T>
	concept Polymorphic = Impl::IsPolymorphic<T>::value;

	/// @brief Type must have a virtual destructor.
	template <class T>
	concept Virtual = Impl::IsVirtual<T>::value;
	
	/// @brief Type must NOT be constant.
	template <class T>
	concept NonConstant = Type::Different<T, T const>;

	/// @brief Type must have a default (empty) constructor.
	template <class T>
	concept DefaultConstructible = Type::Constructible<T>;
	
	/// @brief Type must be copy constructible.
	template <class T>
	concept CopyConstructible = Type::Constructible<T, T const&>;

	/// @brief Type must be move constructible.
	template <class T>
	concept MoveConstructible = Type::Constructible<T, T&&>;

	/// @brief Type must be copy assignable.
	template <class T>
	concept CopyAssignable = requires (T a, T const& b) {a = b;};

	/// @brief Type must be move assignable.
	template <class T>
	concept MoveAssignable = requires (T a, T&& b) {a = b;};
	
	/// @brief `*A` must be a valid operation.
	template <typename A>
	concept Dereferenciable = requires (A a) {*a;};

	/// @brief Type must be some form of an iterator.
	template <class T>
	concept Iterator =
		PreIncrementable<T>
	&&	Addable<T, usize>
	&&	Subtractable<T, usize>
	&&	Subtractable<T, T>
	&&	Comparable::NotEquals<T, T>
	&&	Comparable::Equals<T, T>
	;

	/// @brief Type must be a reference iterator.
	template <class T>
	concept ReferenceIterator = Iterator<T> && Dereferenciable<T>;

	/// @brief Type must be `To`, or be (implicitly) convertible to it.
	template<class T, class To>
	concept CanBecome = Type::Equal<T, To> || Type::Convertible<T, To>;

	
	/// @brief Type must match a given rule.
	template <class T, template <class> class TRule>
	concept Matches = TRule<T>::value;
}

CTL_NAMESPACE_END

#endif // CTL_TYPETRAITS_TRAITS_H
