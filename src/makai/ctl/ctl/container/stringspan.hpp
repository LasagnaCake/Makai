#ifndef CTL_CONTAINER_STRINGREF_H
#define CTL_CONTAINER_STRINGREF_H

#include "span.hpp"
#include "string.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Fixed-size view of a string.
/// @tparam TChar Character type.
/// @tparam TIndex Index size.
template<Type::ASCII TChar, Type::Integer TIndex>
struct BaseStringSpan:
	public Span<TChar const, DYNAMIC_SIZE, TIndex, ExtentSize::CES_DYNAMIC>,
	public SelfIdentified<BaseStringSpan<TChar, TIndex>>,
	public Derived<Span<TChar const, DYNAMIC_SIZE, TIndex, ExtentSize::CES_DYNAMIC>>,
	public CStringable<TChar>,
	public Streamable<TChar> {
	using SelfIdentified	= ::CTL::SelfIdentified<BaseStringSpan<TChar, TIndex>>;
	using Derived			= ::CTL::Derived<Span<TChar const, DYNAMIC_SIZE, TIndex, ExtentSize::CES_DYNAMIC>>;
	
	using typename Derived::BaseType;

	using typename SelfIdentified::SelfType;

	using 
		typename BaseType::DataType,
		typename BaseType::PointerType,
		typename BaseType::ConstPointerType,
		typename BaseType::SizeType,
		typename BaseType::IndexType,
		typename BaseType::IteratorType,
		typename BaseType::ConstIteratorType
	;

	using
		BaseType::empty,
		BaseType::size,
		BaseType::data,
		BaseType::begin,
		BaseType::end,
		BaseType::rbegin,
		BaseType::rend,
		BaseType::cbegin,
		BaseType::cend
	;

	constexpr BaseStringSpan(): BaseType() {}

	constexpr BaseStringSpan(ConstPointerType const data, SizeType const size):
		BaseType(data, size) {}

	constexpr BaseStringSpan(ConstPointerType const data):
		BaseType(data, endOf(data)) {}

	constexpr BaseStringSpan(ConstIteratorType const& begin, ConstIteratorType const& end):
		BaseType(begin, end) {}

	constexpr BaseStringSpan(ConstIteratorType const& begin):
		BaseType(begin, endOf(begin.raw())) {}

	/// @brief Constructs a `StringSpan` from a ranged object.
	/// @tparam T Ranged object type.
	/// @param other Object to view from.
	template<Type::Container::Ranged<IteratorType, ConstIteratorType> T>
	constexpr BaseStringSpan(T const& other)
	requires (!Type::Derived<T, SelfType>):
		BaseType(other.begin(), other.end())	{}

	/// @brief Constructs a `StringSpan` from a bounded object.
	/// @tparam T Bounded object type.
	/// @param other Object to view from.
	template<Type::Container::Bounded<ConstPointerType, SizeType> T>
	constexpr BaseStringSpan(T const& other)
	requires (
		!Type::Derived<T, SelfType>
	&&	!Type::Container::Ranged<T, IteratorType, ConstIteratorType>
	): BaseType(other.data(), other.size())	{}

	constexpr BaseStringSpan(SelfType const& other):
		BaseType(other) {}

private:
	constexpr static SizeType endOf(ConstPointerType const start) {
		ConstPointerType end = start;
		while(end) ++end;
		return end - start + 1;
	}
};

CTL_NAMESPACE_END

#endif