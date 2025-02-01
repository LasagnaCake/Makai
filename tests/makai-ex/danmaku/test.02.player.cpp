#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64, 64 * (4.0/3.0)) / 2;

Danmaku::GameArea
	board		= {gamearea, gamearea},
	playfield	= {gamearea, gamearea}
;

Danmaku::PlayerConfig cfg = {board, playfield};

struct TestPlayer: Danmaku::APlayer {
	MkGraph::Renderable body;
	Makai::Instance<Makai::Ex::Game::Sprite> sprite;

	TestPlayer(): APlayer(cfg), sprite(body.createReference<Makai::Ex::Game::Sprite>()) {}

	void onUpdate(float delta) override {
		if (!active) return;
		APlayer::onUpdate(delta);
		if (paused()) return;
		body.trans.position		= trans.position;
		body.trans.rotation.z	= trans.rotation;
		body.trans.scale		= trans.scale;
	}

	virtual void onItem(Makai::Reference<Danmaku::Item> const& item) override										{				}
	virtual void onGraze(Makai::Reference<Danmaku::AServerObject> const& object) override							{				}
	virtual void onBomb() override																					{				}
	virtual void onShot() override																					{				}
	virtual TestPlayer& spawn() override																			{return *this;	}
	virtual TestPlayer& despawn() override																			{return *this;	}
	virtual TestPlayer& takeDamage(Makai::Reference<Danmaku::AGameObject> const&, CollisionMask const&) override	{return *this;	}
	virtual TestPlayer& takeDamage(float const damage) override														{return *this;	}
};

struct TestApp: Makai::Ex::Game::App {
	TestPlayer player;

	TestApp(): App(Makai::Config::App{{800, 600, "Test 02", false}}) {}
};

int main() {
	try {
		TestApp app;
		app.run();
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.what());
	}
	return 0;
}