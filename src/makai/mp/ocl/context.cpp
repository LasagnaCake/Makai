#include "context.hpp"
#include <CL/cl.h>

using namespace Makai;
using namespace Makai::MP::OpenCL;

struct Context::Impl {
	cl_context context;
};

Context::Deleter Component<Context>::deleter = Context::deleterFor<Context::Impl>();
