#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), (gamearea / 4)},
	playfield	= {gamearea * Makai::Vector2(1, -1), (gamearea * 1.5) / 4}
;

using BaseBulletServer = Danmaku::BulletServer<>;

struct MeshHolder {
	MkGraph::Renderable m, gm;

	MeshHolder() {
		m.setRenderLayer(Danmaku::Render::Layer::ENEMY1_BULLET_LAYER);
		gm.setRenderLayer(Danmaku::Render::Layer::ENEMY1_BULLET_LAYER+1);
		gm.setBlendEquation(Makai::Graph::BlendEquation::BE_ADD);
	}
};

struct TestBulletServer: MeshHolder, BaseBulletServer {
	TestBulletServer(): MeshHolder(), BaseBulletServer({1024, m, gm, ::board, ::playfield}) {}
};

Danmaku::Bullet::PromiseType btask(Danmaku::Bullet& bullet) {
	co_yield 60;
	DEBUGLN("Oh no I died");
	bullet.free();
	co_return 1;
}

struct TestApp: Makai::Ex::Game::App {
	TestBulletServer server;

	TestApp(): App(Makai::Config::App{{800, 600, "Test 03", false}}) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(64, Makai::Vector2(4, 3) / 3.0);
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::Render::Layer::BOSS1_SPELL_BG_BOTTOM_LAYER);
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	float framerate[MAX_FRCOUNT];
	
	void createShots() {
		for (usize i = 0; i < 10; ++i) {
			auto bullet = server.acquire().as<Danmaku::Bullet>();
			if (!bullet) return;
			float const crot = (TAU / 10) * (i + (getCurrentCycle() * 0.5));
			bullet->trans.position = playfield.center;
			bullet->velocity.value = 30;
			bullet->rotation = {
				crot,
				true,
				crot,
				crot + static_cast<float>(TAU),
				.01,
				Makai::Math::Ease::InOut::back
			};
			bullet->bouncy	= true;
			bullet->loopy	= true;
//			bullet->task = btask(*bullet);
		}
	}

	void onUpdate(float delta) {
		if (frcount < MAX_FRCOUNT)
			framerate[frcount++] = 1000.0 / getCycleDelta();
		else {
			createShots();
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