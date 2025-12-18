#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H

#include "../../../../../lexer/lexer.hpp"
#include "../../runtime/program.hpp"
#include "../../instruction.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Context {
		using TokenStream	= Lexer::CStyle::TokenStream;
		using Program		= Runtime::Program;

		struct Scope {
			struct Member {
				enum class Type {
					AV2_TA_SMT_VARIABLE,
					AV2_TA_SMT_FUNCTION,
					AV2_TA_SMT_CLASS,
					AV2_TA_SMT_TYPE,
				};

				Type		type;
				Data::Value	value;
			};

			constexpr bool contains(String const& name) const {
				return members.contains(name);
			}

			constexpr uint64 addVariable(String const& name) {
				if (members.contains(name))
					return members[name].value.get<uint64>();
				members[name].value = Data::Value::object();
				members[name] = {Member::Type::AV2_TA_SMT_VARIABLE};
				members[name].value["stack_id"]	= stackc + varc++;
				members[name].value["init"]		= false;
				members[name].value["use"]		= false;
				return varc-1;
			}

			constexpr void addFunction(String const& name) {
				if (members.contains(name))
					return;
				members[name].value					= Data::Value::object();
				members[name].value["overloads"]	= Data::Value::object();
				members[name] = {Member::Type::AV2_TA_SMT_FUNCTION};
				return;
			}
			
			uint64				entry	= 0;
			Data::Value::Kind	result = Data::Value::Kind::DVK_UNDEFINED;
			String				name;
			String				label;
			uint64				varc	= 0;
			uint64				stackc	= 0;
			Dictionary<Member>	members;
		};

		struct Jumps {
			Dictionary<uint64>			labels;
			Dictionary<List<uint64>>	unmapped;

			constexpr StringList map(Program& program) {
				StringList  stillUnmapped;
				for (auto const& [label, jumps]: unmapped)
					if (labels.contains(label))
						for (auto& jump: jumps)
							program.code[jump] = Cast::bit<Instruction>(labels[label]);
					else stillUnmapped.pushBack(label);
				unmapped.clear();
				return stillUnmapped;
			}
		};

		constexpr StringList mapJumps() {
			return jumps.map(program);
		}

		constexpr void addJumpTarget(String const& label) {
			if (jumps.labels.contains(label)) {
				program.code.pushBack(Makai::Cast::bit<Instruction>(jumps.labels[label]));
			} else {
				jumps.unmapped[label].pushBack(program.code.size());
				program.code.pushBack({});
			}
		}
		constexpr uint64 addJumpLabel(String const& label, uint64 const to) {
			if (jumps.labels.contains(label)) {
				return jumps.labels[label];
			} else {
				auto const id = jumps.labels[label] = program.jumpTable.size();
				program.jumpTable.pushBack(to);
				return id;
			}
		}

		[[nodiscard]]
		constexpr usize addEmptyInstruction() {
			program.code.pushBack({});
			return program.code.size() - 1;
		}
		[[nodiscard]]
		constexpr usize addNamedInstruction(Instruction::Name const name) {
			program.code.pushBack({name});
			return program.code.size() - 1;
		}

		template <class T>
		constexpr usize addInstruction(T const& inst)
		requires (sizeof(T) == sizeof(Instruction)) {
			program.code.pushBack(Cast::bit<Instruction, T>(inst));
			return program.code.size() - 1;
		}

		template <class T>
		constexpr static void addInstructionType(Instruction& inst, T const& type)
		requires (sizeof(T) == sizeof(uint32)) {
			inst.type = Cast::bit<uint32, T>(type);
		}

		template <class T>
		constexpr void addInstructionType(usize const id, T const& type)
		requires (sizeof(T) == sizeof(uint32)) {
			addInstructionType(instruction(id), type);
		}

		constexpr Instruction& instruction(usize const i) {
			return program.code[i];
		}

		constexpr usize addConstant(Data::Value const& value) {
			auto i = program.constants.find(value);
			if (i == -1)
				i = program.constants.pushBack(value).size() - 1;
			return i;
		}

		constexpr void startScope() {
			if (scope.size())
				scope.pushBack({.stackc = currentScope().varc});
			else scope.pushBack({});
		}

		constexpr Scope& currentScope() {
			return scope.back();
		}

		constexpr void addStackEntry(Instruction::StackPush const& entry) {
			if (scope.empty()) return;
			addInstructionType(addNamedInstruction(Instruction::Name::AV2_IN_STACK_PUSH), entry);
		}

		constexpr void endScope() {
			if (scope.empty()) return;
			scope.popBack();
		}

		constexpr bool hasSymbol(String const& name) const {
			for (auto const& sc: scope)
				if (sc.contains(name)) return true;
			return false;
		}

		constexpr Scope::Member& getSymbolByName(String const& name) {
			for (auto& sc: Range::reverse(scope))
				if (sc.contains(name)) return sc.members[name];
			throw Error::FailedAction("Context does not contain symbol '"+name+"'!");
		}
		
		constexpr Function<Scope::Member&()> symbol(String const& name) {
			return [&, name] () -> auto& {return getSymbolByName(name);};
		}

		constexpr String scopePath() const {
			String path;
			for (auto& sc: scope)
				if (path.empty()) path = sc.name;
				else path += "_" + sc.name;
			return path;
		}

		template <class... Args>
		constexpr void writeLine(Args const&... args) {
			ir += toString(args..., "\n");
		}

		constexpr static bool isCastable(Data::Value::Kind const type) {
			return Data::Value::isScalar(type);
		}

		inline String uniqueName() {
			return Makai::toString("_", ir.size(), "_", rng.integer(), "_", Random::CTPRNG<uint64>);
		}

		List<Scope>				scope;
		Jumps					jumps;
		TokenStream				stream;
		Program					program;
		String					fileName;
		String					ir;
		Random::SecureGenerator	rng;
	};
}

#endif