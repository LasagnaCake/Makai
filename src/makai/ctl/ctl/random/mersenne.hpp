#ifndef CTL_RANDOM_MERSENNE_H
#define CTL_RANDOM_MERSENNE_H

#include "../math/math.hpp"
#include "../typetraits/traits.hpp"
#include "../namespace.hpp"
#include "../typeinfo.hpp"

CTL_NAMESPACE_BEGIN

// Thanks, Wikipedia!

/// @brief Random number engine implelementations.
namespace Random::Engine::Impl {
	/// @brief Base mersenne twister implementation.
	/// @tparam I Integer type. Must be either `uint32` or `uint64`.
	template<
		Type::Integer I,
		usize W,
		usize N,
		usize M,
		usize R,
		I A,
		usize U,
		I D,
		I S,
		I B,
		usize T,
		I C, 
		usize L,
		I F
	>
	struct BaseMersenne {
	private:
		using DataType = I;

		/// @brief The engine's current state.
		struct State {
			Decay::AsType<DataType[N]>	array;
			usize						index;
		} state;

		/// @brief The engine's current seed.
		DataType seed;

		constexpr static DataType UMASK = (NumberLimit<DataType>::HIGHEST << R);
		constexpr static DataType LMASK = (NumberLimit<DataType>::HIGHEST >> (W-R));
	public:
		/// @brief Constructs the engine with a given seed.
		/// @param seed Seed to use.
		constexpr BaseMersenne(DataType const& seed) {setSeed(seed);}

		/// @brief Copy constructor.
		/// @param other `BaseMersenne` engine to copy from.
		BaseMersenne(BaseMersenne const& other)				= default;
		/// @brief Move constructor.
		/// @param other `BaseMersenne` engine to move.
		BaseMersenne(BaseMersenne&& other)					= default;

		/// @brief Copy assignment operator.
		/// @param other `BaseMersenne` engine to copy from.
		BaseMersenne& operator=(BaseMersenne const& other)	= default;
		/// @brief Move assignment operator.
		/// @param other `BaseMersenne` engine to move.
		BaseMersenne& operator=(BaseMersenne&& other)		= default;

		/// @brief Sets the engine's current seed.
		/// @param seed Seed to use.
		/// @return Reference to self.
		constexpr BaseMersenne& setSeed(DataType seed) {
			state.array[0] = seed;
			for (usize i = 1; i < N; ++i) {
				seed = F * (seed ^ (seed >> (W-2))) + i;
				state.array[i] = seed; 
			}
			state.index = 0;
			this->seed = seed;
			return *this;
		}

		/// @brief Returns the engine's current seed.
		/// @return Current seed.
		constexpr DataType getSeed() const {return seed;}

		/// @brief Generates a new random number.
		/// @return Generated number.
		constexpr DataType next() {
			usize k = state.index;
			ssize j = k - (N-1);
			if (j < 0) j += N;
			DataType x = (state.array[k] & UMASK) | (state.array[j] & LMASK);
			DataType xA = x >> 1;
			if (x & 0x1ULL) xA ^= A;
			j = k - (N-M);
			if (j < 0) j += N;
			x = state.array[j] ^ xA;
			state.array[k++] = x;
			if (k >= N) k = 0;
			state.index = k;
			DataType y = x ^ (x >> U);
			y ^= ((y << S) & B);
			y ^= ((y << T) & C);
			return y ^ (y >> L);
		}
	};

	/// @brief 32-bit mersenne twister engine.
	using Mersenne32 = BaseMersenne<
		uint32,
		32, 624, 397, 31,
		0x9908b0df, 11,
		0xffffffff, 7,
		0x9d2c5680, 15,
		0xefc60000, 18, 1812433253
	>;

	/// @brief 64-bit mersenne twister engine.
	using Mersenne64 = BaseMersenne<
		uint64,
		64, 312, 156, 31,
		0xb5026f5aa96619e9, 29,
		0x5555555555555555, 17,
		0x71d67fffeda60000, 37,
		0xfff7eee000000000, 43,
		6364136223846793005
	>;
	
	/// @brief Decays to an appropriate engine type, depending on the CPU architecture.
	using Mersenne = Meta::DualType<sizeof(usize) == sizeof(uint64), Impl::Mersenne64, Impl::Mersenne32>;
}

CTL_NAMESPACE_END

#endif