#ifndef MAKAILIB_ANIMA_V2_CORE_DYNLIB_H
#define MAKAILIB_ANIMA_V2_CORE_DYNLIB_H

#include "context.hpp"

namespace Makai::Anima::V2::Core {
	struct ALibrary {
		virtual ~ALibrary();
		virtual void open();
		virtual void load(Context::Adder const& context)		= 0;
		virtual void unload(Context::Remover const& remover);
		virtual void close();

		virtual String			name() const	= 0;
		virtual Data::Version	version() const	{return Data::Version{0};		}
		virtual usize			hash() const	{return Makai::hash(name());	}

		pointer operator new(usize sz) noexcept;
		pointer operator new[](usize sz) noexcept;

		void operator delete(pointer mem, usize sz) noexcept;
		void operator delete[](pointer mem, usize sz) noexcept;
	};

	template <Makai::Type::Subclass<ALibrary> T>
	inline owner<ALibrary> createLibrary() {
		return new T();
	}
}

#define AV2_Library_Windows(LIB)\
	static_assert(\
		Makai::Type::Subclass<LIB, Makai::Anima::V2::Core::ALibrary>\
	);\
	CTL_CDECL CTL_DYNEXPORT auto AV2_Extern_getLibrary() {\
		return Makai::Anima::V2::Core::createLibrary<LIB>();\
	}\
	int main() {\
		return 0;\
	}

#define AV2_Library_Unix(LIB)\
		static_assert(\
			Makai::Type::Subclass<LIB, Makai::Anima::V2::Core::ALibrary>\
		);\
		CTL_CDECL CTL_DYNEXPORT auto AV2_Extern_getLibrary() {\
			return Makai::Anima::V2::Core::createLibrary<LIB>();\
		}

#if defined (CTL_ON_UNIX)
#define AV2_Library(LIB) AV2_Library_Unix(LIB)
#elif defined (CTL_ON_WINDOWS)
#define AV2_Library(LIB) AV2_Library_Windows(LIB)
#elif defined (MAKAI_AV2_DYNLIBS_ARE_REQUIRED)
#error "Invalid/Unsupported system for AV2 Shared Libraries!"
#else
#define AV2_Library(LIB)
#endif

#endif
