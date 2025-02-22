#include <makai/makai.hpp>
#include <makai-ex/game/dialog/dialog.hpp>

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

struct TextBox: Makai::Ex::Game::Dialog::Box {
	TextBox(Makai::Graph::FontFace const& font): Box() {
		setTitle({""});
		setBody({""});
		body.text->rectAlign.x	=
		title.text->rectAlign.x	= 0.5;
		title.text->rect		= {80, 1};
		body.text->rect			= {80, 4};
		title.trans.position	= gamearea * Makai::Vector2(1, -1.5);
		body.trans.position		= title.trans.position + Makai::Vector2::DOWN() * 2;
		title.font				= font;
	}
};

struct TestActor: Makai::Ex::Game::Dialog::Actor {
	TestActor(Makai::String const& name, Makai::Graph::FontFace const& font): Actor(new TextBox(font)) {
		dialog->setTitle({name});
	}
};

struct TestScene: Makai::Ex::Game::Dialog::Scene {
	Makai::Instance<TestActor> actors[3];

	TestScene(Makai::Graph::FontFace const& font): Scene() {
		actors[0] =	new TestActor("Alice", font);
		actors[1] =	new TestActor("Bob", font);
		actors[2] =	new TestActor("Charlie", font);
		dialog = new TextBox(font);
		choice = new Makai::Ex::Game::Dialog::ChoiceMenu();
		dialog->setTitle({"Society"});
		dialog->setBody({""});
		cast[Makai::ConstHasher::hash("alice")]		= actors[0].asWeak();
		cast[Makai::ConstHasher::hash("bob")]		= actors[1].asWeak();
		cast[Makai::ConstHasher::hash("charlie")]	= actors[2].asWeak();
	}
};

struct TestApp: Makai::Ex::Game::App {
	TestScene scene;
	Makai::Graph::FontFace font;

	Makai::Ex::Game::Dialog::ScenePlayer player;

	TestApp(Makai::String const& path): App(Makai::Config::App{{800, 600, "Test 02", false}}), player(scene), scene(font) {
		font = Makai::Graph::FontData{Makai::Graph::Texture2D("../tests/makai/files/TestFontGrid-lotuscoder.png")};
		player.setProgram(path);
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(64, Makai::Vector2(4, 3) / 3.0);
		player.start();
		auto& keybinds = Makai::Input::Manager::binds;
		keybinds["dialog/next"]				= Makai::Input::KeyCode::KC_Z;
		keybinds["dialog/skip"]				= Makai::Input::KeyCode::KC_X;
		keybinds["dialog/choice/next"]		= Makai::Input::KeyCode::KC_UP;
		keybinds["dialog/choice/previous"]	= Makai::Input::KeyCode::KC_DOWN;
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(true);
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	float framerate[MAX_FRCOUNT];

	void onUpdate(float delta) {
		if (player.finished()) {
			player.scene.dialog->show();
			player.scene.dialog->title.text->content	= "DONE";
			player.scene.dialog->body.text->content		= "";
		}
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

int main(int argc, char** argv) {
	try {
		TestApp app(argv[1]);
		app.run();
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.what());
	}
	return 0;
}