#ifndef MAKAILIB_EX_HAME_ANIMAPLAYER_H
#define MAKAILIB_EX_HAME_ANIMAPLAYER_H

#include <makai/makai.hpp>

#include "../anima/anima.hpp"

#include "scene.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief AVM-based dialog player.
	struct AnimaPlayer: private AVM::Engine, IPlayable, IUpdateable {
		using Engine::state, Engine::error;

		using typename Engine::State;

		/// @brief Constructs the dialog player.
		/// @param scene Scene to use. By default, it is `nullptr` (none).
		AnimaPlayer(Instance<Scene> const& scene = nullptr): AVM::Engine() {}

		/// @brief Dialog scene.
		Instance<Scene> scene;

		/// @brief Constructs the dialog player.
		/// @param binpath Path to dialog program.
		/// @param scene Scene to use. By default, it is `nullptr` (none).
		AnimaPlayer(String const& binpath, Instance<Scene> const& scene = nullptr) {
			setProgram(binpath);
		}

		/// @brief Sets the dialog program to use.
		/// @param binpath Path to dialog program.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		AnimaPlayer& setProgram(String const& binpath) {
			setProgram(AVM::Anima::fromBytes(File::getBinary(binpath)));
		}

		/// @brief Sets the dialog program to use.
		/// @param diag Dialog program to use.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		AnimaPlayer& setProgram(AVM::Anima const& diag) {
			stop();
			Engine::setProgram(diag);
			return *this;
		}

		/// @brief Executed every update cycle.
		void onUpdate(auto, auto) {
			if (state() != State::AVM_ES_RUNNING) {
				stop();
				return;
			}
			if (isFinished || paused)	return;
			advanceCounters();
			if (syncing()) return;
			if (autoplay && waiting()) return;
			if (waitForUser && userAdvanced())	next();
			else if (!waiting())				next();
			else if (!waitForUser)				next();
		}

		/// @brief Starts the dialog.
		/// @return Reference to self. 
		AnimaPlayer& start() override final {
			inSync		=
			autoplay	=
			waitForUser	= false;
			actionDelay = 0;
			resetCounters();
			beginProgram();
			return play();
		}

		/// @brief Stops the dialog.
		/// @return Reference to self.
		AnimaPlayer& stop()	override final		{isFinished = true; return *this; endProgram();	}
		/// @brief Unpauses the dialog.
		/// @return Reference to self.
		AnimaPlayer& play()	override final		{paused = false; return *this;					}
		/// @brief Pauses the dialog.
		/// @return Reference to self.
		AnimaPlayer& pause()	override final		{paused = true; return *this;					}

		/// @brief Input manager.
		Input::Manager		input;
		/// @brief Input bind map.
		Dictionary<String>	bindmap	= Dictionary<String>({
			{"next", "dialog-next"},
			{"skip", "dialog-skip"}
		});

	protected:
		/// @brief Returns a color by a name hash.
		/// @param name Name hash.
		/// @return Color.
		virtual Vector4 getColorByName(uint64 const name) {
			switch (name) {
				case (ConstHasher::hash("white")):		return Graph::Color::WHITE;
				case (ConstHasher::hash("gray")):		return Graph::Color::GRAY;
				case (ConstHasher::hash("black")):		return Graph::Color::BLACK;
				case (ConstHasher::hash("red")):		return Graph::Color::RED;
				case (ConstHasher::hash("yellow")):		return Graph::Color::YELLOW;
				case (ConstHasher::hash("green")):		return Graph::Color::GREEN;
				case (ConstHasher::hash("cyan")):		return Graph::Color::CYAN;
				case (ConstHasher::hash("blue")):		return Graph::Color::BLUE;
				case (ConstHasher::hash("magenta")):	return Graph::Color::MAGENTA;
			}
			return Graph::Color::WHITE;
		}

		/// @brief Sets a global by a name hash.
		/// @param name Global to set.
		/// @param values Values to set to.
		virtual void execute(usize const name, Parameters const& params) {
			switch (name) {
				case (ConstHasher::hash("autoplay")):	autoplay	= toBool(params[0]);
				case (ConstHasher::hash("delay")):		delay		= toUInt64(params[0]);
			}
		}

		/// @brief Max time to wait for user input.
		usize delay = 600;

	private:
		/// @brief Whether the engine is waiting for actions to finish processing.
		bool	inSync			= false;
		/// @brief Whether autoplay is enabled.
		bool	autoplay		= false;
		/// @brief Whether to wait for user input.
		bool	waitForUser		= false;
		/// @brief Counter for the auto-advance timer.
		usize	autoCounter		= 0;
		/// @brief Counter for the action sync timer.
		usize	actionCounter	= 0;
		/// @brief Time to wait for actions to finish processing.
		usize	actionDelay		= 0;

		AnimaPlayer& next() {
			if (isFinished) return;
			inSync		=
			waitForUser	= false;
			resetCounters();
			clearActionDelay();
			Engine::process();
			if (state() != Engine::State::AVM_ES_RUNNING)
				isFinished = true;
		}

		void opSay(ActiveCast const& actors, String const& line) override final {
			if (actors.actors.empty() && scene) {
				setActionDelay(scene->say(Content{line}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->say(Content{line}));
		}

		void opAdd(ActiveCast const& actors, String const& line) override final {
			if (actors.actors.empty() && scene) {
				setActionDelay(scene->add(Content{line}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->add(Content{line}));
		}

		void opEmote(ActiveCast const& actors, uint64 const emotion) override final {
			if (actors.actors.empty() && scene) {
				setActionDelay(scene->emote(Emotion{emotion}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->emote(Emotion{emotion}));
		}

		void opPerform(ActiveCast const& actors, uint64 const action, Parameters const& params) override final {
			if (actors.actors.empty() && scene) {
				setActionDelay(scene->perform(Action{action, params}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->perform(Action{action, params}));
		}

		void opColor(ActiveCast const& actors, uint64 const color) override final {
			if (actors.actors.empty() && scene) {
				scene->color(Graph::Color::fromHexCodeRGBA(color));
				return;
			}
			for (auto actor: getActors(actors))
				actor->color(Graph::Color::fromHexCodeRGBA(color));
		}

		void opColorRef(ActiveCast const& actors, uint64 const color) override final {
			if (actors.actors.empty() && scene) {
				scene->color(getColorByName(color));
				return;
			}
			for (auto actor: getActors(actors))
				actor->color(getColorByName(color));
		}

		void opDelay(uint64 const time) override final {
			actionDelay = time;
		}

		void opWaitForActions(bool const async) override final {
			inSync = true;
		}

		void opWaitForUser() override final {
			waitForUser = true;
		}

		void opNamedCallSingle(uint64 const name, String const& param) override final {
			execute(name, Parameters{param});
		}

		void opNamedCallMultiple(uint64 const name, Parameters const& params) override final {
			execute(name, params);
		}

		Scene::Actors getActors(ActiveCast const& actors) {
			Scene::Actors out;
			if (!scene) return out;
			for (auto& [id, actor] : scene->cast) {
				auto match = actors.actors.find(id) != -1;
				if (actor && actors.exclude ? !match : match)
					out.pushBack(actor);
			}
			return out;
		}

		void resetCounters() {
			autoCounter		= 0;
			actionCounter	= 0;
		}

		void clearActionDelay() {
			actionDelay = 0;
		}

		void advanceCounters() {
			++autoCounter;
			++actionCounter;
		}

		bool userAdvanced() {
			return (
				input.isButtonJustPressed(bindmap["next"])
			||	input.isButtonDown(bindmap["skip"])
			);
		}

		bool waiting() {
			return autoCounter < delay;
		}

		bool syncing() {
			if (inSync && actionCounter < actionDelay)
				return true;
			resetCounters();
			return false;
		}

		void setActionDelay(uint64 const time) {
			if (actionDelay < time) actionDelay = time;
		}
	};
}

#endif