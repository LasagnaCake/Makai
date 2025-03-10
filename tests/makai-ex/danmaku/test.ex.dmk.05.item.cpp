#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), (gamearea)},
	playfield	= {gamearea * Makai::Vector2(1, -1), (gamearea * 1.5)}
;

using BaseItemServer = Danmaku::ItemServer<>;

struct MeshHolder {
	MkGraph::Renderable m, gm;

	MeshHolder() {
		m.setRenderLayer(Danmaku::Render::Layer::PLAYER1_ITEM_LAYER);
		gm.setRenderLayer(Danmaku::Render::Layer::PLAYER1_ITEM_LAYER+1);
		gm.setBlendEquation(Makai::Graph::BlendEquation::BE_ADD);
	}
};

struct TestItemServer: MeshHolder, BaseItemServer {
	TestItemServer(): MeshHolder(), BaseItemServer({256, m, gm, ::board, ::playfield}) {}
};


struct TestApp: Makai::Ex::Game::App {
	TestItemServer server;

	TestApp(): App(Makai::Config::App{{800, 600, "Test 05", false}}) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(64, Makai::Vector2(4, 3) / 3.0);
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::Render::Layer::BOSS1_SPELL_BG_BOTTOM_LAYER);
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	float framerate[MAX_FRCOUNT];

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