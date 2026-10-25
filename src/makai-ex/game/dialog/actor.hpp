#ifndef MAKAILIB_EX_GAME_DIALOG_ACTOR_H
#define MAKAILIB_EX_GAME_DIALOG_ACTOR_H

#include "core.hpp"

/// @brief Game dialog extensions.
namespace Makai::Ex::Game::Dialog {
	struct Actor: IActor, AUpdateable {
		Instance<IBox> box;

		void onUpdate(float delta, App& app) {}

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
	};

	struct StandardActor: Actor {
		Instance<IBox> box;

		void onUpdate(float delta, App& app) {}

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
	};
}

#endif
