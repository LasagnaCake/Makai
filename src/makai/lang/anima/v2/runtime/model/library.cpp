#include "dynlib.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

pointer ALibrary::operator new(usize sz) noexcept {
	return MX::malloc(sz);
}

pointer ALibrary::operator new[](usize sz) noexcept {
	return MX::malloc(sz);
}

void ALibrary::operator delete(pointer mem, usize sz) noexcept {
	return MX::free(mem);
}

void ALibrary::operator delete[](pointer mem, usize sz) noexcept {
	return MX::free(mem);
}

ALibrary::~ALibrary() {}
void ALibrary::open()									{MAKAILIB_DEBUGLN_FULL("Opening [", name(), "]...");		}
void ALibrary::unload(Context::Remover const& remover)	{MAKAILIB_DEBUGLN_FULL("Unloading [", name(), "]...");	}
void ALibrary::close()									{MAKAILIB_DEBUGLN_FULL("Closing [", name(), "]...");		}
