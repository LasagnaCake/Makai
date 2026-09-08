#include "program.hpp"
#include "clhelper.cc"

using namespace Makai;
using namespace Makai::MP::OpenCL;

using Program	= Context::Program;
using Kernel	= Context::Program::Kernel;
using Argument	= Context::Program::Kernel::Argument;

struct Program::Impl: Program::IResource {
	cl_program program = nullptr;
	Context context;

	pointer resource() const override {return (pointer)program;}

	virtual ~Impl();

	Impl();
};

struct Kernel::Impl: Program::IResource {
	cl_kernel			kernel = nullptr;
	Program				program;
	ArgumentIndexMap	argIndices;
	ArgumentNameMap		argNames;
	String				name;

	pointer resource() const override {return (pointer)kernel;}

	virtual ~Impl();

	Impl();
};

struct Argument::Impl: Program::IResource {
	Kernel	kernel;
	String	name;
	usize	index = -1;

	pointer resource() const override {return (pointer)index;}

	virtual ~Impl();

	Impl();
};

Program::Impl::~Impl() {
	if (program) clReleaseProgram(program);
}

Kernel::Impl::~Impl() {
	if (kernel) clReleaseKernel(kernel);
}

Argument::Impl::~Impl() {
}

Program::Program(): Component(new Impl) {
}

Program::Program(Context const& context): Program() {
	impl(*this).context = context;
}

Nullable<Program::SourceError> Program::setSource(String const& source) {
	if (impl(*this).program)
		return SourceError::OCL_PSE_ALREADY_HAS_PROGRAM;
	carr<cstring, 1> src{source.cstr()};
	carr<size_t, 1> sz{source.size()};
	cl_int err;
	impl(*this).program = clCreateProgramWithSource(
		(cl_context)context().resource(),
		1,
		src,
		sz,
		&err
	);
	switch (err) {
		using enum SourceError;
		case CL_SUCCESS:			return null;
		case CL_OUT_OF_RESOURCES:	return OCL_PSE_OUT_OF_RESOURCES;
		case CL_OUT_OF_HOST_MEMORY:	return OCL_PSE_OUT_OF_HOST_MEMORY;
	}
	return null;
}

Nullable<Program::BuildError> Program::build(String const& options) {
	if (!impl(*this).program)
		return BuildError::OCL_PBE_NO_SOURCE_ASSIGNED;
	auto const err = clBuildProgram(
		impl(*this).program,
		0,
		NULL,
		options.cstr(),
		NULL,
		NULL
	);
	switch (err) {
		using enum BuildError;
		case CL_SUCCESS: return null;
		case CL_BUILD_PROGRAM_FAILURE:	return OCL_PBE_FAILED_TO_BUILD;
		case CL_INVALID_OPERATION:		return OCL_PBE_PROGRAM_HAS_BEEN_BUILT_ALREADY;
		case CL_INVALID_BINARY:			return OCL_PBE_INVALID_BINARY;
		case CL_INVALID_BUILD_OPTIONS:	return OCL_PBE_INVALID_BUILD_OPTIONS;
		case CL_COMPILER_NOT_AVAILABLE:	return OCL_PBE_COMPILER_NOT_AVAILABLE;
		case CL_OUT_OF_RESOURCES:		return OCL_PBE_OUT_OF_RESOURCES;
		case CL_OUT_OF_HOST_MEMORY:		return OCL_PBE_OUT_OF_HOST_MEMORY;
	}
	return null;
}

Result<Kernel, Kernel::SetError> Program::kernel(String const& name) const {
	if (!impl(*this).program)
		return Kernel::SetError::OCL_PKSE_PROGRAM_HAS_NOT_BEEN_BUILT;
	Kernel kernel(*this);
	if (auto const err = kernel.set(name))
		return *err;
	return kernel;
}

Result<Kernel, Kernel::SetError> Program::operator[](String const& name) const {
	return kernel(name);
}

Kernel::Kernel(): Component(new Impl) {
}

Kernel::Kernel(Program const& program): Kernel() {
	impl(*this).program = program;
}

Result<Kernel::Argument, Kernel::Argument::SetError> Kernel::argument(String const& name) const {
	if (!impl(*this).kernel)
		return Kernel::Argument::SetError::OCL_PKASE_KERNEL_DOES_NOT_EXIST;
	Argument arg(*this);
	if (auto const err = arg.set(name))
		return *err;
	return arg;
}

Result<Kernel::Argument, Kernel::Argument::SetError> Kernel::argument(usize const index) const {
	if (!impl(*this).kernel)
		return Kernel::Argument::SetError::OCL_PKASE_KERNEL_DOES_NOT_EXIST;
	Argument arg(*this);
	if (auto const err = arg.set(index))
		return *err;
	return arg;
}

Result<Kernel::Argument, Kernel::Argument::SetError> Kernel::operator[](String const& name) const {
	return argument(name);
}

Result<Kernel::Argument, Kernel::Argument::SetError> Kernel::operator[](usize const index) const {
	return argument(index);
}

Nullable<Kernel::SetError> Kernel::set(String const& name) {
	cl_int err;
	if (name.empty())
		return SetError::OCL_PKSE_MISSING_KERNEL_NAME;
	impl(*this).kernel = clCreateKernel(
		(cl_program)program().resource(),
		name.cstr(),
		&err
	);
	impl(*this).name = name;
	switch (err) {
		using enum SetError;
		case CL_SUCCESS: return null;
		case CL_INVALID_PROGRAM_EXECUTABLE:	return OCL_PKSE_PROGRAM_HAS_NOT_BEEN_BUILT;
		case CL_INVALID_KERNEL_NAME:		return OCL_PKSE_KERNEL_DOES_NOT_EXIST;
		case CL_INVALID_KERNEL_DEFINITION:	return OCL_PKSE_MALFORMED_KERNEL;
		case CL_OUT_OF_RESOURCES:			return OCL_PKSE_OUT_OF_RESOURCES;
		case CL_OUT_OF_HOST_MEMORY:			return OCL_PKSE_OUT_OF_HOST_MEMORY;
	}
	usize argSize = 0;
	usize _ = 0;
	err = clGetKernelInfo(
		impl(*this).kernel,
		CL_KERNEL_NUM_ARGS,
		sizeof(usize),
		&argSize,
		NULL
	);
	if (err != CL_SUCCESS) return SetError::OCL_PKSE_MALFORMED_KERNEL;
	for (usize i = 0; i < argSize; ++i) {
		String name;
		size_t sz;
		name.reserve(1024, '\0');
		err = clGetKernelArgInfo(
			impl(*this).kernel,
			i,
			CL_KERNEL_ARG_NAME,
			name.size(),
			name.data(),
			&sz
		);
		if (err != CL_SUCCESS) break;
		name.resize(sz);
		impl(*this).argNames[i]			= name;
		impl(*this).argIndices[name]	= i;
	}
	return null;
}

Kernel::Argument::Argument(): Component(new Impl) {
}

Kernel::Argument::Argument(Kernel const& kernel): Argument() {
	impl(*this).kernel = kernel;
}

Nullable<Kernel::Argument::SetError> Argument::set(String const& name) {
	if (!kernel().resource())
		return SetError::OCL_PKASE_KERNEL_DOES_NOT_EXIST;
	auto const k = kernel();
	if (!impl(k).argIndices.contains(name))
		return SetError::OCL_PKASE_ARGUMENT_DOES_NOT_EXIST;
	impl(*this).index = impl(k).argIndices[name];
	impl(*this).name = name;
	return null;
}

Nullable<Kernel::Argument::SetError> Argument::set(usize const index) {
	if (!kernel().resource())
		return SetError::OCL_PKASE_KERNEL_DOES_NOT_EXIST;
	auto const k = kernel();
	if (!impl(k).argNames.contains(index))
		return SetError::OCL_PKASE_ARGUMENT_DOES_NOT_EXIST;
	impl(*this).name = impl(k).argNames[index];
	impl(*this).index = index;
	return null;
}

Nullable<String> Argument::name() const {
	if (index() == Limit::MAX<usize>)
		return null;
	String name;
	size_t sz;
	name.reserve(1024, '\0');
	clGetKernelArgInfo(
		(cl_kernel)kernel().resource(),
		impl(*this).index,
		CL_KERNEL_ARG_TYPE_NAME,
		name.size(),
		name.data(),
		&sz
	);
	return name.resize(sz);
}

Nullable<String> Argument::type() const {
	if (index() == Limit::MAX<usize>)
		return null;
	return impl(*this).name;
}

Nullable<usize> Argument::index() const {
	if (impl(*this).index == Limit::MAX<usize>)
		return null;
	return impl(*this).index;
}

Context Program::context() const {
	return impl(*this).context;
}

Context Kernel::context() const {
	return program().context();
}

Program Kernel::program() const {
	return impl(*this).program;
}

Context Argument::context() const {
	return kernel().context();
}

Program Argument::program() const {
	return kernel().program();
}

Kernel Argument::kernel() const {
	return impl(*this).kernel;
}
