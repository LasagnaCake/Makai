#include <makai/makai.hpp>

struct TestApp: Makai::App {
	TestApp(): Makai::App(Makai::Config::App{{600, 400, "Test 02", false}}) {}
};

int main() {
	DEBUGLN("Running app ", __FILE__, "...");
	try {
		TestApp app;
		app.run();
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.what());
	}
	return 0;
}
