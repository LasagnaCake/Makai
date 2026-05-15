#include "context.hpp"
#include "dynlib.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

Context::~Context()				{unloadLibraries();	}
Context::Library::~Library()	{close();			}

bool Context::openLibrary(Makai::String const& path) {
	if (dynlibs.contains(path)) return true;
	DEBUGLN("Fetching library...");
	Library lib;
	if (!lib.open(path, *this))
		return false;
	dynlibs[path] = lib;
	return true;
}

void Context::Library::close() {
	if(!impl) return;
	impl->close();
}

void Context::loadLibraries() {
	for (auto& lib: toBeLoaded)
		lib->load(*this);
	toBeLoaded.clear();
}

void Context::unloadLibraries() {
	for (auto& [name, lib]: dynlibs)
		lib.impl->unload(*this);
	dynlibs.clear();
}

bool Context::Library::open(Makai::String const& path, Context& context) {
	if (!Makai::OS::FS::exists(path)) return false;
	DEBUGLN("Opening library...");
	Library lib;
	lib.dll.open(path);
	DEBUGLN("Getting entrypoint...");
	auto const fn = lib.dll.function<owner<ALibrary>()>("AV2_Extern_getLibrary");
	lib.impl = fn();
	if (!lib.impl) return false;
	DEBUGLN("<library>");
	DEBUGLN("<name>", lib.impl->name(), "</name>");
	DEBUGLN("<version>", lib.impl->version().serialize().toFLOWString(), "</version>");
	DEBUGLN("</library>");
	lib.impl->open();
	return true;
}
