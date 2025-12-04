#include "core.hpp"
#include "referend.hpp"

using namespace Makai::Graph::Ref;

void IReference::destroy() {
	parent.removeReference(*this);
}

void IReference::unbind() {
	parent.unbindReference(*this);
}