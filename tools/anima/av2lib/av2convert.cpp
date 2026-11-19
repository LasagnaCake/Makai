#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ConvertLib: ALibrary {
	void load(Context::Adder const& adder) override {
	}

	CSpan<char> name() const override {
		static auto const cname[] = "av2/convert";
		return {cname, cname + sizeof(cname)};
	}
};

AV2_Library(ConvertLib);
