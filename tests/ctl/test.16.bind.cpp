#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

int fun(int n1, int n2, int n3, int n4, int n5) {
	return n1 - n2 + n3 - n4 + n5;
}

int main() {
	auto f1 = CTL::bind(fun, CTL::arg<1>, CTL::arg<2>, 3, 4, CTL::arg<3>);
	auto f2 = CTL::bind(fun, CTL::arg<1>, CTL::arg<1>, 3, 4, CTL::arg<1>);
	auto f3 = CTL::bind(fun, CTL::arg<3>, CTL::arg<2>, 3, 4, CTL::arg<1>);
	DEBUGLN(f1(1, 2, 5));
	DEBUGLN(f2(1));
	DEBUGLN(f3(1, 2, 5));
	DEBUGLN("Bind tests passed!");
	return 0;
}