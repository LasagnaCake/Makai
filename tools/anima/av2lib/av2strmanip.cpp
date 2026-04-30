#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct StringManipLib: ILibrary {

	void load(Context::TypeAdder const& types, Context::MethodAdder const& methods) {
	}
};

AV2_Library(StringManipLib);
