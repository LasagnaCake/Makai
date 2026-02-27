#include "location.hpp"

using namespace Makai::Anima::V2::Runtime;

Location& Location::bind(Location const& other)  {
	if (other.content == content) return *this;
	unbind();
	content = other.content;
	++content->refs;
	return *this;
}

Location& Location::unbind() {
	if (!content) return *this;
	--content->refs;
	if (!content->refs)
		delete content;
	return *this;
}
