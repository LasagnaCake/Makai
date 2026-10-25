#ifndef MAKAILIB_EX_GAME_DIALOG_ACTOR_H
#define MAKAILIB_EX_GAME_DIALOG_ACTOR_H

#include "core.hpp"
#include "box.hpp"
#include "../core/sprite.hpp"

/// @brief Game dialog extensions.
namespace Makai::Ex::Game::Dialog {
	struct Actor: IActor, AUpdateable {
		Instance<IBox> box;

		Actor(Data::Value const& props, Instance<IBox> const& box): box(box) {}

		void onUpdate(float delta, App& app) {}

		void show() override {if (box) box->show();}
		void hide() override {if (box) box->hide();}

		usize enter() override		{if (box) return box->enter(); return 0;	}
		usize exit() override		{if (box) return box->exit(); return 0;		}
		usize present() override	{if (box) return box->present(); return 0;	}
		usize withdraw() override	{if (box) return box->withdraw(); return 0;	}

		usize title(UTF8String const& what) override	{if (box) return box->title(what); return 0;	}
		usize text(UTF8String const& what) override		{if (box) return box->text(what); return 0;		}
		usize display(TextContent const& what) override	{if (box) return box->display(what); return 0;	}

		usize say(UTF8String const& what) override	{if (box) return box->say(what); return 0;}
		usize add(UTF8String const& what) override	{if (box) return box->add(what); return 0;}

		usize perform(String const& action) override	{return 0;}
		usize emote(String const& emotion) override		{return 0;}
		usize change(String const& costume) override	{return 0;}
	};

	struct StandardActor: Actor {
		struct Configuration {
			struct Layers {
				usize body;
				usize face;
			} layers;
			struct Animation {
				usize duration = 30;
			};
			struct Base {
				Transform3D actor;
				Transform3D box;
			} base;
		};

		struct Animator {
			Tween<Vector3> position;
			Tween<float> alpha;
		} animator;

		Graph::Renderable body;
		Graph::Renderable face;

		TileHolder bodySprite, faceSprite;

		Configuration::Animation animation = {};

		Configuration::Base base = {};

		Transform3D trans;

		Dictionary<Vector2> emotions;
		Dictionary<Vector2> costumes;

		StandardActor(Data::Value const& props, Configuration const& config = {}, Instance<IBox> const& box = new StandardBox()):
		Actor(props, box),
		animation(config.animation),
		base(config.base) {
			body.setRenderLayer(config.layers.body);
			face.setRenderLayer(config.layers.face);
			body.material.texture = Graph::Texture2D::deserialize(props.fetch("body/image"));
			face.material.texture = Graph::Texture2D::deserialize(props.fetch("face/image"));
			bodySprite = body.createReference<Sprite>();
			faceSprite = face.createReference<Sprite>();
			bodySprite->local = Transform3D {
				props.fetch("body/trans/position").vectorize(0),
				props.fetch("body/trans/rotation").vectorize(0),
				props.fetch("body/trans/scale").vectorize(1)
			};
			faceSprite->local = Transform3D {
				props.fetch("face/trans/position").vectorize(0),
				props.fetch("face/trans/rotation").vectorize(0),
				props.fetch("face/trans/scale").vectorize(1)
			};
			trans = Transform3D {
				props.fetch("trans/position").vectorize(0),
				props.fetch("trans/rotation").vectorize(0),
				props.fetch("trans/scale").vectorize(1)
			};
			body.active = false;
			face.active = false;
			animator.position.reinterpolate(Vector3::UP(), Vector3::UP(), 1, interpolator<Vector3>(Math::Ease::Out::cubic));
			animator.alpha.reinterpolate(0, 0, 1, interpolator<float>(Math::Ease::Out::linear));
			if (box) {
				box->setTransform(
					Transform3D {
						props.fetch("box/trans/position").vectorize(0),
						props.fetch("box/trans/rotation").vectorize(0),
						props.fetch("box/trans/scale").vectorize(1)
					} * base.box
				);
			}
		}

		void onUpdate(float delta, App& app) {
			updateDisplay();
		}

		void show() override {Actor::show(); body.active = face.active = true;	}
		void hide() override {Actor::hide(); body.active = face.active = false;	}

		usize enter() override	{animator.alpha.reinterpolateTo(1, animation.duration); return Math::highest(Actor::enter(), animation.duration);	}
		usize exit() override	{animator.alpha.reinterpolateTo(0, animation.duration); return Math::highest(Actor::exit(), animation.duration);	}

		usize present() override	{animator.position.reinterpolateTo(0, animation.duration); return Math::highest(Actor::present(), animation.duration);				}
		usize withdraw() override	{animator.position.reinterpolateTo(Vector3::UP(), animation.duration); return Math::highest(Actor::withdraw(), animation.duration);	}

		usize emote(String const& emotion) override	 {
			if (emotions.contains(emotion)) {
				auto const what = emotions[emotion];
				faceSprite->tile = {what.x, what.y};
			}
			return 0;
		}

		usize change(String const& costume) override {
			if (costumes.contains(costume)) {
				auto const what = costumes[costume];
				bodySprite->tile = {what.x, what.y};
			}
			return 0;
		}

		void setTransform(Transform3D const& tt) override	{trans = tt;	}
		Transform3D getTransform() const override			{return trans;	}

	private:
		void updateDisplay() {
			Transform3D anim {
				animator.position.value(),
				0,
				1
			};
			body.trans =
			face.trans = trans * anim * base.actor;
			body.material.color.a =
			face.material.color.a = animator.alpha.value();
		}
	};
}

#endif
