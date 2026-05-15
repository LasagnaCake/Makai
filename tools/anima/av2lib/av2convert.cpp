#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ConvertLib: ALibrary {
	void load(Context::Adder const& adder) override {
	}

	String name() const override {return "av2/convert";}
};

AV2_Library(ConvertLib);
