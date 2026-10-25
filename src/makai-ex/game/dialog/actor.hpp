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

		usize enter() override {}
		usize exit() override {}
		usize present() override {}
		usize withdraw() override {}

		usize title(UTF8String const& what) override	{if (box) return box->title(what); return 0;	}
		usize text(UTF8String const& what) override		{if (box) return box->text(what); return 0;		}
		usize display(TextContent const& what) override	{if (box) return box->display(what); return 0;	}

		usize say(UTF8String const& what) override	{if (box) return box->say(what); return 0;}
		usize add(UTF8String const& what) override	{if (box) return box->add(what); return 0;}

		usize perform(String const& action) override	{}
		usize emote(String const& emotion) override		{}
		usize change(String const& costume) override	{}
	};

	struct StandardActor: Actor {
		struct Configuration {
			struct Layers {
				usize body;
				usize face;
			} layers;
		};

		Graph::Renderable body;
		Graph::Renderable face;

		SpriteHolder bodySprite, faceSprite;

		Transform3D trans;

		StandardActor(Data::Value const& props, Configuration const& config = {}, Instance<IBox> const& box = new StandardBox()): Actor(props, box) {
			body.setRenderLayer(config.layers.body);
			face.setRenderLayer(config.layers.face);
			body.material.texture = Graph::Texture2D::deserialize(props.fetch("body/image"));
			face.material.texture = Graph::Texture2D::deserialize(props.fetch("face/image"));
			bodySprite = body.createReference<Sprite>();
			faceSprite = face.createReference<Sprite>();
			bodySprite->local = {
				props.fetch("body/trans/position").vectorize(0),
				props.fetch("body/trans/rotation").vectorize(0),
				props.fetch("body/trans/scale").vectorize(1)
			};
			faceSprite->local = {
				props.fetch("face/trans/position").vectorize(0),
				props.fetch("face/trans/rotation").vectorize(0),
				props.fetch("face/trans/scale").vectorize(1)
			};
		}

		void onUpdate(float delta, App& app) {
			updateDisplay();
		}

		void show() override {}
		void hide() override {}

		usize enter() override {}
		usize exit() override {}
		usize present() override {}
		usize withdraw() override {}

		usize title(UTF8String const& what) override	{if (box) return box->title(what); return 0;	}
		usize text(UTF8String const& what) override		{if (box) return box->text(what); return 0;		}
		usize display(TextContent const& what) override	{if (box) return box->display(what); return 0;	}

		usize say(UTF8String const& what) override	{if (box) return box->say(what); return 0;}
		usize add(UTF8String const& what) override	{if (box) return box->add(what); return 0;}

		usize perform(String const& action) override	{}
		usize emote(String const& emotion) override		{}
		usize change(String const& costume) override	{}

	private:
		void updateDisplay() {
			Transform3D anim;
			body.trans =
			face.trans = trans * anim;
		}
	};
}

#endif
