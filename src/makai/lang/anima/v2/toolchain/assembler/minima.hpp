#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Minima: AAssembler {
		struct Context: BaseContext {

			struct Method: Core::Module::Method {
				bool local = false;
			};
			using Declaration	= Core::Module::Declaration;
			using Namespace		= Core::Module::Namespace;

			using OpCode = Core::Instruction::Name;

			usize add(OpCode const& opcode = OpCode::AV2_IN_NO_OP, String const type = 0);
			usize add(uint64 const& value);

			template <class T>
			usize add(OpCode const& opcode, T const& val)
		 	requires (!Type::Integer<T> && (sizeof(T) == sizeof(uint32))){
				return add(opcode, Makai::Cast::bit<uint32, T>(val));
			}

			uint64 addStringLiteral(String const& val);
			uint64 addGlobal(String const& name);

			void addJumpTarget(String const& name);
			uint64 getJumpTarget(String const& name);
			bool hasJumpTarget(String const& name);

			void finalize();

			virtual Core::Module onImport(String const& file);

			void addModule(Instance<Namespace> const& module);
			void addMethod(Instance<Method> const& method);
			void addType(Instance<Declaration> const& type);

			Instance<Namespace>		getModule(String const& name);
			Instance<Method>		getMethod(String const& name);
			Instance<Declaration>	getType(String const& name);

			Core::Module						program;
			Dictionary<Instance<Declaration>>	types;
			Dictionary<Instance<Method>>		methods;

			List<Instance<Method>>		methodStack;
			List<Instance<Namespace>>	moduleStack;
		};

		Minima(Context& context): AAssembler(context), context(context) {}
		void invoke() override;

		Context& context;
	};
}

#endif
