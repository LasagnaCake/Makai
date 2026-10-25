#ifndef MAKAILIB_EX_GAME_DIALOG_BOX_H
#define MAKAILIB_EX_GAME_DIALOG_BOX_H

#include "core.hpp"
#include "../core/sprite.hpp"

/// @brief Game dialog extensions.
namespace Makai::Ex::Game::Dialog {
	struct Box: IBox, AUpdateable {
		struct Display {
			Graph::Renderable	background;
			Graph::UTF8Label	title;
			Graph::UTF8Label	text;

			SpriteHolder backTitleSprite	= background.createReference<Sprite>();
			SpriteHolder backTextSprite		= background.createReference<Sprite>();
		} display;

		struct Configuration {
			struct Layers {
				usize title;
				usize text;
				usize background;
			} layers;
		};

		Box(Configuration const& config) {
			display.background.setRenderLayer(layers.background);
			display.title.setRenderLayer(layers.title);
			display.text.setRenderLayer(layers.text);
			background.active =
			display.title.active =
			display.text.active = false;
			display.background.material.color.a = 0.5;
			// Title
			display.backTitleSprite->local.scale = Vector3(40, 1) / 2;
			display.backTitleSprite->local.position = Vector3(0, 1.5);
			display.backTitleSprite->setColor(Color::WHITE);
			display.title.text->lineWrap = Graph::LineWrap::LW_HYPHEN_WORD;
			display.title.text->rect = {40, 5};
			display.title.text->rectAlign = .5;
			display.title.text->content = "";
			// Text
			display.backTextSprite->local.scale = Vector3(40, 4) / 2;
			display.backTextSprite->local.position = Vector3(0, -0.5);
			display.backTextSprite->setColor(Color::BLACK);
			display.text.text->lineWrap = Graph::LineWrap::LW_HYPHEN_WORD;
			display.text.text->rectAlign = .5;
			display.text.text->rect = {40, 5};
			display.text.text->content = "";
		}

		void onUpdate(float delta, App& app) {}

		void show() override {display.background.active = display.title.active = display.text.active = true;	}
		void hide() override {display.background.active = display.title.active = display.text.active = false;	}

		usize enter() override	{show();}
		usize exit() override	{hide();}

		usize present() override	{show();}
		usize withdraw() override	{hide();}

		usize title(UTF8String const& what) override	{display.title.text->content = what; return 0;			}
		usize text(UTF8String const& what) override		{display.text.text->content = "\n" + what; return 0;	}
		usize display(TextContent const& what) override	{title(what.title); text(what.text); return 0;			}

		usize say(UTF8String const& what) override	{text(what); return 0;								}
		usize add(UTF8String const& what) override	{text(display.text.text->content + what); return 0;	}
	};

	struct StandardBox: Box {
		struct Configuration: Box::Configuration {
			struct Animation {
				enum class Mode: uint8 {
					GDB_CDM_NONE,
					GDB_CDM_STEP_CHARACTER,
					GDB_CDM_STEP_WORD_FIXED,
					GDB_CDM_STEP_WORD_VARIABLE
				} mode = Mode::GDB_CDM_NONE;
				Nullable<float> textSpeed;
				usize duration = 30;
			} display;
		};

		struct Animator {
			Tween<Vector3> position;
			Tween<Vector3> scale;
		} animator;

		Configuration::Animation animation = {};

		Transform3D trans;

		StandardBox(Configuration const& config = {}): animation(config.animation) {
			background.setRenderLayer(layers.background);
			textDisplay.setRenderLayer(layers.text);
			background.active =
			textDisplay.active = false;
			animator.position.reinterpolate(Vector3::DOWN(), Vector3::DOWN(), 1, interpolator<Vector3>(Math::Ease::Out::cubic));
			animator.scale.reinterpolate(0, 0, 1, interpolator<Vector3>(Math::Ease::Out::cubic));
			animator.position.start();
			animator.scale.start();
		}

		void onUpdate(float delta, App& app) {
			updateDisplay();
			if (delay) delay -= speed.orElse(0);
			else if (animation.textSpeed)
				updateText();
			else for (ssize i = 0; i < Math::max<ssize>(animation.textSpeed.value(), 1); ++i)
				updateText();
		}

		usize enter() override	{animator.scale.reinterpolateTo(Vector3::UP(), animation.duration); return animation.duration;	}
		usize exit() override	{animator.scale.reinterpolateTo(0, animation.duration); return animation.duration;				}

		usize present() override	{animator.position.reinterpolateTo(0, animation.duration); return animation.duration;				}
		usize withdraw() override	{animator.position.reinterpolateTo(Vector3::DOWN(), animation.duration); return animation.duration;	}

		usize text(UTF8String const& what) override {
			words = getWords(what);
			currentWord = 0;
			display.text.text->content = what;
			currentChar = 0;
			if (animation.mode == Animation::Mode::GDB_CDM_NONE)
				display.text.text->maxChars = 0;
			return totalDuration();
		}

		usize say(UTF8String const& what) override {
			return text(what);
		}

		usize add(UTF8String const& what) override {
			auto const previous = totalDuration();
			display.text.text->maxChars = display.text.text->content.size();
			display.text.text->content += what;
			words.pushBack(getWords(what));
			return totalDuration() - previous;
		}

	private:
		void updateDisplay() {
			Transform3D anim {
				animator.position.value(),
				0,
				animator.scale.value()
			};
			display.background.trans =
			display.title.trans =
			display.text.trans = trans * anim;
		}

		void updateText() {
			if (!animation.textSpeed)
				display.text.text->maxChars = -1;
			else switch (animation.mode) {
				using enum Animation::Mode;
				case GDB_CDM_NONE: return;
				case GDB_CDM_STEP_CHARACTER:
					++currentChar;
				break;
				case GDB_CDM_STEP_WORD_VARIABLE:
					delay += words[currentWord];
				case GDB_CDM_STEP_WORD_FIXED:
					currentChar += words[currentWord];
				break;
			}
			display.text.text->maxChars = currentChar;
		}

		usize totalDuration() {
			switch (animation.mode) {
				using enum Animation::Mode;
				case GDB_CDM_NONE: return 0;
				case GDB_CDM_STEP_WORD_VARIABLE:
				case GDB_CDM_STEP_CHARACTER: return display.text.text->content.size() / animation.textSpeed.orElse(0);
				case GDB_CDM_STEP_WORD_FIXED: return words.size() / animation.textSpeed.orElse(0);
				break;
			}
		}

		static List<usize> getWords(UTF8String const& str) {
			List<usize> words;
			usize count = 0;
			for (auto const& c: str)
				if (isIdentifierNameChar(Cast::as<char, u32>(c)))
					++count;
				else {
					words.pushBack(count);
					count = 0;
				}
			words.pushBack(count);
			return words;
		}

		List<usize>	words;
		usize currentChar = 0;
		usize currentWord = 0;
		float delay = 0;
	};
}

#endif
