#include "image.hpp"
#include <CL/cl.h>

using namespace Makai;
using namespace Makai::MP::OpenCL;

using Image	= Context::Image;

struct Context::Impl {
	cl_context context;

	~Impl();
};
