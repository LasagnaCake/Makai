#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>
#include <ctl/async/co/co.hpp>

CTL::Random::SecureGenerator rng;

CTL::Co::Promise<usize, true> cofun() {
	for (usize i = 0; i < 10; ++i)
		co_yield i;
	co_return 100;
}

CTL::Co::Promise<bool> cofun2(int ci) {
	for (usize i = 0; i < 5; ++i) {
		DEBUGLN("Coroutine: ", ci, ", Cycle: ", i+1);
		auto yielder = CTL::Co::yield(rng.integer<usize>(1, 10));
		while (yielder.next())
			co_yield false;
	}
	DEBUGLN("Coroutine: ", ci, ", Done!!!");
	co_return true;
}

void testCoroutines() {
	auto p = cofun();
	while(p) {
		DEBUGLN("Value: ", p.next());
	}
}

void testYield() {
	CTL::Co::Promise<bool> promises[5] = {
		cofun2(0),
		cofun2(1),
		cofun2(2),
		cofun2(3),
		cofun2(4)
	};
	bool processing = true;
	while (processing) {
		processing = false;
		for (auto& p: promises)
			if (p) {
				p.process();
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