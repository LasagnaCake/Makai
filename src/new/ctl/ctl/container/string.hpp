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

template<
	Type::ASCII TChar,
	Type::Integer TIndex = usize,
	template <class> class TAlloc = HeapAllocator
>
struct BaseString:
	public List<TChar, TIndex, TAlloc>,
	public SelfIdentified<BaseString<TChar, TIndex>>,
	public Derived<List<TChar, TIndex>>,
	public StringLiterable<TChar>,
	public Streamable<TChar> {
public:
	using Iteratable		= ::CTL::Iteratable<TChar, TIndex>;
	using SelfIdentified	= ::CTL::SelfIdentified<BaseString<TChar, TIndex>>;
	using Derived			= ::CTL::Derived<List<TChar, TIndex>>;
	using Streamable		= ::CTL::Streamable<TChar>;
	using StringLiterable	= ::CTL::StringLiterable<TChar>;

	using typename Derived::BaseType;

	using typename BaseType::OrderType;

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
		typename BaseType::ReverseIteratorType
	;

	using
		typename BaseType::AllocatorType
	;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename StringLiterable::StringLiteralType
	;

	using
		typename Streamable::InputStreamType,
		typename Streamable::OutputStreamType
	;

	using STDViewType	= std::basic_string_view<DataType>;
	using STDStringType	= std::basic_string<DataType>;

	using
		BaseType::BaseType,
		BaseType::allocator,
		BaseType::clear,
		BaseType::reserve,
		BaseType::resize,
		BaseType::tight,
		BaseType::tighten,
		BaseType::data,
		BaseType::cbegin,
		BaseType::cend,
		BaseType::begin,
		BaseType::end,
		BaseType::rbegin,
		BaseType::rend,
		BaseType::front,
		BaseType::back,
		BaseType::transformed,
		BaseType::validate,
		BaseType::size,
		BaseType::empty,
		BaseType::find,
		BaseType::rfind,
		BaseType::bsearch,
		BaseType::capacity,
		BaseType::appendBack,
		BaseType::pushBack,
		BaseType::popBack
	;

	constexpr BaseString(): BaseType() {}

	constexpr ~BaseString() {if (strbuf) allocator().deallocate(strbuf);}

	constexpr BaseString(StringLiteralType const& v) {
		SizeType len = 0;
		while (v[len++] != '\0' && len <= MAX_SIZE);
		reserve(len);
		appendBack(BaseType(v, v+len-1));
	}

	template<SizeType S>
	constexpr BaseString(Decay::AsType<ConstantType[S]> const& v) {
		reserve(S);
		MX::memcpy<DataType>(cbegin(), v, S);
	}

	constexpr BaseString(BaseType const& other):	BaseType(other)				{}
	constexpr BaseString(BaseType&& other):			BaseType(CTL::move(other))	{}

	constexpr BaseString(SelfType const& other):	BaseType(other)				{}
	constexpr BaseString(SelfType&& other):			BaseType(CTL::move(other))	{}

	template<class T>
	constexpr BaseString(T const& other)
	requires requires (T t) {
		{t.begin()} -> Type::Convertible<IteratorType>;
		{t.end()} -> Type::Convertible<IteratorType>;
	}:
		BaseType(other.begin(), other.end()) {
	}

	template<class T>
	constexpr explicit BaseString(T const& other)
	requires requires (T t) {
		{t.data()} -> Type::Convertible<PointerType>;
		{t.size()} -> Type::Convertible<SizeType>;
	}:
		BaseType(other.data(), other.data() + other.size()) {
	}

	template<class... Args>
	constexpr BaseString(Args const&... args)
	requires (... && Type::Equal<Args, SelfType>) {
		(*this) += (... + args);
	}

	constexpr BaseString(STDViewType const& str):	BaseType(str.data(), str.data() + str.size())	{}
	constexpr BaseString(STDStringType const& str):	BaseType(str.data(), str.data() + str.size())	{}

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

	constexpr List<SelfType, SizeType> divide(IndexType index) const {
		List<SelfType, SizeType> res;
		res.pushBack(sliced(0, index));
		res.pushBack(sliced(index+1));
		return res;
	}

	constexpr SelfType sliced(IndexType const& start) const							{return BaseType::sliced(start);		}
	constexpr SelfType sliced(IndexType const& start, IndexType const& stop) const	{return BaseType::sliced(start, stop);	}

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
	
	constexpr SelfType& replace(DataType const& val, DataType const& rep) {
		for (DataType& v: *this)
			if (v == val) v = rep;
		return *this;
	}

	constexpr SelfType& replace(BaseType const& values, DataType const& rep) {
		for (DataType const& val: values)
			replace(val, rep);
		return *this;
	}

	struct Replacement {
		BaseType	targets;
		DataType	replacement;
	};

	constexpr SelfType& replace(Replacement const& rep) {
		replace(rep.targets, rep.replacement);
		return *this;
	}

	constexpr SelfType& replace(List<Replacement, SizeType> const& reps) {
		for (Replacement const& rep: reps)
			replace(rep);
		return *this;
	}

	constexpr SelfType replaced(DataType const& val, DataType const& rep) const				{return SelfType(*this).replace(val, rep);		}
	constexpr SelfType replaced(BaseType const& values, DataType const& rep) const			{return SelfType(*this).replace(values, rep);	}
//	constexpr SelfType replaced(ArgumentListType const& values, DataType const& rep) const	{return SelfType(*this).replace(values, rep);	}

	constexpr SelfType replaced(Replacement const& rep) const					{return SelfType(*this).replace(rep);	}
	constexpr SelfType replaced(List<Replacement, SizeType> const& reps) const	{return SelfType(*this).replace(reps);	}
//	constexpr SelfType replaced(Arguments<Replacement> const& reps) const		{return SelfType(*this).replace(reps);	}

	constexpr OutputStreamType& operator<<(OutputStreamType& o) const	{if (!empty()) o << cstr(); return o;}
	constexpr OutputStreamType& operator<<(OutputStreamType& o)			{if (!empty()) o << cstr(); return o;}

	friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType& self)			{if (!self.empty()) o << self.cstr(); return o;}
	friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType const& self)	{if (!self.empty()) o << self.cstr(); return o;}

	constexpr InputStreamType& readFrom(InputStreamType& i, DataType const& stop) {
		DataType buf[32];
		while(i.getline(buf, 32, stop))
			appendBack(buf, i.gcount());
		return i;
	}
	
	constexpr InputStreamType& readFrom(InputStreamType& i) {
		return readFromStream(i, '\0');
	}

	constexpr SelfType& operator=(SelfType const& other)			{BaseType::operator=(other); return *this;				}
	constexpr SelfType& operator=(SelfType&& other)					{BaseType::operator=(CTL::move(other)); return *this;	}
	constexpr SelfType& operator=(StringLiteralType const& other)	{BaseType::operator=(SelfType(other)); return *this;	}

	constexpr SelfType const& operator<<(SelfType& other) const	{other.appendBack(*this); return *this;}
	constexpr SelfType& operator<<(SelfType& other)				{other.appendBack(*this); return *this;}

	constexpr SelfType& operator>>(SelfType const& other)	{appendBack(other); return other;}

	constexpr SelfType operator+(DataType const& value) const	{return SelfType(*this).pushBack(value);	}
	constexpr SelfType operator+(SelfType const& other) const	{return SelfType(*this).appendBack(other);	}

	constexpr SelfType operator+(StringLiteralType const& str) const				{return (*this) + SelfType(str);}
	template<SizeType S>
	constexpr SelfType operator+(Decay::AsType<ConstantType[S]> const& str) const	{return (*this) + SelfType(str);}

	friend constexpr SelfType operator+(DataType const& value, SelfType const& self)	{return SelfType().pushBack(value) + self;	}

	friend constexpr SelfType operator+(StringLiteralType const& str, SelfType const& self)					{return SelfType(str) + (self);}
	template<SizeType S>
	friend constexpr SelfType operator+(Decay::AsType<ConstantType[S]> const& str, SelfType const& self)	{return SelfType(str) + (self);}

	constexpr SelfType& operator+=(DataType const& value)				{pushBack(value); return *this;				}
	constexpr SelfType& operator+=(SelfType const& other)				{appendBack(other); return *this;			}
	constexpr SelfType& operator+=(StringLiteralType const& str)		{appendBack(SelfType(str)); return *this;	}
	template<SizeType S>
	constexpr SelfType& operator+=(Decay::AsType<ConstantType[S]> str)	{appendBack(str); return *this;		}

	constexpr SelfType operator*(IndexType const& times) const {
		if (times < 1) return SelfType();
		if (times == 1) return *this;
		SelfType result(size() * usize(times));
		for (SizeType i = 0; i < usize(times); ++i)
			result.appendBack(*this);
		return result;
	}

	constexpr SelfType& operator*=(IndexType const& times) {
		if (times < 1) return SelfType();
		if (times == 1) return *this;
		SelfType copy = *this;
		for (SizeType i = 0; i < times; ++i)
			appendBack(copy);
		return *this;
	}

	template<SizeType S>
	constexpr bool operator==(Decay::AsType<ConstantType[S]> const& str) const	{return *this == SelfType(str);			}
	constexpr bool operator==(StringLiteralType const& str) const				{return *this == SelfType(str);			}
	constexpr bool operator==(SelfType const& other) const						{return BaseType::operator==(other);	}
	template<SizeType S>
	constexpr OrderType operator<=>(Decay::AsType<ConstantType[S]> const& str) const	{return *this <=> SelfType(str);		}
	constexpr OrderType operator<=>(StringLiteralType const& str) const					{return *this <=> SelfType(str);		}
	constexpr OrderType operator<=>(SelfType const& other) const						{return BaseType::operator<=>(other);	}

	constexpr explicit operator const char*() const {return cstr();}

	template <Type::ASCII C>
	constexpr operator BaseString<C, IndexType>() const
	requires Type::Different<DataType, C> {
		BaseString<C, IndexType> result(size(), '\0');
		for (SizeType i = 0; i < size(); ++i)
			result[i] = data()[i];
		return result;
	}

	SelfType substring(IndexType const& start) const {
		return sliced(start);
	}

	SelfType substring(IndexType start, SizeType const& length) const {
		assertIsInBounds(start);
		while (start < 0) start += size();
		return sliced(start, start + length);
	}

	constexpr bool nullTerminated() const {return back() == '\0';}
	
	constexpr StringLiteralType cstr() const {
		if (nullTerminated()) return cbegin();
		strbuflen = size() + 1;
		strbufalloc.resize(strbuf, strbuflen);
		MX::memcpy<DataType>(strbuf, cbegin(), size());
		strbuf[size()] = '\0';
		return strbuf;
	}

	constexpr SelfType lower() const {return transformed(toLowerImpl);	}
	constexpr SelfType upper() const {return transformed(toUpperImpl);	}

	constexpr bool isHex() const			{return validate(isHexImpl);			}
	constexpr bool isNullOrSpaces() const	{return validate(isNullOrSpaceImpl);	}

	// Most likely wrong
	constexpr BaseString<char, SizeType> toString() const
	requires Type::Equal<DataType, wchar> {
		BaseString<char, SizeType> result;
		for (DataType const& c: *this)
			result.pushBack(char(c));
		return result;
	}

	constexpr SelfType toString() const
	requires Type::Equal<DataType, char> {return *this;}

	// Most likely wrong
	constexpr BaseString<wchar, SizeType> toWideString() const
	requires Type::Equal<DataType, char> {
		BaseString<wchar, SizeType> result;
		for (DataType const& c: *this)
			result.pushBack(wchar(c));
		return result;
	}

	constexpr SelfType toWideString() const
	requires Type::Equal<DataType, wchar> {return *this;}

	constexpr static bool toNumber(SelfType const& str) {
		return toNumber<uint8>(str);
	}

	template<Type::Integer T>
	constexpr static T toNumber(SelfType const& str, T const& base = 0) 
	requires Type::Different<T, bool> {
		T val = T();
		if (!atoi<T, DataType>(str.data(), str.size(), val, base))
			throw FailedActionException("String-to-Integer conversion failure!");
		return val;
	}

	template<Type::Real T>
	constexpr static T toNumber(SelfType const& str) {
		T val = T();
		if (!atof<T, DataType>(str.data(), str.size(), val))
			throw FailedActionException("String-to-Float conversion failure!");
		return val;
	}

	constexpr static SelfType fromNumber(bool const& val) {
		return fromNumber<uint8>(val);
	}

	template<Type::Integer T>
	constexpr static SelfType fromNumber(T const& val, T const& base = 10)
	requires Type::Different<T, bool> {
		SelfType result(sizeof(T)*4, '\0');
		ssize sz = itoa<T, DataType>(val, result.data(), result.size(), base);
		if (sz < 0) throw FailedActionException("Integer-to-String conversion failure!");
		result.resize(sz);
		return result;
	}

	template<Type::Real T>
	constexpr static SelfType fromNumber(T const& val, usize const& precision = sizeof(T)*2) {
		SelfType result(sizeof(T)*4, '\0');
		ssize sz = ftoa<T, DataType>(val, result.data(), result.size(), precision);
		if (sz < 0) throw FailedActionException("Float-to-String conversion failure!");
		result.resize(sz);
		return result;
	}

	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap<BaseType>(a, b);
		swap(a.strbuf, b.strbuf);
		swap(a.strbuflen, b.strbuflen);
	}

	constexpr STDViewType stdView() const		{return STDViewType(cbegin(), cend());		}
	constexpr STDStringType std() const			{return STDStringType(cbegin(), cend());	}
	constexpr operator STDStringType() const	{return std();								}

private:
	constexpr static DataType toLowerImpl(DataType const& c)	{return toLowerChar<DataType>(c);		}
	constexpr static DataType toUpperImpl(DataType const& c)	{return toUpperChar<DataType>(c);		}
	constexpr static bool isHexImpl(DataType const& c)			{return isHexChar<DataType>(c);			}
	constexpr static bool isNullOrSpaceImpl(DataType const& c)	{return isNullOrSpaceChar<DataType>(c);	}

	PointerType mutable		strbuf			= nullptr;
	usize mutable			strbuflen		= 0;
	AllocatorType mutable	strbufalloc;

	[[noreturn]] void invalidNumberError(StringLiteralType const& v) const {
		throw InvalidValueException("Not a valid number!");
	}

	void assertIsInBounds(IndexType const& index) const {
		if (index >= 0 && usize(index) > size()-1) outOfBoundsError(index);
	}

	[[noreturn]] constexpr void outOfBoundsError(IndexType const& index) const {
		throw OutOfBoundsException("Index is out of bounds!");
	}
};

template<Type::ASCII TChar, Type::Integer TIndex = usize>
constexpr typename BaseString<TChar, TIndex>::InputStreamType& operator>>(
	typename BaseString<TChar, TIndex>::InputStreamType& stream,
	BaseString<TChar, TIndex>& string
) {
	return string.readFrom(stream);
}



template<Type::ASCII TChar, Type::Integer TIndex = usize>
constexpr BaseString<TChar, TIndex>
operator+(
	typename BaseString<TChar, TIndex>::StringLiteralType const& str,
	BaseString<TChar, TIndex> const& self
) {
	return BaseString<TChar, TIndex>(str) + self;
}

template<Type::ASCII TChar, Type::Integer TIndex = usize, AsUnsigned<TIndex> S>
constexpr BaseString<TChar, TIndex>
operator+(
	Decay::AsType<typename BaseString<TChar, TIndex>::ConstantType[S]> const& str,
	BaseString<TChar, TIndex> const& self
) {
	return BaseString<TChar, TIndex>(str) + self;
}

typedef BaseString<char>	String;
typedef BaseString<wchar_t>	WideString;

template<Type::ASCII TChar, usize N, Type::Integer TIndex = usize>
struct BaseStaticString:
	Array<TChar, N, TIndex>,
	StringLiterable<TChar>,
	SelfIdentified<BaseStaticString<TChar, N, TIndex>>,
	Derived<Array<TChar, N, TIndex>> {
public:
	using Derived			= Derived<Array<TChar, N, TIndex>>;
	using SelfIdentified	= SelfIdentified<BaseStaticString<TChar, N, TIndex>>;
	using StringLiterable	= StringLiterable<TChar>;

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

	using typename StringLiterable::StringLiteralType;

private:
	constexpr static IndexType wrapAround(IndexType value) {
		while (value < 0) value += SIZE;
		return value;
	}

public:
	constexpr BaseStaticString(StringLiteralType const& str) {
		SizeType len = 0;
		while (str[len++] != '\0' && len <= MAX_SIZE);
		MX::memcpy(str, data(), (len < SIZE ? len : SIZE));
	}

	template<IndexType BEGIN, SizeType S = SIZE>
	constexpr auto substring() const {
		constexpr SizeType start	= wrapAround(BEGIN);
		constexpr SizeType stop		= ((start + S) < SIZE) ? start + S : SIZE;
		BaseStaticString<TChar, stop - start + 1, TIndex> result('\0');
		MX::memcpy(result.data(), data() + start, stop - start);
		return result;
	}

	constexpr StringLiteralType cstr() const {
		return data();
	}

	constexpr BaseString<TChar, TIndex> toString() const {
		return BaseString<TChar, TIndex>(begin(), end());
	}
};

typedef List<String>			StringList;
typedef Arguments<String>		StringArguments;
typedef Pair<String, String>	StringPair;

template<usize N> using StaticString		= BaseStaticString<char,	N>;
template<usize N> using StaticWideString	= BaseStaticString<wchar_t,	N>;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
namespace Literals::Text {
	constexpr String operator "" s		(cstring cstr, usize sz)	{return String(cstr, cstr + sz);				}
	constexpr String operator "" s		(cwstring cstr, usize sz)	{return WideString(cstr, cstr + sz).toString();	}
	constexpr WideString operator "" ws	(cstring cstr, usize sz)	{return String(cstr, cstr + sz).toWideString();	}
	constexpr WideString operator "" ws	(cwstring cstr, usize sz)	{return WideString(cstr, cstr + sz);			}
}
#pragma GCC diagnostic pop

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_STRING_H