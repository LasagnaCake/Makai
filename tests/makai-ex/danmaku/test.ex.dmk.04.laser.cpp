#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), gamearea / 2},
	playfield	= {gamearea * Makai::Vector2(1, -1), gamearea / 2}
;

using BaseLaserServer = Danmaku::LaserServer<>;

struct MeshHolder {
	MkGraph::Renderable gm;

	MeshHolder() {
		gm.setRenderLayer(Danmaku::Render::Layer::ENEMY1_LASER_LAYER);
		gm.setBlendEquation(Makai::Graph::BlendEquation::BE_ADD);
	}
};


struct TestLaserServer: MeshHolder, BaseLaserServer {
	TestLaserServer(): MeshHolder(), BaseLaserServer({32, gm, ::board, ::playfield}) {}
};

struct TestApp: Makai::Ex::Game::App {
	TestLaserServer server;

	TestApp(): App(Makai::Config::App{{800, 600, "Test 04", false}}) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(64, Makai::Vector2(4, 3) / 3.0);
	}

	void onOpen() override {
		createAttacks();
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::Render::Layer::BOSS1_SPELL_BG_BOTTOM_LAYER);
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	float framerate[MAX_FRCOUNT];

	void createAttacks() {
		for (usize i = 0; i < 10; ++i) {
			auto laser = server.acquire().as<Danmaku::Laser>();
			if (!laser) return;
			float const crot = (TAU / 10) * (i + (getCurrentCycle() * 0.5));
			laser->trans.position = playfield.center;
			laser->velocity.value = 30;
			laser->rotation = {
				crot,
				true,
				crot,
				crot + static_cast<float>(TAU),
				.01,
				Makai::Math::Ease::InOut::back
			};
		}
	}

	void onUpdate(float delta) {
		if (frcount < MAX_FRCOUNT)
			framerate[frcount++] = 1000.0 / getCycleDelta();
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