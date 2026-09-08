#include "image.hpp"
#include "clhelper.cc"

using namespace Makai;
using namespace Makai::MP::OpenCL;

using Image	= Context::Image;

struct Image::Impl: Component::IResource {
	virtual ~Impl();
};

Image::Impl::~Impl() {

}
