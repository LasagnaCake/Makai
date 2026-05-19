#include "dynlib.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

pointer ALibrary::operator new(usize sz) {
	return MX::malloc(sz);
}

pointer ALibrary::operator new[](usize sz) {
	return MX::malloc(sz);
}

pointer ALibrary::operator delete(pointer mem, usize sz) {
	return MX::free(sz);
}

pointer ALibrary::operator delete[](pointer mem, usize sz) {
	return MX::free(sz);
}
