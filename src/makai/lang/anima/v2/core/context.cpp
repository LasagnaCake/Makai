#include "context.hpp"
#include "dynlib.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

struct Context::Library::Impl {
	Reference<ALibrary>	lib;
	CPP::Library		dll;

	~Impl();

	bool open(String const& path, Context& context);

	void close();
};

Context::Library::Library(): impl(new Context::Library::Impl()) {
}


Context::~Context()				{unloadLibraries();	}
Context::Library::~Library()	{delete impl;		}

bool Context::openLibrary(Makai::String const& path) {
	if (dynlibs.contains(path)) return true;
	DEBUGLN("Fetching library...");
	Instance<Library> lib = lib.create();
	if (!lib->impl->open(path, *this))
		return false;
	toBeLoaded.pushBack(lib->impl->lib);
	dynlibs[path] = lib;
	return true;
}

void Context::Library::Impl::close() {
	if(!lib) return;
	lib->close();
}

void Context::loadLibraries() {
	for (auto& lib: toBeLoaded)
		lib->load(*this);
	toBeLoaded.clear();
}

void Context::unloadLibraries() {
	for (auto& [name, lib]: dynlibs)
		lib->impl->lib->unload(*this);
	dynlibs.clear();
}

bool Context::Library::Impl::open(Makai::String const& path, Context& context) {
	if (!Makai::OS::FS::exists(path)) return false;
	DEBUGLN("Opening library...");
	dll.open(path);
	DEBUGLN("Getting entrypoint...");
	auto const fn = dll.function<owner<ALibrary>()>("AV2_Extern_getLibrary");
	lib = fn();
	if (!lib) return false;
	DEBUGLN("<library>");
	DEBUGLN("<name>", lib->name(), "</name>");
	DEBUGLN("<version>", lib->version().serialize().toFLOWString(), "</version>");
	DEBUGLN("</library>");
	lib->open();
	return true;
}
