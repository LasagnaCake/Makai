#include <makai/makai.hpp>

struct TestApp: Makai::App {
	TestApp(): Makai::App(Makai::Config::App{{{600, 400}, "Test 05", false}}) {
		DEBUGLN("Loading shaders...");
		loadDefaultShaders();
	}
	
	Makai::Graph::Renderable danceCube;

	Makai::Graph::Camera3D& camera = Makai::Graph::Global::camera;

	virtual ~TestApp() {}

	void onOpen() override {
		DEBUGLN("1...");
		loadDefaultShaders();
		DEBUGLN("2...");
		danceCube.extendFromDefinitionFile("../tests/makai/files/dancing-cube.mrod");
		DEBUGLN("Done!");
	}

	void animateCube() {
		float as, ac;
		Makai::Math::absincos<float>((getCurrentCycle() / 15.0), as, ac);
		danceCube.armature.pose[1].rotation.x = -ac * 0.2 * Makai::Math::Constants::PI;
		danceCube.armature.pose[2].rotation.x = ac * 0.2 * Makai::Math::Constants::PI;
		danceCube.armature.pose[3].rotation.x = as * 0.2 * Makai::Math::Constants::PI;
		danceCube.armature.pose[0].scale.y = 0.8 + ac * 0.2;
	}

	void onUpdate(float delta) override {
		float as, ac;
		Makai::Math::sincos<float>((getCurrentCycle() / 180.0), as, ac);
		camera.eye	= Makai::Vec3(as, 0.1, ac) * 5.0;
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
