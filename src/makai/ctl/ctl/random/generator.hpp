#ifndef CTL_RANDOM_GENERATOR_H
#define CTL_RANDOM_GENERATOR_H

#include "../math/math.hpp"
#include "../typetraits/traits.hpp"
#include "engine.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Random number generation.
namespace Random {

/// @brief Base classes.
namespace Base {
	template<class T>
	struct Generator;

	/// @brief Pseudo-random number generator.
	/// @tparam TEngine Random number engine type.
	template <Type::Subclass<ISimpleEngine> TEngine>
	struct Generator<TEngine> {
		/// @brief Random number engine type.
		using EngineType = TEngine;

		/// @brief Whether the random number generator is ecure.
		constexpr static bool SECURE = false;

		/// @brief Default constructor.
		constexpr Generator(): engine()					{						}
		/// @brief Constructs the generator from a seed.
		/// @param seed Seed to use.
		constexpr Generator(usize const seed)			{engine.setSeed(seed);	}
		/// @brief Copy constructor.
		/// @param other `Generator` to copy from.
		constexpr Generator(Generator const& other)		= delete;
		/// @brief Move constructor.
		/// @param other `Generator` to move.
		constexpr Generator(Generator&& other)			= default;

		/// @brief Copy assignment operator (deleted).
		Generator& operator=(Generator const& other)	= delete;
		/// @brief Move constructor.
		/// @param other `Generator` to move.
		Generator& operator=(Generator&& other)			= default;

		/// @brief Returns a random floating point number between 0 and 1.
		/// @tparam T Floating point type.
		/// @return Random number.
		template<Type::Real T = float>
		constexpr T real() {
			return static_cast<T>(engine.next()) / static_cast<T>(NumberLimit<usize>::HIGHEST);
		}

		/// @brief Returns a random integer between lowest and highest value the integer can hold.
		/// @tparam T Integer type.
		/// @return Random number.
		template<Type::Integer T = ssize>
		constexpr T integer() {
			return engine.next();
		}

		/// @brief Sets the random number generator's seed.
		/// @param seed Seed to use.
		constexpr void setSeed(usize const seed) {
			engine.setSeed(seed);
		}

		/// @brief Gets the random number generator's current seed.
		/// @return Current seed.
		constexpr usize getSeed() const {
			return engine.getSeed();
		}

		/// @brief Gets a seed based on the current clock's time, and a random number.
		/// @return New seed.
		constexpr usize generateNewSeed() {
			return OS::Time::now() ^ !(integer() << 0x2F);
		}
		
	protected:
		/// @brief Underlying engine used.
		EngineType engine;
	};

	/// @brief Cryptographically secure random number generator.
	/// @tparam TEngine Random number engine type.
	template <Type::Subclass<ISecureEngine> TEngine>
	struct Generator<TEngine> {
		/// @brief Random number engine type.
		using EngineType = TEngine;

		/// @brief Whether the random number generator is ecure.
		constexpr static bool SECURE = true;

		/// @brief Default constructor.
		Generator(): engine()							{}
		Generator(usize const): Generator()				{}

		/// @brief Copy constructor (deleted).
		Generator(Generator const& other)				= delete;
		/// @brief Move constructor.
		/// @param other `Generator` to move.
		Generator(Generator&& other)					= default;

		/// @brief Copy assignment operator (deleted).
		Generator& operator=(Generator const& other)	= delete;
		/// @brief Move constructor.
		/// @param other `Generator` to move.
		Generator& operator=(Generator&& other)			= default;

		/// @brief Returns a random floating point number between 0 and 1.
		/// @tparam T Floating point type.
		/// @return Random number.
		template<Type::Real T = float>
		T real() {
			return static_cast<T>(engine.next()) / static_cast<T>(NumberLimit<usize>::HIGHEST);
		}

		/// @brief Returns a random integer between lowest and highest value the integer can hold.
		/// @tparam T Integer type.
		/// @return Random number.
		template<Type::Integer T = ssize>
		T integer() {
			return engine.next();
		}
		
	protected:
		/// @brief Underlying engine used.
		EngineType engine;
	};
}

template <class TEngine>
struct BaseGenerator: Base::Generator<TEngine> {
	using
		Base::Generator<TEngine>::Generator,
		Base::Generator<TEngine>::real,
		Base::Generator<TEngine>::integer
	;

	/// @brief Returns a random floating point number between the given range.
	/// @tparam T Floating point type.
	/// @param min Lowest value.
	/// @param max Highest value.
	/// @return Random number.
	template<Type::Real T = float>
	constexpr T real(T const& min, T const& max) {
		if (min > max) return real<T>(max, min);
		return real<T>() * (max - min) + min;
	}

	/// @brief Returns a random integer between the given range.
	/// @tparam T Integer type.
	/// @param min Lowest value.
	/// @param max Highest value.
	/// @return Random number.
	template<Type::Integer T = ssize>
	constexpr T integer(T const& min, T const& max) {
		if (min > max) return integer<T>(max, min);
		return (integer<AsUnsigned<T>>() % (max - min + 1)) + min;
	}

	/// @brief Returns a random integer between lowest and highest value the integer can hold.
	/// @tparam T Integer type.
	/// @return Random number.
	template<Type::Integer T = ssize>
	constexpr T number()							{return integer<T>();			}

	/// @brief Returns a random floating point number between 0 and 1.
	/// @tparam T Floating point type.
	/// @return Random number.
	template<Type::Real T = float>
	constexpr T number() 							{return real<T>();				}

	/// @brief Returns a random integer between the given range.
	/// @tparam T Integer type.
	/// @param min Lowest value.
	/// @param max Highest value.
	/// @return Random number.
	template<Type::Integer T = ssize>
	constexpr T number(T const& min, T const& max)	{return integer<T>(min, max);	}

	/// @brief Returns a random floating point number between the given range.
	/// @tparam T Floating point type.
	/// @param min Lowest value.
	/// @param max Highest value.
	/// @return Random number.
	template<Type::Real T = float>
	constexpr T number(T const& min, T const& max)	{return real<T>(min, max);		}

protected:
	using Base::Generator<TEngine>::engine;
};

/// @brief Pseudo-random number generator.
using SimpleGenerator = BaseGenerator<Engine::Mersenne>;
/// @brief Cryptographically secure random number generator.
using SecureGenerator = BaseGenerator<Engine::Secure>;

/// @brief Basic random number generator.
using Generator = SimpleGenerator;

}

CTL_NAMESPACE_END

#endif