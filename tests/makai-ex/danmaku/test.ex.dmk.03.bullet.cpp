#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), gamearea},
	playfield	= {gamearea * Makai::Vector2(1, -1), gamearea}
;

using BaseBulletServer = Danmaku::BulletServer<>;

struct TestBulletServer: BaseBulletServer {
	MkGraph::Renderable mesh, glowMesh;
	
	TestBulletServer(): BaseBulletServer({1024, mesh, glowMesh, board, playfield}) {}
};

struct TestApp: Makai::Ex::Game::App {
	TestBulletServer server;

	TestApp(): App(Makai::Config::App{{800, 600, "Test 03", false}}) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(32, Makai::Vector2(4, 3));
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::RenderLayer::BOSS1_SPELL_BG_BOTTOM_LAYER);
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
	return 0;
}