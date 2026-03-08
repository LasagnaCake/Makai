#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Minima: AAssembler {
		struct Context: BaseContext {
			struct Namespace {
				Dictionary<Instance<Namespace>>	namespaces;
				Dictionary<Instance<Decl>>		types;
				Dictionary<Instance<Method>>	methods;
			};

			struct Method: ID::Identifiable<Method const, uint64> {
				uint64			retType;
				List<uint64>	argTypes;
				bool			out		= false;
				bool			local	= false;
				String			entry;
			};

			struct Decl: ID::Identifiable<Decl const, uint64> {
				uint64						flags		= 0;
				Nullable<Core::BasicType>	basic		= Core::BasicType::AV2_BT_VOID;
				Nullable<uint64>			base		= null;
				uint64						alignment	= 1;
				List<uint64>				fields;
				List<uint64>				operators;
			};

			using OpCode = Core::Instruction::Name;

			usize add(OpCode const& opcode = OpCode::AV2_IN_NO_OP, uint32 const type = 0);
			usize add(uint64 const& value);

			template <class T>
			usize add(OpCode const& opcode, T const& val)
		 	requires (!Type::Integer<T> && (sizeof(T) == sizeof(uint32))){
				return add(opcode, Makai::Cast::bit<uint32, T>(val));
			}

			usize addConstant(Data::Value const& val);
			uint64 addGlobal(String const& name);

			void addJumpTarget(String const& name);
			uint64 getJumpTarget(String const& name);
			bool hasJumpTarget(String const& name);

			StringList mapJumps();

			enum class ImportAction {
				AV2_TA_MCIA_ALLOW_IMPORT,
				AV2_TA_MCIA_SKIP,
				AV2_TA_MCIA_ERROR,
			};

			Program							program;
			ImportAction					imports	= ImportAction::AV2_TA_MCIA_ERROR;
			Nullable<String>				parent;
			Nullable<String>				module;
			Dictionary<Instance<Decl>>		types;
			Dictionary<Instance<Method>>	methods;
		};

		Minima(Context& context): AAssembler(context), context(context) {}
		void execute() override;

		Context& context;
	};
}

#endif
