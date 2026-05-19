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
