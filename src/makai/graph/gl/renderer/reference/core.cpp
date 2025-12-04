#include "core.hpp"
#include "referend.hpp"

using namespace Makai::Graph::Ref;

void AReference::destroy() {
	parent.removeReference(*this);
}

void AReference::unbind() {
	parent.unbindReference(*this);
}