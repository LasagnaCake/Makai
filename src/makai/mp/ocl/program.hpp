#ifndef MAKAILIB_MP_OCL_PROGRAM_H
#define MAKAILIB_MP_OCL_PROGRAM_H

#include "component.hpp"
#include "context.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	struct Context::Program: Component<Context::Program> {
		struct Impl;

		struct Kernel;

		enum class SourceError {
			OCL_SE_ALREADY_HAS_PROGRAM,
			OCL_SE_OUT_OF_RESOURCES,
			OCL_SE_OUT_OF_HOST_MEMORY,
		};

		enum class BuildError {
			OCL_BE_NO_SOURCE_ASSIGNED,
			OCL_BE_PROGRAM_HAS_BEEN_BUILT_ALREADY,
			OCL_BE_FAILED_TO_BUILD,
			OCL_BE_INVALID_BINARY,
			OCL_BE_INVALID_BUILD_OPTIONS,
			OCL_BE_COMPILER_NOT_AVAILABLE,
			OCL_BE_OUT_OF_RESOURCES,
			OCL_BE_OUT_OF_HOST_MEMORY,
		};

		Nullable<CompilationError>	setSource(String const& source);
		Nullable<BuildError>		build(String const& options);

		Program(Context const& context);
	};
}

#endif
