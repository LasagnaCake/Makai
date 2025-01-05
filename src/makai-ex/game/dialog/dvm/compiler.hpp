#ifndef MAKAILIB_EX_GAME_DIALOG_DVM_COMPILER_H
#define MAKAILIB_EX_GAME_DIALOG_DVM_COMPILER_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

/// @brief Dialog Virtual Machine.
namespace Makai::Ex::Game::Dialog::DVM {
	namespace RegexMatches {
		constexpr String ANY_CHAR		= String("[\\S\\s]");
		constexpr String PARAM_CHAR		= String("[^,]");
		constexpr String NAME_CHAR		= String("[0-z\\-_]");
		constexpr String NON_NAME_CHAR	= String("[^0-z\\-_]");
		constexpr String COMPLEX_TOKEN	= String("[\\w&!@#$%&><+\\-_']");
		constexpr String SIMPLE_TOKEN	= String("[*.,;]");

		constexpr String makePack(String const& begin, String const& end) {
			return String() + begin + ANY_CHAR + "*?" + end;
		}

		constexpr String STRINGS		= makePack("\"", "\"");
		constexpr String PARENTHESES	= makePack("(", ")");
		constexpr String ANGLE_BRACKETS	= makePack("[", "]");
		constexpr String LINE_COMMENTS	= String("\\/\\/.*");
		constexpr String BLOCK_COMMENTS = makePack("/*", "*/");

		template<class... Args>
		constexpr String concat(String const& first, Args const&... args)
		requires (... && Type::Convertible<Args, String>) {
			return first + "|" + (... + "|" + args);
		}

		constexpr String PACKS		= concat(STRINGS, PARENTHESES, ANGLE_BRACKETS, LINE_COMMENTS, BLOCK_COMMENTS);
		constexpr String ALL_TOKENS	= concat(COMPLEX_TOKEN + "+", PACKS);
		constexpr String ALL_TOKENS	= concat(ANY_PARAM_CHAR + "+", PACKS);
	}

	struct SyntaxTree {
		struct ParameterPack {
			StringList args;

			constexpr ParameterPack() {}

			ParameterPack(String const& packString) {	
			}

			constexpr ParameterPack(StringList const& args): args(args) {}

			template<class... Args>
			constexpr explicit ParameterPack(Args const&... args)
			requires (... && Type::Convertible<Args, String>):
				args(StringList{args...}) {}
			
			ParameterPack(ParameterPack const& other)	= default;
			ParameterPack(ParameterPack&& other)		= default;
		};

		struct Token {
			Operation		type;
			String			name	= "";
			ParameterPack	params	= ParamPack();
			uint64			mode	= 0;

			constexpr uint16 operation(uint16 const sp) {
				if (sp)
					return enumcast(type) | (sp << 12);
				return enumcast(type) | (mode << 12);
			}
		};

		using Tokens = List<Token>;
		Tokens tokens;
			
		SyntaxTree(SyntaxTree const& other)	= default;
		SyntaxTree(SyntaxTree&& other)		= default;

		SyntaxTree(StringList const& nodes) {
			if (nodes.empty())
				throw Error::NonexistentValue("No nodes were given!", CTL_CPP_PRETTY_SOURCE);
			for (usize i = 0; i < nodes.size(); ++i) {
				String node = nodes[i], next;
				if (i+1 < nodes.size())
					next = nodes[i+1];
				switch (node[0]) {
					case '/': continue;
					case '@': {
						assertValidNamedNode(node);
						if (next.size() && next[0] == '(') {
							tokens.pushBack({
								.type	= Operation::DVM_O_ACTION,
								.name	= node.substring(1),
								.params	= next
							});
							++i;
						}
					} break;
					case '$': {
						assertValidNamedNode(node);
						String const name = node.substring(1);
						if (next.size()) {
							if (next[0] == '(')
								tokens.pushBack({
									.type	= Operation::DVM_O_NAMED_CALL,
									.name	= name,
									.params	= next
								});
							else if (next[0] == '"')
								tokens.pushBack({
									.type	= Operation::DVM_O_NAMED_CALL,
									.name	= name,
									.params	= ParameterPack(next.substring(1, next.size()-1))
								});
							else if (!Regex::count(next, RegexMatches::NON_NAME_CHAR))
								tokens.pushBack({
									.type	= Operation::DVM_O_NAMED_CALL,
									.name	= name,
									.params	= ParameterPack(next)
								});
							else throw Error::InvalidValue(
								toString("Invalid value of '", next, "' for '", node, "'!"),
								CTL_CPP_PRETTY_SOURCE
							);
						} else throw Error::InvalidValue(
							toString("Missing value for '", node, "'!"),
							"Maybe you confused '$' with '+' or '-', perhaps?",
							CTL_CPP_PRETTY_SOURCE
						);
						++i;
					} break;
					case '!': {
						assertValidNamedNode(node);
						tokens.pushBack({
							.type	= Operation::DVM_O_EMOTION,
							.name	= node.substring(1)
						});
					} break;
					case '\'': {
						assertValidNamedNode(node);
						tokens.pushBack({
							.type	= Operation::DVM_O_EMOTION,
							.params	= ParameterPack(node.substring(1))
						});
					} break;
					case '#': {
						
					}
				}
			}
		}

	private:
		void assertValidNamedNode(String const& node) {
			if (node.size() < 2)
				throw Error::InvalidValue(
					toString("Invalid token '", node, "'!"),
					"Token is too small!",
					CTL_CPP_PRETTY_SOURCE
				);
		}
	};

	/// @brief Dialog compiler.
	struct Compiler {
		/// @brief Empty constructor.
		Compiler()										{					}
		/// @brief Compiles a dialog source.
		/// @param source Source to compile.
		Compiler(String const& source): source(source)	{compileScript();	}
	
	private:
		/// @brief Whether the dialog source was compiled.
		bool	compiled = false;
		/// @brief Dialog source code.
		String	source;
		/// @brief Resulting compiled dialog program.
		Dialog	binary;
		/// @brief Current data index.
		usize	dataIndex = 1;
		/// @brief Values used for errors.		
		usize	lineIndex = 1, columnIndex = 1;

	};

	/// @brief Dialog source compiler.
	struct Compiler {
		/// @brief Empty constructor.
		Compiler()										{					}
		/// @brief Compiles a dialog source.
		/// @param source Source to compile.
		Compiler(String const& source): source(source)	{compileScript();	}

		/// @brief Returns the compiled dialog program.
		/// @return Compiled dialog program.
		Dialog result() {
			return out;
		}

		/// @brief Compiles the dialog.
		/// @return Reference to self.
		Compiler& compile(String const& source) {
			this->source = source;
			compiled = false;
			compileScript();
			return *this;
		}

		/// @brief Returns whether the given character is a valid name character.
		/// @param c Character to check.
		/// @return Whether character is a valid name character.
		constexpr static bool isNameChar(char const& c) {
			return (
				isNumberChar(c)
			||	isUppercaseChar(c)
			||	isLowercaseChar(c)
			||	(c == '_')
			||	(c == '-')
			);
		}

		/// @brief Returns whether the given character is a valid extendable operation.
		/// @param c Character to check.
		/// @return Whether character is a valid extendable operation.
		constexpr static bool isExtendedOperationChar(char const& c) {
			return (
				(c == '"')
			||	(c == '[')
			);
		}
	
	private:
		/// @brief Whether the dialog source was compiled.
		bool	compiled = false;
		/// @brief Dialog source code.
		String	source;
		/// @brief Resulting compiled dialog program.
		Dialog	out;
		/// @brief Current data index.
		usize	dataIndex = 1;
		/// @brief Values used for errors.		
		usize	lineIndex = 1, columnIndex = 1;

		struct ScopeDelimiter {
			char begin, end;
			constexpr ScopeDelimiter(char const ch) {
				switch(ch) {
					case '[':
					case ']': begin = '['; end = ']'; break;
					case '(':
					case ')': begin = '('; end = ')'; break;
					case '{':
					case '}': begin = '{'; end = '}'; break;
					case '<':
					case '>': begin = '<'; end = '>'; break;
					case '"': begin = end = '"'; break;
					case '\'': begin = end = '\''; break;
				}
			}
		};

		Operation curOp = Operation::DVM_O_NO_OP;

		static void opcopy(Decay::AsType<uint16[4]>& buf, uint64 const val) {
			MX::memcpy((void*)buf, (void*)&val, sizeof(uint64));
		}

		void initialize() {
			out = {.data = {""}};
			dataIndex	= 1;
			lineIndex	= 1;
			columnIndex	= 1;
		}

		void compileScript() {
			if (compiled) return;
			compiled = true;
			initialize();
			removeComments();
			processScript();
		}

		static String format(String str) {
			str = Regex::replace(str, "\\\\", "\\");
			str = Regex::replace(str, "\\/", "/");
			str = Regex::replace(str, "\\n", "\n");
			str = Regex::replace(str, "\\t", "\t");
			str = Regex::replace(str, "\\v", "\v");
			str = Regex::replace(str, "\\r", "\r");
			str = Regex::replace(str, "\\f", "\f");
			str = Regex::replace(str, "\\b", "\b");
			str = Regex::replace(str, "\\0", "");
			str = Regex::replace(str, "\\\"", "\"");
			return str.terminated();
		}

		void addOperation(Operation const op, uint16 sp = 0) {
			out.code.pushBack(enumcast(op) | spFlag(sp));
		}

		void addOperand(uint64 const op) {
			uint16 opbuf[4];
			opcopy(opbuf, op);
			out.code.appendBack(opbuf);
		}

		void addStringOperand(String const& str) {
			addOperand(dataIndex++);
			out.data.pushBack(format(str));
		}

		void addLine(String const& str) {
			addOperation(Operation::DVM_O_LINE);
			if (str.empty())
				return addOperand(0);
			addStringOperand(str);
		}

		void addActors(StringList const& strs) {
			if (strs.empty()) {
				addOperation(Operation::DVM_O_ACTOR);
				addOperand(0);
				return;
			}
			uint16 esp = 0;
			if (strs[0] == "...") esp = 0b10;
			for (usize i = 0; i < strs.size(); ++i) {
				if (strs[i] == "...") {
					if (i != 0)
						duplicateError();
					addOperation(Operation::DVM_O_ACTOR, esp);
					continue;
				}
				addOperation(Operation::DVM_O_ACTOR, (i) ? 1 : 0);
				addOperand(Hasher::hash(strs[i]));
			}
		}

		void addEmotion(String const& str) {
			addOperation(Operation::DVM_O_EMOTION);
			addOperand(Hasher::hash(str));
		}

		void addAction(String const& str, bool const sp = false) {
			addOperation(Operation::DVM_O_ACTION, sp ? 1 : 0);
			addOperand(Hasher::hash(str));
		}

		void addFlag(String const& str, bool const state = false) {
			addOperation(Operation::DVM_O_NAMED_CALL);
			addOperand(Hasher::hash(str));
			addStringOperand(state ? "true" : "false");
		}

		void addGlobal(String const& str, bool const sp = false) {
			addOperation(Operation::DVM_O_NAMED_CALL, sp ? 1 : 0);
		}

		void addColor(String const& str) {
			addOperation(Operation::DVM_O_COLOR);
			addOperand(Graph::Color::toHexCodeRGBA(Graph::Color::fromHexCodeString(str)));
		}

		void addColorRef(String const& str) {
			addOperation(Operation::DVM_O_COLOR, 1);
			addOperand(Hasher::hash(str));
		}

		void addWait(String const& str) {
			addOperation(Operation::DVM_O_WAIT);
			addOperand(toUInt64(str));
		}

		void addParamPack(StringList const& strs) {
			for (String const& str: strs)
				out.data.pushBack(format(str));
			if (strs.empty()) {
				addOperand(0);
				addOperand(0);
				return;
			}
			addOperand(dataIndex);
			addOperand(strs.size());
			dataIndex += strs.size();
		}

		template<class T>
		StringList processParamPack(T& c, T& end, ScopeDelimiter const& sd = ']') {
			String buf;
			StringList params;
			bool space = true;
			while (c < end && *c != sd.end) {
				lineIterate(*c);
				if (!isNullOrSpaceChar(*c) && *c != ',' && space)
					invalidParameterError();
				else if (isNullOrSpaceChar(*c)) {
					if (!buf.empty())
						space = true;
				} else if (isScopeChar(*c) && !isQuoteChar(*c))
					params.appendBack(processParamPack(c, end, *c++))
				else if (isQuoteChar(*c))
					params.pushBack(processString(c, end, *c++));
				else if (*c == ',') {
					params.pushBack(buf.stripped().terminated());
					buf.clear();
				} else buf.pushBack(*c);
				++c;
			}
			if (!buf.empty()) params.pushBack(buf.stripped().terminated());
			return params;
		}

		template<class T>
		String processString(T& c, T& end, char const scope = '"') {
			String buf;
			while (c < end && *c != scope) {
				lineIterate(*c);
				if (*c == '\\') {
					buf.pushBack(*c++);
					if (c >= end) malformedError();
				}
				buf.pushBack(*c++);
			}
			return format(buf);
		}

		template<class T>
		String processCommand(T& c, T& end) {
			String buf;
			if (!isNameChar(*c))
				malformedError();
			while (c < end && isNameChar(*c)) {
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			if (buf.empty()) malformedError();
			return buf.terminated();
		}

		template<class T>
		String processHex(T& c, T& end) {
			String buf;
			auto start = c;
			if (!isHexChar(*c))
				malformedError();
			while (c < end && isHexChar(*c) && (c - start) < 8) {
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			if (buf.empty()) malformedError();
			return buf;
		}

		template<class T>
		String processNumberInt(T& c, T& end) {
			String buf;
			auto start = c;
			if (!isNumberChar(*c))
				malformedError();
			while (c < end && isNumberChar(*c)) {
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			if (buf.empty()) malformedError();
			return buf;
		}

		template<class T>
		String processNumberFloat(T& c, T& end) {
			String buf;
			auto start = c;
			if (!isNumberChar(*c))
				malformedError();
			bool dotted = false;
			while (c < end && (isNumberChar(*c) || (*c == '.'))) {
				if (*c == '.') {
					if (!dotted) dotted = true;
					else break;
				}
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			if (buf.empty()) malformedError();
			return buf;
		}

		void processScript() {
			auto c		= source.begin();
			auto end	= source.end();
			while (c != end) {
				lineIterate(*c);
				switch (*c) {
					case '*':
						while (c < end && isNullOrSpaceChar(*c))
							lineIterate(*++c);
						if (!isExtendedOperationChar(*c))
							invalidExtendedOperationError();
						addOperation(Operation::DVM_O_NO_OP, 1);
						break;
					case '.':
						addOperation(Operation::DVM_O_SYNC);
						break;
					case ';':
						addOperation(Operation::DVM_O_USER_INPUT);
						break;
					case '"':
						addLine(processString(++c, end));
						break;
					case '[':
						addActors(processParamPack(++c, end, ']'));
						break;
					case '(':
						addParamPack(processParamPack(++c, end, ')'));
						break;
					case '@': {
						String cmd = processCommand(++c, end);
						while (c != end && isNullOrSpaceChar(*c)) lineIterate(*++c);
						if (*c == '(') {
							auto pp = processParamPack(++c, end, ')');
							if (pp.empty()) addAction(cmd, 0);
							else if (pp.size() == 1) {
								addAction(cmd);
								addStringOperand(pp.back());
							} else {
								addAction(cmd, 1);
								addParamPack(pp);
							}
						} else addAction(cmd, 0);
						break;
					}
					case '!':
						addEmotion(processCommand(++c, end));
						break;
					case '+':
						addFlag(processCommand(++c, end), true);
						break;
					case '-':
						addFlag(processCommand(++c, end), false);
						break;
					case '$': {
						String cmd = processCommand(++c, end);
						StringList args;
						while (c < end && isNullOrSpaceChar(*c)) lineIterate(*++c);
						if (*c == '(')
							args = processParamPack(++c, end, ')');
						addGlobal(cmd, args.size() > 1);
						if (args.size() > 1)
							addParamPack(args);
						else if (args.size() == 1)
							addStringOperand(args.back());
						else addStringOperand(
							isQuoteChar(*c++)
							? processString(c, end)
							: processCommand(c, end)
						);
						break;
					}
					case '#':
						lineIterate(*c++);
						if (c == end) malformedError();
						if (*c == '#')
							addColorRef(processCommand(++c, end));
						addColor(processHex(c, end));
					case '\'':
						addWait(processNumberInt(++c, end));
						break;
					default: if (!isNullOrSpaceChar(*c)) invalidOperationError();
				}
				++c;
			}
		}

		void removeComments() {
			source = Regex::replace(source, "(\\/\\/.*$|\\/\\*(.|\\n)*?(\\*\\/))+", "");
			source = Regex::replace(source, "(\\/\\*(.*|\\n)*)", "");
		}

		void lineIterate(char const c) {
			if (c == '\n') {
				columnIndex = 1;
				++lineIndex;
			} else ++columnIndex;
		}
		
		[[noreturn]]
		void invalidOperationError() {
			throw Error::InvalidValue(
				"Invalid operation!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}

		[[noreturn]]
		void invalidExtendedOperationError() {
			throw Error::InvalidValue(
				"This operation is not an extensible operation!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}
		
		[[noreturn]]
		void malformedError() {
			throw Error::InvalidValue(
				"Malformed operation/parameter!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}

		[[noreturn]]
		void duplicateError() {
			throw Error::InvalidValue(
				"Duplicate operation/parameter!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}

		[[noreturn]]
		void invalidParameterError() {
			throw Error::InvalidValue(
				"Invalid parameter!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}
	};
}

#endif