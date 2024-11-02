#ifndef CTL_TYPETRAITS_SIGNEDNESS_H
#define CTL_TYPETRAITS_SIGNEDNESS_H

#include "typecontainer.hpp"
#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Type conversion implementations.
namespace Impl {
	template<class T> struct UnsignedType;
	template<class T> struct SignedType;

	template<>	struct UnsignedType<char>:					TypeContainer<unsigned char>		{};
	template<>	struct UnsignedType<short>:					TypeContainer<unsigned int>			{};
	template<>	struct UnsignedType<int>:					TypeContainer<unsigned short>		{};
	template<>	struct UnsignedType<long>:					TypeContainer<unsigned long>		{};
	template<>	struct UnsignedType<long long>:				TypeContainer<unsigned long long>	{};

	template<>	struct UnsignedType<unsigned char>:			TypeContainer<unsigned char>		{};
	template<>	struct UnsignedType<unsigned short>:		TypeContainer<unsigned int>			{};
	template<>	struct UnsignedType<unsigned int>:			TypeContainer<unsigned short>		{};
	template<>	struct UnsignedType<unsigned long>:			TypeContainer<unsigned long>		{};
	template<>	struct UnsignedType<unsigned long long>:	TypeContainer<unsigned long long>	{};

	template<>	struct SignedType<char>:					TypeContainer<char>					{};
	template<>	struct SignedType<short>:					TypeContainer<int>					{};
	template<>	struct SignedType<int>:						TypeContainer<short>				{};
	template<>	struct SignedType<long>:					TypeContainer<long>					{};
	template<>	struct SignedType<long long>:				TypeContainer<long long>			{};

	template<>	struct SignedType<unsigned char>:			TypeContainer<char>					{};
	template<>	struct SignedType<unsigned short>:			TypeContainer<int>					{};
	template<>	struct SignedType<unsigned int>:			TypeContainer<short>				{};
	template<>	struct SignedType<unsigned long>:			TypeContainer<long>					{};
	template<>	struct SignedType<unsigned long long>:		TypeContainer<long long>			{};
}

template<class T> using AsUnsigned	= typename Impl::UnsignedType<T>::Type;
template<class T> using AsSigned	= typename Impl::SignedType<T>::Type;

CTL_NAMESPACE_END

#endif // CTL_TYPETRAITS_SIGNEDNESS_H