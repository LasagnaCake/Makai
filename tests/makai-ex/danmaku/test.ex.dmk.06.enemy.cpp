#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), (gamearea)},
	playfield	= {gamearea * Makai::Vector2(1, -1), (gamearea * 1.5)}
;

struct TestEnemy;

using TestRegistry = Makai::Ex::Game::Registry<TestEnemy>;

struct TestEnemy: Danmaku::AEnemy, TestRegistry::Member {
	Makai::Graph::Renderable mesh;

	Makai::Ex::Game::SpriteInstance sprite;

	usize const offset;

	TestEnemy(usize const offset): AEnemy({::board, ::playfield}), offset(offset) {
		trans.position = playfield.center;
		sprite = mesh.createReference<Makai::Ex::Game::Sprite>();
		mesh.setRenderLayer(Danmaku::Render::Layer::ENEMY1_LAYER);
	}

	void onUpdate(float delta, Makai::App& app) override {
		if (!active) return;
		AEnemy::onUpdate(delta, app);
		if (paused()) return;
		trans.position.x = sin((app.getCurrentCycle() - offset) / 60.0) * 24 + playfield.center.x;
		mesh.trans.position		= trans.position;
		mesh.trans.rotation.z	= trans.rotation;
		mesh.trans.scale		= trans.scale;
	}

	virtual TestEnemy& spawn() override		{return *this;}
	virtual TestEnemy& despawn() override	{return *this;}

	void onDeath() override {
		queueDestroy();
	}
};

struct TestApp: Makai::Ex::Game::App {
	Makai::Random::SecureGenerator rng;

	TestApp(): App(Makai::Config::App{{800, 600, "Test 06", false}}) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(64, Makai::Vector2(4, 3) / 3.0);
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::Render::Layer::BOSS1_SPELL_BG_BOTTOM_LAYER);
	}

	void spawnEnemy() {
		enemies[current] = TestRegistry::create<TestEnemy>(getCurrentCycle());
		enemies[current]->trans.position.y = rng.number<float>(24, -24) + playfield.center.y;
		(++current) %= MAX_ENEMIES;
	}

	constexpr static usize MAX_FRCOUNT = 10;
	constexpr static usize MAX_ENEMIES = 10;

	usize frcount = 0;
	usize counter = 12;
	usize current = 0;

	float framerate[MAX_FRCOUNT];

	Makai::Instance<TestEnemy> enemies[MAX_ENEMIES];

	void onUpdate(float delta) {
		if (frcount < MAX_FRCOUNT)
			framerate[frcount++] = 1000.0 / getCycleDelta();
		else {
			if (counter > 0) --counter;
			else {
				counter = 12;
				spawnEnemy();
			}
			float fravg = 0;
			for(float& f: framerate) fravg += f;
			fravg *= (1.0 / (float)MAX_FRCOUNT);
			fravg = Makai::Math::clamp<float>(fravg, 0, maxFrameRate);
			DEBUGLN("Framerate: ", Makai::Format::prettify(Makai::Math::round(fravg, 2), 2, 0));
			frcount = 0;
		}
		TestRegistry::destroyQueued();
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