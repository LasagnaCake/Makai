#ifndef MAKAILIB_ANIMA_V2_CORE_DYNLIB_H
#define MAKAILIB_ANIMA_V2_CORE_DYNLIB_H

#include "context.hpp"

namespace Makai::Anima::V2::Core {
	struct ILibrary {
		virtual ~ILibrary() {}
		virtual void open()																				{}
		virtual void load(Context::TypeAdder const& types, Context::MethodAdder const& methods)			= 0;
		virtual void unload(Context::TypeRemover const& types, Context::MethodRemover const& methods)	{}
		virtual void close()																			{}
	};
}

#define AV2_Library(LIB)\
	static_assert(Makai::Type::Subclass<LIB, Makai::Anima::V2::Core::ILibrary>);\
	CTL_CDECL CTL_DYNEXPORT owner<Makai::Anima::V2::Core::ILibrary> AV2_Extern_getLibrary() {return new LIB();}

#endif
