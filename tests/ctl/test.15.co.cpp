#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>
#include <ctl/async/co/co.hpp>

CTL::Random::SecureGenerator rng;

CTL::Co::Promise<usize, true> cofun() {
	for (usize i = 0; i < 10; ++i)
		co_yield i;
	co_return 100;
}

void testCoroutines() {
	auto p = cofun();
	while(p) {
		DEBUGLN("Value: ", p.next());
	}
}

int main() {
	testCoroutines();
	DEBUGLN("Coroutine tests passed!");
	return 0;
}