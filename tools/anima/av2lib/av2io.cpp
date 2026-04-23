#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct IOLib: ILibrary {
	void open() {

	}

	void load(Context::MethodAdder const& context) {

	}

	void unload(Context::MethodRemover const& context) {

	}

	void close() {

	}
};

AV2_Library(IOLib);
