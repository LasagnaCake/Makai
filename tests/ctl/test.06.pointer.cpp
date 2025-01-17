#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

using CTL::Instance, CTL::Handle, CTL::Unique, CTL::Reference;

template<class T>
void print(Handle<T> const& p) {
	DEBUG("?: ", p.exists(), ",\tI: ", p.count(), ",\tV: ");
	if (p)	DEBUGLN(*p);
	else	DEBUGLN("Oops!");
}

template<class T>
void print2(Instance<T> p) {
	if (p.raw())
		throw CTL::DebugCrash<__LINE__>();
	print<int>(p);
}

template<class T>
void f1(Instance<T> const& p) {
	print<T>(p);
}

template<class T>
void f2(Instance<T> p) {
	print<T>(p);
}

template<class T>
void f3(Instance<T> p1, Instance<T> const& p2, Instance<T> p3) {
	print<T>(p2);
	p1.unbind();
	print<T>(p3);
}

template<class T>
void f4(Instance<T> p1, Instance<T> const& p2, Instance<T> p3) {
	print<T>(p2);
	p1.destroy();
	print<T>(p3);
}

void testIntPointer() {
	Instance<int> ptr = new int(4);
	print<int>(ptr);
	f1(ptr);
	f2(ptr);
	f3(ptr, ptr, ptr);
	print<int>(ptr);
	f4(ptr, ptr, ptr);
	print2(ptr);
}

int main() {
	testIntPointer();
	DEBUGLN("Pointer test passed!");
	return 0;
}
