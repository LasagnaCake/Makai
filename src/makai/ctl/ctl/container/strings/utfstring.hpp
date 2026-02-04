#ifndef CTL_CONTAINER_STRINGS_UTFSTRING_H
#define CTL_CONTAINER_STRINGS_UTFSTRING_H

#include "../span.hpp"
#include "string.hpp"

CTL_NAMESPACE_BEGIN

namespace UTF {
	/// @brief Unicode scalar value.
	/// @tparam S UTF encoding of choice. Can only be `8` or `32`.
	template<usize S>
	struct Character: Ordered {
		/// @brief Code point.
		struct Point {
			/// @brief Code point size.
			uint32 size:	4;
			/// @brief Code point value.
			uint32 point:	28;
		};

		/// @brief Character type.
		constexpr static usize const TYPE	= S;
		/// @brief Character encoding byte size.
		constexpr static usize const SIZE	= TYPE >> 3;

		/// @brief Code point mask (utf-8).
		constexpr static uint32 const CODE_POINT_MASK_U8	= 0x0FFFFFFF;
		/// @brief Character size mask (utf-8).
		constexpr static uint32 const CODE_SIZE_MASK_U8		= 0xF0000000;

		static_assert(TYPE == 8 || TYPE == 16 || TYPE == 32, "Unicode character size must be 8 or 32 bits wide!");

		static_assert(TYPE != 16, "UTF-16 is currently unimplemented!");

		/// @brief Empty constructor.
		constexpr Character(): id(0)													{}
		/// @brief Constructs the unicode character from an ASCII character.
		constexpr Character(char const chr): id(chr | static_cast<uint32>(1 << 28))		{}
		/// @brief Constructs the unicode character from a character byte string.
		template<usize C>
		constexpr Character(As<char const[C]> const& chr): Character(chr, chr + C)		{}
		/// @brief Constructs the unicode character from an unicode character literal.
		template<usize C>
		constexpr Character(As<u8char const[C]> const& chr): Character(chr, chr + C)	{}
		/// @brief Constructs the unicode character from a set of bytes.
		template<usize C>
		constexpr Character(As<uint8 const[C]> const& chr): Character(chr, chr + C)		{}
		/// @brief Constructs the unicode character from a given character ID.
		constexpr explicit Character(uint32 const id): id(id)							{}
		/// @brief Constructs the unicode character from a character in a different encoding.
		template<usize C>
		constexpr explicit Character(Character<C> const& other)
		requires (C != TYPE): Character(other.value())							{updateCodeSize();}
		/// @brief Copy constructor.
		constexpr Character(Character const& other): Character(other.id)		{}

		/// @brief Returns the unicode scalar value for this character.
		constexpr operator uint32() const {return value();}

		/// @brief Returns the underlying ID for this character.
		constexpr uint32 raw() const {return id;}

		/// @brief Returns the unicode scalar value for this character.
		constexpr uint32 value() const requires (TYPE == 8)		{return id & CODE_POINT_MASK_U8;				}
		/// @brief Returns the unicode scalar value for this character.
		constexpr uint32 value() const requires (TYPE == 32)	{return id;										}

		/// @brief Returns the unicode character size.
		constexpr usize size() const requires (TYPE == 8)		{return ((id & CODE_SIZE_MASK_U8) >> 28) + 1;	}
		/// @brief Returns the unicode character size.
		constexpr usize size() const requires (TYPE == 32)		{return 4;										}

		/// @brief Constructs the unicode character from a given range.
		template<class T>
		constexpr explicit Character(ref<T const> begin, ref<T const> const end)
		requires (TYPE == 8): Character() {
			As<uint8[4]> buf = {0, 0, 0, 0};
			if (end <= begin) return;
			usize sz = 0;
			char const lead = *begin++;
			buf[sz++] = lead;
			if (!(lead & 0b10000000)) {
				id = lead;
				return;
			}
			while ((lead << sz) & 0b10000000 && sz < 4) ++sz;
			for (usize i = 1; (i < sz && begin < end); ++i) {
				if ((*begin & 0b11000000) != 0b10000000)
					break;
				buf[i] = *begin++;
			}
			id = toScalar(buf, sz) | ((sz-1) << 28);
		}

		/// @brief Constructs the unicode character from a given range.
		template<class T>
		constexpr explicit Character(ref<T const> begin, ref<T const> const end)
		requires (TYPE == 32): Character() {
			As<uint8[4]> buf = {0, 0, 0, 0};
			if (end <= begin) return;
			for (usize i = 1; (i < 4 && begin < end); ++i)
				buf[i] = *begin++;
			id = toScalar(buf, 4);
		}

		/// @brief Converts the unicode character to its UTF code point equivalent.
		template<class T>
		constexpr usize toBytes(As<T[4]>& out) const
		requires (TYPE == 8) {
			auto const cid = value();
			auto sz = size();
			if (sz < 1 || sz > 4)
				sz = 1
				+	bool(id & (1 << 6))
				+	bool(id & (1 << 12))
				+	bool(id & (1 << 18))
				;
			for (usize i = 0; i < 4; ++i)
				out[i] = 0;
			// This could be done in WAY better ways, but brain is bricked
			switch (sz) {
			default:
			case 1:
				out[0] = recast<T>(cid & 0b0111'1111);
			break;
			case 2:
				out[0] = recast<T>(0b1100'0000 | ((cid >> 6) & 0b0001'1111));
				out[1] = recast<T>(0b1000'0000 | (cid & 0b0011'1111));
			break;
			case 3:
				out[0] = recast<T>(0b1110'0000 | ((cid >> 12) & 0b0000'1111));
				out[1] = recast<T>(0b1000'0000 | ((cid >> 6) & 0b0011'1111));
				out[2] = recast<T>(0b1000'0000 | (cid & 0b0011'1111));
			break;
			case 4:
				out[1] = recast<T>(0b1110'0000 | ((cid >> 18) & 0b0000'0111));
				out[1] = recast<T>(0b1110'0000 | ((cid >> 12) & 0b0011'1111));
				out[2] = recast<T>(0b1000'0000 | ((cid >> 6) & 0b0011'1111));
				out[3] = recast<T>(0b1000'0000 | (cid & 0b0011'1111));
			break;
			}
			return sz;
		}

		/// @brief Converts the unicode character to its UTF code point equivalent.
		template<class T>
		constexpr usize toBytes(As<T[4]>& out) const
		requires (TYPE == 32) {
			for (usize i = 0; i < 4; ++i)
				out[i] = 0;
			for (usize i = 0; i < 4; ++i)
				out[i] = bitcast<char>(static_cast<uint8>((id >> (i * 8)) & 0xFF));
			return 4;
		}

		/// @brief Converts an unicode code point to a scalar value.
		constexpr static uint32 toScalar(As<uint8 const[4]> const& bytes, usize const sz)
		requires (TYPE == 8) {
			uint32 chr{0};
			switch (sz) {
			case 4:
				chr |= (recast<uint32>(bytes[0]) & 0b00000111) << 18;
				chr |= (recast<uint32>(bytes[1]) & 0b00111111) << 12;
				chr |= (recast<uint32>(bytes[2]) & 0b00111111) << 6;
				chr |= (recast<uint32>(bytes[3]) & 0b00111111);
			break;
			case 3:
				chr |= (recast<uint32>(bytes[0]) & 0b00001111) << 12;
				chr |= (recast<uint32>(bytes[1]) & 0b00111111) << 6;
				chr |= (recast<uint32>(bytes[2]) & 0b00111111);
			break;
			case 2:
				chr |= (recast<uint32>(bytes[0]) & 0b00011111) << 6;
				chr |= (recast<uint32>(bytes[1]) & 0b00111111);
			break;
			case 1:
				chr |= recast<uint32>(bytes[1]);
			break;
			}
			return chr & CODE_POINT_MASK_U8;
		}

		/// @brief Converts an unicode code point to a scalar value.
		constexpr static uint32 toScalar(As<uint8 const[4]> const& bytes, usize const sz)
		requires (TYPE == 32) {
			return bitcast<uint32>(bytes);
		}

		/// @brief Equality comparison operator.
		constexpr bool operator==(Character const& other) const 		{return value() == other.value();	}
		/// @brief Threeway comparison operator.
		constexpr OrderType operator<=>(Character const& other) const	{return value() <=> other.value();	}

	private:
		uint32 id = 0;

		template<class TDst, class TSrc>
		constexpr static TDst recast(TSrc const& src) {
			using PassType = Meta::DualType<Type::Unsigned<TSrc>, AsUnsigned<TDst>, AsSigned<TDst>>;
			return bitcast<TDst>(static_cast<PassType>(src));
		}

		constexpr void updateCodeSize() requires (TYPE == 32) {}
	};

	static_assert(sizeof(Character<8>) == sizeof(uint32));
	static_assert(sizeof(Character<32>) == sizeof(uint32));

	/// @brief Default "unknown character" replacement character.
	/// @tparam S UTF encoding. MUST be `8` or `32`.
	template<usize S>
	constexpr Character<S> const REP_CHAR = Character<S>(Character<8>("�"));

	static_assert(REP_CHAR<8>.value() == 0xFFFD);
	static_assert(REP_CHAR<32>.value() == 0xFFFD);
	static_assert(REP_CHAR<8>.size() == 3);
	static_assert(REP_CHAR<32>.size() == 4);

	namespace {
		consteval bool doEncodeTest() {
			auto const repc = "�";
			As<AsNormal<decltype(repc[0])>[4]> buf;
			REP_CHAR<8>.toBytes(buf);
			if (buf[0] != repc[0]) return false;
			if (buf[1] != repc[1]) return false;
			if (buf[2] != repc[2]) return false;
			return true;
		}
	}

	static_assert(doEncodeTest());

	/// @brief Dynamic unicode strings.
	/// @tparam UTF encoding. MUST be `8` or `32`.
	/// @tparam TIndex Index type.
	/// @tparam TAlloc<class> Runtime allocator type. By default, it is `HeapAllocator`.
	/// @tparam TConstAlloc<class> Compile-time allocator type. By default, it is `ConstantAllocator`.
	template<
		usize UTF,
		Type::Integer TIndex = usize,
		template <class> class TAlloc		= HeapAllocator,
		template <class> class TConstAlloc	= ConstantAllocator
	>
	struct UTFString:
		private List<Character<UTF>, TIndex, TAlloc>,
		public SelfIdentified<UTFString<UTF, TIndex, TAlloc, TConstAlloc>>,
		public Derived<List<Character<UTF>, TIndex, TAlloc, TConstAlloc>>,
		public Streamable<char> {
		using Iteratable		= ::CTL::Iteratable<Character<UTF>, TIndex>;
		using SelfIdentified	= ::CTL::SelfIdentified<UTFString<UTF, TIndex, TAlloc, TConstAlloc>>;
		using Derived			= ::CTL::Derived<List<Character<UTF>, TIndex, TAlloc, TConstAlloc>>;
		using Streamable		= ::CTL::Streamable<char>;

		using typename Derived::BaseType;

		using typename BaseType::OrderType;

		using
			typename BaseType::PredicateType,
			typename BaseType::CompareType,
			typename BaseType::TransformType
		;

		using
			typename BaseType::DataType,
			typename BaseType::ConstantType,
			typename BaseType::PointerType,
			typename BaseType::ConstPointerType,
			typename BaseType::ReferenceType,
			typename BaseType::ConstReferenceType
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
			typename SelfIdentified::SelfType
		;

		using
			typename Streamable::InputStreamType,
			typename Streamable::OutputStreamType
		;

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

		/// @brief Default constructor.
		constexpr UTFString(): BaseType() {
			BaseType::pushBack('\0');
		}

		/// @brief Constructs the `UTFString` with a preallocated capacity.
		/// @param size Size to preallocate.
		constexpr explicit UTFString(SizeType const size): BaseType(size+1) {}

		/// @brief Constructs a `UTFString` of a given size and a given fill.
		/// @param size Size to allocate.
		/// @param fill Value to set for each character.
		constexpr explicit UTFString(SizeType const size, DataType const& fill): BaseType(size+1, fill) {
			BaseType::back() = '\0';
		}

		/// @brief Copy constructor.
		/// @param other `UTFString` to copy from.
		constexpr UTFString(SelfType const& other) {
			BaseType::resize(other.size()+1);
			BaseType::appendBack(other.begin(), other.end());
			BaseType::pushBack('\0');
			BaseType::tighten();
		}

		/// @brief Move constructor.
		/// @param other `UTFString` to move from.
		constexpr UTFString(SelfType&& other) {
			BaseType::resize(other.size()+1);
			BaseType::appendBack(other.begin(), other.end());
			BaseType::pushBack('\0');
			BaseType::tighten();
		}

		/// @brief Constructs the `UTFString` from a fixed array of characters.
		/// @tparam S Size of array.
		/// @param values Characters to add to `UTFString`.
		template<SizeType S>
		constexpr UTFString(As<char const[S]> const& values): UTFString(values, values + S) {}

		/// @brief Constructs the `UTFString` from a fixed array of characters.
		/// @tparam S Size of array.
		/// @param values Characters to add to `UTFString`.
		template<SizeType S>
		constexpr UTFString(As<u8char const[S]> const& values): UTFString(values, values + S) {}

		/// @brief Constructs an `UTFString` from a "C-style" range of characters.
		/// @param start Start of range.
		/// @param size Size of range.
		constexpr explicit UTFString(cstring const start, SizeType const size): UTFString(start, start + size) {}

		/// @brief Constructs an `UTFString` from a "C-style" range of characters.
		/// @param start Start of range.
		/// @param size Size of range.
		constexpr explicit UTFString(u8cstring const start, SizeType const size): UTFString(start, start + size) {}

		/// @brief Constructs an `UTFString` from a range of characters.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		constexpr UTFString(typename String::ConstIteratorType begin, typename String::ConstIteratorType const& end): BaseType() {
			if (end <= begin) return;
			BaseType::resize(end - begin + (*(end-1) == '\0' ? 1 : 2));
			while (begin < end) {
				BaseType::pushBack(DataType(begin.raw(), end.raw()));
				begin = begin + BaseType::back().size();
			}
			if (*(end-1) != '\0') BaseType::pushBack('\0');
			BaseType::tighten();
		}

		/// @brief Constructs an `UTFString` from a range of characters.
		/// @param begin Reverse iterator to beginning of range.
		/// @param end Reverse iterator to end of range.
		constexpr UTFString(typename String::ConstReverseIteratorType begin, typename String::ConstReverseIteratorType const& end): BaseType() {
			if (end <= begin) return;
			BaseType::resize(end - begin + (*(end-1) == '\0' ? 1 : 2));
			while (begin < end) {
				BaseType::pushBack(DataType(begin.raw(), end.raw()));
				begin = begin + BaseType::back().size();
			}
			if (*(end-1) != '\0') BaseType::pushBack('\0');
			BaseType::tighten();
		}

		/// @brief Constructs an `UTFString` from a range of characters.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		constexpr UTFString(ConstIteratorType const& begin, ConstIteratorType const& end): BaseType() {
			if (end <= begin) return;
			BaseType::resize(end - begin + (*(end-1) == DataType('\0') ? 1 : 2));
			BaseType::appendBack(begin, end);
			if (BaseType::back() != DataType('\0'))
				BaseType::pushBack('\0');
			BaseType::tighten();
		}

		/// @brief Constructs an `UTFString` from a range of characters.
		/// @param begin Reverse iterator to beginning of range.
		/// @param end Reverse iterator to end of range.
		constexpr UTFString(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end): BaseType() {
			if (end <= begin) return;
			BaseType::resize(end - begin + (*(end-1) == DataType('\0') ? 1 : 2));
			BaseType::appendBack(begin, end);
			if (BaseType::back() != DataType('\0'))
				BaseType::pushBack('\0');
			BaseType::tighten();
		}

		using U8IteratorType				= Iterator<u8char, false, SizeType>;
		using U8ReverseIteratorType			= Iterator<u8char, true, SizeType>;
		using U8ConstIteratorType			= Iterator<u8char const, false, SizeType>;
		using U8ConstReverseIteratorType	= Iterator<u8char const, true, SizeType>;

		/// @brief Constructs an `UTFString` from a range of characters.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		constexpr UTFString(U8ConstIteratorType begin, U8ConstIteratorType const& end): BaseType() {
			if (end <= begin) return;
			BaseType::resize(end - begin + (*(end-1) == '\0' ? 1 : 2));
			while (begin < end) {
				BaseType::pushBack(Character<8>(begin.raw(), end.raw()));
				begin = begin + BaseType::back().size();
			}
			if (*(end-1) != '\0') BaseType::pushBack('\0');
			BaseType::tighten();
		}

		/// @brief Constructs an `UTFString` from a range of characters.
		/// @param begin Reverse iterator to beginning of range.
		/// @param end Reverse iterator to end of range.
		constexpr UTFString(U8ConstReverseIteratorType begin, U8ConstReverseIteratorType const& end): BaseType() {
			if (end <= begin) return;
			BaseType::resize(end - begin + (*(end-1) == '\0' ? 1 : 2));
			while (begin < end) {
				BaseType::pushBack(Character<8>(begin.raw(), end.raw()));
				begin = begin + BaseType::back().size();
			}
			if (*(end-1) != '\0') BaseType::pushBack('\0');
			BaseType::tighten();
		}

		/// @brief Constructs an `UTFString` from a "C-style" range of characters.
		/// @param start Start of range.
		/// @param size Size of range.
		constexpr explicit UTFString(ConstPointerType const& start, SizeType const size): UTFString(start, start + size) {}

		/// @brief Constructs an `UTFString`, from a ranged object of (non-subclass) type T.
		/// @tparam T Ranged type.
		/// @param other Object to copy from.
		template<Type::Container::Ranged<IteratorType, ConstIteratorType> T>
		constexpr UTFString(T const& other)
		requires (!Type::Subclass<T, SelfType>):
			UTFString(other.begin(), other.end()) {}

		/// @brief Constructs an `UTFString`, from a bounded object of (non-list) type T.
		/// @tparam T Ranged type.
		/// @param other Object to copy from.
		template<Type::Container::Bounded<PointerType, SizeType> T>
		constexpr explicit UTFString(T const& other)
		requires (
			!Type::Container::List<T>
		&&	!Type::Container::Ranged<T, IteratorType, ConstIteratorType>
		): UTFString(other.data(), other.size()) {}


		/// @brief Constructs an `UTFString`, from a ranged object of (non-subclass) type T.
		/// @tparam T Ranged type.
		/// @param other Object to copy from.
		template<Type::Container::Ranged<typename String::IteratorType, typename String::ConstIteratorType> T>
		constexpr UTFString(T const& other): UTFString(other.begin(), other.end()) {}

		/// @brief Constructs an `UTFString`, from a bounded object of (non-list) type T.
		/// @tparam T Ranged type.
		/// @param other Object to copy from.
		template<Type::Container::Bounded<ref<char>, SizeType> T>
		constexpr explicit UTFString(T const& other)
		requires (!Type::Container::Ranged<T, typename String::IteratorType, typename String::ConstIteratorType>):
			UTFString(other.data(), other.size()) {}

		/// @brief Constructs an `UTFString`, from a ranged object of (non-subclass) type T.
		/// @tparam T Ranged type.
		/// @param other Object to copy from.
		template<Type::Container::Ranged<U8IteratorType, U8ConstIteratorType> T>
		constexpr UTFString(T const& other): UTFString(other.begin(), other.end()) {}

		/// @brief Constructs an `UTFString`, from a bounded object of (non-list) type T.
		/// @tparam T Ranged type.
		/// @param other Object to copy from.
		template<Type::Container::Bounded<ref<u8char>, SizeType> T>
		constexpr explicit UTFString(T const& other)
		requires (!Type::Container::Ranged<T, U8IteratorType, U8ConstIteratorType>):
			UTFString(other.data(), other.size()) {}

		/// @brief Constructs an `UTFString` from a null-terminated string.
		/// @param v String to copy from.
		constexpr UTFString(cstring const v) {
			SizeType len = 0;
			while (v[len++] != '\0' && len <= MAX_SIZE);
			BaseType::reserve(len);
			BaseType::appendBack(SelfType(v, v+len));
			BaseType::tighten();
		}

		/// @brief Constructs an `UTFString` from a null-terminated unicode string.
		/// @param v String to copy from.
		constexpr UTFString(u8cstring const v) {
			SizeType len = 0;
			while (v[len++] != bitcast<u8char>('\0') && len <= MAX_SIZE);
			BaseType::reserve(len);
			BaseType::appendBack(SelfType(v, v+len));
			BaseType::tighten();
		}

		/// @brief Constructos an `UTFString` from a STL view analog.
		/// @param str View to copy from.
		constexpr UTFString(typename String::STDViewType const& str):	UTFString(&*str.begin(), &*str.end())	{}
		/// @brief Constructos an `UTFString` from a STL string analog.
		/// @param str View to copy from.
		constexpr UTFString(typename String::STDStringType const& str):	UTFString(&*str.begin(), &*str.end())	{}

		/// @brief Destructor.
		constexpr ~UTFString() {}

		/// @brief Adds a new character to the end of the `UTFString`.
		/// @param value Character to add.
		/// @return Reference to self.
		constexpr SelfType& pushBack(DataType const& value) {
			BaseType::back() = value;
			BaseType::pushBack('\0');
			return *this;
		}

		/// @brief Removes an character from the end of the `UTFString`.
		/// @return Removed character.
		/// @throw OutOfBoundsException when `UTFString` is empty.
		constexpr DataType popBack() {
			if (empty()) emptyError();
			BaseType::popBack();
			DataType value = BaseType::back();
			BaseType::back() = '\0';
			return value;
		}

		/// @brief Inserts a character at a specified index in the `UTFString`.
		/// @param value Character to insert.
		/// @param index Index of which to insert in.
		/// @return Reference to self.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		constexpr SelfType& insert(DataType const& value, IndexType index) {
			assertIsInBounds(index);
			wrapBounds(index);
			BaseType::insert(value, index);
			return *this;
		}

		/// @brief Inserts a `UTFString` at a specified index in the `UTFString`.
		/// @param other `UTFString` to insert.
		/// @param index Index of which to insert in.
		/// @return Reference to self.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		constexpr SelfType& insert(SelfType const& other, IndexType index) {
			assertIsInBounds(index);
			wrapBounds(index);
			BaseType::insert(BaseType(other.begin(), other.end()), index);
			return *this;
		}

		/// @brief Inserts a given character, a given amount of times, at a specified index in the `UTFString`.
		/// @param value Character to be inserted.
		/// @param count Amount of times to insert the character.
		/// @param index Index of which to insert in.
		/// @return Reference to self.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		constexpr SelfType& insert(DataType const& value, SizeType const count, IndexType index) {
			assertIsInBounds(index);
			wrapBounds(index);
			BaseType::insert(count, value, index);
			return *this;
		}

		/// @brief Inserts a fixed array of characters at a specified index in the `UTFString`.
		/// @tparam S Size of fixed array.
		/// @param values Characters to insert.
		/// @param index Index of which to insert in.
		/// @return Reference to self.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		template<SizeType S>
		constexpr SelfType& insert(As<ConstantType[S]> const& values, IndexType index) {
			assertIsInBounds(index);
			wrapBounds(index);
			BaseType::insert(values, index);
			return *this;
		}

		/// @brief Inserts a fixed array of characters at a specified index in the `UTFString`.
		/// @tparam S Size of fixed array.
		/// @param values Characters to insert.
		/// @param index Index of which to insert in.
		/// @return Reference to self.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		template<SizeType S>
		constexpr SelfType& insert(As<char const[S]> const& values, IndexType index) {
			assertIsInBounds(index);
			wrapBounds(index);
			BaseType::insert(BaseType(values), index);
			return *this;
		}

		/// @brief Ensures the `UTFString` can hold AT LEAST a given capacity.
		/// @param count Minimum size of the `UTFString`.
		/// @return Reference to self.
		/// @note
		///		This guarantees the capacity will be AT LEAST `count`,
		/// 	but does not guarantee the capacity will be EXACTLY `count`.
		///		For that, use `resize`.
		constexpr SelfType& reserve(SizeType const count) {
			BaseType::reserve(count + 1);
			return *this;
		}

		/// @brief Resizes the `UTFString`, so the capacity is of a given size.
		/// @param newSize New `UTFString` size.
		/// @return Reference to self.
		/// @note
		///		This guarantees the capacity will be EXACTLY of `newSize`.
		/// 	If you need the capacity to be AT LEAST `newSize`, use `reserve`.
		constexpr SelfType& resize(SizeType const newSize) {
			BaseType::resize(newSize + 1);
			return *this;
		}

		/// @brief Expands the `UTFString`, such that it can hold AT LEAST `size()` + `count`.
		/// @param count Count to increase by.
		/// @return Reference to self.
		constexpr SelfType& expand(SizeType const count) {
			BaseType::expand(count + 1);
			return *this;
		}

		/// @brief
		///		Ensures the `UTFString` can hold AT LEAST a given capacity.
		/// @param count Minimum size of the `UTFString`.
		/// @param fill Character to use as fill.
		/// @return Reference to self.
		/// @note
		///		If current size is smaller,
		///		then it fills the extra space added with the given `fill`,
		///		up to `count`, and sets current size to it.
		/// @note
		///		This guarantees the capacity will be AT LEAST `count`,
		/// 	but does not guarantee the capacity will be EXACTLY `count`.
		///		For that, use `resize`.
		constexpr SelfType& reserve(SizeType const count, DataType const& fill) {
			BaseType::back() = fill;
			BaseType::reserve(count + 1, fill);
			BaseType::back() = '\0';
			return *this;
		}

		/// @brief Resizes the `UTFString`, so the capacity is of a given size, then sets current size to it.
		/// @param newSize New `UTFString` size.
		/// @param fill Character to use as fill.
		/// @return Reference to self.
		///	@note
		///		If current size is smaller,
		///		then it fills the extra space added with the given `fill`.
		/// @note
		///		This guarantees the capacity will be EXACTLY of `newSize`.
		///		If you need the capacity to be AT LEAST `newSize`, use `reserve`.
		constexpr SelfType& resize(SizeType const newSize, DataType const& fill) {
			BaseType::back() = fill;
			BaseType::resize(newSize + 1, fill);
			BaseType::back() = '\0';
			return *this;
		}

		/// @brief
		///		Expands the `UTFString`, such that it can hold AT LEAST the current size,
		///		plus a given `count`.
		/// @param count Count to increase by.
		/// @param fill Character to use as fill.
		/// @return Reference to self.
		///	@note
		///		If current size is smaller,
		///		then it fills the extra space added with the given `fill`.
		constexpr SelfType& expand(SizeType const count, DataType const& fill) {
			BaseType::back() = fill;
			BaseType::expand(count + 1, fill);
			BaseType::back() = '\0';
			return *this;
		}

		/// @brief Ensures the current capacity is EXACTLY the current size.
		/// @return Reference to self.
		constexpr SelfType& tighten() {
			BaseType::tighten();
			return *this;
		}

		/// @brief Reverses the `UTFString`.
		/// @return Reference to self.
		constexpr SelfType& reverse() {
			::CTL::reverse(data(), size());
			return *this;
		}

		/// @brief Returns a reversed copy of the `UTFString`.
		/// @return A reversed copy of the `UTFString`.
		constexpr SelfType reversed() const {
			return SelfType(*this).reverse();
		}

		/// @brief Finds the the position of the first character that matches a value.
		/// @param value Value to search for.
		/// @return The index of the value, or -1 if not found.
		constexpr IndexType find(DataType const& value) const {
			IndexType location = BaseType::find(value);
			return (location == IndexType(size())) ? -1 : location;
		}

		/// @brief Finds the the position of the last character that matches a value.
		/// @param value Value to search for.
		/// @return The index of the value, or -1 if not found.
		constexpr IndexType rfind(DataType const& value) const {
			if (empty()) return -1;
			auto const start = rbegin(), stop = rend();
			for (auto i = start; i != stop; ++i)
				if (BaseType::ComparatorType::equals(*i, value))
					return size()-(i-start)-1;
			return -1;
		}

		/// @brief Performs a binary search to find the index of a character that matches a value.
		/// @param value Value to search for.
		/// @return The index of the value, or -1 if not found.
		/// @note Requires the string to be sorted.
		constexpr IndexType bsearch(DataType const& value) const {
			IndexType location = BaseType::find(value);
			return (location == IndexType(size())) ? -1 : location;
		}

		/// @brief Removes a character at a given index.
		/// @param index Index of the character to remove.
		/// @return Reference to self.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note
		///		Does not resize `UTFString`, merely moves it to the end, and destructs it.
		///		If you need the `UTFString` size to change, use `erase`.
		constexpr SelfType& remove(IndexType index) {
			assertIsInBounds(index);
			wrapBounds(index);
			return BaseType::remove(index);
		}

		/// @brief Removes characters that match a given character.
		/// @param value Character to match.
		/// @return Count of characters removed.
		/// @note
		///		Does not resize `UTFString`, merely moves it to the end, and destructs it.
		///		If you need the `UTFString` size to change, use `erase`.
		constexpr SizeType removeLike(DataType const& value) {
			SizeType count = BaseType::removeLike(value) - (value == '\0');
			if (value == '\0') BaseType::back() = '\0';
			return count;
		}

		/// @brief Removes characters that do not match a given character.
		/// @param value Character to match.
		/// @return Count of characters removed.
		/// @note
		///		Does not resize `UTFString`, merely moves it to the end, and destructs it.
		///		If you need the `UTFString` size to change, use `erase`.
		constexpr SizeType removeUnlike(DataType const& value) {
			SizeType count = BaseType::removeUnlike(value) - (value == '\0');
			if (value == '\0') BaseType::back() = '\0';
			return count;
		}

		/// @brief Removes characters that match a given predicate.
		/// @tparam TPredicate Predicate type.
		/// @param predicate Predicate to use as check.
		/// @return Count of characters removed.
		/// @note
		///		Does not resize `UTFString`, merely moves it to the end, and destructs it.
		///		If you need the `UTFString` size to change, use `erase`.
		template<class TPredicate>
		constexpr SizeType removeIf(TPredicate const& predicate) {
			SizeType count = BaseType::removeIf(predicate);
			if (predicate(BaseType::back()))
				--count;
			return count;
		}

		/// @brief Removes characters that do not match a given predicate.
		/// @tparam TPredicate Predicate type.
		/// @param predicate Predicate to use as check.
		/// @return Count of characters removed.
		/// @note
		///		Does not resize `UTFString`, merely moves it to the end, and destructs it.
		///		If you need the `UTFString` size to change, use `erase`.
		template<class TPredicate>
		constexpr SizeType removeIfNot(TPredicate const& predicate) {
			SizeType count = BaseType::removeIfNot(predicate);
			if (!predicate(BaseType::back()))
				--count;
			return count;
		}

		/// @brief Erases a character at a given index.
		/// @param index Index of the character to erase.
		/// @return Reference to self.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note
		///		Resizes the `UTFString`.
		///		If you need the `UTFString` size to remain the same, use `remove`.
		constexpr SelfType& erase(IndexType const index) {
			assertIsInBounds(index);
			wrapBounds(index);
			return BaseType::erase(index);
		}

		/// @brief Erases characters that match a given value.
		/// @param value Value to match.
		/// @return Count of characters removed.
		/// @note
		///		Resizes the `UTFString`.
		///		If you need the `UTFString` size to remain the same, use `remove`.
		constexpr SelfType& eraseLike(DataType const& value) {
			resize(removeLike(value));
			return *this;
		}

		/// @brief Erases characters that do not a given value.
		/// @param value Value to match.
		/// @return Count of characters removed.
		/// @note
		///		Resizes the `UTFString`.
		///		If you need the `UTFString` size to remain the same, use `remove`.
		constexpr SelfType& eraseUnlike(DataType const& value) {
			resize(removeUnlike(value));
			return *this;
		}

		/// @brief Erases characters that match a given predicate.
		/// @tparam TPredicate Predicate type.
		/// @param predicate Predicate to use as check.
		/// @note
		///		Resizes the `UTFString`.
		///		If you need the `UTFString` size to remain the same, use `remove`.
		template<Type::Functional<PredicateType> TPredicate>
		constexpr SelfType& eraseIf(TPredicate const& predicate) {
			resize(removeIf(predicate));
			return *this;
		}

		/// @brief Erases characters that do not match a given predicate.
		/// @tparam TPredicate Predicate type.
		/// @param predicate Predicate to use as check.
		/// @note
		///		Resizes the `UTFString`.
		///		If you need the `UTFString` size to remain the same, use `remove`.
		template<Type::Functional<PredicateType> TPredicate>
		constexpr SelfType& eraseIfNot(TPredicate const& predicate) {
			resize(removeIfNot(predicate));
			return *this;
		}

		/// @brief Returns a substring starting from a given index.
		/// @param start Starting index to copy from.
		/// @return Substring.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		constexpr SelfType sliced(IndexType start) const {
			if (IndexType(size()) < start) return SelfType();
			wrapBounds(start);
			return SelfType(begin() + start, end());
		}

		/// @brief Returns a substring ranging from between two indices.
		/// @param start Starting index to copy from.
		/// @param stop End index to stop copying from.
		/// @return Substring.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		constexpr SelfType sliced(IndexType start, IndexType stop) const {
			if (IndexType(size()) < start) return SelfType();
			wrapBounds(start);
			if (IndexType(size()) < stop) return sliced(start);
			wrapBounds(stop);
			if (stop < start) return SelfType();
			return SelfType(begin() + start, begin() + stop + 1);
		}

		/// @brief Appends another `UTFString` to the end of the `UTFString`.
		/// @param other `UTFString` to copy contents from.
		/// @return Reference to self.
		constexpr SelfType& appendBack(SelfType const& other) {
			expand(other.size());
			BaseType::popBack();
			BaseType::appendBack(other.begin(), other.end());
			BaseType::pushBack('\0');
			return *this;
		}

		/// @brief Appends a character several times to the end of the `UTFString`.
		/// @param count Amount of character to append.
		/// @param fill Value of the characters.
		/// @return Reference to self.
		constexpr SelfType& appendBack(SizeType const count, DataType const& fill) {
			return expand(count, fill);
		}

		/// @brief Appends a range of characters to the end of the `UTFString`.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator pointing to end of range.
		/// @return Reference to self.
		constexpr SelfType& appendBack(ConstIteratorType const& begin, ConstIteratorType const& end) {
			expand(end - begin);
			BaseType::popBack();
			BaseType::appendBack(begin, end);
			pushBack('\0');
			return *this;
		}

		/// @brief Appends a range of characters to the end of the `UTFString`.
		/// @param begin Reverse iterator to beginning of range.
		/// @param end Reverse iterator pointing to end of range.
		/// @return Reference to self.
		constexpr SelfType& appendBack(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end) {
			expand(end - begin);
			BaseType::popBack();
			BaseType::appendBack(begin, end);
			BaseType::pushBack('\0');
			return *this;
		}

		/// @brief Appends a fixed array of characters to the end of the `UTFString`.
		/// @tparam S Size of the array.
		/// @param values Characters to append.
		/// @return Reference to self.
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

		/// @brief Clears the `UTFString`.
		/// @return Reference to self.
		/// @note
		///		Does not free the underlying character array held by the `UTFString`.
		///		To actually free the underlying character array, call `dispose`.
		constexpr SelfType& clear() {
			BaseType::clear();
			BaseType::pushBack('\0');
			return *this;
		}

		/// @brief Frees the underlying character array held by the `UTFString`.
		/// @return Reference to self.
		/// @note To not free the underlying character array, call `clear`.
		constexpr SelfType& dispose() {
			BaseType::dump();
			return *this;
		}

		/// @brief Returns an iterator to the end of the `UTFString`.
		/// @return Iterator to the end of the `UTFString`.
		constexpr IteratorType				end()			{return data()+size();								}
		/// @brief Returns an iterator to the end of the `UTFString`.
		/// @return Iterator to the end of the `UTFString`.
		constexpr ConstIteratorType			end() const		{return data()+size();								}
		/// @brief Returns a reverse iterator to the end of the `UTFString`.
		/// @return Reverse iterator to the end of the `UTFString`.
		constexpr PointerType				cend()			{return data()+size();								}
		/// @brief Returns a reverse iterator to the end of the `UTFString`.
		/// @return Reverse iterator to the end of the `UTFString`.
		constexpr ConstPointerType			cend() const	{return data()+size();								}
		/// @brief Returns a reverse iterator to the beginning of the `UTFString`.
		/// @return Reverse iterator to the beginning of the `UTFString`.
		constexpr ReverseIteratorType		rbegin()		{return ReverseIteratorType(data()+size());			}
		/// @brief Returns a reverse iterator to the beginning of the `UTFString`.
		/// @return Reverse iterator to the beginning of the `UTFString`.
		constexpr ConstReverseIteratorType	rbegin() const	{return ConstReverseIteratorType(data()+size());	}

		/// @brief Returns the last character.
		/// @return Last character.
		/// @throw OutOfBoundsException when `UTFString` is empty.
		constexpr ReferenceType 	back()			{return at(size()-1);	}
		/// @brief Returns the last character.
		/// @return Last character.
		/// @throw OutOfBoundsException when `UTFString` is empty.
		constexpr DataType 			back() const	{return at(size()-1);	}

		/// @brief Returns the character at a given index.
		/// @param index Index of the character.
		/// @return Reference to the character.
		/// @throw OutOfBoundsException when `UTFString` is empty.
		constexpr DataType& at(IndexType index) {
			assertIsInBounds(index);
			wrapBounds(index);
			return BaseType::at(index);
		}

		/// @brief Returns the character at a given index.
		/// @param index Index of the character.
		/// @return Character.
		/// @throw OutOfBoundsException when `UTFString` is empty.
		constexpr DataType at(IndexType index) const {
			assertIsInBounds(index);
			wrapBounds(index);
			return BaseType::at(index);
		}

		/// @brief Returns the character at a given index.
		/// @param index Index of the character.
		/// @return Reference to the character.
		/// @throw OutOfBoundsException when `UTFString` is empty.
		constexpr ReferenceType	operator[](IndexType const index)	{return at(index);}
		/// @brief Returns the character at a given index.
		/// @param index Index of the character.
		/// @return Character.
		/// @throw OutOfBoundsException when `UTFString` is empty.
		constexpr DataType operator[](IndexType const index) const	{return at(index);}

		/// @brief Equality operator.
		/// @param other Other `UTFString` to compare with.
		/// @return Whether they're equal.
		constexpr bool operator==(SelfType const& other) const {
			return BaseType::equals(other);
		}

		/// @brief Threeway comparison operator.
		/// @param other Other `UTFString` to compare with.
		/// @return Order between both `UTFString`s.
		constexpr OrderType operator<=>(SelfType const& other) const {
			return compare(other);
		}

		/// @brief Returns whether this `UTFString` is equal to another `UTFString`.
		/// @param other Other `UTFString` to compare with.
		/// @return Whether they're equal.
		constexpr SizeType equals(SelfType const& other) const {
			return BaseType::equals(other);
		}

		/// @brief Returns the result of a threeway comparison with another `UTFString`.
		/// @param other Other `UTFString` to compare with.
		/// @return Order between both `UTFString`s.
		constexpr OrderType compare(SelfType const& other) const {
			return BaseType::compare(other);
		}

		/// @brief Returns how different this `UTFString` is from another `UTFString`.
		/// @param other Other `UTFString` to compare with.
		/// @return How different it is.
		constexpr SizeType disparity(SelfType const& other) const {
			return BaseType::disparity(other);
		}

		/// @brief Apllies a procedure to all characters in the `UTFString`.
		/// @tparam TProcedure Procedure type.
		/// @param fun Procedure to apply.
		/// @return Reference to self.
		template <Type::Functional<TransformType> TProcedure>
		constexpr SelfType& transform(TProcedure const& fun) {
			for(DataType& v: *this)
				v = fun(v);
			return *this;
		}

		/// @brief Returns a `transform`ed `UTFString`.
		/// @tparam TProcedure Procedure type.
		/// @param fun Procedure to apply.
		/// @return Transformed string.
		template<Type::Functional<TransformType> TProcedure>
		constexpr SelfType transformed(TProcedure const& fun) const {
			return SelfType(*this).transform(fun);
		}

		/// @brief Apllies a procedure to all characters in the `UTFString`.
		/// @tparam TProcedure Procedure type.
		/// @param fun Procedure to apply.
		/// @return Reference to self.
		template <Type::Functional<TransformType> TProcedure>
		constexpr SelfType& operator|=(TProcedure const& fun) {
			return transform(fun);
		}

		/// @brief Returns a `transform`ed `UTFString`.
		/// @tparam TProcedure Procedure type.
		/// @param fun Procedure to apply.
		/// @return Transformed string.
		template <Type::Functional<TransformType> TProcedure>
		constexpr SelfType operator|(TProcedure const& fun) const {
			return transformed(fun);
		}

		/// @brief Apllies a procedure to all characters in the `UTFString`.
		/// @tparam TProcedure Procedure type.
		/// @param fun Procedure to apply.
		/// @return Reference to self.
		template <Type::Functional<SelfType&(SelfType&)> TProcedure>
		constexpr SelfType& operator|=(TProcedure const& fun) {
			return fun(*this);
		}

		/// @brief Returns a copy of the `UTFString`, with the given procedure applied to it.
		/// @tparam TProcedure Procedure type.
		/// @param fun Procedure to apply.
		/// @return Transformed string.
		template <Type::Functional<SelfType(SelfType const&)> TProcedure>
		constexpr SelfType operator|(TProcedure const& fun) const {
			return fun(*this);
		}

		/// @brief Returns whether all characters match a given predicate.
		/// @tparam TPredicate Predicate type.
		/// @param cond Predicate to match.
		/// @return Whether all characters match.
		template<class TPredicate>
		constexpr bool validate(TPredicate const& cond) const {
			if (empty()) return false;
			for (DataType const& c: *this)
				if (!cond(c))
					return false;
			return true;
		}

		/// @brief Removes all characters that do not match a given predicate.
		/// @tparam TPredicate Predicate type.
		/// @param filter Predicate to match.
		/// @return Reference to self.
		template<Type::Functional<PredicateType> TPredicate>
		constexpr SelfType& filter(TPredicate const& filter) {
			return eraseIfNot(filter);
		}

		/// @brief Removes all characters that fail a given comparison.
		/// @tparam TCompare Compare type.
		/// @param compare Comparison to make.
		/// @return Reference to self.
		template<Type::Functional<CompareType> TCompare>
		constexpr SelfType& filter(TCompare const& compare) {
			return *this = filtered(compare);
		}

		/// @brief Returns a `filter`ed `UTFString`.
		/// @tparam TPredicate Predicate type.
		/// @param filter Predicate to match.
		/// @return `filter`ed `UTFString`.
		template<Type::Functional<PredicateType> TPredicate>
		constexpr SelfType filtered(TPredicate const& filter) const {
			return SelfType(*this).eraseIfNot(filter);
		}

		/// @brief Returns a `filter`ed `UTFString`.
		/// @tparam TCompare Compare type.
		/// @param compare Comparison to make.
		/// @return `filter`ed `UTFString`.
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

		/// @brief Returns the current `UTFString`, divided at a given index.
		/// @param index The index to use as pivot.
		/// @return A `List`  containing the two halves of this `UTFString`.
		/// @throw OutOfBoundsException when index is bigger than `UTFString` size.
		/// @note If index is negative, it will be interpreted as starting from the end of the `UTFString`.
		constexpr List<SelfType, SizeType> divide(IndexType index) const {
			List<SelfType, SizeType> res;
			assertIsInBounds(res);
			wrapBounds(index);
			res.pushBack(sliced(0, index));
			res.pushBack(sliced(index+1));
			return res;
		}

		/// @brief Splits the string by a separator.
		/// @param sep Separator.
		/// @return List of split strings.
		constexpr List<SelfType, SizeType> split(DataType const& sep) const {
			List<SelfType, SizeType> res;
			SelfType buf;
			for (ConstReferenceType v: *this) {
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

		/// @brief Stream insertion operator.
		constexpr OutputStreamType& operator<<(OutputStreamType& o) const	{if (!empty()) o << toString(); return o;}
		/// @brief Stream insertion operator.
		constexpr OutputStreamType& operator<<(OutputStreamType& o)			{if (!empty()) o << toString(); return o;}

		/// @brief Stream insertion operator.
		friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType& self)			{if (!self.empty()) o << self.toString(); return o;}
		/// @brief Stream insertion operator.
		friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType const& self)	{if (!self.empty()) o << self.toString(); return o;}

		/// @brief Copy assignment operator (`UTFString`).
		/// @param other `UTFString` to copy from.
		/// @return Reference to self.
		constexpr SelfType& operator=(SelfType const& other)						{BaseType::operator=(other); return *this;				}
		/// @brief Move assignment operator (`UTFString`).
		/// @param other `UTFString` to move.
		/// @return Reference to self.
		constexpr SelfType& operator=(SelfType&& other)								{BaseType::operator=(CTL::move(other)); return *this;	}
		/// @brief Copy assignment operator (null-terminated string).
		/// @param other String to copy from.
		/// @return Reference to self.
		constexpr SelfType& operator=(typename String::CStringType const& other)	{BaseType::operator=(SelfType(other)); return *this;	}
		/// @brief Copy assignment operator (null-terminated unicode string).
		/// @param other String to copy from.
		/// @return Reference to self.
		constexpr SelfType& operator=(u8cstring const& cstr)						{BaseType::operator=(UTFString<8>(cstr)); return *this;	}

		/// @brief Equality comparison operator (char array).
		/// @tparam S Array size.
		/// @param str Array to compare with.
		/// @return Whether they're equal.
		template<SizeType S>
		constexpr bool operator==(As<char const[S]> const& str) const						{return *this == SelfType(str);			}
		/// @brief Equality comparison operator (null-terminated string).
		/// @param str String to compare with.
		/// @return Whether they're equal.
		constexpr bool operator==(typename String::CStringType const& str) const			{return *this == SelfType(str);			}
		/// @brief Threeway comparison operator (char array).
		/// @tparam S Array size.
		/// @param str Char array to compare with.
		/// @return Order between objects.
		template<SizeType S>
		constexpr OrderType operator<=>(As<char const[S]> const& str) const					{return *this <=> SelfType(str);		}
		/// @brief Threeway comparison operator (null-terminated string).
		/// @param str String to compare with.
		/// @return Order between objects.
		constexpr OrderType operator<=>(typename String::CStringType const& str) const		{return *this <=> SelfType(str);		}

		/// @brief String concatenation operator (character).
		/// @param value Character to concatenate.
		/// @return Resulting concatenated string.
		constexpr SelfType operator+(DataType const& value) const	{return SelfType(*this).pushBack(value);	}
		/// @brief String concatenation operator (`UTFString`).
		/// @param value `UTFString` to concatenate.
		/// @return Resulting concatenated string.
		constexpr SelfType operator+(SelfType const& other) const	{return SelfType(*this).appendBack(other);	}

		/// @brief String concatenation operator (null-terminated string).
		/// @param value String to concatenate.
		/// @return Resulting concatenated string.
		constexpr SelfType operator+(typename String::CStringType const& str) const	{return (*this) + SelfType(str);}
		/// @brief String concatenation operator (null-terminated string).
		/// @param value String to concatenate.
		/// @return Resulting concatenated string.
		constexpr SelfType operator+(u8cstring const& str) const					{return (*this) + SelfType(str);}
		/// @brief String concatenation operator (char array).
		/// @tparam S Array size.
		/// @param value Char array to concatenate.
		/// @return Resulting concatenated string.
		template<SizeType S>
		constexpr SelfType operator+(As<char const[S]> const& str) const			{return (*this) + SelfType(str);}
		/// @brief String concatenation operator (char array).
		/// @tparam S Array size.
		/// @param value Char array to concatenate.
		/// @return Resulting concatenated string.
		template<SizeType S>
		constexpr SelfType operator+(As<u8char const[S]> const& str) const			{return (*this) + UTFString<8>(str);}

		/// @brief String concatenation operator (character).
		/// @param value Character to concatenate.
		/// @param self `UTFString` to concatenate with.
		/// @return Resulting concatenated string.
		friend constexpr SelfType operator+(DataType const& value, SelfType const& self)	{return SelfType().pushBack(value) + self;	}

		/// @brief String concatenation operator (null-terminated string).
		/// @param value String to concatenate.
		/// @param self `UTFString` to concatenate with.
		/// @return Resulting concatenated string.
		friend constexpr SelfType operator+(typename String::CStringType const& str, SelfType const& self)	{return SelfType(str) + (self);}
		/// @brief String concatenation operator (null-terminated string).
		/// @param value String to concatenate.
		/// @param self `UTFString` to concatenate with.
		/// @return Resulting concatenated string.
		friend constexpr SelfType operator+(u8cstring const& str, SelfType const& self)						{return SelfType(str) + (self);}
		/// @brief String concatenation operator (char array).
		/// @tparam S Array size.
		/// @param value Char array to concatenate.
		/// @param self `UTFString` to concatenate with.
		/// @return Resulting concatenated string.
		template<SizeType S>
		friend constexpr SelfType operator+(As<char const[S]> const& str, SelfType const& self)				{return SelfType(str) + (self);}
		/// @brief String concatenation operator (unicode char array).
		/// @tparam S Array size.
		/// @param value Char array to concatenate.
		/// @param self `UTFString` to concatenate with.
		/// @return Resulting concatenated string.
		template<SizeType S>
		friend constexpr SelfType operator+(As<u8char const[S]> const& str, SelfType const& self)			{return UTFString<8>(str) + (self);}

		/// @brief String appending operator (character).
		/// @param value Caracter to append.
		/// @return Reference to self.
		constexpr SelfType& operator+=(DataType const& value)					{pushBack(value); return *this;					}
		/// @brief String appending operator (`UTFString`).
		/// @param value `UTFString` to append.
		/// @return Reference to self.
		constexpr SelfType& operator+=(SelfType const& other)					{appendBack(other); return *this;				}
		/// @brief String appending operator (null-terminated string).
		/// @param value String to append.
		/// @return Reference to self.
		constexpr SelfType& operator+=(typename String::CStringType const& str)	{appendBack(SelfType(str)); return *this;		}
		/// @brief String appending operator (null-terminated string).
		/// @param value String to append.
		/// @return Reference to self.
		constexpr SelfType& operator+=(u8cstring const& str)					{appendBack(SelfType(str)); return *this;		}
		/// @brief String appending operator (char array).
		/// @tparam S Array size.
		/// @param value Char array to append.
		/// @return Reference to self.
		template<SizeType S>
		constexpr SelfType& operator+=(As<char const[S]> str)					{appendBack(str); return *this;					}
		/// @brief String appending operator (unicode char array).
		/// @tparam S Array size.
		/// @param value Char array to append.
		/// @return Reference to self.
		template<SizeType S>
		constexpr SelfType& operator+=(As<u8char const[S]> str)					{appendBack(UTFString<8>(str)); return *this;	}

		/// @brief Returns the current size of the underlying character array.
		/// @return Size of the underlying character array.
		constexpr SizeType capacity() const	{return BaseType::capacity() - 1;	}

		/// @brief Returns whether the string is empty.
		/// @return Whether the string is empty.
		constexpr SizeType empty() const	{return size() == 0;				}

		/// @brief Returns the string's size.
		/// @return Size of string.
		constexpr SizeType size() const {
			if (BaseType::empty()) return 0;
			return BaseType::size() - 1;
		}

		/// @brief Swap algorithm for `UTFString`.
		/// @param a `UTFString` to swap.
		/// @param b `UTFString` to swap with.
		friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
			swap<BaseType>(a, b);
		}

		/// @brief Returns the string as a standard string.
		/// @return String as standard string.
		constexpr String toString() const {
			String out;
			if (empty()) return out;
			As<char[4]> buf;
			MX::memset(buf, 4, '\0');
			out.reserve(size() * DataType::SIZE);
			for (DataType const& ch: *this) {
				auto const sz = ch.toBytes(buf);
				out.appendBack(buf, buf + sz);
			}
			out.tighten();
			return out;
		}

		/// @brief Converts the string to a different encoding.
		/// @tparam NE New encoding.
		/// @return String as new encoding.
		template<usize NE>
		constexpr UTFString<NE> toUTF() const
		requires (UTF != NE) {
			UTFString<NE> out;
			if (empty()) return out;
			out.reserve(size());
			for (auto const& ch: this)
				out.pushBack(ch);
			return out;
		}

		/// @brief Converts the string to a different encoding.
		/// @tparam NE New encoding.
		/// @return String as new encoding.
		template<usize NE>
		constexpr UTFString<NE> toUTF() const requires (UTF == NE) {return *this;}

		/// @brief Converts the string to an UTF-8 string.
		/// @return String as new encoding.
		constexpr UTFString<8> toUTF8() const	{return toUTF<8>();		}
		/// @brief Converts the string to an UTF-32 string.
		/// @return String as new encoding.
		constexpr UTFString<32> toUTF32() const	{return toUTF<32>();	}

	private:
		constexpr void assertIsInBounds(IndexType const index) const {
			if (index >= 0 && usize(index) > (size()-1)) outOfBoundsError(index);
		}

		constexpr void wrapBounds(IndexType& index) const {
			Iteratable::wrapBounds(index, size());
		}

		[[noreturn]] constexpr void outOfBoundsError(IndexType const index) const {
			throw OutOfBoundsException("Index is out of bounds!");
		}

		[[noreturn]] constexpr static void emptyError() {
			throw OutOfBoundsException("String is empty!");
		}
	};

	template<usize S>
	using UString	= UTFString<S>;

	using U8String	= UString<8>;
	using U32String	= UString<32>;

	using U8Char	= typename UString<8>::DataType;
	using U32Char	= typename UString<32>::DataType;
}

using UTF8String	= UTF::U8String;
using UTF32String	= UTF::U32String;

using UTF8Char	= UTF::U8Char;
using UTF32Char	= UTF::U32Char;

//static_assert(UTF8String("Compile-time Magics!").size());

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wliteral-suffix"
#else
#pragma GCC diagnostic ignored "-Wuser-defined-literals"
#endif
/// @brief String literals.
namespace Literals::Text::Unicode {
	/// @brief CTL `UTF8String` literal.
	constexpr UTF8String operator "" u8s	(cstring cstr, litsize sz)	{return UTF8String(cstr, sz);	}
	/// @brief CTL `UTF32String` literal.
	constexpr UTF32String operator "" u32s	(cstring cstr, litsize sz)	{return UTF32String(cstr, sz);	}
}

CTL_NAMESPACE_END

#endif
