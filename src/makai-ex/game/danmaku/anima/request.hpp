#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_REQUEST_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_REQUEST_H

#include "../server.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct IRequestable {
		virtual ~IRequestable() {}

		using Parameters = Map<usize, StringList>;
		using Parameter = typename Parameters::PairType;

		virtual bool onRequest(Parameters const& params) = 0;
	};

	struct ANamedRequestable;

	using PeriodicRequest = CTL::Ex::APeriodic<ANamedRequestable, usize const, IRequestable::Parameters const&>;

	struct ANamedRequestable: IRequestable, PeriodicRequest {
		usize const id;

		ANamedRequestable(usize const id): id(id) {}

		void onUpdate(usize const message, Parameters const& params) override final {
			if (message == id)
				onRequest(params);
		}
	};
}

#endif