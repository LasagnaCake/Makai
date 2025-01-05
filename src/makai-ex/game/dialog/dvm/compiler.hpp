#ifndef MAKAILIB_EX_GAME_DIALOG_DVM_COMPILER_H
#define MAKAILIB_EX_GAME_DIALOG_DVM_COMPILER_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

/// @brief Dialog Virtual Machine.
namespace Makai::Ex::Game::Dialog::DVM::Compiler {
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

		constexpr String PACKS			= concat(STRINGS, PARENTHESES, ANGLE_BRACKETS, LINE_COMMENTS, BLOCK_COMMENTS);
		constexpr String ALL_TOKENS		= concat(COMPLEX_TOKEN + "+", PACKS);
		constexpr String ALL_PARAMETERS	= concat(ANY_PARAM_CHAR + "+", PACKS);
	}

	struct SyntaxTree {
		struct ParameterPack {
			StringList args;

			constexpr ParameterPack() {}

			static ParameterPack fromString(String const& str) {
				ParameterPack pack;
				auto matches = Regex::find(str.sliced(1, -2), RegexMatches::ALL_PARAMETERS);
				StringList nodes;
				usize index = 0;
				for (auto& match: matches) {
					auto& arg = pack.args.pushBack(match.match.stripped()).back();
					if (arg == "..." && index != 0)
						throw Error::InvalidValue(
							toString("Invalid value list '", str, "'!"),
							"'...' may ONLY appear at the beginning of the value list!",
							CTL_CPP_PRETTY_SOURCE
						);
					if (
						Regex::count(arg, RegexMatches::NON_NAME_CHAR) > 0
					&&	!Regex::matches(arg, RegexMatches::PACKS)
					)
						throw Error::InvalidValue(
							toString("Invalid value list '", str, "'!"),
							toString("'", arg, "' is not a valid value!"),
							CTL_CPP_PRETTY_SOURCE
						);
					++index;
				}
				return pack;
			}

			constexpr ParameterPack(StringList const& args): args(args) {}

			template<class... Args>
			constexpr explicit ParameterPack(Args const&... args)
			requires (... && Type::Convertible<Args, String>):
				args(StringList({args...})) {}
			
			ParameterPack(ParameterPack const& other)	= default;
			ParameterPack(ParameterPack&& other)		= default;
		};

		struct Token {
			Operation		type	= Operation::DVM_O_NO_OP;
			String			name	= String();
			uint64			value	= 0;
			ParameterPack	pack	= ParamPack();
			uint64			mode	= 0;

			constexpr uint16 operation(uint16 const sp = 0) const {
				if (sp)
					return enumcast(type) | (sp << 12);
				return enumcast(type) | (mode << 12);
			}

			constexpr operator uint16() const {return operation();}
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
								.pack	= ParameterPack::fromString(next)
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
									.pack	= ParameterPack::fromString(next)
								});
							else if (next[0] == '"')
								tokens.pushBack({
									.type	= Operation::DVM_O_NAMED_CALL,
									.name	= name,
									.pack	= ParameterPack(next.sliced(1, -2))
								});
							else if (!Regex::count(next, RegexMatches::NON_NAME_CHAR))
								tokens.pushBack({
									.type	= Operation::DVM_O_NAMED_CALL,
									.name	= name,
									.pack	= ParameterPack(next)
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
							.type	= Operation::DVM_O_WAIT,
							.value	= toUInt64(node.substring(1))
						});
					} break;
					case '#': {
						assertValidNamedNode(node, 4);
						if (node[1] == '#')
							tokens.pushBack({
								.type	= Operation::DVM_O_EMOTION,
								.value	= ConstHasher::hash(node.substring(2)),
								.mode	= 1
							});
						tokens.pushBack({
							.type	= Operation::DVM_O_EMOTION,
							.value	= Graph::Color::toHexCodeRGBA(Graph::Color::fromHexCodeString(node.substring(1)))
						});
					} break;
					case '[': {
						tokens.pushBack({
							.type	= Operation::DVM_O_ACTOR,
							.pack	= ParameterPack::fromString(node)
						});
					} break;
					case '*': tokens.pushBack({.mode = 1});						break;
					case '.': tokens.pushBack({Operation::DVM_O_SYNC});			break;
					case ';': tokens.pushBack({Operation::DVM_O_USER_INPUT});	break;
					case '+':
					case '-': {
						tokens.pushBack({
							.type	= Operation::DVM_O_NAMED_CALL,
							.pack	= ParameterPack((node[0] == '+') ? "true" : "false")
						}); break;
					}
					case '(': continue;
					default:
						throw Error::InvalidValue(
							toString("Invalid operation '" + node + "'!"),
							CTL_CPP_PRETTY_SOURCE
						);
				}
			}
			if (tokens.empty())
				throw Error::FailedAction(
					"Failed to parse tree!",
					CTL_CPP_PRETTY_SOURCE
				);
		}

		static SyntaxTree fromSource(String const& src) {
			auto matches = Regex::find(src, RegexMatches::ALL_TOKENS);
			StringList nodes;
			nodes.resize(matches.size());
			for (auto& match: matches) {
				nodes.pushBack(match.match);
			}
			return SyntaxTree(nodes);
		}

	private:
		void assertValidNamedNode(String const& node, usize const min = 2) {
			if (node.size() < min)
				throw Error::InvalidValue(
					toString("Invalid operation '", node, "'!"),
					"Name is too small!",
					CTL_CPP_PRETTY_SOURCE
				);
		}
	};

	struct Binary: Dialog {
		constexpr Binary(): Dialog{
			.data = StringList({"true", "false"})
		} {}

		constexpr Binary& addOperation(uint16 const op) {
			code.pushBack(op);
			return *this;
		}

		constexpr Binary& addOperand(uint64 const op) {
			uint16 opbuf[4];
			opcopy(opbuf, op);
			code.appendBack(opbuf);
			return *this;
		}

		constexpr Binary& addStringOperand(String const& str) {
			addOperand(data.size()+1);
			data.pushBack(str);
			return *this;
		}

		constexpr Binary& addNamedOperand(String const& name) {
			return addOperand(ConstHasher::hash(name));
		}

		constexpr Binary& addParameterPack(StringList const& params) {
			addOperand(data.size()+1);
			addOperand(params.size());
			data.appendBack(params);
			return *this;
		}

		constexpr FileHeader header() const {
			FileHeader fh;
			fh.jumps	= {fh.headerSize, jumps.size()};
			fh.data		= {fh.jumps.offset(), 0};
			for (String const& s: data) fh.data.size += s.nullTerminated() ? s.size() : s.size()+1;
			fh.code		= {fh.data.offset(), code.size()};
			return fh;
		}

		static Binary fromTree(SyntaxTree const& tree) {
			Binary out;
			for (auto& token: tree.tokens) {
				switch (token.type) {
					case Operation::DVM_O_NO_OP:
					case Operation::DVM_O_HALT:
					case Operation::DVM_O_SYNC:
					case Operation::DVM_O_USER_INPUT: out.addOperation(token); break;
					case Operation::DVM_O_LINE:
						out.addOperation(token);
						out.addStringOperand(token.pack.args[0]);
						break;
					case Operation::DVM_O_ACTOR:
						for (usize i = 0; i < token.pack.args.size(); ++i) {
							if (token.pack.args[i] == "...") {
								out.addOperation(token.operation(i));
								continue;
							}
							out.addOperation(token.operation(i > 0));
							out.addNamedOperand(token.pack.args[i]);
						}
						break;
					case Operation::DVM_O_EMOTION:
						out.addOperation(token);
						out.addNamedOperand(token.name);
						break;
					case Operation::DVM_O_JUMP:
					case Operation::DVM_O_WAIT:
					case Operation::DVM_O_COLOR:
						out.addOperation(token);
						out.addOperand(token.value);
						break;
					case Operation::DVM_O_ACTION:
						out.addOperation(token.operation(token.pack.args.size() > 0));
						if (token.pack.args.size())
							out.addParameterPack(token.pack.args);
						break;
					case Operation::DVM_O_NAMED_CALL:
						out.addOperation(token.operation(token.pack.args.size() > 2));
						if (token.pack.args.size() > 2)
							out.addParameterPack(token.pack.args);
						else {
							String const& val = token.pack.args[0];
							if (val == "true")			out.addOperand(2);
							else if (val == "false")	out.addOperand(1);
							else						out.addStringOperand(val);
						}
						break;
				}
			}
			return out;
		}

	private:
		constexpr static void opcopy(Decay::AsType<uint16[4]>& buf, uint64 const val) {
			MX::memcpy((void*)buf, (void*)&val, sizeof(uint64));
		}
	};

	Binary const compileSource(String const& source) {
		return Binary::fromTree(SyntaxTree::fromSource(source));
	}

	Binary const compileFile(String const& path) {
		return compileSource(File::getText(path));
	}
}

#endif