#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

CTL::Random::SecureGenerator rng;

CTL::Co::Generator<usize> cofun() {
	for (usize i = 0; i < 10; ++i)
		co_yield i;
	co_return 100;
}

CTL::Co::Routine cofun2(int ci) {
	for (usize i = 0; i < 5; ++i) {
		usize const time = rng.integer<usize>(0, 10);
		DEBUGLN("Coroutine: ", ci, ", Cycle: ", i+1, ", Wait: ", time);
		co_await CTL::Co::yield(time);
	}
	DEBUGLN("Coroutine: ", ci, ", Done!!!");
	co_return;
}

void testCoroutines() {
	auto p = cofun();
	while(p) {
		DEBUGLN("Value: ", p.next());
	}
}

void testYield() {
	CTL::Co::Routine routines[5] = {
		cofun2(0),
		cofun2(1),
		cofun2(2),
		cofun2(3),
		cofun2(4)
	};
	bool processing = true;
	while (processing) {
		processing = false;
		for (auto& r: routines)
			if (r) {
				r.process();
				processing = true;
			}
	}
}

int main() {
	testCoroutines();
	testYield();
	DEBUGLN("Coroutine tests passed!");
	return 0;
}