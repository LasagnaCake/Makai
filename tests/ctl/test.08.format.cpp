#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

using CTL::String;

void testStringFormatting() {
	DEBUGLN(CTL::Format::pad<String>("abc", '0', 8, CTL::Format::Justify::CFJ_LEFT));
	DEBUGLN(CTL::Format::pad<String>("abc", '0', 8, CTL::Format::Justify::CFJ_RIGHT));
	DEBUGLN(CTL::Format::pad<String>("abcd", '0', 8, CTL::Format::Justify::CFJ_CENTER));
	DEBUGLN(CTL::Format::prettify(148.34, 4, 4));
	DEBUGLN(CTL::Format::prettify(148.34, 2, 4));
	DEBUGLN(CTL::Format::prettify(148.34, 4, 2));
	DEBUGLN(CTL::Format::prettify(148.34, 2, 2));
	DEBUGLN(CTL::Format::prettify(148.34, 5, 5));
	DEBUGLN(CTL::Format::prettify(148.34, 8, 5));
	DEBUGLN(CTL::Format::prettify(148.34, 5, 8));
	DEBUGLN(CTL::Format::prettify(148.34, 8, 8));
}

int main() {
	testStringFormatting();
	DEBUGLN("String formatting test passed!");
	return 0;
}
