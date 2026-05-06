#ifndef MAKAILIB_ANIMA_V2_CORE_DYNLIB_H
#define MAKAILIB_ANIMA_V2_CORE_DYNLIB_H

#include "context.hpp"

namespace Makai::Anima::V2::Core {
	struct ILibrary {
		virtual ~ILibrary() {}
		virtual void open()										{DEBUGLN("Opening [", name(), "]...");}
		virtual void load(Context::Adder const& context)		= 0;
		virtual void unload(Context::Remover const& remover)	{DEBUGLN("Unloading [", name(), "]...");}
		virtual void close()									{DEBUGLN("Closing [", name(), "]...");}

		virtual String			name() const	= 0;
		virtual Data::Version	version() const	{return Data::Version{0};		}
		virtual usize			hash() const	{return Makai::hash(name());	}
	};
}

#define AV2_Library(LIB)\
	static_assert(Makai::Type::Subclass<LIB, Makai::Anima::V2::Core::ILibrary>);\
	CTL_CDECL CTL_DYNEXPORT owner<Makai::Anima::V2::Core::ILibrary> AV2_Extern_getLibrary() {return new LIB();}

#endif
