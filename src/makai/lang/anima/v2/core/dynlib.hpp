#ifndef MAKAILIB_ANIMA_V2_CORE_DYNLIB_H
#define MAKAILIB_ANIMA_V2_CORE_DYNLIB_H

#include "context.hpp"

namespace Makai::Anima::V2::Core {
	struct ILibrary {
		virtual ~ILibrary() {}
		virtual void open()											= 0;
		virtual void load(Context::MethodAdder const& context)		= 0;
		virtual void unload(Context::MethodRemover const& context)	= 0;
		virtual void close()										= 0;
	};
}

#define AV2_Library(LIB)\
	static_assert(Makai::Type::Subclass<LIB, Makai::Anima::V2::Core::ILibrary>);\
	CTL_CDECL CTL_DYNEXPORT Makai::Anima::V2::Core::ILibrary* AV2_Extern_getLibrary() {static LIB av2lib; return &av2lib;}

#endif
