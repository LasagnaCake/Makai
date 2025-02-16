#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), gamearea},
	playfield	= {gamearea * Makai::Vector2(1, -1), gamearea}
;

Danmaku::PlayerConfig cfg = {board, playfield};

struct TestPlayer: Danmaku::APlayer {
	MkGraph::Renderable body;
	Makai::Instance<Makai::Ex::Game::Sprite> sprite;

	TestPlayer(): APlayer(cfg), sprite(body.createReference<Makai::Ex::Game::Sprite>()) {
		body.setRenderLayer(Danmaku::Render::Layer::PLAYER1_LAYER);
		trans.position = board.center;
		input.binds["player/up"] 	= {Makai::Input::KeyCode::KC_UP};
		input.binds["player/down"]	= {Makai::Input::KeyCode::KC_DOWN};
		input.binds["player/left"] 	= {Makai::Input::KeyCode::KC_LEFT};
		input.binds["player/right"]	= {Makai::Input::KeyCode::KC_RIGHT};
		input.binds["player/focus"]	= {Makai::Input::KeyCode::KC_LEFT_SHIFT};
		velocity = {20, 10};
	}

	virtual ~TestPlayer() {}

	void onUpdate(float delta) override {
		if (!active) return;
		APlayer::onUpdate(delta);
		if (paused()) return;
		body.trans.position		= trans.position;
		body.trans.rotation.z	= trans.rotation;
		body.trans.scale		= trans.scale;
	}

	void onUpdate(float delta, Makai::App& app) override {
		if (!active) return;
		APlayer::onUpdate(delta, app);
		if (paused()) return;
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

	TestApp(): App(Makai::Config::App{{800, 600, "Test 02", false}}) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(32, Makai::Vector2(4, 3));
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::Render::Layer::BOSS1_SPELL_BG_BOTTOM_LAYER);
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	float framerate[MAX_FRCOUNT];

	void onUpdate(float delta) {
		if (frcount < MAX_FRCOUNT)
			framerate[frcount++] = 1000.0 / getFrameDelta();
		else {
			float fravg = 0;
			for(float& f: framerate) fravg += f;
			fravg *= (1.0 / (float)MAX_FRCOUNT);
			fravg = Makai::Math::clamp<float>(fravg, 0, maxFrameRate);
			DEBUGLN("Framerate: ", Makai::Format::prettify(Makai::Math::round(fravg, 2), 2, 0));
			frcount = 0;
		}
	}
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