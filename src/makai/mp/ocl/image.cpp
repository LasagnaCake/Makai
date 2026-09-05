#include "image.hpp"
#include <CL/cl.h>

using namespace Makai;
using namespace Makai::MP::OpenCL;

using Image	= Context::Image;

template<> struct Image::Impl {
	~Impl();
};

template<> Context::Deleter Component<Image>::deleter = Context::deleterFor<Image::Impl>();
