#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ConvertLib: ILibrary {
	void load(Context::MethodAdder const& context) {
	}
};

AV2_Library(ConvertLib);
