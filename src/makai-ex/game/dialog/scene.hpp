#ifndef MAKAILIB_EX_GAME_DIALOG_SCENE_H
#define MAKAILIB_EX_GAME_DIALOG_SCENE_H

#include <makai/makai.hpp>

#include "actor.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Scene: IPerformer {
		using Actors	= List<Handle<Actor>>;
		using Cast		= Map<usize, Handle<Actor>>;
		Cast			cast;
		Instance<Box>	dialog;

		ActorRef actor(usize const& hash) {
			if (cast.contains(hash))
				return {cast.at(hash)};
			return {cast.at(hash) = nullptr};
		}

		ActorRef actor(String const& name) {
			return actor(Hasher::hash(name));
		}

		void color(Vector4 const& color) override		{													}
		usize say(Content const& line) override			{if (dialog) dialog->setBody(line);		return 0;	}
		usize add(Content const& line) override			{if (dialog) dialog->appendBody(line);	return 0;	}
		usize emote(Emotion const& emotion) override	{return 0;											}
		usize perform(Action const& action) override	{return 0;											}

		template<Type::OneOf<usize, String> T>
		void say(Line const& line, List<T> const& actors) {
			for (auto& a: actors)
				actor(a).say(line);
		}

		template<Type::OneOf<usize, String> T>
		void add(Line const& line, List<T> const& actors) {
			for (auto& a: actors)
				actor(a).add(line);
		}

		template<Type::OneOf<usize, String> T>
		void say(Content const& line, List<T> const& actors) {
			for (auto& a: actors)
				actor(a).say(line);
		}

		template<Type::OneOf<usize, String> T>
		void add(Content const& line, List<T> const& actors) {
			for (auto& a: actors)
				actor(a).add(line);
		}

		template<Type::OneOf<usize, String> T>
		void perform(Action const& action, List<T> const& actors) {
			for (auto& a: actors)
				actor(a).act(action);
		}

		template<Type::OneOf<usize, String> T>
		void emote(Emotion const& emotion, List<T> const& actors) {
			for (auto& a: actors)
				actor(a).emote(emotion);
		}
	};
}

#endif