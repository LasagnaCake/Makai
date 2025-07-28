#include <makai/makai.hpp>

struct TestApp: Makai::App {
	Makai::Instance<Makai::Audio::Engine::Sound> testSound;
	
	TestApp(): Makai::App(Makai::Config::App{{{600, 400}, "Test 05", false}}) {
		testSound = audio.createSound(Makai::File::getBinary("../tests/makai/files/spell_old.wav"));
		if (!testSound) throw Makai::Error::FailedAction("Failed to create sound!");
	}

	virtual ~TestApp() {}

	void onOpen() override {
		if (testSound) testSound->play();
	}

	void onUpdate(float delta) override {}
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
