#include "context.hpp"
#include "dynlib.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

struct Context::Library::Impl {
	Instance<ALibrary>	lib;
	CPP::Library		dll;

	~Impl();

	bool open(String const& path, Context& context);

	void close();
};

Context::Library::Library(): impl(new Context::Library::Impl()) {
}

Instance<OutputStringWriter> Context::writer = new OutputStringWriter();

void Context::debugArgs(List<Object::Storage> const& args) {
	//CPP::Debug::breakpoint();
	writer->writeLine("Argc: ", args.size());
	for (auto& arg: args)
	 	writer->writeLine("Argument: ", arg->toDynamicValue().toFLOWString());
	//CPP::Debug::breakpoint();
}

Context::~Context()				{unloadLibraries();	}
Context::Library::~Library()	{delete impl;		}
Context::Library::Impl::~Impl()	{close();			}

bool Context::openLibrary(Makai::String const& path) {
	if (dynlibs.contains(path)) return true;
	DEBUGLN("Fetching library...");
	Instance<Library> lib = lib.create();
	if (!lib->impl->open(path, *this))
		return false;
	toBeLoaded.pushBack(lib->impl->lib.reference());
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

Nullable<Context::Error> Context::ExternalMethod::validate(Context& context, List<Object::Storage> const& args)  {
	if (retType && context.types.byNameHash(retType->hash).empty())
		return Error::AV2_CCE_MISSING_ART_TYPE;
	if (args.size() < argc)
		return Error::AV2_CCE_MISSING_ARGS;
	DEBUGLN("Validating method...");
	Context::debugArgs(args);
	return null;
}

bool Context::addExternalMethod(usize const hash, usize const argc, ExternalInvocation const& invoker) {
	if (hasExternalMethod(hash)) return false;
	DEBUGLN("Adding method [", hash, "]...");
	ExternalMethod method;
	method.out		= true;
	method.argc		= argc;
	method.hash		= hash;
	method.invoker	= invoker;
	externalMethods[hash] = method;
	return true;
}

Context::MethodResult Context::invokeExternalMethod(usize const hash, List<Object::Storage> const& args) {
	DEBUGLN("Looking for method ", hash, "...");
	for (auto& m: externalMethods)
		DEBUGLN("  > ", m.key);
	if (!hasExternalMethod(hash)) return Error::AV2_CCE_MISSING_METHOD;
	DEBUGLN("!!! Method exists !!!");
	DEBUGLN("Invoker? ", externalMethods[hash].invoker.exists());
	if (!externalMethods[hash].invoker) return Error::AV2_CCE_MISSING_INVOKER;
	if (auto err = externalMethods[hash].validate(*this, args))
		return *err;
	return externalMethods[hash].invoker->invoke(*this, externalMethods[hash], args).value();
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
