#ifndef CTL_EX_CONTAINER_OBFUSCATOR_H
#define CTL_EX_CONTAINER_OBFUSCATOR_H

#include "../../ctl/ctl.hpp"
#include "../../ctl/exnamespace.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Decays to a "C-style" fixed array.
/// @tparam T Element type.
/// @tparam S Array size.
template<class T, usize S>
using CArray = Decay::AsType<T[S]>;

/// @brief Decays to a fixed-size "C-style" string.
/// @tparam S String size.
template<usize S>
using FixedCString = CArray<const char, S>;

/// @brief Container-related type constraints.
namespace Type::Ex::Container {
	/// @brief Type must be a valid static string obfuscator.
	template<typename T, usize S, typename TString>
	concept StaticStringObfuscator =
		::CTL::Type::Constructible<T>
	&&	::CTL::Type::Constructible<T, FixedCString<S> const&>
	&&	requires (T t) {
			{t.demangled()} -> ::CTL::Type::Equal<String>;
		}
	;
}

/// @brief Value obfuscator interface.
/// @tparam TData Value type.
template<typename TData>
struct IObfuscator: CTL::Typed<TData> {
	using Typed = CTL::Typed<TData>;

	using
		typename Typed::DataType
	;

	/// @brief Destructor.
	constexpr virtual ~IObfuscator() {}

	/// @brief Returns the deobfuscated value.
	/// @return Deobfuscated value.
	/// @note Must be implemented in the derived classes.
	virtual DataType deobfuscated() const = 0;

	/// @brief Returns the deobfuscated value.
	/// @return Deobfuscated value.
	DataType operator()() const	{return deobfuscated();}
	/// @brief Returns the deobfuscated value.
	/// @return Deobfuscated value.
	DataType operator*() const	{return deobfuscated();}
	/// @brief Returns the deobfuscated value.
	/// @return Deobfuscated value.
	operator DataType() const 	{return deobfuscated();}
};

/// @brief Implementations.
namespace Impl {
	/// @brief Returns whether a number is a prime.
	/// @param v Number to check.
	/// @return Whether it is prime.
	consteval bool isPrime(usize const v) {
		if (v == 0) return false;
		for (usize i = 2; i < v; ++i)
			if (v % i == 0)
				return false;
		return true;
	}

	/// @brief Returns the nearest prime to a number, that is less than or equal to it.
	/// @param v Number to get nearest prime.
	/// @param excludeSelf Whether to exclude the number itself, if it is prime.
	/// @return Nearest prime.
	consteval usize nearestPrime(usize const v, bool excludeSelf = false) {
		if (!v) return 0;
		if (v < 2) return excludeSelf ? v-1 : v;
		for (usize i = (excludeSelf ? (v-1) : v); i > 0; --i)
			if (isPrime(i))
				return i;
		return 0;
	}

	static_assert(nearestPrime(128, true) == 127);
	static_assert(nearestPrime(127, true) == 113);
	static_assert(nearestPrime(113, true) == 109);

	/// @brief Contains information on the primality of a number.
	/// @tparam N Number to check.
	template<usize N>
	struct PrimeNumber {
		/// @brief Number.
		constexpr static usize VALUE	= N;
		/// @brief Whether it is prime.
		constexpr static bool IS_PRIME	= isPrime(N);
		/// @brief Nearest prime to it, excluding itself.
		constexpr static usize NEAREST	= nearestPrime(N, true);
		/// @brief Nearest prime to it, including itself.
		constexpr static usize CLOSEST	= nearestPrime(N, false);
	};

	/// @brief Shuffles bytes around.
	template <Type::Unsigned T>
	constexpr T shuffle(T const& val) {
		if constexpr (sizeof(T) == 1) return val;
		else if constexpr (sizeof(T) == 2)
			return (val << 8 & 0xFF00) | (val >> 8 & 0x00FF);
		else if constexpr (sizeof(T) == 4)
			return
				(val << 8 & 0x00FF0000)
			|	(val >> 8 & 0x0000FF00)
			|	(val & 0xFF0000FF)
			;
		constexpr usize H		= sizeof(T) / 2;
		constexpr T MASK		= NumberLimit<T>::HIGHEST;
		constexpr T BYTE_MASK	= 0xFF;
		constexpr T HALF_MASK	= MASK >> (H*8);
		T res = 0;
		const T left	= val & HALF_MASK;
		const T right	= val & ~HALF_MASK;
		for (usize i = 0; i < H; ++i) {
			res |= ((left >> (i*8)) & BYTE_MASK) << (i*8);
			res |= ((right >> (i*8 + 8)) & BYTE_MASK) << (i*8 + 8);
		}
		return res;
	}

	static_assert(shuffle<uint8>(shuffle<uint8>(0xfe)) == uint8(0xfe));
	static_assert(shuffle<uint16>(shuffle<uint16>(0xfe3c)) == uint16(0xfe3c));
	static_assert(shuffle<uint32>(shuffle<uint32>(0xfe3c2da1)) == uint32(0xfe3c2da1));
	static_assert(shuffle<uint64>(shuffle<uint64>(0xfe3c2da123)) == uint64(0xfe3c2da123));
}

/// @brief Static string obfuscators.
namespace StaticStringMangler {
	/// @brief Binary shuffle.
	/// @tparam S String size.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize S, usize MASK, bool PARITY, class TSize>
	struct BinaryShuffle;

	/// @brief Binary shuffle.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize MASK, bool PARITY, class TS>
	struct BinaryShuffle<0, MASK, PARITY, TS> {
		/// @brief String size.
		constexpr static usize SIZE = 0;

		constexpr BinaryShuffle()			{}
		template<class T>
		constexpr BinaryShuffle(T const&)	{}

		String mangled() const		{return String();}
		String demangled() const	{return String();}

	private:
	};

	/// @brief Binary shuffle.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize MASK, bool PARITY, class T>
	struct BinaryShuffle<1, MASK, PARITY, T> {
		/// @brief String size.
		constexpr static usize SIZE = 1;

		/// @brief Empty constructor.
		constexpr BinaryShuffle()							{				}
		/// @brief Constructs the mangled string from a single character.
		/// @param dat Character to mangle.
		constexpr BinaryShuffle(FixedCString<1> const& dat)	{c = dat[0];	}
		/// @brief Constructs the mangled string from a single byte.
		/// @param dat Byte to mangle.
		constexpr BinaryShuffle(Array<uint8, 1> const& dat)	{c = dat[0];	}

		/// @brief Returns the mangled string.
		/// @return Mangled string.
		String mangled() const		{return toString(c);}
		/// @brief Returns the demangled string.
		/// @return Demangled string.
		String demangled() const	{return toString(c);}

	private:
		/// @brief String character.
		char c;
	};

	/// @brief Binary shuffle.
	/// @tparam S String size.
	/// @tparam M Shuffle mask.
	/// @tparam P Shuffle parity.
	template<usize S, usize M, bool P, class TSize = CTL::Decay::Number::AsUnsigned<S>>
	struct BinaryShuffle {
	private:
		using SizeType = TSize;
	public:
		constexpr static usize
		/// @brief String size.
			SIZE = S,
		/// @brief Shuffle mask.
			MASK = M
		;
		/// @brief Shuffle parity.
		constexpr static bool PARITY = P;

		/// @brief Empty constructor.
		constexpr BinaryShuffle(): trueSize(Impl::shuffle<SizeType>(SIZE)) {}

		/// @brief Constructs a mangled string from a fixed string.
		/// @tparam CS String size.
		/// @param dat String to mangle.
		template<usize CS>
		constexpr BinaryShuffle(FixedCString<CS> const& dat): trueSize(Impl::shuffle<SizeType>(CS)) {
			static_assert(CS <= SIZE, "String must not be bigger than maximum size!");
			Array<uint8, SIZE> str {'\0'};
			for(usize i = 0; i < CS; ++i)
				str[i] = dat[i];
			decompose<Array<uint8, SIZE>>(str);
		}

		/// @brief Constructs a mangled string from a byte array.
		/// @tparam CS Array size.
		/// @param dat Bytes to mangle.
		template<usize CS>
		constexpr BinaryShuffle(Array<uint8, CS> const& dat): trueSize(Impl::shuffle<SizeType>(CS)) {
			static_assert(CS <= SIZE, "String must not be bigger than maximum size!");
			Array<uint8, SIZE> str {'\0'};
			for(usize i = 0; i < CS; ++i)
				str[i] = dat[i];
			decompose<Array<uint8, SIZE>>(str);
		}

		/// @brief Returns the mangled string.
		/// @return Mangled string.
		String mangled() const						{return (left.mangled() + right.mangled());						}

		/// @brief Returns the demangled string.
		/// @return Demangled string.
		String demangled() const requires (PARITY)	{return (right.demangled() + left.demangled()).resize(isize());	}
		/// @brief Returns the demangled string.
		/// @return Demangled string.
		String demangled() const requires (!PARITY)	{return (left.demangled() + right.demangled()).resize(isize());	}
	private:
		/// @brief Mask of the subsequent left & right sides.
		constexpr static usize NEW_MASK = MASK ^ (MASK >> 2);

		constexpr static bool
			/// @brief Whether the string is an even size.
			EVEN_SIZE	= (SIZE % 2 == 0),
			/// @brief Parity of the left side.
			LHS_PARITY	= (MASK & 0b10) ? !PARITY : PARITY,
			/// @brief Parity of the right side.
			RHS_PARITY	= (MASK & 0b01) ? !PARITY : PARITY
		;

		constexpr static usize
			/// @brief Size of the first half.
			H1{(EVEN_SIZE) ? (SIZE/2) : (SIZE/2+1)},
			/// @brief Size of the second half.
			H2{(SIZE/2)}
		;

		constexpr static usize
			/// @brief Size of the first half.
			LEFT_SIZE{(PARITY) ? H1 : H2},
			/// @brief Size of the second half.
			RIGHT_SIZE{(PARITY) ? H2 : H1}
		;

		/// @brief Type of the left side.
		typedef BinaryShuffle<LEFT_SIZE, NEW_MASK, LHS_PARITY>	LeftType;
		/// @brief Type of the right side.
		typedef BinaryShuffle<RIGHT_SIZE, NEW_MASK, RHS_PARITY>	RightType;

		constexpr SizeType isize() const {return Impl::shuffle<SizeType>(trueSize);}

		/// @brief Mangles a string.
		/// @tparam TArray String container type.
		/// @param dat String to decompose.
		template<class TArray>
		constexpr void decompose(TArray const& dat) {
			Array<uint8, LEFT_SIZE>		l {'\0'};
			Array<uint8, RIGHT_SIZE>	r {'\0'};
			for (usize i = 0; i < H2; ++i) {
				if constexpr (PARITY) {
					l[i] = dat[i+H2];
					r[i] = dat[i];
				} else {
					l[i] = dat[i];
					r[i] = dat[i+H2];
				}
			}
			if constexpr(H2 < H1) {
				if constexpr (PARITY)
					l[H1-1] = dat[SIZE-1];
				else
					r[H1-1] = dat[SIZE-1];
			}
			left	= LeftType(l);
			right	= RightType(r);
		}

		/// @brief Left side of the string.
		LeftType	left;
		/// @brief True size of the stored string.
		SizeType const trueSize;
		/// @brief Right side of the string.
		RightType	right;
	};

	/// @brief Prime number shuffle.
	/// @tparam S String size.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize S, usize MASK, bool PARITY, class TSize>
	struct PrimeShuffle;

	/// @brief Prime number shuffle.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize MASK, bool PARITY, class T>
	struct PrimeShuffle<0, MASK, PARITY, T> {
		/// @brief String size.
		constexpr static usize SIZE = 0;

		constexpr PrimeShuffle()			{}
		template<class TS>
		constexpr PrimeShuffle(TS const&)	{}

		String mangled() const		{return String();}
		String demangled() const	{return String();}

	private:
	};

	/// @brief Prime number shuffle.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize MASK, bool PARITY, class T>
	struct PrimeShuffle<1, MASK, PARITY, T> {
		/// @brief String size.
		constexpr static usize SIZE = 1;

		/// @brief Constructs the mangled string from a single character.
		/// @param dat Character to mangle.
		constexpr PrimeShuffle()							{				}
		/// @brief Constructs the mangled string from a single character.
		/// @param dat Character to mangle.
		constexpr PrimeShuffle(FixedCString<1> const& dat)	{c = dat[0];	}
		/// @brief Constructs the mangled string from a single byte.
		/// @param dat Byte to mangle.
		constexpr PrimeShuffle(Array<uint8, 1> const& dat)	{c = dat[0];	}

		/// @brief Returns the mangled string.
		/// @return Mangled string.
		String mangled() const		{return toString(c);}
		/// @brief Returns the demangled string.
		/// @return Demangled string.
		String demangled() const	{return toString(c);}

	private:
		char c;
	};

	/// @brief Prime number shuffle.
	/// @tparam S String size.
	/// @tparam M Shuffle mask.
	/// @tparam P Shuffle parity.
	template<usize S, usize M, bool P, class TSize = Decay::Number::AsUnsigned<S>>
	struct PrimeShuffle {
	private:
		using SizeType = TSize;
	public:
		constexpr static usize
		/// @brief String size.
			SIZE = S,
		/// @brief Shuffle mask.
			MASK = M
		;
		constexpr static bool PARITY = P;

		static_assert(SIZE > 1);
		static_assert(Impl::nearestPrime(SIZE, true) != SIZE);

		/// @brief Empty constructor.
		constexpr PrimeShuffle(): trueSize(Impl::shuffle<SizeType>(SIZE)) {}

		/// @brief Constructs a mangled string from a fixed string.
		/// @tparam CS String size.
		/// @param dat String to mangle.
		template<usize CS>
		constexpr PrimeShuffle(FixedCString<CS> const& dat): trueSize(Impl::shuffle<SizeType>(CS)) {
			static_assert(CS <= SIZE, "String must not be bigger than maximum size!");
			Array<uint8, SIZE> str {'\0'};
			for(usize i = 0; i < CS; ++i)
				str[i] = dat[i];
			decompose<Array<uint8, SIZE>>(str);
		}

		/// @brief Constructs a mangled string from a byte array.
		/// @tparam CS Array size.
		/// @param dat Bytes to mangle.
		template<usize CS>
		constexpr PrimeShuffle(Array<uint8, CS> const& dat): trueSize(Impl::shuffle<SizeType>(CS)) {
			static_assert(CS <= SIZE, "String must not be bigger than maximum size!");
			Array<uint8, SIZE> str {'\0'};
			for(usize i = 0; i < CS; ++i)
				str[i] = dat[i];
			decompose<Array<uint8, SIZE>>(str);
		}

		/// @brief Returns the mangled string.
		/// @return Mangled string.
		String mangled() const						{return (left.mangled() + right.mangled());						}

		/// @brief Returns the demangled string.
		/// @return Demangled string.
		String demangled() const requires (PARITY)	{return (right.demangled() + left.demangled()).resize(isize());	}
		/// @brief Returns the demangled string.
		/// @return Demangled string.
		String demangled() const requires (!PARITY)	{return (left.demangled() + right.demangled()).resize(isize());	}
	private:
		/// @brief Mask of the subsequent left & right sides.
		constexpr static usize NEW_MASK = MASK ^ (MASK >> 2);

		constexpr static bool
			/// @brief Parity of the left side.
			LHS_PARITY	= (MASK & 0b10) ? !PARITY : PARITY,
			/// @brief Parity of the right side.
			RHS_PARITY	= (MASK & 0b01) ? !PARITY : PARITY
		;

		constexpr static usize
			/// @brief Size of the first half.
			H1{Impl::nearestPrime(S, true)},
			/// @brief Size of the second half.
			H2{S - Impl::nearestPrime(S, true)}
		;

		constexpr static usize
			/// @brief Size of the left side.
			LEFT_SIZE{(PARITY) ? H1 : H2},
			/// @brief Size of the left side.
			RIGHT_SIZE{(PARITY) ? H2 : H1}
		;

		/// @brief Type of the left side.
		typedef PrimeShuffle<LEFT_SIZE, NEW_MASK, LHS_PARITY>	LeftType;
		/// @brief Type of the right side.
		typedef PrimeShuffle<RIGHT_SIZE, NEW_MASK, RHS_PARITY>	RightType;

		constexpr SizeType isize() const {return Impl::shuffle<SizeType>(trueSize);}

		/// @brief Mangles a string.
		/// @tparam TArray String container type.
		/// @param dat String to mangle.
		template<class TArray>
		constexpr void decompose(TArray const& dat) {
			Array<uint8, LEFT_SIZE>		l;
			Array<uint8, RIGHT_SIZE>	r;
			for (usize i = 0; i < LEFT_SIZE; ++i) {
				if constexpr (PARITY)	l[i] = dat[i+RIGHT_SIZE];
				else					l[i] = dat[i];
			}
			for (usize i = 0; i < RIGHT_SIZE; ++i) {
				if constexpr (PARITY)	r[i] = dat[i];
				else					r[i] = dat[i+LEFT_SIZE];
			}
			left	= LeftType(l);
			right	= RightType(r);
		}

		/// @brief Left side of the string.
		LeftType	left;
		/// @brief True size of the stored string.
		SizeType	trueSize;
		/// @brief Right side of the string.
		RightType	right;
	};
}

/// @brief Static string mangler.
/// @tparam S String size.
template<usize S>
using MangledStaticString = StaticStringMangler::PrimeShuffle<
	S,
	Impl::PrimeNumber<S>::CLOSEST,
	!Impl::PrimeNumber<S>::IS_PRIME,
	Decay::Number::AsUnsigned<S>
>;
//using MangledStaticString = StaticStringMangler::PrimeShuffle<S, 0b00010110, true>;
/*
using MangledStaticString = StaticStringMangler::BinaryShuffle<
	S,
	Impl::PrimeNumber<S>::CLOSEST,
	!Impl::PrimeNumber<S>::IS_PRIME
>;
*/
//using MangledStaticString =Static StringMangler::BinaryShuffle<S, 0b00010110, true>;

/// @brief Creates a mangled string.
/// @tparam S String size.
/// @param str String to mangle.
/// @return Mangled string.
template<usize S>
MangledStaticString<S> makeMangled(FixedCString<S> const& str) {
	return MangledString<S>(str);
}

/// @brief Static string obfuscator.
/// @tparam N String size.
/// @tparam TContainer<usize> String mangler.
template <usize N, template<usize> class TContainer = MangledStaticString>
struct ObfuscatedStaticString: IObfuscator<String> {
private:
	using SizeType = Decay::Number::AsUnsigned<N>;
public:
	/// @brief String size.
	constexpr static usize SIZE = N+1;

	/// @brief Constructs an obfuscated string from a fixed string.
	/// @tparam CS String size.
	/// @param str String to mangle.
	template<usize CS>
	constexpr ObfuscatedStaticString(FixedCString<CS> const& str):
		trueSize(Impl::shuffle<SizeType>(CS)),
		data(decompose(str))
	{}

	/// @brief Destructor.
	constexpr virtual ~ObfuscatedStaticString() {}

	/// @brief Deobfuscates the string.
	/// @return Deobfuscated string.
	String deobfuscated() const final {
		String result;
		uint8 off = 0;
		String const dd = data.demangled();
		for(uint8 c : dd)
			result.pushBack(off += c);
		return result.resize(isize());
	}

private:
	typedef TContainer<SIZE> ContainerType;

	/// @brief True size of the stored string.
	SizeType trueSize;

	/// @brief Underlying mangled string.
	ContainerType data;

	constexpr SizeType isize() const {return Impl::shuffle<SizeType>(trueSize);}

	/// @brief Obfuscates a string.
	/// @tparam TArray String container type.
	/// @param str String to obfuscate.
	template<usize CS>
	constexpr static ContainerType decompose(FixedCString<CS> const& str) {
		static_assert(CS <= SIZE, "String must not be bigger than maximum size!");
		Array<uint8, CS> result {'\0'};
		uint8 off = 0;
		for (usize i = 0; i < CS; ++i) {
			result[i] = (str[i] - off);
			off = str[i];
		}
		return ContainerType(result);
	}
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_CONTAINER_OBFUSCATOR_H
