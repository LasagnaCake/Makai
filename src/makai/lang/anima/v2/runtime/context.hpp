#ifndef MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H
#define MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H

#include "../../../../compat/ctl.hpp"
#include "../core/instruction.hpp"
#include "program.hpp"

#define ANIMA_V2_SHARED_FN_NAME_PREFIX "anima/env/share/"

namespace Makai::Anima::V2::Runtime {
	struct Context {
		using Storage = Instance<Data::Value>;

		struct IInvokable {
			virtual ~IInvokable() {}

			virtual Storage invoke(List<Storage> const& args) = 0;
		};

		struct Pointers {
			usize	offset		= 0;
			usize	function	= 0;
			usize	instruction	= -1;
		};

		struct Scope {
			List<Storage>		localStack;
			Core::ContextMode	mode		= Core::ContextMode::AV2_CM_STRICT;
			Core::ContextMode	prevMode	= Core::ContextMode::AV2_CM_STRICT;
		};

		using VariableBank = Map<uint64, Data::Value>;

		Pointers			pointers;
		List<Storage>		globalValueStack;
		List<Pointers>		pointerStack;
		List<Scope>			scopeStack;
		Map<usize, Storage>	globals;
		Storage				temporary = Storage::create();

		struct SharedSpace {
			using Function	= Instance<IInvokable>;

			struct Namespace {
				Dictionary<Function>	functions;
			};

			using LibraryCall = void(Namespace*);

			Dictionary<Namespace>		ns;
			Dictionary<CPP::Library>	libraries;

			~SharedSpace() {
				for (auto& [name, lib] : libraries) {
					auto const exit = lib.function<LibraryCall>(toString(ANIMA_V2_SHARED_FN_NAME_PREFIX) + "/v2/exit");
					if (exit) exit(&ns[name]);
				}
			}

			void addLibrary(String const& name, String const& libpath) {
				if (!libraries.contains(name))
					libraries[name].open(libpath);
				auto const init = libraries[name].function<LibraryCall>(toString(ANIMA_V2_SHARED_FN_NAME_PREFIX) + "/v2/init");
				if (init) init(&ns[name]);
			}

			Function fetch(String const& lib, String const& fname) {
				if (libraries.contains(lib) && ns[lib].functions.contains(fname))
					return ns[lib].functions[fname];
				return nullptr;
			}

			bool has(String const& lib, String const& fname) {
				return libraries.contains(lib) && ns[lib].functions.contains(fname);
			}
		} shared;

		void prepare(Program const& program) {
			for (auto const& [name, path]: program.ani.shared) {
				if (OS::FS::exists(path))
					return shared.addLibrary(name, path);
				String spath = OS::FS::sourceLocation() + "/" + path;
				if (OS::FS::exists(spath))
					return shared.addLibrary(name, spath);
				throw Error::FailedAction(
					"Failed to load library \"" +name + "\"!",
					"Library does not exist at the given path |" + path + "|",
					CTL_CPP_PRETTY_SOURCE
				);
			}
		}
	};
}

#define ANIMA_V2_SHARED_LIB_DECL extern "C" __stdcall
#define ANIMA_V2_SHARED_LIB_CALL(NAME, EXPORT)\
	ANIMA_V2_SHARED_LIB_DECL void NAME(Makai::Anima::V2::Runtime::Context::SharedSpace::Library*) asm(ANIMA_V2_SHARED_FN_NAME_PREFIX EXPORT);\
	ANIMA_V2_SHARED_LIB_DECL void NAME(Makai::Anima::V2::Runtime::Context::SharedSpace::Library*)

#define ANIMA_V2_SHARED_INIT ANIMA_V2_SHARED_LIB_CALL(mk_av2_shared_entryPoint, "v2/init")
#define ANIMA_V2_SHARED_EXIT ANIMA_V2_SHARED_LIB_CALL(mk_av2_shared_exitPoint, "v2/exit")

#endif
