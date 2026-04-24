#include "context.hpp"
#include "dynlib.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

Context::Library::~Library() {close();}

bool Context::loadLibrary(Makai::String const& path) {
	auto const lib = Library::open(path, *this);
	if (!lib) return false;
	dynlibs.pushBack(*lib);
	return true;
}
