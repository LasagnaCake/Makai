#include "context.hpp"
#include <CL/cl.h>

using namespace Makai;
using namespace Makai::MP::OpenCL;

struct Context::Impl: Component::IResource {
	cl_context context = nullptr;

	pointer resource() const override {return (pointer)context;}

	virtual ~Impl();
};

Context::Impl::~Impl() {
	if (context) clReleaseContext(context);
}

Context::Context(): Component(new Impl) {
	cl_context_properties props;
	cl_int err;
	impl(*this).context = clCreateContext(&props, 0, NULL, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		throw Error::FailedAction(
			"Failed to create OpenCL context!",
			CTL_CPP_PRETTY_SOURCE
		);
}
