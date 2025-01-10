#ifndef CTL_CONTAINER_STRING_H
#define CTL_CONTAINER_STRING_H

#include <stdlib.h>
#include <string_view>
#include <string>

#include "list.hpp"
#include "array.hpp"
#include "pair.hpp"
#include "arguments.hpp"
#include "../typeinfo.hpp"
#include "../cpperror.hpp"
#include "../io/stream.hpp"
#include "../algorithm/aton.hpp"
#include "../algorithm/transform.hpp"
#include "../algorithm/validate.hpp"
#include "../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Dynamic string of characters.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Allocator type.
template<
	Type::ASCII TChar,
	Type::Integer TIndex = usize,
	template <class> class TAlloc = HeapAllocator
>
struct BaseString:
	private List<TChar, TIndex, TAlloc>,
	public SelfIdentified<BaseString<TChar, TIndex>>,
	public Derived<List<TChar, TIndex>>,
	public CStringable<TChar>,
	public Streamable<TChar> {
public:
	using Iteratable		= ::CTL::Iteratable<TChar, TIndex>;
	using SelfIdentified	= ::CTL::SelfIdentified<BaseString<TChar, TIndex>>;
	using Derived			= ::CTL::Derived<List<TChar, TIndex>>;
	using Streamable		= ::CTL::Streamable<TChar>;
	using CStringable		= ::CTL::CStringable<TChar>;

	using typename Derived::BaseType;

	using typename BaseType::OrderType;

	using
		typename BaseType::PredicateType,
		typename BaseType::CompareType
	;

	using
		typename BaseType::DataType,
		typename BaseType::ConstantType,
		typename BaseType::PointerType,
		typename BaseType::ConstPointerType,
		typename BaseType::ReferenceType,
		typename BaseType::ConstReferenceType,
		typename BaseType::ArgumentListType
	;

	using
		typename BaseType::IndexType,
		typename BaseType::SizeType
	;

	using BaseType::MAX_SIZE;

	using
		typename BaseType::IteratorType,
		typename BaseType::ConstIteratorType,
		typename BaseType::ReverseIteratorType,
		typename BaseType::ConstReverseIteratorType
	;

	using
		typename BaseType::AllocatorType
	;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename CStringable::CStringType
	;

	using
		typename Streamable::InputStreamType,
		typename Streamable::OutputStreamType
	;

	/// @brief STL library view analog.
	using STDViewType	= std::basic_string_view<DataType>;
	/// @brief STL library string analog.
	using STDStringType	= std::basic_string<DataType>;

	using
//		BaseType::BaseType,
		BaseType::allocator,
//		BaseType::clear,
//		BaseType::reserve,
//		BaseType::resize,
		BaseType::tight,
//		BaseType::tighten,
		BaseType::data,
		BaseType::cbegin,
//		BaseType::cend,
		BaseType::begin,
//		BaseType::end,
//		BaseType::rbegin,
		BaseType::rend,
		BaseType::front
//		BaseType::back,
//		BaseType::transformed,
//		BaseType::validate,
//		BaseType::size,
//		BaseType::empty,
//		BaseType::find,
//		BaseType::rfind,
//		BaseType::bsearch,
//		BaseType::capacity,
//		BaseType::appendBack,
//		BaseType::pushBack,
//		BaseType::popBack
	;

	/// @brief Empty constructor.
	constexpr BaseString(): BaseType() {
		pushBack('\0');
	}

	/// @brief Constructs the `BaseString` with a preallocated capacity.
	/// @param size Size to preallocate.
	constexpr explicit BaseString(SizeType const size): BaseType(size+1) {}

	/// @brief Constructs a `BaseString` of a given size and a given fill.
	/// @param size Size to allocate.
	/// @param fill Value to set for each character.
	constexpr explicit BaseString(SizeType const size, DataType const& fill): BaseType(size+1, fill) {
		BaseType::back() = '\0';
	}

	/// @brief Constructs the `BaseString` from a fixed array of characters.
	/// @tparam S Size of array.
	/// @param values Characters to add to `BaseString`.
	template<SizeType S>
	constexpr BaseString(As<ConstantType[S]> const& values) {
		BaseType::resize(values[S-1] == '\0' ? S : S+1);
		BaseType::appendBack(values);
		if (values[S-1] != '\0') BaseType::pushBack('\0');
	}

	/// @brief Copy constructor.
	/// @param other `BaseString` to copy from.
	constexpr BaseString(SelfType const& other) {
		BaseType::resize(other.size()+1);
		BaseType::appendBack(other.begin(), other.end());
		BaseType::pushBack('\0');
	}

	/// @brief Move constructor.
	/// @param other `BaseString` to move from.
	constexpr BaseString(SelfType&& other) {
		BaseType::resize(other.size()+1);
		BaseType::appendBack(other.begin(), other.end());
		BaseType::pushBack('\0');
	}

	/// @brief Constructs a `BaseString` from a range of characters.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	constexpr BaseString(ConstIteratorType const& begin, ConstIteratorType const& end) {
		if (end <= begin) return;
		if (*(end-1) == '\0') {
			BaseType::resize(end - begin + 1);
			BaseType::appendBack(begin, end);
		} else {
			BaseType::resize(end - begin + 2);
			BaseType::appendBack(begin, end);
			BaseType::pushBack('\0');
			
		}
	}

	/// @brief Constructs a `BaseString` from a range of characters.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator to end of range.
	constexpr BaseString(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end) {
		if (end <= begin) return;
		if (*(end-1) == '\0') {
			BaseType::resize(end - begin + 1);
			BaseType::appendBack(begin, end);
		} else {
			BaseType::resize(end - begin + 2);
			BaseType::appendBack(begin, end);
			BaseType::pushBack('\0');
			
		}
	}

	/// @brief Constructs a `BaseString` from a "C-style" range of characters.
	/// @param start Start of range.
	/// @param size Size of range.
	constexpr BaseString(ConstPointerType const& start, SizeType const size): BaseString(start, start + size) {}

	/// @brief Constructs a `BaseString`, from a ranged object of (non-subclass) type T.
	/// @tparam T Ranged type.
	/// @param other Object to copy from.
	template<Type::Container::Ranged<IteratorType, ConstIteratorType> T>
	constexpr BaseString(T const& other)
	requires (!Type::Subclass<T, SelfType>):
		BaseString(other.begin(), other.end()) {}
	
	/// @brief Constructs a `BaseString`, from a bounded object of (non-list) type T.
	/// @tparam T Ranged type.
	/// @param other Object to copy from.
	template<Type::Container::Bounded<PointerType, SizeType> T>
	constexpr explicit BaseString(T const& other)
	requires (
		!Type::Container::List<T>
	&&	!Type::Container::Ranged<T, IteratorType, ConstIteratorType>
	): BaseString(other.data(), other.size()) {}

	/// @brief Destructor.
	constexpr ~BaseString() {}
	
	/// @brief Constructs a `BaseString` from a null-terminated strin.
	/// @param v String to copy from.
	constexpr BaseString(CStringType const& v) {
		SizeType len = 0;
		while (v[len++] != '\0' && len <= MAX_SIZE);
		BaseType::reserve(len);
		BaseType::appendBack(BaseType(v, v+len));
		BaseType::pushBack('\0');
	}

	/// @brief Constructs a `BaseString` from a series of arguments.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	template<class... Args>
	constexpr BaseString(Args const&... args)
	requires (... && Type::Equal<Args, SelfType>) {
		(*this) += (... + args);
	}

	/// @brief Constructos a `BaseString` from a STL view analog.
	/// @param str View to copy from.
	constexpr BaseString(STDViewType const& str):	BaseType(&*str.begin(), &*str.end())	{}
	/// @brief Constructos a `BaseString` from a STL string analog.
	/// @param str View to copy from.
	constexpr BaseString(STDStringType const& str):	BaseType(&*str.begin(), &*str.end())	{}
	
	/// @brief Adds a new character to the end of the `BaseString`. 
	/// @param value Character to add.
	/// @return Reference to self.
	constexpr SelfType& pushBack(DataType const& value) {
		BaseType::back() = value;
		BaseType::pushBack('\0');
		return *this;
	}

	constexpr DataType popBack() {
		BaseType::popBack();
		DataType value = BaseType::back();
		BaseType::back() = '\0';
		return value;
	}
	
	constexpr SelfType& insert(DataType const& value, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(value, index);
		return *this;
	}

	constexpr SelfType& insert(SelfType const& other, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(BaseType(other.begin(), other.end()), index);
		return *this;
	}
	
	constexpr SelfType& insert(DataType const& value, SizeType const count, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(count, value, index);
		return *this;
	}

	template<SizeType S>
	constexpr SelfType& insert(As<ConstantType[S]> const& values, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(values, index);
		return *this;
	}

	constexpr SelfType& reserve(SizeType const count) {
		BaseType::reserve(count + 1);
		return *this;
	}

	constexpr SelfType& resize(SizeType const newSize) {
		BaseType::resize(newSize + 1);
		return *this;
	}

	constexpr SelfType& expand(SizeType const count) {
		BaseType::expand(count + 1);
		return *this;
	}

	constexpr SelfType& reserve(SizeType const count, DataType const& fill) {
		BaseType::back() = fill;
		BaseType::reserve(count + 1, fill);
		BaseType::back() = '\0';
		return *this;
	}
	
	constexpr SelfType& resize(SizeType const newSize, DataType const& fill) {
		BaseType::back() = fill;
		BaseType::resize(newSize + 1, fill);
		BaseType::back() = '\0';
		return *this;
	}
	
	constexpr SelfType& expand(SizeType const count, DataType const& fill) {
		BaseType::back() = fill;
		BaseType::expand(count + 1, fill);
		BaseType::back() = '\0';
		return *this;
	}

	constexpr SelfType& tighten() {
		BaseType::tighten();
		return *this;
	}

	constexpr SelfType& reverse() {
		::CTL::reverse(data(), size());
		return *this;
	}

	constexpr SelfType reversed() const {
		return SelfType(*this).reverse();
	}

	constexpr IndexType find(DataType const& value) const {
		IndexType location = BaseType::find(value);
		return (location == IndexType(size())) ? -1 : location;
	}

	constexpr IndexType rfind(DataType const& value) const {
		IndexType location = BaseType::find(value);
		return (location == IndexType(size())) ? -1 : location;
	}

	constexpr IndexType bsearch(DataType const& value) const {
		IndexType location = BaseType::find(value);
		return (location == IndexType(size())) ? -1 : location;
	}

	constexpr SelfType& remove(IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::remove(index);
	}
	
	constexpr SizeType removeLike(DataType const& value) {
		SizeType count = BaseType::removeLike(value) - (value == '\0');
		if (value == '\0') BaseType::back() = '\0';
		return count;
	}
	
	constexpr SizeType removeUnlike(DataType const& value) {
		SizeType count = BaseType::removeUnlike(value) - (value == '\0');
		if (value == '\0') BaseType::back() = '\0';
		return count;
	}

	template<class TPredicate>
	constexpr SizeType removeIf(TPredicate const& predicate) {
		SizeType count = BaseType::removeIf(predicate);
		if (predicate(BaseType::back()))
			--count;
		return count;
	}
	template<class TPredicate>
	constexpr SizeType removeIfNot(TPredicate const& predicate) {
		SizeType count = BaseType::removeIfNot(predicate);
		if (!predicate(BaseType::back()))
			--count;
		return count;
	}

	constexpr SelfType& erase(IndexType const index) {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::erase(index);
	}

	constexpr SelfType& eraseLike(DataType const& value) {
		resize(removeLike(value));
		return *this;
	}

	constexpr SelfType& eraseUnlike(DataType const& value) {
		resize(removeUnlike(value));
		return *this;
	}

	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType& eraseIf(TPredicate const& predicate) {
		resize(removeIf(predicate));
		return *this;
	}

	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType& eraseIfNot(TPredicate const& predicate) {
		resize(removeIfNot(predicate));
		return *this;
	}

	constexpr SelfType sliced(IndexType start) const {
		if (IndexType(size()) < start) return SelfType();
		assertIsInBounds(start);
		wrapBounds(start);
		return SelfType(begin() + start, end());
	}

	constexpr SelfType sliced(IndexType start, IndexType stop) const {
		if (IndexType(size()) < start) return SelfType();
		assertIsInBounds(start);
		wrapBounds(start);
		if (IndexType(size()) < stop) return sliced(start);
		assertIsInBounds(stop);
		wrapBounds(stop);
		if (stop < start) return SelfType();
		return SelfType(begin() + start, begin() + stop + 1);
	}

	constexpr SelfType& appendBack(SelfType const& other) {
		expand(other.size());
		BaseType::popBack();
		BaseType::appendBack(other.begin(), other.end());
		BaseType::pushBack('\0');
		return *this;
	}

	constexpr SelfType& appendBack(SizeType const count, DataType const& fill) {
		return expand(count, fill);
	}

	constexpr SelfType& appendBack(ConstIteratorType const& begin, ConstIteratorType const& end) {
		expand(end - begin);
		BaseType::popBack();
		BaseType::appendBack(begin, end);
		pushBack('\0');
		return *this;
	}

	constexpr SelfType& appendBack(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end) {
		expand(end - begin);
		BaseType::popBack();
		BaseType::appendBack(begin, end);
		BaseType::pushBack('\0');
		return *this;
	}

	template<SizeType S>
	constexpr SelfType& appendBack(As<DataType[S]> const& values) {
		if (values[S-1] == '\0') {
			expand(S);
			BaseType::popBack();
			BaseType::appendBack(values);
		} else {
			expand(S+1);
			BaseType::popBack();
			BaseType::appendBack(values);
			BaseType::pushBack('\0');
		}
		return *this;
	}

	constexpr SelfType& clear() {
		BaseType::clear();
		BaseType::pushBack('\0');
		return *this;
	}

	constexpr SelfType& dispose() {
		BaseType::dump();
		return *this;
	}

	constexpr IteratorType				end()			{return data()+size();								}
	constexpr ConstIteratorType			end() const		{return data()+size();								}
	constexpr PointerType				cend()			{return data()+size();								}
	constexpr ConstPointerType			cend() const	{return data()+size();								}
	constexpr ReverseIteratorType		rbegin()		{return ReverseIteratorType(data()+size());			}
	constexpr ConstReverseIteratorType	rbegin() const	{return ConstReverseIteratorType(data()+size());	}
	
	constexpr ReferenceType 	back()			{return at(size()-1);	}
	constexpr DataType 			back() const	{return at(size()-1);	}
	
	constexpr DataType& at(IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::at(index);
	}

	constexpr DataType at(IndexType index) const {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::at(index);
	}

	constexpr ReferenceType	operator[](IndexType index)			{return at(index);}
	constexpr DataType operator[](IndexType const index) const	{return at(index);}
	
	constexpr SizeType capacity() const	{return BaseType::capacity() - 1;	}
	constexpr SizeType empty() const	{return size() == 0;				}

	constexpr bool operator==(SelfType const& other) const {
		return BaseType::equals(other);
	}

	constexpr OrderType operator<=>(SelfType const& other) const {
		return compare(other);
	}

	constexpr SizeType equals(SelfType const& other) const {
		return BaseType::equals(other);
	}

	constexpr OrderType compare(SelfType const& other) const {
		return BaseType::compare(other);
	}

	constexpr SizeType disparity(SelfType const& other) const {
		return BaseType::disparity(other);
	}

	template <class TProcedure>
	constexpr SelfType& transform(TProcedure const& fun) {
		for(DataType& v: *this)
			v = fun(v);
		return *this;
	}

	template<class TProcedure>
	constexpr SelfType transformed(TProcedure const& fun) const {
		return SelfType(*this).transform(fun);
	}

	template<class TPredicate>
	constexpr bool validate(TPredicate const& cond) const {
		if (empty()) return false;
		for (DataType const& c: *this)
			if (!cond(c))
				return false;
		return true;
	}

	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType& filter(TPredicate const& filter) {
		return eraseIfNot(filter);
	}

	template<Type::Functional<CompareType> TCompare>
	constexpr SelfType& filter(TCompare const& compare) {
		return *this = filtered(compare);
	}

	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType filtered(TPredicate const& filter) const {
		return SelfType(*this).eraseIfNot(filter);
	}

	template<Type::Functional<CompareType> TCompare>
	constexpr SelfType filtered(TCompare const& compare) const {
		SelfType result;
		for (SizeType i = 0; i < size(); ++i) {
			bool miss = false;
			for(SizeType j = size() - 1; j >= 0; --j) {
				if (i == j) break;
				if ((miss = !compare(data()[i], data()[j])))
					break;
			}
			if (!miss) result.pushBack(data()[i]);
		}
		return result;
	}

	constexpr SelfType uniques() {
		return filtered([](ConstReferenceType a, ConstReferenceType b){return a != b;});
	}

	constexpr List<SelfType, SizeType> divide(IndexType index) const {
		List<SelfType, SizeType> res;
		assertIsInBounds(res);
		wrapBounds(index);
		res.pushBack(sliced(0, index));
		res.pushBack(sliced(index+1));
		return res;
	}

	/// @brief Returns a string without any leading whitespace characters.
	/// @return String without leading whitespace.
	constexpr SelfType stripped() {
		auto start	= begin();
		auto stop	= end();
		while (start != stop && isNullOrSpaceChar(*start))
			++start;
		while (stop != start && isNullOrSpaceChar(*stop))
			--stop;
		return SelfType(start, stop);
	}

	/// @brief Strips the string of any leading whitespace characters.
	/// @return reference to self.
	constexpr SelfType& strip() {
		*this = stripped();
	}

	/// @brief Splits the string by a separator.
	/// @param sep Separator.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> split(DataType const& sep) const {
		List<SelfType, SizeType> res;
		SelfType buf;
		for (ConstReferenceType v : *this) {
			if (v == sep) {
				res.pushBack(buf);
				buf.clear();
				continue;
			}
			buf += v;
		}
		res.pushBack(buf);
		if (res.empty()) res.pushBack(*this);
		return res;
	}
	
	/// @brief Splits the string by a series of separators.
	/// @param seps Separators.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> split(BaseType const& seps) const {
		List<SelfType, SizeType> res;
		SelfType buf;
		for (ConstReferenceType v : *this) {
			if (seps.find(v)) {
				res.pushBack(buf);
				buf.clear();
				continue;
			}
			buf.pushBack(v);
		}
		res.pushBack(buf);
		if (res.empty()) res.pushBack(*this);
		return res;
	}
	
	/// @brief Splits the string at the first character that matches the separator.
	/// @param sep Separator.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtFirst(DataType const& sep) const {
		List<SelfType, SizeType> res;
		IndexType idx = find(sep);
		if (idx < 0) res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}

	/// @brief Splits the string at the first character that matches one of the separators.
	/// @param seps Separators.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtFirst(BaseType const& seps) const {
		List<SelfType, SizeType> res;
		IndexType idx = -1;
		for(ConstReferenceType sep: seps) {
			IndexType i = find(sep);
			if (i > -1 && i < idx)
				idx = i;
		}
		if (idx < 0)	res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}

	/// @brief Splits the string at the last character that matches the separator.
	/// @param sep Separator.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtLast(DataType const& sep) const {
		List<SelfType, SizeType> res;
		IndexType idx = rfind(sep);
		if (idx < 0) res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}

	/// @brief Splits the string at the last character that matches one of the separators.
	/// @param seps Separators.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtLast(BaseType const& seps) const {
		List<SelfType, SizeType> res;
		IndexType idx = -1;
		for(ConstReferenceType sep: seps) {
			IndexType i = rfind(sep);
			if (i > -1 && i > idx)
				idx = i;
		}
		if (idx < 0) res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}
	
	/// @brief Replaces any character that matches, with the replacement.
	/// @param val Character to match.
	/// @param rep Replacement.
	/// @return Reference to self.
	constexpr SelfType& replace(DataType const& val, DataType const& rep) {
		for (DataType& v: *this)
			if (v == val) v = rep;
		return *this;
	}

	/// @brief Replaces any character that matches the set, with the replacement.
	/// @param values Characters to match.
	/// @param rep Replacement.
	/// @return Reference to self.
	constexpr SelfType& replace(BaseType const& values, DataType const& rep) {
		for (DataType const& val: values)
			replace(val, rep);
		return *this;
	}

	/// @brief Character replacement rule.
	struct Replacement {
		/// @brief Characters to replace.
		BaseType	targets;
		/// @brief Character to replace with.
		DataType	replacement;
	};

	/// @brief Replaces characters, acoording to a given replacement rule.
	/// @param rep Replacement rule.
	/// @return Reference to self.
	constexpr SelfType& replace(Replacement const& rep) {
		replace(rep.targets, rep.replacement);
		return *this;
	}

	/// @brief Replaces characters, acoording to a given list of rules.
	/// @param reps Replacement rules.
	/// @return Reference to self.
	constexpr SelfType& replace(List<Replacement, SizeType> const& reps) {
		for (Replacement const& rep: reps)
			replace(rep);
		return *this;
	}

	/// @brief Returns a string with any character that matches replaced.
	/// @param val Character to match.
	/// @param rep Replacement.
	/// @return Resulting string.
	constexpr SelfType replaced(DataType const& val, DataType const& rep) const				{return SelfType(*this).replace(val, rep);		}
	/// @brief Returns a string with any character that matches replaced.
	/// @param values Characters to match.
	/// @param rep Replacement.
	/// @return Resulting string.
	constexpr SelfType replaced(BaseType const& values, DataType const& rep) const			{return SelfType(*this).replace(values, rep);	}
//	constexpr SelfType replaced(ArgumentListType const& values, DataType const& rep) const	{return SelfType(*this).replace(values, rep);	}

	/// @brief Returns a string with any rule that matches replaced.
	/// @param rep Replacement rule.
	/// @return Resulting string.
	constexpr SelfType replaced(Replacement const& rep) const					{return SelfType(*this).replace(rep);	}
	/// @brief Returns a string with any rule that matches replaced.
	/// @param reps Replacement rules.
	/// @return Resulting string.
	constexpr SelfType replaced(List<Replacement, SizeType> const& reps) const	{return SelfType(*this).replace(reps);	}
//	constexpr SelfType replaced(Arguments<Replacement> const& reps) const		{return SelfType(*this).replace(reps);	}

	/// @brief Stream insertion operator.
	constexpr OutputStreamType& operator<<(OutputStreamType& o) const	{if (!empty()) o << cstr(); return o;}
	/// @brief Stream insertion operator.
	constexpr OutputStreamType& operator<<(OutputStreamType& o)			{if (!empty()) o << cstr(); return o;}
	
	/// @brief Stream insertion operator.
	friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType& self)			{if (!self.empty()) o << self.cstr(); return o;}
	/// @brief Stream insertion operator.
	friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType const& self)	{if (!self.empty()) o << self.cstr(); return o;}

	/// @brief Reads from an input stream until a character is reached.
	/// @param i Input stream.
	/// @param stop Stop character.
	/// @return Input stream.
	constexpr InputStreamType& readFrom(InputStreamType& i, DataType const& stop) {
		DataType buf[32];
		while(i.getline(buf, 32, stop))
			appendBack(buf, i.gcount());
		return i;
	}
	
	/// @brief Reads from an input stream until a null character is reached.
	/// @param i Input stream.
	/// @return Input stream.
	constexpr InputStreamType& readFrom(InputStreamType& i) {
		return readFromStream(i, '\0');
	}

	/// @brief Copy assignment operator (`BaseString`).
	/// @param other `BaseString` to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other)			{BaseType::operator=(other); return *this;				}
	/// @brief Move assignment operator (`BaseString`).
	/// @param other `BaseString` to move.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other)					{BaseType::operator=(CTL::move(other)); return *this;	}
	/// @brief Copy assignment operator (null-terminated string).
	/// @param other String to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(CStringType const& other)			{BaseType::operator=(SelfType(other)); return *this;	}

	/// @brief Insertion operator (`BaseString`).
	/// @param other `Basestring` to insert into.
	/// @return Const reference to self.
	constexpr SelfType const& operator<<(SelfType& other) const	{other.appendBack(*this); return *this;}
	/// @brief Insertion operator (`BaseString`).
	/// @param other `Basestring` to insert into.
	/// @return Reference to self.
	constexpr SelfType& operator<<(SelfType& other)				{other.appendBack(*this); return *this;}

	/// @brief Extraction operator (`BaseString`).
	/// @param other `Basestring` to extract from.
	/// @return Reference to self.
	constexpr SelfType& operator>>(SelfType const& other)	{appendBack(other); return other;}

	/// @brief String concatenation operator (character).
	/// @param value Character to concatenate.
	/// @return Resulting concatenated string.
	constexpr SelfType operator+(DataType const& value) const	{return SelfType(*this).pushBack(value);	}
	/// @brief String concatenation operator (`BaseString`).
	/// @param value `BaseString` to concatenate.
	/// @return Resulting concatenated string.
	constexpr SelfType operator+(SelfType const& other) const	{return SelfType(*this).appendBack(other);	}

	/// @brief String concatenation operator (null-terminated string).
	/// @param value String to concatenate.
	/// @return Resulting concatenated string.
	constexpr SelfType operator+(CStringType const& str) const				{return (*this) + SelfType(str);}
	/// @brief String concatenation operator (char array).
	/// @tparam S Array size.
	/// @param value Char array to concatenate.
	/// @return Resulting concatenated string.
	template<SizeType S>
	constexpr SelfType operator+(As<ConstantType[S]> const& str) const	{return (*this) + SelfType(str);}

	/// @brief String concatenation operator (character).
	/// @param value Character to concatenate.
	/// @param self `BaseString` to concatenate with.
	/// @return Resulting concatenated string.
	friend constexpr SelfType operator+(DataType const& value, SelfType const& self)	{return SelfType().pushBack(value) + self;	}

	/// @brief String concatenation operator (null-terminated string).
	/// @param value String to concatenate.
	/// @param self `BaseString` to concatenate with.
	/// @return Resulting concatenated string.
	friend constexpr SelfType operator+(CStringType const& str, SelfType const& self)					{return SelfType(str) + (self);}
	/// @brief String concatenation operator (char array).
	/// @tparam S Array size.
	/// @param value Char array to concatenate.
	/// @param self `BaseString` to concatenate with.
	/// @return Resulting concatenated string.
	template<SizeType S>
	friend constexpr SelfType operator+(As<ConstantType[S]> const& str, SelfType const& self)	{return SelfType(str) + (self);}
	
	/// @brief String appending operator (character).
	/// @param value Caracter to append.
	/// @return Reference to self.
	constexpr SelfType& operator+=(DataType const& value)				{pushBack(value); return *this;				}
	/// @brief String appending operator (`BaseString`).
	/// @param value `BaseString` to append.
	/// @return Reference to self.
	constexpr SelfType& operator+=(SelfType const& other)				{appendBack(other); return *this;			}
	/// @brief String appending operator (null-terminated string).
	/// @param value String to append.
	/// @return Reference to self.
	constexpr SelfType& operator+=(CStringType const& str)		{appendBack(SelfType(str)); return *this;	}
	/// @brief String appending operator (char array).
	/// @tparam S Array size.
	/// @param value Char array to append.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& operator+=(As<ConstantType[S]> str)	{appendBack(str); return *this;				}

	/// @brief Returns a string, repeated a given amount of times.
	/// @param times Amount of times to repeat.
	/// @return Resulting string.
	constexpr SelfType operator*(IndexType const times) const {
		if (times < 1) return SelfType();
		if (times == 1) return *this;
		SelfType result(size() * usize(times));
		for (SizeType i = 0; i < usize(times); ++i)
			result.appendBack(*this);
		return result;
	}

	/// @brief Repeats the string a given amount of times.
	/// @param times Amount of times to repeat.
	/// @return Reference to self.
	constexpr SelfType& operator*=(IndexType const times) {
		if (times < 1) return SelfType();
		if (times == 1) return *this;
		SelfType copy = *this;
		for (SizeType i = 0; i < times; ++i)
			appendBack(copy);
		return *this;
	}
	
	/// @brief Equality comparison operator (char array).
	/// @tparam S Array size.
	/// @param str Array to compare with.
	/// @return Whether they're equal.
	template<SizeType S>
	constexpr bool operator==(As<ConstantType[S]> const& str) const	{return *this == SelfType(str);			}
	/// @brief Equality comparison operator (null-terminated string).
	/// @param str String to compare with.
	/// @return Whether they're equal.
	constexpr bool operator==(CStringType const& str) const			{return *this == SelfType(str);			}
	/// @brief Threeway comparison operator (char array).
	/// @tparam S Array size.
	/// @param str Char array to compare with.
	/// @return Order between objects.
	template<SizeType S>
	constexpr OrderType operator<=>(As<ConstantType[S]> const& str) const	{return *this <=> SelfType(str);		}
	/// @brief Threeway comparison operator (null-terminated string).
	/// @param str String to compare with.
	/// @return Order between objects.
	constexpr OrderType operator<=>(CStringType const& str) const			{return *this <=> SelfType(str);		}

	/// @brief Returns this string as a "C-style" string.
	constexpr explicit operator const char*() const {return cstr();}

	/// @brief Converts the `BaseString` to a `BaseString` of a different character type.
	/// @tparam C Character type.
	/// @note Character type must be different from this `BaseString`'s character type.
	template <Type::ASCII C>
	constexpr operator BaseString<C, IndexType>() const
	requires Type::Different<DataType, C> {
		BaseString<C, IndexType> result(size(), '\0');
		for (SizeType i = 0; i < size(); ++i)
			result[i] = data()[i];
		return result;
	}
	
	/// @brief Returns a substring, starting at a given point.
	/// @param start Start of new string.
	/// @return Resulting substring.
	SelfType substring(IndexType const start) const {
		return sliced(start);
	}

	/// @brief Returns a substring, starting at a given point, and going for a given size.
	/// @param start Start of new string.
	/// @param length Length of new string.
	/// @return Resulting substring.
	SelfType substring(IndexType start, SizeType const length) const {
		assertIsInBounds(start);
		wrapBounds(start);
		while (start < 0) start += size();
		return sliced(start, start + length);
	}
	
	/// @brief Returns the string as a "c-style" string.
	/// @return Pointer to beginning of the "c-style" string.
	constexpr CStringType cstr() const {
		return data();
	}

	constexpr SizeType size() const {
		return BaseType::size() - 1;
	}
	

	/// @brief Returns the string as lowercase.
	/// @return Lowercase string.
	constexpr SelfType lower() const {return transformed(toLowerImpl);	}
	/// @brief Returns the string as uppercase.
	/// @return Uppercase string.
	constexpr SelfType upper() const {return transformed(toUpperImpl);	}

	/// @brief Returns whether the string represents a hex value.
	/// @return Whether the string represents a hex value.
	constexpr bool isHex() const			{return validate(isHexImpl);			}
	/// @brief Returns whether the string is composed fully of null or whitespace characters.
	/// @return Whether the string is null or entirely whitespace.
	constexpr bool isNullOrSpaces() const	{return validate(isNullOrSpaceImpl);	}

	// Most likely wrong
	/// @brief Converts the string to a `char` string.
	/// @return String as `char` string.
	constexpr BaseString<char, SizeType> toString() const
	requires Type::Equal<DataType, wchar> {
		BaseString<char, SizeType> result;
		for (DataType const& c: *this)
			result.pushBack(char(c));
		return result;
	}

	/// @brief Converts the string to a `char` string.
	/// @return String as `char` string.
	constexpr SelfType toString() const
	requires Type::Equal<DataType, char> {return *this;}

	// Most likely wrong
	/// @brief Converts the string to a `wchar` string.
	/// @return String as `wchar` string.
	constexpr BaseString<wchar, SizeType> toWideString() const
	requires Type::Equal<DataType, char> {
		BaseString<wchar, SizeType> result;
		for (DataType const& c: *this)
			result.pushBack(wchar(c));
		return result;
	}

	/// @brief Converts the string to a `wchar` string.
	/// @return String as `wchar` string.
	constexpr SelfType toWideString() const
	requires Type::Equal<DataType, wchar> {return *this;}
	
	/// @brief String-to-boolean conversion.
	///@tparam T Boolean type (`bool`).
	/// @param str String to convert.
	/// @return Converted value as a boolean.
	template<Type::Equal<bool> T>
	constexpr static T toNumber(SelfType const& str) {
		if (str == "true") return true;
		if (str == "false") return false;
		return toNumber<uint8>(str);
	}

	/// @brief String-to-integer conversion.
	///@tparam T Integer type.
	/// @param str String to convert.
	/// @param base Base of the string. Will be used, if non-zero.
	/// @return Converted value as number.
	/// @throw FailedActionException if conversion fails.
	template<Type::Integer T>
	constexpr static T toNumber(SelfType const& str, T const& base = 0) 
	requires Type::Different<T, bool> {
		T val = T();
		if (!atoi<T, DataType>(str.data(), str.size(), val, base))
			throw FailedActionException("String-to-Integer conversion failure!");
		return val;
	}
	
	/// @brief String-to-float conversion.
	///@tparam T Floating point type.
	/// @param str String to convert.
	/// @return Converted value as number.
	/// @throw FailedActionException if conversion fails.
	template<Type::Real T>
	constexpr static T toNumber(SelfType const& str) {
		T val = T();
		if (!atof<T, DataType>(str.data(), str.size(), val))
			throw FailedActionException("String-to-Float conversion failure!");
		return val;
	}

	/// @brief Boolean-to-string conversion.
	///@tparam T Boolean type (`bool`).
	/// @param val Value to convert.
	/// @param text Whether to convert as text ("true" or "false"), or a number (0 or 1).
	/// @return Converted value as string.
	template<Type::Equal<bool> T>
	constexpr static SelfType fromNumber(T const& val, bool const text = false) {
		if (text) return val ? "true" : "false";
		return val ? "1" : "0";
	}

	/// @brief Integer-to-string conversion.
	/// @tparam T Integer type.
	/// @param val Value to convert.
	/// @param base Base to convert as. By default, it is base 10.
	/// @return Converted value as string.
	/// @throw FailedActionException if conversion fails.
	template<Type::Integer T>
	constexpr static SelfType fromNumber(T const& val, T const& base = 10)
	requires Type::Different<T, bool> {
		SelfType result(sizeof(T)*4, '\0');
		ssize sz = itoa<T, DataType>(val, result.data(), result.size(), base);
		if (sz < 0) throw FailedActionException("Integer-to-String conversion failure!");
		result.resize(sz);
		return result;
	}

	/// @brief Floating-point-to-string conversion.
	/// @tparam T Floating point type.
	/// @param val Value to convert.
	/// @param
	///		precision Amount of decimal spaces to include.
	///		By default, it is equal to double the byte size of the floating point type.
	/// @return Converted value as string.
	/// @throw FailedActionException if conversion fails.
	/// @note
	///		Default value of `precision` for:
	///
	///		- `float`s: 8 decimal spaces.
	///
	///		- `double`s: 16 decimal spaces.
	///
	///		- `long double`s: 32 decimal spaces.
	template<Type::Real T>
	constexpr static SelfType fromNumber(T const& val, usize const precision = sizeof(T)*2) {
		SelfType result(sizeof(T)*4, '\0');
		ssize sz = ftoa<T, DataType>(val, result.data(), result.size(), precision);
		if (sz < 0) throw FailedActionException("Float-to-String conversion failure!");
		result.resize(sz);
		return result;
	}

	/// @brief Swap algorithm for `BaseString`.
	/// @param a `BaseString` to swap.
	/// @param b `BaseString` to swap with.
	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap<BaseType>(a, b);
		swap(a.strbuf, b.strbuf);
		swap(a.strbuflen, b.strbuflen);
	}

	/// @brief Returns an STL view for the string.
	/// @return STL view for the string.
	constexpr STDViewType stdView() const		{return STDViewType(cbegin(), cend());		}
	/// @brief Returns the string as an STL string.
	/// @return String as an STL string.
	constexpr STDStringType std() const			{return STDStringType(cbegin(), cend());	}
	/// @brief Returns the string as an STL string.
	/// @return String as an STL string.
	constexpr operator STDStringType() const	{return std();								}

private:
	constexpr static DataType toLowerImpl(DataType const& c)	{return toLowerChar<DataType>(c);		}
	constexpr static DataType toUpperImpl(DataType const& c)	{return toUpperChar<DataType>(c);		}
	constexpr static bool isHexImpl(DataType const& c)			{return isHexChar<DataType>(c);			}
	constexpr static bool isNullOrSpaceImpl(DataType const& c)	{return isNullOrSpaceChar<DataType>(c);	}

	[[noreturn]] void invalidNumberError(CStringType const& v) const {
		throw InvalidValueException("Not a valid number!");
	}

	void assertIsInBounds(IndexType const index) const {
		if (index >= 0 && usize(index) > size()-1) outOfBoundsError(index);
	}

	constexpr void wrapBounds(IndexType& index) const {
		if (index < 0)
			index = (size()-1) - index % (size());
	}

	[[noreturn]] constexpr void outOfBoundsError(IndexType const index) const {
		throw OutOfBoundsException("Index is out of bounds!");
	}
};

/// @brief Stream extraction operator overloading.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @param stream Stream to extract from.
/// @param string `BaseString` to extract to.
/// @return Input stream.
template<Type::ASCII TChar, Type::Integer TIndex = usize>
constexpr typename BaseString<TChar, TIndex>::InputStreamType& operator>>(
	typename BaseString<TChar, TIndex>::InputStreamType& stream,
	BaseString<TChar, TIndex>& string
) {
	return string.readFrom(stream);
}

/// @brief String concatenation operator.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @param str String to append.
/// @param self `BaseString` to append to.
/// @return Resulting concatenated string.
template<Type::ASCII TChar, Type::Integer TIndex = usize>
constexpr BaseString<TChar, TIndex>
operator+(
	typename BaseString<TChar, TIndex>::CStringType const& str,
	BaseString<TChar, TIndex> const& self
) {
	return BaseString<TChar, TIndex>(str) + self;
}

/// @brief String concatenation operator.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @tparam S Array size.
/// @param str Char array to append.
/// @param self `BaseString` to append to.
/// @return Resulting concatenated string.
template<Type::ASCII TChar, Type::Integer TIndex = usize, AsUnsigned<TIndex> S>
constexpr BaseString<TChar, TIndex>
operator+(
	As<typename BaseString<TChar, TIndex>::ConstantType[S]> const& str,
	BaseString<TChar, TIndex> const& self
) {
	return BaseString<TChar, TIndex>(str) + self;
}

/// `BaseString` analog for a `char` string.
typedef BaseString<char>	String;
/// `BaseString` analog for a `wchar` string.
typedef BaseString<wchar_t>	WideString;

/// @brief Static string of characters.
/// @tparam TChar Character type.
/// @tparam N String size.
/// @tparam TIndex Index type.
template<Type::ASCII TChar, usize N, Type::Integer TIndex = usize>
struct BaseStaticString:
	Array<TChar, N, TIndex>,
	CStringable<TChar>,
	SelfIdentified<BaseStaticString<TChar, N, TIndex>>,
	Derived<Array<TChar, N, TIndex>> {
public:
	using Derived			= Derived<Array<TChar, N, TIndex>>;
	using SelfIdentified	= SelfIdentified<BaseStaticString<TChar, N, TIndex>>;
	using CStringable	= CStringable<TChar>;

	using typename Derived::BaseType;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename BaseType::DataType,
		typename BaseType::ConstPointerType,
		typename BaseType::SizeType,
		typename BaseType::IndexType
	;

	using
		BaseType::BaseType,
		BaseType::data,
		BaseType::cbegin,
		BaseType::cend,
		BaseType::begin,
		BaseType::end,
		BaseType::rbegin,
		BaseType::rend
	;

	using BaseType::SIZE, BaseType::MAX_SIZE;

	using typename CStringable::CStringType;

private:
	constexpr static IndexType wrapAround(IndexType value) {
		while (value < 0) value += SIZE;
		return value;
	}

public:
	/// @brief Constructs the `BaseStaticString` from a null-terminated string.
	/// @param str String to copy from.
	constexpr BaseStaticString(CStringType const& str) {
		SizeType len = 0;
		while (str[len++] != '\0' && len <= MAX_SIZE);
		MX::memcpy(str, data(), (len < SIZE ? len : SIZE));
	}

	/// @brief Returns a static substring, starting at a given point, and going for a given size.
	/// @tparam BEGIN Start of new string.
	/// @tparam S Length of new string.
	/// @return Resulting substring.
	template<IndexType BEGIN, SizeType S = SIZE>
	constexpr auto substring() const {
		constexpr SizeType start	= wrapAround(BEGIN);
		constexpr SizeType stop		= ((start + S) < SIZE) ? start + S : SIZE;
		BaseStaticString<TChar, stop - start + 1, TIndex> result('\0');
		MX::memcpy(result.data(), data() + start, stop - start);
		return result;
	}

	/// @brief Returns the static string as a "c-style" string.
	/// @return Static string as a "c-style" string.
	constexpr CStringType cstr() const {
		return data();
	}

	/// @brief Returns the static string as a dynamic string.
	/// @return Static string as dynamic string.
	constexpr BaseString<TChar, TIndex> toString() const {
		return BaseString<TChar, TIndex>(begin(), end());
	}
};

/// @brief List of strings.
typedef List<String>			StringList;
/// @brief String arguments.
typedef Arguments<String>		StringArguments;
/// @brief Pair of strings.
typedef Pair<String, String>	StringPair;

/// @brief `BaseStaticString` analog for a `char` static string.
/// @tparam N Static string size.
template<usize N> using StaticString		= BaseStaticString<char,	N>;
/// @brief `BaseStaticString` analog for a `wchar` static string.
/// @tparam N Static string size.
template<usize N> using StaticWideString	= BaseStaticString<wchar_t,	N>;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
/// @brief String literals.
namespace Literals::Text {
	/// @brief CTL `String` literal.
	constexpr String operator "" s		(cstring cstr, usize sz)	{return String(cstr, sz);					}
	/// @brief CTL `String` literal.
	constexpr String operator "" s		(cwstring cstr, usize sz)	{return WideString(cstr, sz).toString();	}
	/// @brief CTL `WideString` literal.
	constexpr WideString operator "" ws	(cstring cstr, usize sz)	{return String(cstr, sz).toWideString();	}
	/// @brief CTL `WideString` literal.
	constexpr WideString operator "" ws	(cwstring cstr, usize sz)	{return WideString(cstr, sz);				}
}
#pragma GCC diagnostic pop

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_STRING_H
