#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

using CTL::List;

template<typename T>
void print(List<T> const& lst) {
	DEBUG(
		"S: ",
		lst.size(),
		", C: ",
		lst.capacity(),
		", I: [ "
	);
	for (auto& i: lst) {
		DEBUG(i, " ");
	}
	DEBUGLN("];");
}

List<int> testList() {
	List<int> lst;
	print(lst);
	lst.pushBack(69);							print(lst);
	lst.appendBack({4, 420, 8, 6, 4, 8, 8, 4});	print(lst);
	lst.popBack();								print(lst);
	lst.eraseIf([](auto e){return e > 10;});	print(lst);
	lst.eraseLike(8);							print(lst);
	lst.eraseLike(4);							print(lst);
	lst.reserve(48);							print(lst);
	lst.appendBack({-5, -20});					print(lst);
	lst.insert({-5, 9, 9, 8, 9, -20, 4, 8}, 1);	print(lst);
	lst = lst.uniques();						print(lst);
	lst.reverse();								print(lst);
	DEBUG("![Sorted] ");
	lst.sort();									print(lst);
	lst = List<int>({20, -64, -20, 20, 0, -1, 17, -5, 1, 28, 34, 6, 4, -1, 1, -4, 57, 8, -19, 20, 19, 8});
	print(lst);
	DEBUG("![Sorted] ");
	lst.sort();									print(lst);
	lst.eraseLike(20);							print(lst);
	lst = lst.sliced(3, 11);					print(lst);
	lst = lst.sliced(1, -2);					print(lst);
	DEBUGLN("Exiting test function...");
	return lst;
}

int main() {
	DEBUGLN("List test passed!");
	List<int> lst = testList();
	print(lst);
	lst = lst.sliced(3, 11);	print(lst);
	lst = lst.sliced(1, -2);	print(lst);
	lst.resize(10, 0);			print(lst);
	lst.clear();				print(lst);
	lst.dispose();				print(lst);
	return 0;
}
