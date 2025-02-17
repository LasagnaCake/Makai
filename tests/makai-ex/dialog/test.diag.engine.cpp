#include <makai/makai.hpp>
#include <makai-ex/game/dialog/dialog.hpp>

struct TestActor: Makai::Ex::Game::Dialog::Actor {
	TestActor(): Actor(new Makai::Ex::Game::Dialog::Box()) {}
};

struct TestScene: Makai::Ex::Game::Dialog::Scene {
	Makai::Instance<TestActor> actors[3] = {
		new TestActor(),
		new TestActor(),
		new TestActor()
	};

	TestScene(): Scene() {
		dialog = new Makai::Ex::Game::Dialog::Box();
		choice = new Makai::Ex::Game::Dialog::ChoiceMenu();
		cast[Makai::ConstHasher::hash("alice")]		= actors[0].asWeak();
		cast[Makai::ConstHasher::hash("bob")]		= actors[1].asWeak();
		cast[Makai::ConstHasher::hash("charlie")]	= actors[2].asWeak();
	}
};

int main() {
	return 0;
}