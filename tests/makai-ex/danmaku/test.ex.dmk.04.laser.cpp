#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), (gamearea)},
	playfield	= {gamearea * Makai::Vector2(1, -1), (gamearea * 1.5)}
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
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::Render::Layer::BOSS1_SPELL_BG_BOTTOM_LAYER);
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	usize delay = 5;

	float framerate[MAX_FRCOUNT];

	bool fired = false;

	void createAttacks() {
		if (fired) return;
		fired = true;
		DEBUGLN("Creating shots...");
		for (usize i = 0; i < 16; ++i) {
			auto laser = server.acquire().as<Danmaku::Laser>();
			if (!laser) return;
			float const crot = (TAU / 16) * i;
			laser->trans.position = playfield.center;
			//laser->velocity.value = 30;
			laser->rotation = {
				crot,
				true,
				0,
				crot + static_cast<float>(TAU),
				.005,
				Makai::Math::Ease::InOut::back
			};
			laser->length = {32};
			laser->spawn();
			laser->spawnTime	= 60;
			laser->toggleTime	= 60;
			laser->onAction = [=] (auto& object, auto action) {
				Danmaku::Laser& l = (Danmaku::Laser&)object;
				if (action == Danmaku::AServerObject::Action::SOA_SPAWN_END) {
					l.toggle(true);
					l.onAction.clear();
				}
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
			if (delay > 0) --delay;
			else delay = 5;
			if (!delay) createAttacks();
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