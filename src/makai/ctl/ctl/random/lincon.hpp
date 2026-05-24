#ifndef CTL_RANDOM_LINCON_H
#define CTL_RANDOM_LINCON_H

#include "../typetraits/traits.hpp"
#include "../namespace.hpp"
#include "../typeinfo.hpp"

CTL_NAMESPACE_BEGIN

// Thanks, Wikipedia!

/// @brief Random number engine implelementations.
namespace Random::Engine::Impl {
	/// @brief Base linear congruential implementation.
	template<Type::Integer I, usize M = 6364136223846793005, usize A = 1442695040888963407>
	struct BaseLinCon {
	private:
		constexpr static I const MUL = M;
		constexpr static I const ADD = A;

		/// @brief The engine's current seed.
		usize state;
	public:
		/// @brief Constructs the engine with a given seed.
		/// @param seed Seed to use.
		constexpr BaseLinCon(I const& seed) {setSeed(seed);}

		/// @brief Copy constructor.
		/// @param other `BaseMersenne` engine to copy from.
		constexpr BaseLinCon(BaseLinCon const& other)				= default;
		/// @brief Move constructor.
		/// @param other `BaseMersenne` engine to move.
		constexpr BaseLinCon(BaseLinCon&& other)					= default;

		/// @brief Copy assignment operator.
		/// @param other `BaseMersenne` engine to copy from.
		constexpr BaseLinCon& operator=(BaseLinCon const& other)	= default;
		/// @brief Move assignment operator.
		/// @param other `BaseMersenne` engine to move.
		constexpr BaseLinCon& operator=(BaseLinCon&& other)			= default;

		/// @brief Sets the engine's current seed.
		/// @param seed Seed to use.
		/// @return Reference to self.
		constexpr BaseLinCon& setSeed(I const seed) {
			state = seed + ADD;
			next();
			return *this;
		}

		/// @brief Returns the engine's current seed.
		/// @return Current seed.
		constexpr I getSeed() const {return state;}

		/// @brief Generates a new random number.
		/// @return Generated number.
		constexpr I next() {
			return state = state * MUL + ADD;
		}
	};

	/// @brief 64-bit linear congruential implementation.
	using LinCon64 = BaseLinCon<uint64, 6364136223846793005, 1442695040888963407>;
	/// @brief 32-bit linear congruential implementation.
	using LinCon32 = BaseLinCon<uint32, 6364136223846793005, 1442695040888963407>;

	/// @brief Decays to an appropriate engine type, depending on the CPU architecture.
	using LinCon = Meta::If<sizeof(usize) == sizeof(uint64), LinCon64, LinCon32>;
}

CTL_NAMESPACE_END

#endif
