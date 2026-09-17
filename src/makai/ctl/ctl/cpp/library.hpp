#ifndef CTL_CPP_LIBRARY_H
#define CTL_CPP_LIBRARY_H

#include "../namespace.hpp"
#include "../container/strings/strings.hpp"
#include "../container/pointer/atomiccell.hpp"
#include "../container/error.hpp"
#include "../container/nullable.hpp"
#include "../container/functor.hpp"
#include "../algorithm/strconv.hpp"
#include "sourcefile.hpp"

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <libloaderapi.h>
#else
#include <dlfcn.h>
#endif

CTL_NAMESPACE_BEGIN

namespace CPP {
	struct Library {
		struct Module {
		#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
			Module (String const& path) {
				lib = LoadLibrary(path.cstr());
				if (!lib) throw Error::FailedAction(
					"Failed to load library '"+path+"'!",
					toString("ERROR CODE ", GetLastError()),
					CTL_CPP_PRETTY_SOURCE
				);
			}
			~Module()									{if (lib) FreeLibrary(lib);							}
			pointer function(String const& name) const	{return (pointer)GetProcAddress(lib, name.cstr());	}
		private:
			HMODULE lib = nullptr;
		#else
		Module (String const& path) {
			lib = dlopen(path.cstr(), RTLD_LAZY | RTLD_LOCAL);
			if (!lib) throw Error::FailedAction(
				"Failed to load library '"+path+"'!",
				dlerror(),
				CTL_CPP_PRETTY_SOURCE
			);
		}

		~Module()									{if (lib) dlclose(lib);						}
		pointer function(String const& name) const	{return (pointer)dlsym(lib, name.cstr());	}
		private:
			pointer lib = nullptr;
		#endif
		};

		template <class T>
		struct Function;

		template <class TReturn, class... TArgs>
		struct Function<TReturn(TArgs...)> {
			using Result = Meta::If<Type::Void<TReturn>, TReturn, Nullable<TReturn>>;

			inline Result invoke(TArgs... args) const {
				return func(args...);
			}

			inline Result operator()(TArgs... args) const {
				return invoke(args...);
			}

			inline constexpr Function() noexcept {}

			inline constexpr operator bool() const noexcept {return func.exists();}

		private:
			friend struct Library;
			Function(ref<TReturn(TArgs...)> const func, AtomicCell<Module> const& lib, String const& name):
				func(*func),
				lib(lib),
				name(name) {}
			::CTL::Functor<TReturn(TArgs...)>	func;
			AtomicCell<Module>					lib;
			String								name;
		};

		Library()						{							}

		Library(String const& path)		{open(path);				}

		void open(String const& path)	{lib = lib.create(path);	}
		void close()					{lib = null;				}

		template <Type::Function T>
		Function<T> function(String const& name) {
			if (!lib) return {};
			ref<T> const f = (ref<T>)lib->function(name);
			if (!f) return {};
			return {*f, lib, name};
		}

		template <Type::Function T>
		ref<T> functionRef(String const& name) {
			if (!lib) return {};
			return (ref<T>)lib->function(name);
		}

		template <class T>
		ref<T(...)> variadicFunctionRef(String const& name) {
			if (!lib) return {};
			return (ref<T(...)>)lib->function(name);
		}

	private:
		AtomicCell<Module> lib;
	};
}

CTL_NAMESPACE_END

#endif
