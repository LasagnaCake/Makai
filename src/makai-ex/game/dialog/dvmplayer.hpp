#ifndef MAKAILIB_EX_HAME_DIALOG_DVMPLAYER_H
#define MAKAILIB_EX_HAME_DIALOG_DVMPLAYER_H

#include <makai/makai.hpp>

#include "dvm/dvm.hpp"

#include "scene.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief DVM-based dialog player.
	struct DVMPlayer: private DVM::Engine, IPlayable, IUpdateable {
		using Engine::state, Engine::error;

		using State = Engine::State;

		/// @brief Constructs the dialog player.
		/// @param scene Scene to use. By default, it is `nullptr` (none).
		DVMPlayer(Instance<Scene> const& scene = nullptr): DVM::Engine() {}

		/// @brief Dialog scene.
		Instance<Scene> scene;

		/// @brief Constructs the dialog player.
		/// @param binpath Path to dialog program.
		/// @param scene Scene to use. By default, it is `nullptr` (none).
		DVMPlayer(String const& binpath, Instance<Scene> const& scene = nullptr) {
			setProgram(binpath);
		}

		/// @brief Sets the dialog program to use.
		/// @param binpath Path to dialog program.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		DVMPlayer& setProgram(String const& binpath) {
			setProgram(DVM::fromBytes(File::getBinary(binpath)));
		}

		/// @brief Sets the dialog program to use.
		/// @param diag Dialog program to use.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		DVMPlayer& setProgram(DVM::Dialog const& diag) {
			stop();
			Engine::setProgram(diag);
			return *this;
		}

		/// @brief Executed every update cycle.
		void onUpdate(auto, auto) {
			if (state() != State::DSES_RUNNING) {
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
		DVMPlayer& start() override final {
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
		DVMPlayer& stop()	override final		{isFinished = true; return *this; endProgram();	}
		/// @brief Unpauses the dialog.
		/// @return Reference to self.
		DVMPlayer& play()	override final		{paused = false; return *this;					}
		/// @brief Pauses the dialog.
		/// @return Reference to self.
		DVMPlayer& pause()	override final		{paused = true; return *this;					}

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
			#if __cpp_constexpr == 202306L
			switch (name) {
				case (Hasher::hash("red")):		return Graph::Color::RED;
				case (Hasher::hash("yellow")):	return Graph::Color::YELLOW;
				case (Hasher::hash("green")):	return Graph::Color::GREEN;
				case (Hasher::hash("cyan")):	return Graph::Color::CYAN;
				case (Hasher::hash("blue")):	return Graph::Color::BLUE;
				case (Hasher::hash("magenta")):	return Graph::Color::MAGENTA;
			}
			#else
			// TODO: this
			return Graph::Color::WHITE;
			#endif
		}

		/// @brief Sets a global by a name hash.
		/// @param name Global to set.
		/// @param value Value to set to.
		virtual void setGlobal(usize const name, String const& value) {
			#if __cpp_constexpr == 202306L
			switch (name) {
				case (Hasher::hash("autoplay")):	autoplay	= toBool(value);
				case (Hasher::hash("delay")):		delay		= toUInt64(value);
			}
			#else
			// TODO: this
			#endif
		}

		/// @brief Sets a global by a name hash.
		/// @param name Global to set.
		/// @param values Values to set to.
		virtual void setGlobal(usize const name, Parameters const& values)		{}

		/// @brief Executes a named operation.
		/// @param operation Name hash to execute.
		/// @param params Parameters to pass to named operation.
		virtual void execute(usize const operation, Parameters const& params)	{}

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

		DVMPlayer& next() {
			if (isFinished) return;
			inSync		=
			waitForUser	= false;
			resetCounters();
			Engine::process();
			if (state() != Engine::State::DSES_RUNNING)
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

		void opSetGlobalValue(uint64 const name, String const& value) override final {
			setGlobal(name, value);
		}

		void opSetGlobalValues(uint64 const name, Parameters const& values) override final {
			setGlobal(name, values);
		}

		void opNamedOperation(uint64 const op, Parameters const& params) override final {
			execute(op, params);
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