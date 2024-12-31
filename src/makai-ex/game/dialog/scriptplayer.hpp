#ifndef MAKAILIB_EX_GAME_DIALOG_ACTIONPLAYER_H
#define MAKAILIB_EX_HAME_DIALOG_ACTIONPLAYER_H

#include <makai/makai.hpp>

#include "svm/svm.hpp"

#include "scene.hpp"

namespace Makai::Ex::Game::Dialog {
	struct ScriptPlayer: private SVM::Engine, IPlayable, IUpdateable {
		ScriptPlayer(Instance<Scene> const& scene = nullptr): SVM::Engine() {}

		Instance<Scene> scene;

		ScriptPlayer(String const& binpath, Instance<Scene> const& scene = nullptr) {
			setProgram(binpath);
		}

		ScriptPlayer& setProgram(String const& binpath) {
			Engine::setProgram(SVM::fromBytes(File::getBinary(binpath)));
		}

		Input::Manager		input;
		Dictionary<String>	bindmap	= Dictionary<String>({
			{"next", "dialog-next"},
			{"skip", "dialog-skip"}
		});

		void onUpdate(auto, auto) {
			if (isFinished || paused)	return;
			advanceCounters();
			if (syncing()) return;
			if (autoplay && waiting()) return;
			if (waitForUser && userAdvanced())	next();
			else if (!waiting())				next();
		}

		ScriptPlayer& start() override final {
			inSync		=
			autoplay	=
			waitForUser	= false;
			actionDelay = 0;
			resetCounters();
			return play();
		}

		ScriptPlayer& stop()	override final		{isFinished = true; return *this;	}
		ScriptPlayer& play()	override final		{paused = false; return *this;		}
		ScriptPlayer& pause()	override final		{paused = true; return *this;		}

	protected:
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

		virtual void setGlobal(usize const name, Parameters const& values)		{}

		virtual void execute(usize const operation, Parameters const& params)	{}

		usize delay = 600;

	private:
		ScriptPlayer& next() {
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
				if (scene)
					setActionDelay(scene->say(Content{line}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->say(Content{line}));
		}

		void opAdd(ActiveCast const& actors, String const& line) override final {
			if (actors.actors.empty() && scene) {
				if (scene)
					setActionDelay(scene->add(Content{line}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->add(Content{line}));
		}

		void opEmote(ActiveCast const& actors, uint64 const emotion) override final {
			if (actors.actors.empty() && scene) {
				if (scene)
					setActionDelay(scene->emote(Emotion{emotion}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->emote(Emotion{emotion}));
		}

		void opPerform(ActiveCast const& actors, uint64 const action, Parameters const& params) override final {
			if (actors.actors.empty() && scene) {
				if (scene)
					setActionDelay(scene->perform(Action{action, params}));
				return;
			}
			for (auto actor: getActors(actors))
				setActionDelay(actor->perform(Action{action, params}));
		}

		void opColor(ActiveCast const& actors, uint64 const color) override final {
			if (actors.actors.empty() && scene) {
				if (scene)
					scene->color(Graph::Color::fromHexCodeRGBA(color));
				return;
			}
			for (auto actor: getActors(actors))
				actor->color(Graph::Color::fromHexCodeRGBA(color));
		}

		void opColorRef(ActiveCast const& actors, uint64 const color) override final {
			if (actors.actors.empty() && scene) {
				if (scene)
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

		void opSetConfigValue(uint64 const name, String const& value) override final {
			setGlobal(name, value);
		}

		void opSetConfigValues(uint64 const name, Parameters const& values) override final {
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
			actionCounter = 0;
			return false;
		}

		void setActionDelay(uint64 const time) {
			if (actionDelay < time) actionDelay = time;
		}

		bool	inSync			= false;
		bool	autoplay		= false;
		bool	waitForUser		= false;
		usize	autoCounter		= 0;
		usize	actionCounter	= 0;
		usize	actionDelay		= 0;
	};
}

#endif