#ifndef CTL_RANDOM_ENGINE_H
#define CTL_RANDOM_ENGINE_H

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
#include <windows.h>
//#include <bcrypt.h>
#else
#endif

#include "../math/math.hpp"
#include "../os/time.hpp"
#include "../typeinfo.hpp"
#include "../meta/logic.hpp"
#include "mersenne.hpp"

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
	struct SimpleEngine: IEngine {
		constexpr static bool SECURE = false;
		/// @brief Generates a new random number.
		/// @return Generated number.
		virtual usize next() = 0;
		/// @brief Returns the engine's current seed.
		/// @return Current seed.
		virtual usize getSeed() = 0;
		/// @brief Sets the engine's current seed.
		/// @param seed Current seed.
		virtual void setSeed(usize const seed) = 0;
		/// @brief Virtual destructor.
		virtual ~SimpleEngine() {}
	};

	/// @brief Cryptographically secure random number engine interface.
	struct SecureEngine: IEngine {
		constexpr static bool SECURE = true;
		/// @brief Generates a new random number.
		/// @return Generated number.
		virtual usize next() = 0;
		/// @brief Virtual destructor.
		virtual ~SecureEngine() {}
	};
}

/// @brief Random number engine implementations.
namespace Engine {
	/// @brief Mersenne twister engine.
	struct Mersenne: Base::SimpleEngine {
	private:
		using InternalEngine = Impl::Mersenne;

		/// @brief Underlying engine used.
		InternalEngine engine;
	public:
		/// @brief Constructs the engine with a given seed.
		/// @param seed Seed to use.
		Mersenne(usize const seed):	engine(seed)				{}
		/// @brief Constructs the engine with the seed being the current time.
		Mersenne():					Mersenne(OS::Time::now())	{}

		/// @brief Copy constructor.
		/// @param other `Mersenne` engine to copy from.
		Mersenne(Mersenne const& other)				= default;
		/// @brief Move constructor.
		/// @param other `Mersenne` engine to move.
		Mersenne(Mersenne&& other)					= default;

		/// @brief Copy assignment operator.
		/// @param other `Mersenne` engine to copy from.
		Mersenne& operator=(Mersenne const& other)	= default;
		/// @brief Move assignment operator.
		/// @param other `Mersenne` engine to move.
		Mersenne& operator=(Mersenne&& other)		= default;

		/// @brief Destructor.
		virtual ~Mersenne() {}

		/// @brief 
		/// @return 
		virtual usize getSeed() final {
			return engine.getSeed();
		}

		/// @brief Sets the engine's current seed.
		/// @param seed Seed to use.
		/// @return Reference to self.
		virtual void setSeed(usize const newSeed) final {
			engine.setSeed(newSeed);
		}

		/// @brief Generates a new random number.
		/// @return Generated number.
		virtual usize next() final {
			return engine.next();
		}
	};

	#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
	/// @brief Secure random engine.
	struct Secure: Base::SecureEngine {
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
		/// @brief Empty constructor.
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
	struct Secure: Base::SecureEngine {
		/// @brief Generates a new random number.
		/// @return Generated number.
		/// @warning Currently unimplemented! Returns zero.
		virtual usize next() final {return 0;}
	};
	#endif
}

}

CTL_NAMESPACE_END

#endif // CTL_RANDOM_ENGINE_H
