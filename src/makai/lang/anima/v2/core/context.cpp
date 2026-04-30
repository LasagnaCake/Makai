#include "context.hpp"
#include "dynlib.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

Context::~Context()				{unloadLibraries();	}
Context::Library::~Library()	{close();			}

bool Context::openLibrary(Makai::String const& path) {
	if (dynlibs.contains(path)) return true;
	auto const lib = Library::open(path, *this);
	if (!lib) return false;
	dynlibs[path] = *lib;
	return true;
}

void Context::Library::close() {
	if(!impl) return;
	impl->close();
}

void Context::loadLibraries() {
	for (auto& lib: toBeLoaded)
		lib->load(*this, *this);
	toBeLoaded.clear();
}

void Context::unloadLibraries() {
	for (auto& [name, lib]: dynlibs)
		lib.impl->unload(*this, *this);
	dynlibs.clear();
}

Makai::Nullable<Context::Library> Context::Library::open(Makai::String const& path, Context& context) {
	if (!Makai::OS::FS::exists(path)) return null;
	Library lib;
	lib.dll.open(path);
	auto const fn = lib.dll.function<owner<ILibrary>()>("AV2_Extern_getLibrary");
	lib.impl = fn();
	if (!lib.impl) return null;
	lib.impl->open();
	return lib;
}
