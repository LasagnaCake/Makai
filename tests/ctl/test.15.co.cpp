#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>
#include <ctl/async/co/co.hpp>

CTL::Random::SecureGenerator rng;

struct Test: CTL::Co::IYieldable {
	int value = 0;

	using IYieldable::IYieldable;

	void run() override {
		value = rng.number<usize>(0, 50);
		yield(rng.number<usize>(20, 40));
		value = rng.number<usize>(0, 50);
		yield(rng.number<usize>(40, 80));
		value = rng.number<usize>(0, 50);
		yield(rng.number<usize>(80, 120));
		value = -1;
	}
};

CTL::Co::Context ctx;

void execute() {
	static usize id = 0;
	usize wait = rng.number<usize>(1, 20);
	usize self = ++id;
	for (usize i = 0; i < 5; ++i) {
		DEBUGLN("ID: ",self,", Value: ", rng.number<usize>(0, 50));
		while (wait-- > 0) ctx.yield();
		wait = rng.number<usize>(10, 20);
	}
	DEBUGLN("ID: ",self,", Done!");
	return;
}

void testContext() {
	for (int i = 0; i < 10; ++i)
		DEBUGLN("Spawned: ", ctx.spawn(execute) - 1);
	ctx.join();
}

void testAsync() {
	Test t1(ctx), t2(ctx);
	ctx.yield();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
	ctx.waitForNext();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
	ctx.yield();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
	ctx.waitForNext();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
	ctx.yield();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
	ctx.waitForNext();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
	ctx.yield();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
	ctx.join();
	DEBUGLN("Value: [ ", t1.value, ", ", t2.value, " ]");
}

int main() {
	testContext();
	testAsync();
	DEBUGLN("Coroutine tests passed!");
	return 0;
}