#include <makai/makai.hpp>

int main() {
	DEBUGLN("Running app ", __FILE__, "...");
	try {
		
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.what());
	}
	return 0;
}
