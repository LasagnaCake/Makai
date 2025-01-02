#ifndef MAKAILIB_EX_GAME_DIALOG_SCENE_H
#define MAKAILIB_EX_GAME_DIALOG_SCENE_H

#include <makai/makai.hpp>

#include "actor.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Dialog scene.
	struct Scene: IPerformer {
		/// @brief Actor list.
		using Actors	= List<Handle<Actor>>;
		/// @brief Actor bank.
		/// @note Key should be their name hash.
		using Cast		= Map<usize, Handle<Actor>>;
		/// @brief Actors in scene.
		/// @note Key should be their name hash.
		Cast			cast;
		/// @brief Scene's dialog box.
		Instance<Box>	dialog = nullptr;

		/// @brief Returns a reference to an actor by its name hash.
		/// @param hash Actor name hash.
		/// @return Reference to actor.
		ActorRef actor(usize const& hash) {
			if (cast.contains(hash))
				return {cast.at(hash)};
			return {cast.at(hash) = nullptr};
		}

		/// @brief Returns a reference to an actor by its name.
		/// @param hash Actor name.
		/// @return Reference to actor.
		ActorRef actor(String const& name) {
			return actor(Hasher::hash(name));
		}

		/// @brief Sets the text body color.
		/// @param color Color to set to.
		void color(Vector4 const& color) override		{if (dialog) dialog->setBodyColor(color);			}
		/// @brief Says a dialog line.
		/// @param line Line to say.
		/// @return Time it takes to say the dialog line.
		usize say(Content const& line) override			{if (dialog) dialog->setBody(line);		return 0;	}
		/// @brief Adds text to the current dialog line.
		/// @param line Line to say.
		/// @return Time it takes to add text.
		usize add(Content const& line) override			{if (dialog) dialog->appendBody(line);	return 0;	}
		/// @brief Emotes an emotion.
		/// @param emotion Emotion to emote.
		/// @return Time it takes to emote the emotion.
		usize emote(Emotion const& emotion) override	{return 0;											}
		/// @brief Performs an action.
		/// @param action Action to perform.
		/// @return Time it takes to perform the action.
		usize perform(Action const& action) override	{return 0;											}

		/// @brief Tells a set of actors to say a line.
		/// @tparam T `String` or `usize`.
		/// @param line Line to say.
		/// @param actors Actors to command.
		/// @return Longest time to say the line.
		template<Type::OneOf<usize, String> T>
		usize say(Content const& line, List<T> const& actors) {
			usize delay, maxDelay = 0;
			for (auto& a: actors)
				if ((delay = actor(a).say(line)) > maxDelay)
					maxDelay = delay;
			return maxDelay;
		}

		/// @brief Tells a set of actors to add a line.
		/// @tparam T `String` or `usize`.
		/// @param line Line to add.
		/// @param actors Actors to command.
		/// @return Longest time to add the line.
		template<Type::OneOf<usize, String> T>
		usize add(Content const& line, List<T> const& actors) {
			usize delay, maxDelay = 0;
			for (auto& a: actors)
				if ((delay = actor(a).add(line)) > maxDelay)
					maxDelay = delay;
			return maxDelay;
		}

		/// @brief Tells a set of actors to do an action.
		/// @tparam T `String` or `usize`.
		/// @param action Action to perform.
		/// @param actors Actors to command.
		/// @return Longest time to do the action.
		template<Type::OneOf<usize, String> T>
		usize perform(Action const& action, List<T> const& actors) {
			usize delay, maxDelay = 0;
			for (auto& a: actors)
				if ((delay = actor(a).act(action)) > maxDelay)
					maxDelay = delay;
			return maxDelay;
		}

		/// @brief Tells a set of actors to emote an emotion.
		/// @tparam T `String` or `usize`.
		/// @param emotion Emotion to emote.
		/// @param actors Actors to command.
		/// @return Longest time to emote the emotion.
		template<Type::OneOf<usize, String> T>
		usize emote(Emotion const& emotion, List<T> const& actors) {
			usize delay, maxDelay = 0;
			for (auto& a: actors)
				if ((delay = actor(a).emote(emotion)) > maxDelay)
					maxDelay = delay;
			return maxDelay;
		}

		/// @brief Tells a set of actors to ser the text color.
		/// @tparam T `String` or `usize`.
		/// @param color Color to set to.
		/// @param actors Actors to command.
		template<Type::OneOf<usize, String> T>
		void color(Vector4 const& color, List<T> const& actors) {
			for(auto& a: actors)
				actor(a).color(color);
		}
	};
}

#endif