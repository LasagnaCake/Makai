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

void execute(CTL::Co::Context& ctx) {
	static usize id = 0;
	usize self = ++id;
	for (usize i = 0; i < 5; ++i) {
		DEBUGLN("ID: ",self,", Value: ", rng.number<usize>(0, 50), ", Cycle: ", i);
		ctx.yield();
	}
	DEBUGLN("ID: ",self,", Done!");
	return;
}

void testContext() {
	CTL::Co::Context ctx;
	for (int i = 0; i < 20; ++i)
		DEBUGLN("Spawned: ", ctx.spawn(execute) - 1);
	ctx.join();
	DEBUGLN("Main path done!");
}

void testAsync() {
	CTL::Co::Context ctx;
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
	DEBUGLN("Main path done!");
}

int main() {
	testContext();
	testAsync();
	DEBUGLN("Coroutine tests passed!");
	return 0;
}