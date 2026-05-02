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

			usize add(OpCode const opcode = OpCode::AV2_IN_NO_OP, uint32 const type = 0);
			usize add(uint64 const value);

			template <class T>
			usize add(OpCode const& opcode, T const& val)
		 	requires (!Type::Integer<T> && (sizeof(T) == sizeof(uint32))){
				return add(opcode, Makai::Cast::bit<uint32, T>(val));
			}

			usize update(usize const instruction, uint32 const type);

			uint64 addStringLiteral(String const& val);
			uint64 addGlobal(String const& name);

			void addJumpTarget(String const& name);
			uint64 getJumpTarget(String const& name);
			bool hasJumpTarget(String const& name);

			void mapJump(String const& name, uint64 const target) {
				jumps[name] = program.jumpTable.size() - 1;
				program.jumpTable.pushBack(target);
			}

			void registerJumpPoint(String const& name) {
				mapJump(name, program.code.size());
			}

			void finalize();

			void addMethod(String const& name, Instance<Method> const& method);
			void addType(String const& name, Instance<Declaration> const& type);

			void addExternalMethod(String const& module, String const& name, Instance<Method> const& method);
			void addExternalType(String const& module, String const& name, Instance<Declaration> const& type);

			Instance<Method>		getMethod(String const& name);
			Instance<Declaration>	getType(String const& name);

			Instance<Method>		getMethodByID(uint64 const& id);
			Instance<Declaration>	getTypeByID(uint64 const& id);

			Core::Module						program;

			Dictionary<Instance<Reference>>		types;
			Dictionary<Instance<Reference>>		methods;

			Dictionary<Instance<Declaration>>	moduleTypes;
			Dictionary<Instance<Method>>		moduleMethods;

			Dictionary<Instance<Declaration>>	externalTypes;
			Dictionary<Instance<Method>>		externalMethods;

			StringList imports;

			List<Instance<Method>>		methodStack;
			StringList					moduleStack;
			Dictionary<List<usize>>		jumpsToMap;
			Dictionary<uint64>			jumps;

			String entry, exit;

			Dictionary<Instance<Core::Module>> linkedModules;

			String fullModulePath() const;
		};

		Minima(Context& context): AAssembler(context), context(context) {}
		void invoke() override;

		Context& context;

		static Core::Module assemble(UTF8String const& fname, UTF8String const& file, bool const strip = false);
	};
}

#endif
