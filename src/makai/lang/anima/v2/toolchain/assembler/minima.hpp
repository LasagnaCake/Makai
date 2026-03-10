#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Minima: AAssembler {
		struct Context: BaseContext {
			struct Reference {
				String module;
				String name;
			};

			struct Method: Core::Module::Method {
				bool local = false;
				String jump;
			};

			struct Declaration: Core::Module::Declaration {
			};

			using OpCode = Core::Instruction::Name;

			usize add(OpCode const& opcode = OpCode::AV2_IN_NO_OP, uint64 const type = 0);
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

			void addMethod(String const& name, Instance<Method> const& method);
			void addType(String const& name, Instance<Declaration> const& type);

			Instance<Method>		getMethod(String const& name);
			Instance<Declaration>	getType(String const& name);

			Core::Module						program;

			Dictionary<Instance<Reference>>		types;
			Dictionary<Instance<Reference>>		methods;

			Dictionary<Instance<Declaration>>	moduleTypes;
			Dictionary<Instance<Method>>		moduleMethods;

			List<Instance<Method>>		methodStack;
			StringList					moduleStack;
			Dictionary<List<usize>>		jumpsToMap;
			Dictionary<uint64>			jumps;

			Instance<Declaration> getSharedType(String const& module, String const& name);

			String fullModulePath() const;
		};

		Minima(Context& context): AAssembler(context), context(context) {}
		void invoke() override;

		Context& context;
	};
}

#endif
