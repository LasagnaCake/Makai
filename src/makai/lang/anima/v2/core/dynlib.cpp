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
void ALibrary::open()									{DEBUGLN("Opening [", name(), "]...");		}
void ALibrary::unload(Context::Remover const& remover)	{DEBUGLN("Unloading [", name(), "]...");	}
void ALibrary::close()									{DEBUGLN("Closing [", name(), "]...");		}
