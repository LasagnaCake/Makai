#include "program.hpp"
#include <CL/cl.h>

using namespace Makai;
using namespace Makai::MP::OpenCL;

struct Program::Impl {
	cl_program program;
};

Program::Deleter Component<Program>::deleter = Program::deleterFor<Program::Impl>();
