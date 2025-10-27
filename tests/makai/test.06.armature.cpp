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
		danceCube.material.texture = {true, Makai::Graph::Texture2D("../tests/makai/files/grid.png"), 0};
		danceCube.material.culling = Makai::Graph::CullMode::OCM_FRONT;
		DEBUGLN("Done!");
		danceCube.setRenderLayer(0);
	}

	void animateCube() {
		float as, ac;
		Makai::Math::sincos<float>((getCurrentCycle() / 30.0), as, ac);
		danceCube.armature["Horiz"]->rotation.x = -ac * 0.2 * Makai::Math::Constants::PI;
		danceCube.armature["Leaf"]->rotation.x = ac * 0.2 * Makai::Math::Constants::PI;
		danceCube.armature["Diag"]->rotation.x = as * 0.2 * Makai::Math::Constants::PI;
		danceCube.armature["Vert"]->scale.y = 0.75 + ac * 0.25;
	}

	void onUpdate(float delta) override {
		if (input.isButtonJustPressed(Makai::Input::KeyCode::KC_ESCAPE))
			close();
		getFrameBuffer().material.background = Makai::Graph::Color::WHITE * (sin(getCurrentFrame() / 180.0) / 2 + .5);
		float as, ac;
		Makai::Math::sincos<float>((getCurrentCycle() / 180.0), as, ac);
		camera.eye	= Makai::Vec3(as, 0.25, ac) * 5.0;
		animateCube();
	}
};

int main() {
	DEBUGLN("Running app ", __FILE__, "...");
	try {
		TestApp app;
		app.run();
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.report());
	}
	return 0;
}
