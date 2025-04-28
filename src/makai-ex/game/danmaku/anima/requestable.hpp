#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_REQUESTABLE_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_REQUESTABLE_H

#include <makai/makai.hpp>

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	template<class... Args>
	struct IRequestable {
		virtual ~IRequestable() {}

		using Parameters = OrderedMap<usize, StringList>;
		using Parameter = typename Parameters::PairType;

		virtual bool onRequest(Parameters const& params, Args...) = 0;
	};

	template<class... Args>
	struct ANamedRequestable;

	template<class... Args>
	using APeriodicRequest = CTL::Ex::APeriodic<ANamedRequestable, usize const, IRequestable::Parameters const&, Args...>;

	template<class... Args>
	struct ANamedRequestable: IRequestable<Args...>, APeriodicRequest<Args...> {
		usize const id;

		ANamedRequestable(usize const id): id(id) {}

		void onUpdate(usize const message, Parameters const& params, Args... args) override final {
			if (message == id)
				onRequest(params, args...);
		}
	};
}

#endif