#ifndef MAKAILIB_EX_ANIMA_COMPILER_H
#define MAKAILIB_EX_ANIMA_COMPILER_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

/// @brief Anima Virtual Machine.
namespace Makai::Ex::AVM::Compiler {
	/// @brief Regex matches used for processing.
	namespace RegexMatches {
		/// @brief Matches any character.
		const static String ANY_CHAR		= String("[\\S\\s]");
		/// @brief Matches any parameter character, except commas.
		const static String PARAM_CHAR		= String("[^,]");
		/// @brief Matches any valid name character.
		const static String NAME_CHAR		= String("[0-z\\-_]");
		/// @brief Matches any invalid name character.
		const static String NON_NAME_CHAR	= String("[^0-z\\-_]");
		/// @brief Matches any complex token.
		const static String COMPLEX_TOKEN	= String("[\\w&!@#$&+\\-_'\\:\\~]");
		/// @brief Matches any simple token.
		const static String SIMPLE_TOKEN	= String("[*.,;]");

		/// @brief Creates a regex that lazily matches all characters between the given tokens.
		/// @param begin Start token.
		/// @param end End token.
		/// @return Regex.
		constexpr String makePack(String const& begin, String const& end) {
			return begin + ANY_CHAR + "*?" + end;
		}

		/// @brief Matches any text string.
		const static String STRINGS			= String("\"(?:[^\"\\\\]|\\\\.)*\"");
		/// @brief Matches any parens pack.
		const static String PARENTHESES		= makePack("\\(", "\\)");
		/// @brief Matches any brackets pack.
		const static String BRACKETS		= makePack("\\[", "\\]");
		/// @brief Matches any squiggle brackets pack.
		const static String SQUIGGLIES		= makePack("\\{", "\\}");
		/// @brief Matches line comments.
		const static String LINE_COMMENTS	= String("\\/\\/.*");
		/// @brief Matches block comments.
		const static String BLOCK_COMMENTS	= makePack("\\/\\*", "\\*\\/");

		/// @brief Concatenats a series of regexes into one to match any.
		/// @tparam ...Args Argument types.
		/// @param first First regex.
		/// @param ...args Following regexes.
		/// @return Concatenated regex.
		template<class... Args>
		constexpr String concat(String const& first, Args const&... args)
		requires (... && Type::Convertible<Args, String>) {
			return first + (... + ("|" + args));
		}

		/// @brief Matches all packs.
		const static String PACKS			= concat(STRINGS, PARENTHESES, BRACKETS, SQUIGGLIES, LINE_COMMENTS, BLOCK_COMMENTS);
		/// @brief Matches all tokens.
		const static String ALL_TOKENS		= concat(COMPLEX_TOKEN + "+", SIMPLE_TOKEN, PACKS);
		/// @brief Matches all parameter tokens.
		const static String ALL_PARAMETERS	= concat(PARAM_CHAR + "+", PACKS);
	}

	/// @brief Unescapes a character.
	/// @param c Character to unescape.
	/// @return Unescaped character.
	constexpr char unescape(char const c) {
		switch (c) {
			case '0': return ' ';
			case 'n': return '\n';
			case 'v': return '\v';
			case 't': return '\t';
			case 'b': return '\b';
			case 'r': return '\r';
			case 'f': return '\f';
			default: return c;
		}
		return c;
	}

	/// @brief Processes all escape sequences on the string.
	/// @param str String to normalize.
	/// @return Normalized string.
	constexpr String normalize(String str) {
		//str.strip();
		String out;
		bool escape = false;
		for (auto& c: str) {
			if (c == '\\') {
				escape = true;
				continue;
			}
			else if (escape) {
				out.pushBack(unescape(c));
				continue;
			} else out.pushBack(c);
		}
		return out;
	}

	/// @brief Structural representation of the program.
	struct OperationTree {
		/// @brief Parameter pack.
		struct ParameterPack {
			/// @brief Parameter pack arguments.
			StringList args;

			/// @brief Default constructor.
			constexpr ParameterPack() {}

			/// @brief Creates a parameter pack from a parameter pack string.
			/// @param str String to create from.
			/// @return Parameter pack.
			/// @throw Error::InvalidValue on syntax errors.
			static ParameterPack fromString(String const& str) {
				ParameterPack pack;
				auto matches = Regex::find(str.sliced(1, -2), RegexMatches::ALL_PARAMETERS);
				StringList nodes;
				usize index = 0;
				for (auto& match: matches) {
					auto& arg = pack.args.pushBack(match.match).back();
					DEBUG(index, ": ", arg, " ");
					arg.strip();
					if (arg == String("...") && index != 0) {
						DEBUGLN("\t[! INVALID EXPANSION EXPRESSION !]");
						throw Error::InvalidValue(
							toString("Invalid values '", str, "'!"),
							"'...' may ONLY appear at the beginning of the value list!",
							CTL_CPP_PRETTY_SOURCE
						);
					}
					else if (arg == String("...")) {
						DEBUGLN("\t[Valid expansion expression]");
						pack.args.pushBack(arg);
						continue;
					}
					DEBUGLN("\t[Parameter]");
					switch (arg[0]) {
						case '"': arg = normalize(arg.sliced(1, -2)); break;
						case '(':
						case '[': {
							String a = pack.args.popBack();
							ParameterPack pp = ParameterPack::fromString(a);
							pack.args.appendBack(pp.args);
						} break;
						default:
							if (Regex::count(arg, RegexMatches::NON_NAME_CHAR) > 0)
								throw Error::InvalidValue(
									toString("Invalid values '", str, "'!"),
									toString("Value '", arg, "' (at position [", index+1, "]) is invalid!\n"
									"Names must only contain letters, numbers, '-' and '_'!"),
									CTL_CPP_PRETTY_SOURCE
								);
					}
					++index;
				}
				return pack;
			}

			/// @brief Constructs a parameter pack from a list of strings.
			/// @param args Strings to construct from.
			constexpr ParameterPack(StringList const& args): args(args) {}

			/// @brief Constructs a parameter pack from a series of values.
			/// @tparam ...Args Argument types.
			/// @param ...args Values to construct from.
			template<class... Args>
			constexpr explicit ParameterPack(Args const&... args)
			requires (... && Type::Convertible<Args, String>):
				args(StringList{args...}) {}
			
			/// @brief Copy constructor (defaulted).
			ParameterPack(ParameterPack const& other)	= default;
			/// @brief Move constructor (defaulted).
			ParameterPack(ParameterPack&& other)		= default;
		};

		/// @brief Operation token.
		struct Token {
			/// @brief Operation type.
			Operation		type	= Operation::AVM_O_NO_OP;
			/// @brief Operation name. Used by some types.
			String			name	= String();
			/// @brief Operation value. Used by some types.
			uint64			value	= 0;
			/// @brief Operation parameters. Used by some types.
			ParameterPack	pack	= ParameterPack();
			/// @brief Operation mode.
			uint64			mode	= 0;
			/// @brief Jump target for a given jump.
			String			entry	= "";

			/// @brief Returns the token's operation.
			/// @param sp SP mode override. Only used if non-zero. By default, it is zero.
			/// @return Operation.
			constexpr uint16 operation(uint16 const sp = 0) const {
				if (sp)
					return enumcast(type) | (sp << 12);
				return enumcast(type) | (mode << 12);
			}

			/// @brief Returns the token's operation.
			/// @return Operation.
			constexpr operator uint16() const {return operation();}
		};

		/// @brief Token list.
		using Tokens = List<Token>;
		/// @brief Token tree operation tokens.
		Tokens tokens;
			
		/// @brief Copy constructor (defaulted).
		OperationTree(OperationTree const& other)	= default;
		/// @brief Move constructor (defaulted).
		OperationTree(OperationTree&& other)		= default;

		/// @brief Constructs the token tree from a series of source file nodes.
		/// @param nodes Nodes to build from.
		/// @throw Error::NonexistentValue when node list is empty.
		/// @throw Error::FailedAction if compilation fails.
		/// @throw Error::InvalidValue on syntax errors.
		OperationTree(StringList const& nodes) {
			if (nodes.empty())
				throw Error::NonexistentValue("No nodes were given!", CTL_CPP_PRETTY_SOURCE);
			for (usize i = 0; i < nodes.size(); ++i) {
				String node = nodes[i], next;
				if (i+1 < nodes.size())
					next = nodes[i+1];
				DEBUGLN(node);
				switch (node[0]) {
					case '/': continue;
					case '@': {
						assertValidNamedNode(node);
						if (next.size() && next[0] == '(') {
							tokens.pushBack(Token{
								.type	= Operation::AVM_O_ACTION,
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
								tokens.pushBack(Token{
									.type	= Operation::AVM_O_NAMED_CALL,
									.name	= name,
									.pack	= ParameterPack::fromString(next)
								});
							else if (next[0] == '"')
								tokens.pushBack(Token{
									.type	= Operation::AVM_O_NAMED_CALL,
									.name	= name,
									.pack	= ParameterPack(normalize(next.sliced(1, -2)))
								});
							else if (!Regex::count(next, RegexMatches::NON_NAME_CHAR))
								tokens.pushBack(Token{
									.type	= Operation::AVM_O_NAMED_CALL,
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
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_EMOTION,
							.name	= node.substring(1)
						});
					} break;
					case '\'': {
						assertValidNamedNode(node);
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_WAIT,
							.value	= toUInt64(node.substring(1))
						});
					} break;
					case '\"': {
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_LINE,
							.pack	= ParameterPack(normalize(next.sliced(1, -2)))
						});
					} break;
					case '#': {
						assertValidNamedNode(node, 4);
						if (node[1] == '#')
							tokens.pushBack(Token{
								.type	= Operation::AVM_O_COLOR,
								.value	= ConstHasher::hash(node.substring(2)),
								.mode	= 1
							});
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_COLOR,
							.value	= hexColor(node.substring(1))
						});
					} break;
					case ':': {
						assertValidNamedNode(node);
						addExtendedOperation(node, next, i, nodes);
					} break;
					case '[': {
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_ACTOR,
							.pack	= ParameterPack::fromString(node)
						});
					} break;
					case '*': tokens.pushBack(Token{.mode = 1});					break;
					case '.': tokens.pushBack(Token{Operation::AVM_O_SYNC});		break;
					case ';': tokens.pushBack(Token{Operation::AVM_O_USER_INPUT});	break;
					case '+':
					case '-': {
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_NAMED_CALL,
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

		/// @brief Creates an operation tree from a source file.
		/// @param src Source file to construct tree from.
		/// @return Operation tree.
		/// @throw Error::NonexistentValue if source file is empty.
		static OperationTree fromSource(String const& src) {
			DEBUGLN("Tokenizer regex: ", RegexMatches::ALL_TOKENS);
			if (src.empty())
				throw Error::NonexistentValue(
					"Source is empty!",
					CTL_CPP_PRETTY_SOURCE
				);
			auto matches = Regex::find(src, RegexMatches::ALL_TOKENS);
			if (matches.empty())
				throw Error::NonexistentValue(
					"Failed to split source tree!",
					CTL_CPP_PRETTY_SOURCE
				);
			StringList nodes;
			nodes.resize(matches.size());
			DEBUGLN("\nParsing tree...\n");
			for (auto& match: matches)
				nodes.pushBack(match.match);
			return OperationTree(nodes);
		}

	private:
		void addActBlock(String const& act, String const& block, char const sep = '*') {
			DEBUGLN("<block:", act, ">");
			auto optree = OperationTree::fromSource(block.sliced(1, -2));
			auto const end = act + "[end]";
			for (auto& token : optree.tokens) {
				if (token.entry.size())
					token.entry = act + sep + token.entry;
				if (token.type == Operation::AVM_O_JUMP)
					token.name = act + sep + token.name;
			}
			optree.tokens.front().entry = act;
			optree.tokens.pushBack(Token{
				.type = Operation::AVM_O_HALT,
				.mode = 1
			});
			optree.tokens.pushBack(Token{
				.type = Operation::AVM_O_NO_OP,
				.entry = end,
			});
			auto& last = tokens.back();
			if (last.type == Operation::AVM_O_NO_OP && last.mode == 0) {
				last.type = Operation::AVM_O_JUMP;
				last.name = end;
			} else tokens.pushBack(Token{
				.type = Operation::AVM_O_JUMP,
				.name = end
			});
			tokens.appendBack(optree.tokens);
			DEBUGLN("</block:", act, ">");
		}

		constexpr void addExtendedOperation(String const& op, String const& val, usize& curNode, StringList const& nodes) {
			switch (auto ophash = ConstHasher::hash(op)) {
				case (ConstHasher::hash(":perform")):
				case (ConstHasher::hash(":next")): {
					if (val.empty())
						throw Error::InvalidValue(
							toString("Missing value for '", op, "'!"),
							CTL_CPP_PRETTY_SOURCE
						);
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_JUMP,
						.name	= val,
						.mode	= ophash == ConstHasher::hash(":perform")
					});
					++curNode;
					return;
				}
				case (ConstHasher::hash(":chapter")):
				case (ConstHasher::hash(":act")): {
					if (val.empty())
						throw Error::InvalidValue(
							toString("Missing act/chapter name!"),
							CTL_CPP_PRETTY_SOURCE
						);
					if (
						nodes.size() < curNode + 2 
					||	nodes[curNode + 2].front() != '{'
					||	nodes[curNode + 2].back() != '}'
					)
						throw Error::InvalidValue(
							toString("Missing/invalid block for '", op, " ", val, "'!"),
							CTL_CPP_PRETTY_SOURCE
						);
					addActBlock(val, nodes[curNode+2], (ophash == ConstHasher::hash(":story")) ? ':' : '*');
					curNode += 2;
					return;
				}
				default:
				throw Error::InvalidValue(
					toString("Invalid operation '", op, "'!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
		}

		constexpr uint32 asByte(As<char const[2]> const& nibbles) {
			return (
				uint32(nibbles[0] - '0') | (uint32(nibbles[1] - '0') << 4)
			);
		}

		constexpr uint32 hexColor(String color) {
			constexpr uint32 ALPHA_MASK = 0x000000ff;
			constexpr uint32 COLOR_MASK = ~ALPHA_MASK;
			if (color.size() < 3) return ALPHA_MASK;
			color = color.eraseLike('#').upper();
			if (color[0] == '0' && color[1] == 'X')
				color = color.substring(2);
			if (color.empty()) return ALPHA_MASK;
			if (color.size() < 3 || color.size() > 8 || !color.isHex())
				return ALPHA_MASK;
			if (color.size() <= 4) {
				String nc;
				nc.appendBack(2, color[0]);
				nc.appendBack(2, color[1]);
				nc.appendBack(2, color[2]);
				if (color.size() == 4)
					nc.appendBack(2, color[3]);
				color = nc;
			}
			uint32 out = 
				(asByte({color[0], color[1]}) << 24) 
			|	(asByte({color[2], color[3]}) << 16)
			|	(asByte({color[4], color[5]}) << 8)
			|	ALPHA_MASK
			;
			if (color.size() == 6) return out;
			return out & (COLOR_MASK | asByte({color[6], color[7]}));
		}

		void assertValidNamedNode(String const& node, usize const min = 2) {
			if (node.size() < min)
				throw Error::InvalidValue(
					toString("Invalid operation '", node, "'!"),
					"Name is too small!",
					CTL_CPP_PRETTY_SOURCE
				);
		}
	};

	/// @brief Anima binary builder.
	struct BinaryBuilder: Anima {
		/// @brief Default constructor.
		constexpr BinaryBuilder(): Anima{
			.data = {"true", "false"}
		} {}

		/// @brief Adds an operation to the binary.
		/// @param op Operation to add.
		/// @return Reference to self.
		constexpr BinaryBuilder& addOperation(uint16 const op) {
			code.pushBack(op);
			return *this;
		}

		/// @brief Adds an operand to the binary.
		/// @param op Operand to add.
		/// @return Reference to self.
		constexpr BinaryBuilder& addOperand(uint64 const op) {
			uint16 opbuf[4];
			opcopy(opbuf, op);
			code.appendBack(opbuf);
			return *this;
		}

		/// @brief Adds a string operand to the binary.
		/// @param str String to add.
		/// @return Reference to self.
		constexpr BinaryBuilder& addStringOperand(String const& str) {
			addOperand(data.size()+1);
			data.pushBack(str);
			return *this;
		}

		/// @brief Adds a named operand to the binary.
		/// @param name Name to add.
		/// @return Reference to self.
		constexpr BinaryBuilder& addNamedOperand(String const& name) {
			return addOperand(ConstHasher::hash(name));
		}

		/// @brief Adds a parameter pack to the binary.
		/// @param params Parameters to add.
		/// @return Reference to self.
		constexpr BinaryBuilder& addParameterPack(StringList const& params) {
			addOperand(data.size()+1);
			addOperand(params.size());
			data.appendBack(params);
			return *this;
		}

		/// @brief Creates a file header for the binary.
		/// @return File header.
		constexpr AnimaBinaryHeader header() const {
			AnimaBinaryHeader fh;
			fh.data		= {fh.headerSize, 0};
			for (String const& s: data) fh.data.size += s.size()+1;
			fh.jumps	= {fh.data.offset(), jumps.size()};
			fh.code		= {fh.jumps.offset(), code.size()};
			return fh;
		}

		/// @brief Creates a binary from an operation tree.
		/// @param tree Tree to create binary from.
		/// @return Constructed binary.
		static BinaryBuilder fromTree(OperationTree const& tree) {
			BinaryBuilder out;
			for (auto& token: tree.tokens) {
				if (token.entry.size())
					out.jumps[ConstHasher::hash(token.entry)] = out.code.size();
				switch (token.type) {
					case Operation::AVM_O_NO_OP:
					case Operation::AVM_O_HALT:
					case Operation::AVM_O_SYNC:
					case Operation::AVM_O_USER_INPUT:
						out.addOperation(token);
						break;
					case Operation::AVM_O_LINE:
						out.addOperation(token);
						out.addStringOperand(token.pack.args[0]);
						break;
					case Operation::AVM_O_ACTOR:
						for (usize i = 0; i < token.pack.args.size(); ++i) {
							auto const& arg = token.pack.args[i];
							if (arg == "...") {
								out.addOperation(token.operation(2));
								continue;
							}
							if (arg.find('.') != -1)
								throw Error::InvalidValue(
									"Invalid parameter name'" + arg + "'!",
									CTL_CPP_PRETTY_SOURCE
								);
							out.addOperation(token.operation(i > 0));
							out.addNamedOperand(arg);
						}
						break;
					case Operation::AVM_O_EMOTION:
						out.addOperation(token);
						out.addNamedOperand(token.name);
						break;
					case Operation::AVM_O_WAIT:
					case Operation::AVM_O_COLOR:
						out.addOperation(token);
						out.addOperand(token.value);
						break;
					case Operation::AVM_O_ACTION:
						out.addOperation(token.operation(token.pack.args.size() > 0));
						if (token.pack.args.size())
							out.addParameterPack(token.pack.args);
						break;
					case Operation::AVM_O_NAMED_CALL:
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
					case Operation::AVM_O_JUMP:
						out.addOperation(token.operation());
						out.addNamedOperand(token.name);
						break;
				}
			}
			return out;
		}
		
		/// @brief Converts the anima binary to a storeable binary file.
		/// @return Anima as file.
		constexpr BinaryData<> toBytes() const {
			BinaryData<>		out;
			AnimaBinaryHeader	fh	= header();
			// Main header
			out.resize(fh.headerSize, '\0');
			MX::memcpy(((void*)out.data()), &fh, fh.headerSize);
			// Data division
			for (String const& s: data) {
				usize const strsz = s.size()+1;
				out.expand(strsz, '\0');
				MX::memcpy(out.end() - strsz, s.data(), s.size());	
			}
			// Jump tables
			if (!jumps.empty()) {
				out.expand(jumps.size() * sizeof(JumpEntry), '\0');
				MX::memcpy(((JumpEntry*)(out.data() + fh.jumps.start)), jumps.data(), jumps.size());
			}
			// Bytecode
			if (!code.empty()) {
				out.expand(code.size() * sizeof(Operation), '\0');
				MX::memcpy(((uint16*)(out.data() + fh.code.start)), code.data(), code.size());
			}
			return out;
		}

	private:
		constexpr static void opcopy(As<uint16[4]>& buf, uint64 const val) {
			MX::memcpy((void*)buf, (void*)&val, sizeof(uint64));
		}
	};

	/// @brief Compiles a anima source.
	/// @param source Source to compile.
	/// @return Anima binary.
	inline BinaryBuilder const compileSource(String const& source) {
		return BinaryBuilder::fromTree(OperationTree::fromSource(source));
	}
	
	/// @brief Compiles a anima source file.
	/// @param path Path to file to compile.
	/// @return Anima binary.
	inline BinaryBuilder const compileFile(String const& path) {
		return compileSource(File::getText(path));
	}

	/// @brief Compiles a anima source, then saves it to a file.
	/// @param source Source to compile.
	/// @param outpath Path to save binary to.
	inline void compileSourceToFile(String const& source, String const& outpath) {
		File::saveBinary(outpath, compileSource(source).toBytes());
	}

	/// @brief Compiles a anima source file, then saves it to a file.
	/// @param source Path to source to compile.
	/// @param outpath Path to save binary to.
	inline void compileFileToFile(String const& path, String const& outpath) {
		File::saveBinary(outpath, compileFile(path).toBytes());
	}
}

#endif