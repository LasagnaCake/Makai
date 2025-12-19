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

			constexpr void addVariable(String const& name, bool const global = false) {
				if (members.contains(name)) return;
				members[name].value = Data::Value::object();
				members[name] = {Member::Type::AV2_TA_SMT_VARIABLE};
				members[name].value["global"]	= global;
				members[name].value["init"]		= false;
				members[name].value["use"]		= false;
				if (!global)
					members[name].value["stack_id"] = stackc + varc++;
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
			Data::Value::Kind	result	= Data::Value::Kind::DVK_UNDEFINED;
			bool				secure	= true;
			bool				isFunc	= true;
			String				name;
			String				label;
			uint64				varc	= 0;
			uint64				stackc	= 0;
			Dictionary<Member>	members;
			String				pre;
			String				code;
			String				post;
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

		constexpr static Data::Value::Kind DVK_ANY = Data::Value::Kind{-2};

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
			auto const sc = scope.popBack();
			writeLine(sc.pre, sc.code, sc.post);
		}

		constexpr void addFunctionExit() {
			uint64 varc = 0;
			for (auto& sc: Range::reverse(scope))
				if (sc.isFunc) {
					varc = sc.varc;
					break;
				}
			if (varc)
				writeLine("clear", varc);
		}

		constexpr bool inFunction() const {
			for (auto& sc: Range::reverse(scope))
				if (sc.isFunc) return true;
			return false;
		}

		constexpr Scope& functionScope() {
			for (auto& sc: Range::reverse(scope))
				if (sc.isFunc) return sc;
			throw Error::FailedAction("Not in function scope!");
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
		constexpr void writeGlobalLine(Args const&... args) {
			auto& content = code;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeGlobalPreamble(Args const&... args) {
			auto& content = pre;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeGlobalPostscript(Args const&... args) {
			auto& content = post;
			content += toString(toString(args, " ")..., "\n") + content;
		}

		template <class... Args>
		constexpr void writeScopeLine(Args const&... args) {
			auto& content = currentScope().code;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeScopePreamble(Args const&... args) {
			auto& content = currentScope().pre;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeScopePostscript(Args const&... args) {
			auto& content = currentScope().post;
			content += toString(toString(args, " ")..., "\n") + content;
		}

		template <class... Args>
		constexpr void writeLine(Args const&... args) {
			if (scope.empty())	writeGlobalLine(args...);
			else				writeScopeLine(args...);
		}

		template <class... Args>
		constexpr void writePreamble(Args const&... args) {
			if (scope.empty())	writeGlobalPreamble(args...);
			else				writeScopePreamble(args...);
		}

		template <class... Args>
		constexpr void writePostscript(Args const&... args) {
			if (scope.empty())	writeGlobalPostscript(args...);
			else				writeScopePostscript(args...);
		}
		
		template <class... Args>
		constexpr void writeAdaptive(Args const&... args) {
			if (scope.size() > 1)	writeLine(args...);
			else					writePreamble(args...);
		}

		constexpr static bool isCastable(Data::Value::Kind const type) {
			return Data::Value::isScalar(type) || Data::Value::isString(type) || type == Data::Value::Kind{-2};
		}

		inline String uniqueName() {
			return Makai::toString("_", code.size(), "_", rng.integer(), "_", Random::CTPRNG<uint64>);
		}

		constexpr String intermediate() const {
			return pre + code + post;
		}

		constexpr static bool isReservedKeyword(String const& name) {
			if (name == "any")												return true;
			if (name == "null")												return true;
			if (name == "nan")												return true;
			if (name == "true" || name == "true")							return true;
			if (name == "undefined" || name == "void")						return true;
			if (name == "boolean" || name == "bool")						return true;
			if (name == "signed" || name == "int")							return true;
			if (name == "unsigned" || name == "uint")						return true;
			if (name == "string" || name == "str")							return true;
			if (name == "array" || name == "arr")							return true;
			if (name == "object" || name == "struct")						return true;
			if (name == "if" || name == "else")								return true;
			if (name == "do" || name == "while")							return true;
			if (name == "for" || name == "in")								return true;
			if (name == "throw")											return true;
			if (name == "switch" || name == "case")							return true;
			if (name == "template" || name == "type")						return true;
			if (name == "typeof" || name == "using")						return true;
			if (name == "abstract" || name == "define")						return true;
			if (name == "copy" || name == "move")							return true;
			if (name == "context" || name == "strict" || name == "loose")	return true;
			if (name == "dynamic" || name == "dyn")							return true;
			if (name == "prop")												return true;
			if (name == "const")											return true;
			if (name == "as" || name == "is")								return true;
			if (name == "function" || name == "func" || name == "fn")		return true;
			if (name == "global" || name == "local")						return true;
			if (name == "stack" || name == "register")						return true;
			if (name == "temporary" || name == "register")					return true;
			if (name == "minima" || name == "asm")							return true;
			if (name == "await" || name == "async" || name == "yield")		return true;
			return false;
		}

		List<Scope>				scope;
		Jumps					jumps;
		TokenStream				stream;
		Program					program;
		String					fileName;
		String					pre, code, post;
		Random::SecureGenerator	rng;
		bool					hasMain		= false;
		String const			mainScope	= "__main" + uniqueName();
	};
}

#endif