#include "context.hpp"
#include <CL/cl.h>

using namespace Makai;
using namespace Makai::MP::OpenCL;

template<> struct Context::Impl {
	cl_context context;

	~Impl();
};

template<> Context::Deleter Component<Context>::deleter = Context::deleterFor<Context::Impl>();

template<> Context::Impl::~Impl() {
	if (context) clReleaseContext(context);
}

Context::Context() {
	impl() = new Impl;
	cl_context_properties props;
	cl_int err;
	impl().context = clCreateContext(&props, 0, NULL, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		throw Error::FailedAction(
			"Failed to create OpenCL context!",
			CTL_CPP_PRETTY_SOURCE
		);
}
