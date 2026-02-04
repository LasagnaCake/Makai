#ifndef CTL_RANDOM_ENGINE_H
#define CTL_RANDOM_ENGINE_H

#include "../os/time.hpp"
#include "../typeinfo.hpp"
#include "../meta/logic.hpp"
#include "../typetraits/verify.hpp"
#include "ctprng.hpp"
#include "mersenne.hpp"

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <windows.h>
//#include <bcrypt.h>
#else
#endif

CTL_NAMESPACE_BEGIN

/// @brief Random number generation.
namespace Random {

/// @brief Base classes for other classes.
namespace Base {
	/// @brief Random number engine interface.
	struct IEngine {
		/// @brief Generates a new random number.
		/// @return Generated number.
		virtual usize next() = 0;
		/// @brief Virtual destructor.
		virtual ~IEngine() {}
	};

	/// @brief Pseudo-random number engine interface.
	struct ISimpleEngine: IEngine {
		constexpr static bool SECURE = false;
		/// @brief Generates a new random number.
		/// @return Generated number.
		constexpr virtual usize next() = 0;
		/// @brief Returns the engine's current seed.
		/// @return Current seed.
		constexpr virtual usize getSeed() = 0;
		/// @brief Sets the engine's current seed.
		/// @param seed Current seed.
		constexpr virtual void setSeed(usize const seed) = 0;
		/// @brief Virtual destructor.
		virtual ~ISimpleEngine() {}
	};

	/// @brief Cryptographically secure random number engine interface.
	struct ISecureEngine: IEngine {
		constexpr static bool SECURE = true;
		/// @brief Generates a new random number.
		/// @return Generated number.
		virtual usize next() = 0;
		/// @brief Virtual destructor.
		virtual ~ISecureEngine() {}
	};
}

/// @brief Random number engine implementations.
namespace Engine {
	/// @brief Mersenne twister engine.
	struct Mersenne: Base::ISimpleEngine {
	private:
		using InternalEngine = Impl::Mersenne;

		/// @brief Underlying engine used.
		InternalEngine engine;

		constexpr static usize startingSeed() {
			if (inCompileTime())	return ctsprng<usize>();
			else					return OS::Time::now();
		}
	public:
		/// @brief Constructs the engine with a given seed.
		/// @param seed Seed to use.
		constexpr Mersenne(usize const seed):	engine(seed)				{}
		/// @brief Constructs the engine with the seed being the current time.
		constexpr Mersenne():					Mersenne(startingSeed())	{}

		/// @brief Move constructor (defaulted).
		constexpr Mersenne(Mersenne&& other)		= default;
		/// @brief Copy constructor (deleted).
		constexpr Mersenne(Mersenne const& other)	= delete;

		/// @brief Move assignment operator (defaulted).
		#ifdef __clang__
		constexpr Mersenne& operator=(Mersenne&& other)			{engine = CTL::move(other.engine); return *this;}
		#else
		constexpr Mersenne& operator=(Mersenne&& other)			= default;
		#endif
		/// @brief Copy assignment operator (deleted).
		constexpr Mersenne& operator=(Mersenne const& other)	= delete;

		/// @brief Destructor.
		virtual ~Mersenne() {}

		/// @brief
		/// @return
		constexpr virtual usize getSeed() final {
			return engine.getSeed();
		}

		/// @brief Sets the engine's current seed.
		/// @param seed Seed to use.
		/// @return Reference to self.
		constexpr virtual void setSeed(usize const newSeed) final {
			engine.setSeed(newSeed);
		}

		/// @brief Generates a new random number.
		/// @return Generated number.
		constexpr virtual usize next() final {
			return engine.next();
		}
	};

	#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
	/// @brief Secure random engine.
	struct Secure: Base::ISecureEngine {
	private:
		/// @brief Random number algorithm wrapper.
		struct Algorithm {
		private:
			/// @brief Windows handle to algorithm.
			BCRYPT_ALG_HANDLE algHandle;
		public:
			/// @brief Constructs the algorithm.
			Algorithm() {
				BCryptOpenAlgorithmProvider(&algHandle, L"RNG", NULL, 0);
			}

			/// @brief Move constructor.
			/// @param other `Algorithm` to move.
			Algorithm(Algorithm&& other)					= default;
			/// @brief Copy constructor (deleted).
			Algorithm(Algorithm const& other)				= delete;

			/// @brief Move constructor.
			/// @param other `Algorithm` to move.
			Algorithm& operator=(Algorithm&& other)			= default;
			/// @brief Copy constructor (deleted).
			Algorithm& operator=(Algorithm const& other)	= delete;

			/// @brief Returns the underlying windows handle.
			operator BCRYPT_ALG_HANDLE() const {
				return algHandle;
			}

			/// @brief Destructor.
			~Algorithm() {
				BCryptCloseAlgorithmProvider(&algHandle, 0);
			}
		};

		/// @brief Random number algorithm to use.
		Algorithm algo;

	public:
		/// @brief Default constructor.
		Secure(): algo()						{}

		/// @brief Move constructor.
		/// @param other `Secure` engine to move.
		Secure(Secure&& other)					= default;
		/// @brief Copy constructor (deleted).
		Secure(Secure const& other)				= delete;

		/// @brief Move constructor.
		/// @param other `Secure` engine to move.
		Secure& operator=(Secure&& other)		= default;
		/// @brief Copy constructor (deleted).
		Secure& operator=(Secure const& other)	= delete;

		/// @brief Destructor.
		virtual ~Secure() {}

		/// @brief Generates a new random number.
		/// @return Generated number.
		virtual usize next() final {
			usize num = 0;
			BCryptGenRandom(
				algo,
				(byte*)&num,
				sizeof(usize),
				0
			);
			return num;
		}
	};
	#else
	/// @brief Secure random engine.
	/// @warning Currently unimplemented!
	struct Secure: Base::ISecureEngine {
		/// @brief Default constructor.
		Secure() {}

		/// @brief Move constructor.
		/// @param other `Secure` engine to move.
		Secure(Secure&& other)					= default;
		/// @brief Copy constructor (deleted).
		Secure(Secure const& other)				= delete;

		/// @brief Move constructor.
		/// @param other `Secure` engine to move.
		Secure& operator=(Secure&& other)		= default;
		/// @brief Copy constructor (deleted).
		Secure& operator=(Secure const& other)	= delete;

		/// @brief Destructor.
		virtual ~Secure() {}
		/// @brief Generates a new random number.
		/// @return Generated number.
		virtual usize next() final {return system("/dev/urandom");}
	};
	#endif
}

}

CTL_NAMESPACE_END

#endif // CTL_RANDOM_ENGINE_H
