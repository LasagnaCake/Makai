#ifndef MAKAILIB_EX_GAME_DIALOG_SCENE_H
#define MAKAILIB_EX_GAME_DIALOG_SCENE_H

#include <makai/makai.hpp>

#include "actor.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Scene {
		using Actors	= List<Handle<Actor>>;
		using Cast		= Dictionary<Handle<Actor>>;
		Cast cast;

		using Step = Co::Yielder;

		Step begin();
		Step end();

		ActorRef actor(String const& name) {
			if (cast.contains(name))
				return {cast.at(name)};
			return {nullptr};
		}

		void highlight(StringList const& actors);
		void detract(StringList const& actors);
		void redirect(StringList const& actors);

		void highlight(Actors const& actors);
		void detract(Actors const& actors);
		void redirect(Actors const& actors);

		void highlight(String const& actor);
		void detract(String const& actor);
		void redirect(String const& actor);

		void order(Action const& action, StringList const& actors);
		void order(Line const& line, Actors const& actors);

		virtual void highlight(Actor const& actor);
		virtual void detract(Actor const& actor);
		virtual void redirect(Actor const& actor);
	};
}

#endif