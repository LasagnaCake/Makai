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

void Context::debugExternalFunction(bool const isExiting) {
	if (isExiting) {
		MAKAILIB_DEBUGLN_ALL("<<<<<<<<<<<<<<<<<<<<<< Destroying external function...");
	} else {
		MAKAILIB_DEBUGLN_ALL(">>>>>>>>>>>>>>>>>>>>>> Creating external function...");
	}
}

Context::~Context()				{unloadLibraries();						}
Context::Library::~Library()	{if (impl) delete impl; impl = nullptr;	}
Context::Library::Impl::~Impl()	{close();								}

bool Context::openLibrary(Makai::String const& path) {
	if (dynlibs.contains(path)) return true;
	MAKAILIB_DEBUGLN_ALL("Fetching library...");
	Instance<Library> lib = lib.create();
	if (!lib->impl->open(path, *this))
		return false;
	toBeLoaded.pushBack(lib->impl->lib.reference());
	loadedLibraries.pushBack(lib);
	dynlibs[path] = lib;
	return true;
}

void Context::Library::Impl::close() {
	if(!lib) return;
	lib->close();
	lib.unbind();
}

void Context::loadLibraries() {
	MAKAILIB_DEBUGLN_ALL("Loading libraries...");
	for (auto& lib: toBeLoaded) {
		MAKAILIB_DEBUGLN_ALL("<", lib->name(), ">");
		lib->load(*this);
		MAKAILIB_DEBUGLN_ALL("</", lib->name(), ">");
	}
	toBeLoaded.clear();
	MAKAILIB_DEBUGLN_ALL("Done loading libraries!");
}

void Context::unloadLibraries() {
	for (auto& [name, lib]: dynlibs)
		lib->impl->lib->unload(*this);
	dynlibs.clear();
	loadedLibraries.clear();
}

Context::MethodAdder::~MethodAdder()		{}
Context::MethodRemover::~MethodRemover()	{}
Context::TypeAdder::~TypeAdder()			{}
Context::TypeRemover::~TypeRemover()		{}

bool Context::MethodAdder::add(usize const hash, usize const argc, ExternalInvocation const& invoker) const {
	return context.addNativeCall(hash, argc, invoker);
}

Nullable<Context::Error> Context::NativeCall::validate(Context& context, List<Object::Storage> const& args)  {
	if (retType && context.types.byNameHash(retType->hash).empty())
		return Error::AV2_CCE_MISSING_ART_TYPE;
	if (args.size() < argc)
		return Error::AV2_CCE_MISSING_ARGS;
	MAKAILIB_DEBUGLN_FULL("Validating method...");
	Context::debugArgs(args);
	return null;
}

bool Context::addNativeCall(usize const hash, usize const argc, ExternalInvocation const& invoker) {
	MAKAILIB_DEBUGLN_ALL("Adding method [", hash, "]");
	if (hasNativeCall(hash)) {
		MAKAILIB_DEBUGLN_ALL("WARN: [", hash, "] duplicate found");
		return false;
	}
	MAKAILIB_DEBUGLN_ALL("OK: [", hash, "] has no duplicates");
	Instance<NativeCall> method = method.create();
	method->flags.isExternal = true;
	method->argc	= argc;
	method->hash	= hash;
	method->invoker	= invoker;
	loadedMethods.pushBack(method);
	externalMethods[hash] = method;
	if (!hasNativeCall(hash))
		throw Makai::Error::FailedAction(
			"Failed to add external function ["+ toString(hash) + "]!",
			CTL_CPP_PRETTY_SOURCE
		);
	return true;
}

Context::MethodResult Context::callNative(usize const hash, List<Object::Storage> const& args) {
	MAKAILIB_DEBUG_BLOCK_FULL {
		MAKAILIB_DEBUGLN_FULL("Looking for method ", hash, "...");
		for (auto& m: externalMethods)
			MAKAILIB_DEBUGLN_ALL("  > ", m.key);
	}
	MAKAILIB_DEBUG_BLOCK_FULL {
		MAKAILIB_DEBUGLN_ALL("Method exists? ", hasNativeCall(hash));
		MAKAILIB_DEBUGLN_ALL("Registered? ", externalMethods.contains(hash));
		if (externalMethods.contains(hash)) {
			MAKAILIB_DEBUGLN_ALL("Created? ", externalMethods[hash].exists());
			if (externalMethods[hash].exists())
				MAKAILIB_DEBUGLN_ALL("Invoker? ", externalMethods[hash]->invoker);
		}
	}
	if (!hasNativeCall(hash)) return Error::AV2_CCE_MISSING_METHOD;
	MAKAILIB_DEBUGLN_FULL("!!! Method exists !!!");
	MAKAILIB_DEBUGLN_FULL("Invoker? ", externalMethods[hash]->invoker);
	if (!externalMethods[hash]->invoker) return Error::AV2_CCE_MISSING_INVOKER;
	if (auto err = externalMethods[hash]->validate(*this, args))
		return *err;
	return externalMethods[hash]->invoker->invoke(*this, *externalMethods[hash], args).value();
}

bool Context::Library::Impl::open(Makai::String const& path, Context& context) {
	if (!Makai::OS::FS::exists(path)) return false;
	MAKAILIB_DEBUGLN_ALL("Opening library...");
	dll.open(path);
	MAKAILIB_DEBUGLN_ALL("Getting entrypoint...");
	auto const fn = dll.function<owner<ALibrary>()>("AV2_Extern_getLibrary");
	lib = fn();
	if (!lib) return false;
	MAKAILIB_DEBUGLN_ALL("<library>");
	MAKAILIB_DEBUGLN_ALL("<name>", lib->name(), "</name>");
	MAKAILIB_DEBUGLN_ALL("<version>", lib->version().serialize().toFLOWString(), "</version>");
	MAKAILIB_DEBUGLN_ALL("</library>");
	lib->open();
	return true;
}
