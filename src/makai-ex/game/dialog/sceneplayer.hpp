#ifndef MAKAILIB_EX_GAME_DIALOG_SCENEPLAYER_H
#define MAKAILIB_EX_GAME_DIALOG_SCENEPLAYER_H

#include <makai/makai.hpp>

#include "scene.hpp"
#include "animaplayer.hpp"
#include "choicemenu.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Anima-based + scene-based dialog player.
	struct ScenePlayer: AAnimaPlayer {

		/// @brief Dialog scene.
		Scene& scene;

		/// @brief Constructs the dialog player.
		/// @param scene Scene to use.
		ScenePlayer(Scene& scene): AAnimaPlayer(), scene(scene) {
			bindmap = Dictionary<String>({	
				{"next", "dialog/next"},
				{"skip", "dialog/skip"}
			});
		}

		/// @brief Constructs the dialog player.
		/// @param binpath Path to dialog program.
		/// @param scene Scene to use.
		ScenePlayer(String const& binpath, Scene& scene): AAnimaPlayer(binpath), scene(scene) {
			setProgram(binpath);
		}

		void onUpdate(float delta, Makai::App& app) override {
			AAnimaPlayer::onUpdate(delta, app);
			if (query.process()) return;
			if (query.exists()) {
				setChoice(query.value());
				postChoice();
				query.clear();
			}
		}

	protected:
		/// @brief Called when a choice is requested.
		/// @param choices Choices to make.
		/// @return Chosen choice.
		void onChoice(Parameters const& choices) override {
			if (!scene.choice) return setChoice(0);
			scene.choice->show();
			scene.choice->setOptions(choices);
			query = getQuery();
		}

		/// @brief Called when a scene dialog line is requested to be said.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		usize onSay(String const& line) override								{return scene.say(Content{line});				}
		/// @brief Called when a scene dialog line is requested to be added.
		/// @param line Line to add.
		/// @return Time it takes to add the line.
		usize onAdd(String const& line) override								{return scene.add(Content{line});				}
		/// @brief Called when a scene emotion is requested to be emoted.
		/// @param line Emotion to emote.
		/// @return Time it takes to emote the emotion.
		usize onEmote(uint64 const emotion) override							{return scene.emote(Emotion{emotion});			}
		/// @brief Called when a scene action is requested to be performed.
		/// @param action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform the action.
		usize onPerform(uint64 const action, Parameters const& params) override	{return scene.perform(Action{action, params});	}
		/// @brief Called when a scene text color change is requested.
		/// @param color Color to set.
		void onTextColor(Vector4 const& color) override							{return scene.color(color);						}
		
		/// @brief Called when actors are requested to say a line.
		/// @param actors Requested actors.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		usize onActorSay(ActiveCast const& actors, String const& line) override {
			usize delay = 0;
			for (auto actor: getActors(actors)) {
				auto d = actor->say(Content{line});
				if (delay < d) delay = d;
			}
			return delay;
		}
		/// @brief Called when actors are requested to add a line.
		/// @param actors Requested actors.
		/// @param line Line to add.
		/// @return Time it takes to say the line.
		usize onActorAdd(ActiveCast const& actors, String const& line) override {
			usize delay = 0;
			for (auto actor: getActors(actors)) {
				auto d = actor->add(Content{line});
				if (delay < d) delay = d;
			}
			return delay;
		}
		/// @brief Called when actors are requested to emote.
		/// @param actors Requested actors.
		/// @param emotion Emotion to emote.
		/// @return Time it takes to emote.
		usize onActorEmote(ActiveCast const& actors, uint64 const emotion) override {
			usize delay = 0;
			for (auto actor: getActors(actors)) {
				auto d = actor->emote(Emotion{emotion});
				if (delay < d) delay = d;
			}
			return delay;
		}
		/// @brief Called when actors are requested to perform.
		/// @param actors Requested actors.
		/// @param Action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform.
		usize onActorPerform(ActiveCast const& actors, uint64 const action, Parameters const& params) override {
			usize delay = 0;
			for (auto actor: getActors(actors)) {
				auto d = actor->perform(Action{action, params});
				if (delay < d) delay = d;
			}
			return delay;
		}
		/// @brief Called when actors are requested to change their text color.
		/// @param actors Requested actors.
		/// @param color Color to set.
		void onActorTextColor(ActiveCast const& actors, Vector4 const& color) override {
			for (auto actor: getActors(actors))
				actor->color(color);
		}
		
		/// @brief Converts an active cast to a proper list of usable actors.
		/// @param actors Actors to get.
		/// @return Actors.
		Scene::Actors getActors(ActiveCast const& actors) {
			Scene::Actors out;
			if (!actors.exclude) [[likely]] {
				for (auto const actor: actors.actors)
					if (auto aref = scene.cast.at(actor))
						out.pushBack(aref);
			} else {
				auto const actorList = actors.actors.sorted();
				for (auto& [id, actor] : scene.cast)
					if (actorList.bsearch(id) == -1)
						out.pushBack(actor);
			}
			return out;
		}

	private:
		/// @brief Choice query handler.
		Co::Generator<ssize> query;

		Co::Generator<ssize> getQuery() {
			if (scene.choice)
				co_return co_await scene.choice->awaiter();
			co_return 0;
		}
	};
}

#endif