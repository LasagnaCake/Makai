#include "program.hpp"
#include <CL/cl.h>

using namespace Makai;
using namespace Makai::MP::OpenCL;

struct Context::Impl {
	cl_context context;

	~Impl();
};

struct Context::Program::Impl {
	cl_program program = nullptr;
	Context context;

	~Impl();
};

Program::Deleter Component<Program>::deleter = Program::deleterFor<Program::Impl>();

Context::Impl::~Impl() {
	clReleaseProgram(program);
}

Program::Program(Context const& context) {
	impl() = new Impl;
	impl().context = context;
}

Nullable<Program::SourceError> Program::setSource(String const& source) {
	if (impl().program)
		return SourceError::OCL_SE_ALREADY_HAS_PROGRAM;
	carr<cstring, 1> src{source.cstr()};
	carr<size_t, 1> sz{source.size()};
	cl_int err;
	impl()->program = clCreateProgramWithSource(
		context.impl().context,
		1,
		src,
		sz,
		&err
	);
	switch (err) {
		using enum SourceError;
		case CL_SUCCESS:			return null;
		case CL_OUT_OF_RESOURCES:	return OCL_SE_OUT_OF_RESOURCES;
		case CL_OUT_OF_HOST_MEMORY:	return OCL_SE_OUT_OF_HOST_MEMORY;
	}
	return null;
}clBuildProgram

Nullable<Program::BuildError> Program::build(String const& options) {
	if (!impl().program)
		return BuildError::OCL_BE_NO_SOURCE_ASSIGNED;
	auto const err = clBuildProgram(
		impl().program,
		o,
		NULL,
		options.cstr(),
		NULL
	);
	switch (err) {
		using enum BuildError;
		case CL_SUCCESS: return null;
		case CL_BUILD_PROGRAM_FAILURE:		return OCL_BE_FAILED_TO_BUILD;
		case CL_INVALID_BINARY:				return OCL_BE_INVALID_BINARY;
		case CL_INVALID_OPERATION:			return OCL_BE_PROGRAM_HAS_BEEN_BUILT_ALREADY;
		case CL_BE_INVALID_BINARY:			return OCL_BE_INVALID_BINARY;
		case CL_BE_INVALID_BUILD_OPTIONS:	return OCL_BE_INVALID_BUILD_OPTIONS;
		case CL_BE_COMPILER_NOT_AVAILABLE:	return OCL_BE_COMPILER_NOT_AVAILABLE;
		case CL_OUT_OF_RESOURCES:			return OCL_BE_OUT_OF_RESOURCES;
		case CL_OUT_OF_HOST_MEMORY:			return OCL_BE_OUT_OF_HOST_MEMORY;
	}
	return null;
}
