#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct SystemLib: ILibrary {
	void load(Context::Adder const& context) override {
	}

	String name() const override {return "av2/system";}
};

AV2_Library(SystemLib);
