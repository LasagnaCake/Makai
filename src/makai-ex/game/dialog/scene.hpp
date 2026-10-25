#ifndef MAKAILIB_EX_GAME_DIALOG_SERVER_H
#define MAKAILIB_EX_GAME_DIALOG_SERVER_H

#include "../core/controlable.hpp"
#include "core.hpp"

/// @brief Game dialog extensions.
namespace Makai::Ex::Game::Dialog {
	struct AScene: AUpdateable, IPlayable, Controllable, Anima::V2::Runtime::Engine {
		using Script = Co::Generator<usize>;
		using Actors = Dictionary<Instance<IActor>>;

		struct Configuration {
			enum class Mode: u8 {
				GDCM_MANUAL,
				GDCM_USER_STEPPED,
				GDCM_AUTO
			};
			Mode	mode	= Mode::GDCM_USER_STEPPED;
			ssize	timeout = -1;
		} config;

		Input::Manager input;

		Instance<IBox> globalBox;

		Actors actors;

		AScene(Configuration const& config = {}) config(config) {isFinished = true;}

		virtual ~AScene() {}

		template <Type::Derived<IActor> T>
		AScene& add(String const& name, Instance<T> const& actor) {
			actors[name] = actor;
		}

		virtual Script script() = 0;

		void onUpdate(float delta, App& app) override {
			if (isFinished) return;
			if (paused) return;
			if (delay > 0) {
				--delay;
				return;
			}
			if (!current)
				stop();
			switch (config.mode) {
				using enum Configuration::Mode;
				case GDCM_USER_STEPPED: if (!interaction()) return;
				default: break;
			}
			do {
				delay = current.next();
			}
			while (!delay && current);
		}

		AScene& start() override {
			if (!isFinished) return *this;
			current = script();
			isFinished = false;
			return *this;
		}

		AScene& stop() override {
			if (isFinished) return *this;
			isFinished = true;
			return *this;
		}

		AScene& play() override {
			if (isFinished) return start();
			paused = false;
			return *this;
		}

		AScene& pause() override {
			if (isFinished) return *this;
			paused = false;
			return *this;
		}

		bool interaction() const {
			return config.mode != Config::Mode::GDCM_AUTO && (userNext() or userSkip());
		}

	protected:

		Script userInput() const {
			co_return co_await userInput(config.timeout);
		}

		Script userInput(ssize timeout) const {
			co_return co_await action([&] {return interaction();}, timeout);
		}

		bool userNext() const {
			return action("next", true);
		}

		bool userSkip() const {
			return action("skip");
		}

		Script action(Signal<void> const& action, ssize timeout = -1) const {
			while (timeout && !action()) {
				if (timeout > 0) --timeout;
				co_yield 1;
			}
			co_return 0;
		}

	private:
		usize delay = 0;
		Script current;
	};

	struct ARTScene: AScene {
		Script script() override {
			while (engine) {
				maxDelay = 0;
				if (autoPresent) for (auto& actor: last) actor->withdraw();
				last.clear();
				engine.process();
				co_yield maxDelay;
			}
			co_return 1;
		}

	protected:
		void wait(usize const delay)		{maxDelay = Math::max(maxDelay, delay);	}
		void addWait(usize const delay)		{maxDelay += delay;						}
		void setDelay(usize const delay)	{maxDelay = delay;						}

		void onLoad() override {
			context.art.addNativeCall<void(UTF8String)>(
				"makai/ex/game/dialog/scene/title/v1",
				[&] (UTF8String const& text) {if (globalBox) wait(globalBox->title(text));}
			);
			context.art.addNativeCall<void(UTF8String)>(
				"makai/ex/game/dialog/scene/say/v1",
				[&] (UTF8String const& text) {if (globalBox) wait(globalBox->say(text));}
			);
			context.art.addNativeCall<void(UTF8String)>(
				"makai/ex/game/dialog/scene/add/v1",
				[&] (UTF8String const& text) {if (globalBox) wait(globalBox->add(text));}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/title/v1",
				[&] (String const& name, UTF8String const& text) {if (actors.contains(name)) wait(actors[name]->title(text));}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/say/v1",
				[&] (String const& name, UTF8String const& text) {
					if (actors.contains(name)) {
						auto const actor = actors[name];
						if (autoPresent) last.pushBack(actor);
						wait(actor->say(text));
					}
				}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/add/v1",
				[&] (String const& name, UTF8String const& text) {
					if (actors.contains(name)) {
						auto const actor = actors[name];
						if (autoPresent) last.pushBack(actor);
						wait(actor->add(text));
					}
				}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/perform/v1",
				[&] (String const& name, String const& action) {if (actors.contains(name)) wait(actors[name]->perform(action));}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/emote/v1",
				[&] (String const& name, String const& emotion) {if (actors.contains(name)) wait(actors[name]->emote(emotion));}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/enter/v1",
				[&] () {if (actors.contains(name)) wait(actors[name]->enter());}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/exit/v1",
				[&] () {if (actors.contains(name)) wait(actors[name]->exit());}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/present/v1",
				[&] () {
					if (actors.contains(name)) {
						auto const actor = actors[name];
						if (autoPresent) last.pushBack(actor);
						wait(actor->present());
					}
				}
			);
			context.art.addNativeCall<void(String, UTF8String)>(
				"makai/ex/game/dialog/scene/actor/withdraw/v1",
				[&] () {if (actors.contains(name)) wait(actors[name]->withdraw());}
			);
			context.art.addNativeCall<void()>(
				"makai/ex/game/dialog/scene/mode/autoplay/v1",
				[&] {config.mode = Configuration::Mode::GDCM_AUTO;}
			);
			context.art.addNativeCall<void()>(
				"makai/ex/game/dialog/scene/mode/manual/v1",
				[&] {config.mode = Configuration::Mode::GDCM_MANUAL;}
			);
			context.art.addNativeCall<void()>(
				"makai/ex/game/dialog/scene/mode/userStepped/v1",
				[&] {config.mode = Configuration::Mode::GDCM_USER_STEPPED;}
			);
			context.art.addNativeCall<void(usize)>("makai/ex/game/dialog/scene/wait/v1",		memberCall(*this, wait));
			context.art.addNativeCall<void(usize)>("makai/ex/game/dialog/scene/addWait/v1",		memberCall(*this, addWait));
			context.art.addNativeCall<void(usize)>("makai/ex/game/dialog/scene/setDelay/v1",	memberCall(*this, setDelay));
			context.art.addNativeCall<void(ssize)>(
				"makai/ex/game/dialog/scene/timeout/v1",
				[&] (ssize const timeout) {config.timeout = timeout;}
			);
			context.art.addNativeCall<void(bool)>(
				"makai/ex/game/dialog/scene/autoPresent/v1",
				[&] (bool const state) {autoPresent = state;}
			);
		}

		bool autoPresent = false;

	private:
		List<Instance<Actor>> last;

		usize maxDelay = 0;
	};

	template <Type::Derived<IActor> TActor, Type::Derived<IBox> TBox>
	struct StandardScene: ARTScene {
	protected:
		String loadActor(String const& path) {
			// TODO: This
		}

		void onLoad() override {
			context.art.addNativeCall<String(String)>(
				"makai/ex/game/dialog/scene/loadActor/v1",
				memberCall(*this, loadActor)
			);
		}
	};
}

#endif
