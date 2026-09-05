#ifndef MAKAILIB_MP_OCL_PROGRAM_H
#define MAKAILIB_MP_OCL_PROGRAM_H

#include "component.hpp"
#include "context.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	using Program = Context::Program;

	struct Context::Program: Component<Program>, Contextual<Program> {
		friend struct Contextual<Program>;
		friend struct Component<Program>;

		struct Kernel;

		friend struct Program::Kernel;

		struct Kernel: Component<Kernel>, Contextual<Kernel> {
			friend struct Contextual<Kernel>;
			friend struct Component<Kernel>;

			struct Argument;

			friend struct Kernel::Argument;

			struct Argument: Component<Argument>, Contextual<Argument> {
				friend struct Contextual<Argument>;
				friend struct Component<Argument>;

				enum class SetError: usize {
					OCL_PKASE_KERNEL_DOES_NOT_EXIST,
					OCL_PKASE_ARGUMENT_DOES_NOT_EXIST,
				};

				Kernel& kernel() const;
				Program& program() const;

				Nullable<String> name() const;
				Nullable<String> type() const;
				Nullable<usize> index() const;

				Argument(Kernel const& kernel);

				Nullable<SetError> set(String const& name);
				Nullable<SetError> set(usize const index);
			};

			enum class SetError: usize {
				OCL_PKSE_PROGRAM_HAS_NOT_BEEN_BUILT,
				OCL_PKSE_MISSING_KERNEL_NAME,
				OCL_PKSE_KERNEL_DOES_NOT_EXIST,
				OCL_PKSE_MALFORMED_KERNEL,
				OCL_PKSE_OUT_OF_RESOURCES,
				OCL_PKSE_OUT_OF_HOST_MEMORY,
			};

			using ArgumentError = Argument::SetError;

			Nullable<SetError> set(String const& name);

			using ArgumentIndexMap	= Dictionary<usize>;
			using ArgumentNameMap	= Map<usize, String>;

			ArgumentNameMap		nameMap()	const;
			ArgumentIndexMap	indexMap()	const;

			Result<Argument, ArgumentError> argument(String const& name)	const;
			Result<Argument, ArgumentError> argument(usize const index)		const;

			Result<Argument, ArgumentError>	operator[](String const& name)	const;
			Result<Argument, ArgumentError>	operator[](usize const index)	const;

			Program& program() const;

			Kernel(Program const& program);
		};

		enum class SourceError: usize {
			OCL_PSE_ALREADY_HAS_PROGRAM,
			OCL_PSE_OUT_OF_RESOURCES,
			OCL_PSE_OUT_OF_HOST_MEMORY,
		};

		enum class BuildError: usize {
			OCL_PBE_NO_SOURCE_ASSIGNED,
			OCL_PBE_PROGRAM_HAS_BEEN_BUILT_ALREADY,
			OCL_PBE_FAILED_TO_BUILD,
			OCL_PBE_INVALID_BINARY,
			OCL_PBE_INVALID_BUILD_OPTIONS,
			OCL_PBE_COMPILER_NOT_AVAILABLE,
			OCL_PBE_OUT_OF_RESOURCES,
			OCL_PBE_OUT_OF_HOST_MEMORY,
		};

		using KernelError = Kernel::SetError;

		Nullable<SourceError>		setSource(String const& source);
		Nullable<SourceError>		setBinary(ConstByteSpan<> const& binary);

		Nullable<BuildError>		build(String const& options);

		Bytes<>						binary();

		Result<Kernel, KernelError>	kernel(String const& name)		const;
		Result<Kernel, KernelError>	operator[](String const& name)	const;

		Program(Context const& context);
	};

	using Kernel = Program::Kernel;

	template <> Context& Contextual<Program>::context();
	template <> Context& Contextual<Kernel>::context();
	template <> Context& Contextual<Argument>::context();
}

#endif
