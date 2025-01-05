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
			Operation		type	= Operation::DVM_O_NO_OP;
			String			name	= String();
			uint64			value	= 0;
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
							.type	= Operation::DVM_O_WAIT,
							.params	= ParameterPack(node.substring(1))
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
							.params = ParameterPack(node)
						});
					} break;
					case '*': tokens.pushBack({.mode = 1});						break;
					case '.': tokens.pushBack({Operation::DVM_O_SYNC});			break;
					case ';': tokens.pushBack({Operation::DVM_O_USER_INPUT});	break;
					case '+':
					case '-': {
						tokens.pushBack({
							.type	= Operation::DVM_O_NAMED_CALL,
							.params	= ParameterPack((node[0] == '+') ? "true" : "false")
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
		constexpr Binary(): Dialog{StringList({"true", "false"})} {}

		constexpr Binary& addOperation(uint16 const op) {
			code.pushBack(op);
		}

		constexpr Binary& addOperand(uint64 const op) {
			Decay::AsType<uint16[4]> opval;
			opcopy(opval, op);
			code.pushBack(opval[0]);
			code.pushBack(opval[1]);
			code.pushBack(opval[2]);
			code.pushBack(opval[3]);
		}

	private:
		constexpr static void opcopy(Decay::AsType<uint16[4]>& buf, uint64 const val) {
			MX::memcpy((void*)buf, (void*)&val, sizeof(uint64));
		}
	};
}

#endif