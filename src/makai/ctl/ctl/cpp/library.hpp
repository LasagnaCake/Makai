#ifndef CTL_CPP_LIBRARY_H
#define CTL_CPP_LIBRARY_H

#include "../namespace.hpp"
#include "../container/strings/strings.hpp"
#include "../container/pointer/pointer.hpp"

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
			Module (String const& path)					{lib = LoadLibrary(path.cstr());					}
			~Module()									{if (lib) FreeLibrary(lib);							}
			pointer function(String const& name) const	{return (pointer)GetProcAddress(lib, name.cstr());	}
		private:
			HMODULE lib = nullptr;
		#else
		#endif
		};

		Library()						{							}

		Library(String const& path)		{open(path);				}

		void open(String const& path)	{lib = new Module(path);	}
		void close()					{lib.unbind();				}

		template <Type::Function T>
		ref<T> function(String const& name) {
			if (!lib) return nullptr;
			return (ref<T>)lib->function(name);
		}
	
	private:
		Instance<Module> lib;
	};
}

CTL_NAMESPACE_END

#endif