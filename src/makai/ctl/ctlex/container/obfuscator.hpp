#ifndef CTL_EX_CONTAINER_OBFUSCATOR_H
#define CTL_EX_CONTAINER_OBFUSCATOR_H

#include "../../ctl/ctl.hpp"
#include "../../ctl/exnamespace.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Decays to a "C-style" fixed array.
/// @tparam T Element type.
/// @tparam S Array size.
template<class T, usize S>
using CArray = As<T[S]>;

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
	constexpr bool isPrime(usize const v) {
		if (
			v == 0
		||	v % 2 == 0
		||	v % 3 == 0
		||	v % 5 == 0
		||	v % 7 == 0
		||	v % 11 == 0
		) return false; 
		for (usize i = 13; i < v/2; ++i)
			if (v % i == 0)
				return false;
		return true;
	}

	/// @brief Returns the nearest prime to a number, that is less than or equal to it.
	/// @param v Number to get nearest prime.
	/// @param excludeSelf Whether to exclude the number itself, if it is prime.
	/// @return Nearest prime.
	constexpr usize nearestPrime(usize const v, bool excludeSelf = false) {
		if (!v) return 0;
		if (v < 2) return excludeSelf ? v-1 : v;
		for (usize i = (excludeSelf ? (v-1) : v); i > 0; --i)
			if (isPrime(i))
				return i;
		return 0;
	}
	
	/// @brief Compile-time pseudo-random number.
	constexpr usize PRNG = (
		ConstHasher::hash(__DATE__)
	+	ConstHasher::hash(__TIME__)
	+	(__INCLUDE_LEVEL__)
	);

	constexpr char filler(usize const offset = 0, usize const min = PRNG % 32, usize const max = 64 + PRNG % 31) {
		return static_cast<char>(((Impl::PRNG + offset) % (max - min)) + min);
	}

	static_assert(nearestPrime(128, true) == 127);
	static_assert(nearestPrime(127, true) == 113);
	static_assert(nearestPrime(113, true) == 109);

	static_assert(filler() >= 32);

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
	template<usize S, usize MASK, bool PARITY, auto NEWSIZE, class TSize>
	struct FunctionShuffle;

	template<usize MASK, bool PARITY, auto NEWSIZE, class TSize>
	struct FunctionShuffle<0, MASK, PARITY, NEWSIZE, TSize> {
		/// @brief String size.
		constexpr static usize SIZE = 0;

		constexpr FunctionShuffle()			{}
		template<class T>
		constexpr FunctionShuffle(T const&)	{}

		String mangled() const		{return String();}
		String demangled() const	{return String();}
	};

	template<usize MASK, bool PARITY, auto NEWSIZE, class TSize>
	struct FunctionShuffle<1, MASK, PARITY, NEWSIZE, TSize> {
		/// @brief String size.
		constexpr static usize SIZE = 1;

		/// @brief Default constructor.
		constexpr FunctionShuffle()								{				}
		/// @brief Constructs the mangled string from a single character.
		/// @param dat Character to mangle.
		constexpr FunctionShuffle(FixedCString<1> const& dat)	{c = dat[0];	}
		/// @brief Constructs the mangled string from a single byte.
		/// @param dat Byte to mangle.
		constexpr FunctionShuffle(Array<uint8, 1> const& dat)	{c = dat[0];	}

		/// @brief Returns the mangled string.
		/// @return Mangled string.
		String mangled() const		{return toString(c);}
		/// @brief Returns the demangled string.
		/// @return Demangled string.
		String demangled() const	{return toString(c);}

	private:
		/// @brief String character.
		char c = Impl::filler(MASK);
	};

	template<usize S, usize M, bool P, auto NEWSIZE, class TSize = CTL::Decay::Number::AsUnsigned<S>>
	struct FunctionShuffle {
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

		static_assert(SIZE > 1);
		static_assert(NEWSIZE(SIZE, true) != SIZE);
		static_assert(NEWSIZE(SIZE, false) != SIZE);
		static_assert(Type::Functional<decltype(NEWSIZE), usize(usize, bool)>, "NEWSIZE must be a valid size function!");

		/// @brief Default constructor.
		constexpr FunctionShuffle(): trueSize(Impl::shuffle<SizeType>(SIZE)) {}

		/// @brief Constructs a mangled string from a fixed string.
		/// @tparam CS String size.
		/// @param dat String to mangle.
		template<usize CS>
		constexpr FunctionShuffle(FixedCString<CS> const& dat): trueSize(Impl::shuffle<SizeType>(CS)) {
			static_assert(CS <= SIZE, "String must not be bigger than maximum size!");
			Array<uint8, SIZE> str {Impl::filler(CS)};
			for(usize i = 0; i < CS; ++i)
				str[i] = dat[i];
			usize off = 0;
			for (usize i = CS; i < SIZE; ++i) {
				str[i] = Impl::filler(CS + SIZE + off);
				off += CS + SIZE + str[i];
			}
			decompose<Array<uint8, SIZE>>(str);
		}

		/// @brief Constructs a mangled string from a byte array.
		/// @tparam CS Array size.
		/// @param dat Bytes to mangle.
		template<usize CS>
		constexpr FunctionShuffle(Array<uint8, CS> const& dat): trueSize(Impl::shuffle<SizeType>(CS)) {
			static_assert(CS <= SIZE, "String must not be bigger than maximum size!");
			Array<uint8, SIZE> str {Impl::filler(CS)};
			for(usize i = 0; i < CS; ++i)
				str[i] = dat[i];
			usize off = 0;
			for (usize i = CS; i < SIZE; ++i) {
				str[i] = Impl::filler(CS + SIZE + off);
				off += CS + SIZE + str[i];
			}
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
			H1{NEWSIZE(SIZE, true)},
			/// @brief Size of the second half.
			H2{NEWSIZE(SIZE, false)}
		;

		constexpr static usize
			/// @brief Size of the first half.
			LEFT_SIZE{(PARITY) ? H1 : H2},
			/// @brief Size of the second half.
			RIGHT_SIZE{(PARITY) ? H2 : H1}
		;

		/// @brief Type of the left side.
		typedef FunctionShuffle<LEFT_SIZE, NEW_MASK, LHS_PARITY, NEWSIZE>	LeftType;
		/// @brief Type of the right side.
		typedef FunctionShuffle<RIGHT_SIZE, NEW_MASK, RHS_PARITY, NEWSIZE>	RightType;

		constexpr SizeType isize() const {return Impl::shuffle<SizeType>(trueSize);}

		/// @brief Mangles a string.
		/// @tparam TArray String container type.
		/// @param dat String to decompose.
		template<class TArray>
		constexpr void decompose(TArray const& dat) {
			Array<uint8, LEFT_SIZE>		l{Impl::filler(RIGHT_SIZE + SIZE)};
			Array<uint8, RIGHT_SIZE>	r{Impl::filler(LEFT_SIZE + SIZE)};
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

	namespace Shuffles {
		constexpr usize binary(usize const sz, bool const firstHalf) {
			if (firstHalf && (sz%2)) return (sz/2)+1;
			return sz/2;
		}

		constexpr usize prime(usize const sz, bool const firstHalf) {
			if (firstHalf) return Impl::nearestPrime(sz, true);
			return sz - Impl::nearestPrime(sz, true);
		}

		constexpr usize prng(usize const sz, bool const firstHalf) {
			constexpr usize SEED = Impl::PRNG;
			if (sz % 2) return prime(sz, firstHalf);
			if (Impl::isPrime(sz) && sz > 16) return binary(sz, firstHalf);
			usize const rng = (SEED % sz) ? (SEED % sz) : (SEED % sz + 1);
			if (firstHalf) return sz - rng;
			return rng;
		}
	}
	
	/// @brief Binary shuffle.
	/// @tparam S String size.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize S, usize MASK, bool PARITY, class TSize>
	using BinaryShuffle = FunctionShuffle<S, MASK, PARITY, Shuffles::binary, TSize>;
	
	/// @brief Prime shuffle.
	/// @tparam S String size.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize S, usize MASK, bool PARITY, class TSize>
	using PrimeShuffle = FunctionShuffle<S, MASK, PARITY, Shuffles::prime, TSize>;

	/// @brief Pseudo-random shuffle.
	/// @tparam S String size.
	/// @tparam MASK Shuffle mask.
	/// @tparam PARITY Shuffle parity.
	template<usize S, usize MASK, bool PARITY, class TSize>
	using PseudoRandomShuffle = FunctionShuffle<S, MASK, PARITY, Shuffles::prng, TSize>;
}

/// @brief Static string mangler.
/// @tparam S String size.
template<usize S>
using MangledStaticString = StaticStringMangler::PseudoRandomShuffle<
	S,
	Impl::PRNG * Impl::PrimeNumber<S>::CLOSEST,
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
		trueSize(Impl::shuffle<SizeType>(CS-1)),
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
		Array<uint8, SIZE> result {Impl::filler(CS + SIZE)};
		uint8 off = 0;
		for (usize i = 0; i < CS; ++i) {
			result[i] = (str[i] - off);
			off = str[i];
		}
		for (usize i = CS; i < SIZE; ++i) {
			result[i] = Impl::filler(CS + SIZE + off);
			off += CS + SIZE + result[i];
		}
		return ContainerType(result);
	}
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_CONTAINER_OBFUSCATOR_H
