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
		struct Generator {
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
	}

	template <Type::Matches<Impl::IsIdentifier> T>
	using Generator = Base::Generator<T::SIZE>;
}

CTL_NAMESPACE_END

#endif