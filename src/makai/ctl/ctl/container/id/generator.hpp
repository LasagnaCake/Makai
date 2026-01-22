#ifndef CTL_CONTAINER_ID_GENERATOR_H
#define CTL_CONTAINER_ID_GENERATOR_H

#include "ssuid.hpp"
#include "../../random/random.hpp"

CTL_NAMESPACE_BEGIN

namespace ID {
	namespace Impl {
		template <class T>
		struct IsIdentifier: BooleanConstant<Type::Equal<T, SSUID<T::SIZE>>> {};
	}

	namespace Base {
		template <usize S, Type::Derived<Random::Base::IEngine> TEngine = Random::Engine::Secure>
		struct RandomGenerator {
			using IdentifierType = SSUID<S>;
			using RNGType = Random::BaseGenerator<TEngine>;

			constexpr IdentifierType id() {
				typename IdentifierType::InternalType arr;
				for (usize i = 0; i < IdentifierType::SIZE; ++i)
					arr[i] = rng.template number<uint64>();
				return IdentifierType::create(arr);
			}

		private:
			RNGType rng{Random::CTPRNG<usize>};
		};

		template <usize S>
		struct SequenceGenerator {
			using IdentifierType = SSUID<S>;

			constexpr IdentifierType id() {return current++;}

		private:
			IdentifierType current = IdentifierType::create(0);
		};

		template <usize S, Type::Derived<Random::Base::IEngine> TEngine = Random::Engine::Secure>
		struct TimestampGenerator {
			using IdentifierType = SSUID<S>;
			using RNGType = Random::BaseGenerator<TEngine>;

			constexpr IdentifierType id() {
				constexpr auto const start = IdentifierType::SIZE > 1 ? 2 : 1;
				typename IdentifierType::InternalType arr;
				arr[0] = OS::Time::Clock::sinceEpoch<OS::Time::Seconds>();
				if constexpr (IdentifierType::SIZE > 1)
					arr[1] = OS::Time::Clock::sinceEpoch<OS::Time::Nanos>();
				for (usize i = start; i < IdentifierType::SIZE; ++i)
					arr[i] = rng.template number<uint64>();
				return IdentifierType::create(arr);
			}

		private:
			RNGType rng{Random::CTPRNG<usize>};
		};
	}

	template <Type::Matches<Impl::IsIdentifier> T, Type::Derived<Random::Base::IEngine> TEngine = Random::Engine::Secure>
	using RandomGenerator		= Base::RandomGenerator<T::SIZE, TEngine>;

	template <Type::Matches<Impl::IsIdentifier> T>
	using SequenceGenerator		= Base::SequenceGenerator<T::SIZE>;

	template <Type::Matches<Impl::IsIdentifier> T, Type::Derived<Random::Base::IEngine> TEngine = Random::Engine::Secure>
	using TimestampGenerator	= Base::TimestampGenerator<T::SIZE, TEngine>;
}

CTL_NAMESPACE_END

#endif