#ifndef MAKAILIB_EX_GAME_DIALOG_SCENE_H
#define MAKAILIB_EX_GAME_DIALOG_SCENE_H

#include <makai/makai.hpp>

#include "actor.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Scene {
		using Actors	= List<Handle<Actor>>;
		using Cast		= Map<usize, Handle<Actor>>;
		Cast cast;

		using Step = Co::Yielder;

		Step begin();
		Step end();

		ActorRef actor(usize const& hash) {
			if (cast.contains(hash))
				return {cast.at(hash)};
			return {cast.at(hash) = nullptr};
		}

		ActorRef actor(String const& name) {
			return actor(Hasher::hash(name));
		}

		void order(Line const& line, StringList const& actors);
		void order(Line const& line, List<usize> const& actors);

		void order(Action const& action, StringList const& actors);
		void order(Action const& action, List<usize> const& actors);

		void order(Emotion const& emotion, StringList const& actors);
		void order(Emotion const& emotion, List<usize> const& actors);
	};
}

#endif