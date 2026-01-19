#ifndef CTL_CPP_LIBRARY_H
#define CTL_CPP_LIBRARY_H

#include "../namespace.hpp"
#include "../container/strings/strings.hpp"
#include "../container/pointer/pointer.hpp"
#include "../container/error.hpp"
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
					CTL_CPP_PRETTY_SOURCE
				);
			}
			~Module()									{if (lib) FreeLibrary(lib);							}
			pointer function(String const& name) const	{return (pointer)GetProcAddress(lib, name.cstr());	}
		private:
			HMODULE lib = nullptr;
		#else
		#endif
		};
		
		template <class T>
		struct Function;

		template <class TReturn, class... TArgs>
		struct Function<TReturn(TArgs...)> {
			
			TReturn invoke(TArgs... args) const {
				if (lib)
					return func(args...);
				throw Error::FailedAction(
					"Function '" +name+ "' does not exist!",
					CTL_CPP_PRETTY_SOURCE
				);
			}

			TReturn operator()(TArgs... args) const	{
				return invoke(args...);
			}

			constexpr Function() noexcept {}

		private:
			friend struct Library;
			Function(ref<TReturn(TArgs...)> const func, Instance<Module> const& lib, String const& name):
				func(*func),
				lib(lib),
				name(name) {}
			::CTL::Function<TReturn(TArgs...)>	func;
			Instance<Module>					lib;
			String								name;
		};

		Library()						{							}

		Library(String const& path)		{open(path);				}

		void open(String const& path)	{lib = new Module(path);	}
		void close()					{lib.unbind();				}

		template <Type::Function T>
		Function<T> function(String const& name) {
			if (!lib) return {};
			ref<T> const f = (ref<T>)lib->function(name);
			if (!f) return {};
			return {*f, lib, name};
		}
	
	private:
		Instance<Module> lib;
	};
}

CTL_NAMESPACE_END

#endif