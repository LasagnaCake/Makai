#ifndef CTL_TYPE_INFO_H
#define CTL_TYPE_INFO_H

#include "typetraits/traits.hpp"
#include "meta/logic.hpp"
#include "typetraits/enum.hpp"
#include "namespace.hpp"
#include <typeinfo>
#include <cxxabi.h>

CTL_NAMESPACE_BEGIN

/// @brief Base classes.
namespace Base {
	template<class T>
	struct BasicInfo {
		/// @brief Info type.
		typedef T	DataType;

		/// @brief Size of type.
		constexpr static usize SIZE		= sizeof(T);
		/// @brief Bit size of type.
		constexpr static usize BIT_SIZE	= SIZE * 8;

		/// @brief Default constructor.
		constexpr BasicInfo() {}
		/// @brief Copy constructor (deleted).
		constexpr BasicInfo(BasicInfo const& other)	= delete;
		/// @brief Move constructor (deleted).
		constexpr BasicInfo(BasicInfo&& other)		= delete;

		/// @brief Compares two types.
		/// @param other Type to compare with.
		/// @return Whether they're equal.
		constexpr bool operator==(BasicInfo const& other) {return id->hash_code() == other.id->hash_code();}

		/// @brief Returns the raw name of the type.
		/// @return Raw name of the type.
		constexpr static cstring rawName()	{return id->name();}
		/// @brief Returns the name of the type.
		/// @return Type name.
		constexpr static cstring name()		{return abi::__cxa_demangle(id->name(),0,0,NULL);}

	private:
		/// @brief Underlying type info structure.
		constexpr static ref<const std::type_info> id = &typeid(T);
	};
}

/// @brief Value limit for a specific type.
/// @tparam T Type.
/// @tparam H Highest value.
/// @tparam S Smallest value.
/// @tparam L Lowest value.
template<class T, auto H, auto S, auto L>
struct ValueLimit {
	constexpr static T HIGHEST	= H;
	constexpr static T SMALLEST	= S;
	constexpr static T LOWEST	= L;
};

/// @brief Number limit.
/// @tparam Type Number type.
template<Type::Number T>
struct NumberLimit;

/// @brief Number limit.
template<> struct NumberLimit<bool>:	ValueLimit<bool,	0x1,				1,	0					>	{};
/// @brief Number limit.
template<> struct NumberLimit<char>:	ValueLimit<char,	0x7F,				1,	-0x80				>	{};

/// @brief Number limit.
template<> struct NumberLimit<int8>:	ValueLimit<int8,	0x7F,				1,	-0x80				>	{};
/// @brief Number limit.
template<> struct NumberLimit<int16>:	ValueLimit<int16,	0x7FFF,				1,	-0x8000				>	{};
/// @brief Number limit.
template<> struct NumberLimit<int32>:	ValueLimit<int32,	0x7FFFFFFF,			1,	-0x80000000			>	{};
/// @brief Number limit.
template<> struct NumberLimit<int64>:	ValueLimit<int64,	0x7FFFFFFFFFFFFFFF,	1,	-0x8000000000000000	>	{};

/// @brief Number limit.
template<> struct NumberLimit<uint8>:	ValueLimit<uint8,	0xFF,				1,	0					>	{};
/// @brief Number limit.
template<> struct NumberLimit<uint16>:	ValueLimit<uint16,	0xFFFF,				1,	0					>	{};
/// @brief Number limit.
template<> struct NumberLimit<uint32>:	ValueLimit<uint32,	0xFFFFFFFF,			1,	0					>	{};
/// @brief Number limit.
template<> struct NumberLimit<uint64>:	ValueLimit<uint64,	0xFFFFFFFFFFFFFFFF,	1,	0					>	{};

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
/// @brief Number limit.
template<> struct NumberLimit<int>:		NumberLimit<Meta::DualType<sizeof(int) == sizeof(int32),	int32,	int16>>		{};
/// @brief Number limit.
template<> struct NumberLimit<uint>:	NumberLimit<Meta::DualType<sizeof(uint) == sizeof(uint32),	uint32,	uint16>>	{};
#endif

/// @brief Number limit.
template<> struct NumberLimit<float>:	ValueLimit<float,	__FLT_MAX__,	__FLT_MIN__,	-__FLT_MAX__>	{};
/// @brief Number limit.
template<> struct NumberLimit<double>:	ValueLimit<double,	__DBL_MAX__,	__DBL_MIN__,	-__DBL_MAX__>	{};
/// @brief Number limit.
template<> struct NumberLimit<ldouble>:	ValueLimit<ldouble,	__LDBL_MAX__,	__LDBL_MIN__,	-__LDBL_MAX__>	{};

/// @brief Type information.
/// @tparam T Type.
template<typename T>
struct TypeInfo;

/// @brief Type information.
/// @tparam T Type.
template<Type::Integer T>
struct TypeInfo<T>: NumberLimit<T>, Base::BasicInfo<T> {};

/// @brief Type information.
/// @tparam T Type.
template<Type::Real T>
struct TypeInfo<T>: NumberLimit<T>, Base::BasicInfo<T> {};

/// @brief Type information.
/// @tparam T Type.
template<Type::Enumerator T>
struct TypeInfo<T>: NumberLimit<Decay::Enum::AsInteger<T>>, Base::BasicInfo<T> {};

/// @brief Type information.
/// @tparam T Type.
template<Type::Class T>
struct TypeInfo<T>: Base::BasicInfo<T> {};

namespace Limit {
	template <Type::Number T> constexpr T const MAX		= TypeInfo<T>::HIGHEST;
	template <Type::Number T> constexpr T const MIN		= TypeInfo<T>::LOWEST;
	template <Type::Number T> constexpr T const STRIDE	= TypeInfo<T>::SMALLEST;
}

CTL_NAMESPACE_END

#endif // CTL_TYPE_INFO_H
