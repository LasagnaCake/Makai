#include <makai/makai.hpp>
#include <makai-ex/game/dialog/dialog.hpp>

struct TestActor: Makai::Ex::Game::Dialog::Actor {
	TestActor(Makai::String const& name): Actor(new Makai::Ex::Game::Dialog::Box()) {
		dialog->setTitle({name});
	}
};

struct TestScene: Makai::Ex::Game::Dialog::Scene {
	Makai::Instance<TestActor> actors[3] = {
		new TestActor("Alice"),
		new TestActor("Bob"),
		new TestActor("Charlie")
	};

	TestScene(): Scene() {
		dialog = new Makai::Ex::Game::Dialog::Box();
		choice = new Makai::Ex::Game::Dialog::ChoiceMenu();
		dialog->setTitle({"Society"});
		cast[Makai::ConstHasher::hash("alice")]		= actors[0].asWeak();
		cast[Makai::ConstHasher::hash("bob")]		= actors[1].asWeak();
		cast[Makai::ConstHasher::hash("charlie")]	= actors[2].asWeak();
	}
};

struct TestApp: Makai::Ex::Game::App {
	TestScene scene;

	Makai::Ex::Game::Dialog::ScenePlayer player;

	TestApp(): App(Makai::Config::App{{800, 600, "Test 02", false}}), player(scene) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(64, Makai::Vector2(4, 3) / 3.0);
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(true);
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