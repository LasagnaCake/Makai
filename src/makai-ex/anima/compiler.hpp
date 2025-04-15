#ifndef MAKAILIB_EX_ANIMA_COMPILER_H
#define MAKAILIB_EX_ANIMA_COMPILER_H

#ifdef MAKAILIB_EX_ANIMA_COMPILER_DEBUG_ABSOLUTELY_EVERYTHING
#define MAKAILIB_EX_ANIMA_COMPILER_DEBUG(...) DEBUG(__VA_ARGS__)
#define MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN(...) DEBUGLN(__VA_ARGS__)
#else
#define MAKAILIB_EX_ANIMA_COMPILER_DEBUG(...)
#define MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN(...)
#endif

#include <makai/makai.hpp>

#include "bytecode.hpp"

#define MKEX_ANIMAC_SOURCE(POSITION, FILENAME) (::CTL::CPP::SourceFile((FILENAME), (POSITION)))

/// @brief Anima Virtual Machine.
namespace Makai::Ex::AVM::Compiler {
	namespace {
		using namespace Literals::Text;
	}

	/// @brief Regex matches used for processing.
	namespace RegexMatches {
		/// @brief Matches any character.
		const static String ANY_CHAR		= String("[\\S\\s]");
		/// @brief Matches any parameter character, except commas.
		const static String PARAM_CHAR		= String("[^,]");
		/// @brief Matches any valid name character.
		const static String NAME_CHAR		= String("[\\w\\-_~:]");
		/// @brief Matches any invalid name character.
		const static String NON_NAME_CHAR	= String("[^\\w\\-_~:]");
		/// @brief Matches any complex token.
		const static String COMPLEX_TOKEN	= String("[\\w&!@#$&+\\-_'\\:\\~\\%]");
		/// @brief Matches any simple token.
		const static String SIMPLE_TOKEN	= String("[*.,;{}<>=\\\\]");

		/// @brief Creates a regex that lazily matches all characters between the given tokens.
		/// @param begin Start token.
		/// @param end End token.
		/// @return Regex.
		constexpr String makePack(String const& begin, String const& end) {
			return begin + ANY_CHAR + "*?" + end;
		}

		/// @brief Matches any text string.
		const static String STRINGS			= String("\"(?:[^\"\\\\]|\\\\.)*\"");
		/// @brief Matches any interpolation.
		const static String INTERPOLATIONS	= String("%([^%\\\\]|\\\\.)*%");
		/// @brief Matches any parens pack.
		const static String PARENTHESES		= makePack("\\(", "\\)");
		/// @brief Matches any brackets pack.
		const static String BRACKETS		= makePack("\\[", "\\]");
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
		const static String PACKS			= concat(LINE_COMMENTS, BLOCK_COMMENTS, STRINGS, PARENTHESES, BRACKETS);
		/// @brief Matches all tokens.
		const static String ALL_TOKENS		= concat(COMPLEX_TOKEN + "+", SIMPLE_TOKEN, PACKS);
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
			case 'a': return '\a';
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
	constexpr String normalize(String const& str) {
		//str.strip();
		String out;
		bool escape = false;
		for (auto const& c: str) {
			if (c == '\\')
				escape = true;
			else if (escape) {
				out.pushBack(unescape(c));
				escape = false;
			} else out.pushBack(c);
		}
		return out;
	}

	/// @brief Structural representation of the program.
	struct OperationTree {
		constexpr static As<char const[]> GLOBAL_BLOCK = "[***]";

		struct Functions {
			struct Composition {
				usize index;
				usize name;
				usize scope;
			};	
			struct Entry {
				StringList	args;
				String		name;
			};
			/// @brief Declared functions.
			Map<usize, Entry>	functions;
			/// @brief Function stack.
			List<Composition>	stack;

			constexpr String parseArgument(String name) const {
				if (name[0] == '%')
					name = name.substring(1);
				ssize place	= -1;
				ssize func	= stack.size()-1;
				//MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Looking for '", name, "' match...");
				for (auto const& fun: stack.reversed()) {
					//MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Func: ", fun.name, ", Args: ('", functions[fun.name].join("', '"), "')");
					place = functions[fun.name].args.find(name);
					if (place != -1) break;
					--func;
				}
				if (func != -1) {
					usize const sz = functions[stack[func].name].args.size();
					//MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("FOUND! Func: ", stack[func].name, ", Index: [", sz-place, "]");
					return toString(SUB_CHAR, stack[func].name, "@", sz-place);
				}
				return "";
			}

			constexpr String parseString(String const& str) const {
				String out;
				String sub;
				bool substitute = false;
				for (auto& c: str) {
					if (c == '%') {
						if (substitute && sub.size())
							out += toString(parseArgument(sub), SUB_CHAR);
						else if (substitute)
							out += '%';
						sub.clear();
						substitute = !substitute;
					} else if (substitute) {
						if (
							isValidNameChar(c)
							||	c == '.'
							||	c == '~'
							||	c == ':'
						) sub.pushBack(c);
						else return "";
					} else out.pushBack(c);
				}
				return out;
			}
		};

		/// @brief Parameter pack.
		struct ParameterPack {
			/// @brief Parameter pack arguments.
			StringList args;

			/// @brief Default constructor.
			constexpr ParameterPack() {}

			/// @brief Parses a parameter pack.
			/// @param pack Pack to parse.
			/// @param fname Source file. For error purposes.
			/// @param funcs Function stack.
			/// @param canUseSubs Can use argument substitutions.
			/// @param minSize Minimum pack size.
			/// @return Parameter pack.
			/// @throw Error::InvalidValue on syntax errors.
			constexpr static StringList parse(
				Regex::Match pack,
				String const& fname,
				Functions const& funcs,
				bool const canUseSubs = true,
				usize const minSize = 0
			) {
				pack.match = pack.match.sliced(1, -2);
				String param	= "";
				String sub		= "";
				StringList out;
				bool inString	= false;
				bool unspaced	= true;
				bool escape		= false;
				for (usize i = 0; i < pack.match.size(); ++i) {
					auto const c = pack.match[i];
					switch (c) {
						case ',': {
							if (!inString) {
								if (param.size() && param[0] == '%') {
									String const arg = funcs.parseArgument(param);
									if (arg.size())
										out.pushBack(arg);
									else
										throw Error::InvalidValue(
											toString("Function argument at [", out.size(), "] does not exist!"),
											CPP::SourceFile(fname, i + pack.position)
										);
								} else out.pushBack(param);
								param.clear();
								unspaced = true;
							} else param.pushBack(c);
						} break;
						case '"':
							if (escape) param.pushBack(c);
							else {
								inString = !inString;
								if (inString) param.pushBack(REP_CHAR);
							}
						break;
						default:
							if (inString) {
								if (c == '\\')
									escape = !escape;
								param.pushBack(c);
								continue;
							}
							else if (isNullOrSpaceChar(c)) {
								if (param.size())
									unspaced = false;
								continue;
							}
							else if (unspaced && (
								isValidNameChar(c)
							||	c == '.'
							||	c == '~'
							||	c == ':'
							||	(c == '%' && param.empty() && canUseSubs)
							)) param.pushBack(c);
							else throw Error::InvalidValue(
								toString("Invalid parameter at position [", out.size(), "]!"),
								toString(
									"Names must only contain letters, numbers, '-', '~', ':' and '_'!",
									canUseSubs ? "\n And '%' may ONLY appear at the begginnig of a name!" : ""
								),
								CPP::SourceFile(fname, i + pack.position)
							);
						break;
					}
					escape = false;
				}
				if (param.size()) out.pushBack(param);
				for (auto& arg: out) {
					String const old = arg;
					switch (arg[0]) {
						case REP_CHAR:	arg = "\x02" + funcs.parseString(arg.substring(1)); break;
						case '%':		arg = funcs.parseArgument(arg); break;
						default:		continue;
					}
					if (arg.empty() && !old.empty())
						throw Error::InvalidAction(
							toString("Invalid argument or string interpolation in parameter pack (", pack.match,")!"),
							toString("Names must only contain letters, numbers, '-', '~', ':' and '_'!"),
							CPP::SourceFile(fname, pack.position)
						);
				}
				if (out.size() < minSize)
					throw Error::InvalidAction(
						toString("Missing arguments in parameter pack!"),
						toString("Necessary argument count is [", minSize, "], but recieved [", out.size(), "] instead."),
						CPP::SourceFile(fname, pack.position)
					);
				return out;
			}

			/// @brief Creates a parameter pack from a parameter pack string.
			/// @param str String to create from.
			/// @param fname Source file. For error purposes.
			/// @return Parameter pack.
			/// @param funcs Function stack.
			/// @param canUseSubs Can use argument substitutions.
			/// @param minSize Minimum pack size.
			/// @throw Error::InvalidValue on syntax errors.
			constexpr static ParameterPack fromString(
				Regex::Match const& ppack,
				String const& fname,
				Functions const& funcs,
				bool const canUseSubs = true,
				usize const minSize = 0
			) {
				return parse(ppack, fname, funcs, canUseSubs, minSize);
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
			/// @brief Operation range. Used by some types.
			uint64			range	= 0;
			/// @brief Operation parameters. Used by some types.
			ParameterPack	pack	= ParameterPack();
			/// @brief Operation mode.
			uint64			mode	= 0;
			/// @brief Jump target for a given jump.
			String			entry	= "";
			/// @brief Token position.
			ssize			pos		= 0;
			/// @brief Token value position.
			ssize			valPos	= 0;
			/// @brief Tags associated with the token.
			usize			tags	= 0;

			/// @brief Tags this token as being part of a choice.
			constexpr static usize CHOICE_BIT = 1;

			/// @brief Returns the token's operation.
			/// @param sp SP mode override. Only used if non-zero. By default, it is zero.
			/// @return Operation.
			constexpr uint16 operation(uint16 const sp = 0) const {
				return enumcast(type) | ((sp ? sp : mode) << 12);
			}

			/// @brief Returns the token's operation.
			/// @return Operation.
			constexpr operator uint16() const {return operation();}
		};

		struct ChoiceEntry {
			String		name;
			StringList	options;
		};

		/// @brief Token list.
		using Tokens = List<Token>;

		/// @brief Token tree operation tokens.
		Tokens tokens;
		/// @brief Declared choices.
		Map<usize, ChoiceEntry>	choices;
		/// @brief Declared functions.
		Functions				functions;

		/// @brief Source file name.
		String const fileName;
			
		/// @brief Copy constructor (defaulted).
		OperationTree(OperationTree const& other)	= default;
		/// @brief Move constructor (defaulted).
		OperationTree(OperationTree&& other)		= default;

		/// @brief Default constructor.
		OperationTree() {}

		/// @brief Constructs the token tree from a series of source file nodes.
		/// @param nodes Nodes to build from.
		/// @param fname Source file. For error purposes.
		/// @throw Error::NonexistentValue when node list is empty.
		/// @throw Error::FailedAction if compilation fails.
		/// @throw Error::InvalidValue on syntax errors.
		OperationTree(List<Regex::Match> const& nodes, String const& fname = "unknown"): fileName(fname) {
			if (nodes.empty())
				throw Error::NonexistentValue("No nodes were given!", CPP::SourceFile(fileName, 0));
			for (usize i = 0; i < nodes.size(); ++i) {
				Regex::Match mnode = nodes[i], mnext;
				if (i+1 < nodes.size())
					mnext = nodes[i+1];
				String& node = mnode.match;
				String& next = mnext.match;
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("[", mnode.position, "]: [", node, "]");
				switch (node[0]) {
					case '/': continue;
					case '@': {
						assertValidNamedNode(mnode);
						if (next.size() && next[0] == '(') {
							tokens.pushBack(Token{
								.type	= Operation::AVM_O_ACTION,
								.name	= node.substring(1),
								.pack	= ParameterPack::fromString(mnext, fileName, functions),
								.pos	= mnode.position,
								.valPos	= mnext.position
							});
							++i;
						} else {
							tokens.pushBack(Token{
								.type	= Operation::AVM_O_ACTION,
								.name	= node.substring(1),
								.pos	= mnode.position,
								.valPos	= mnext.position
							});
						}
					} break;
					case '$': {
						assertValidNamedNode(mnode);
						String const name = node.substring(1);
						if (next.size()) {
							if (next[0] == '(')
								tokens.pushBack(Token{
									.type	= Operation::AVM_O_NAMED_CALL,
									.name	= name,
									.pack	= ParameterPack::fromString(mnext, fileName, functions),
									.pos	= mnode.position,
									.valPos	= mnext.position
								});
							else if (next[0] == '"')
								tokens.pushBack(Token{
									.type	= Operation::AVM_O_NAMED_CALL,
									.name	= name,
									.pack	= ParameterPack(normalize(functions.parseString(next.sliced(1, -2)))),
									.pos	= mnode.position,
									.valPos	= mnext.position
								});
							else if (!Regex::count(next, RegexMatches::NON_NAME_CHAR) || next[0] == '%')
								tokens.pushBack(Token{
									.type	= Operation::AVM_O_NAMED_CALL,
									.name	= name,
									.pack	= ParameterPack(next[0] == '%' ? functions.parseArgument(next) : next),
									.pos	= mnode.position,
									.valPos	= mnext.position
								});
							else throw Error::InvalidValue(
								toString("Invalid value of '", next, "' for '", node, "'!"),
								CPP::SourceFile(fileName, mnode.position)
							);
						} else throw Error::InvalidValue(
							toString("Missing value for '", node, "'!"),
							"Maybe you confused '$' with '+' or '-', perhaps?",
							CPP::SourceFile(fileName, mnode.position)
						);
						++i;
					} break;
					case '!': {
						assertValidNamedNode(mnode);
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_EMOTION,
							.name	= node.substring(1),
							.pos	= mnode.position,
							.valPos	= mnext.position
						});
					} break;
					case '\'': {
						assertValidNamedNode(mnode);
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_WAIT,
							.value	= toUInt64(node.substring(1)),
							.pos	= mnode.position,
							.valPos	= mnext.position
						});
					} break;
					case '\"': {
						auto const rawContent = node.sliced(1, -2);
						MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Normalized: [", normalize(rawContent), "]");
						auto const content = functions.parseString(rawContent);
						if (content.empty() && !rawContent.empty())
							throw Error::InvalidAction(
								toString("Invalid string interpolation!"),
								toString("Names must only contain letters, numbers, '-', '~', ':' and '_'!"),
								CPP::SourceFile(fname, mnode.position)
							);
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_LINE,
							.name	= normalize(content),
							.pos	= mnode.position,
							.valPos	= mnext.position
						});
					} break;
					case '#': {
						assertValidNamedNode(mnode, 4);
						if (node[1] == '#')
							tokens.pushBack(Token{
								.type	= Operation::AVM_O_COLOR,
								.value	= ConstHasher::hash(node.substring(2)),
								.mode	= 1,
								.pos	= mnode.position,
								.valPos	= mnext.position
							});
						else tokens.pushBack(Token{
								.type	= Operation::AVM_O_COLOR,
								.value	= hexColor(node.substring(1)),
								.pos	= mnode.position,
								.valPos	= mnext.position
							});
					} break;
					case '[': {
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_ACTOR,
							.pack	= ParameterPack::fromString(mnode, fileName, functions, false),
							.pos	= mnode.position,
							.valPos	= mnext.position
						});
					} break;
					case '*': tokens.pushBack(Token{.mode = 1, .pos = mnode.position});								break;
					case '.': tokens.pushBack(Token{.type = Operation::AVM_O_SYNC, .pos = mnode.position});			break;
					case ';': tokens.pushBack(Token{.type = Operation::AVM_O_USER_INPUT, .pos = mnode.position});	break;
					case '+':
					case '-': {
						assertValidNamedNode(mnode);
						String const name = node.substring(1);
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_NAMED_CALL,
							.name	= name,
							.pack	= ParameterPack((node[0] == '+') ? "true" : "false"),
							.pos	= mnode.position,
							.valPos	= mnext.position
						}); break;
					}
					case '%': {
						throw Error::InvalidValue(
							toString("Floating argument substitutions are not allowed!"),
							CPP::SourceFile(fileName, mnode.position)
						);
					} break;
					case '\\': {
						processMacro(mnode, mnext, i, nodes);
					} break;
					case '(': continue;
					default:
						if (isLowercaseChar(node[0])) {	
							assertValidNamedNode(mnode);
							addExtendedOperation(mnode, mnext, i, nodes);
						} else if (!customKeyword(mnode, nodes, i))
							throw Error::InvalidValue(
								toString("Invalid operation '" + node + "'!"),
								CPP::SourceFile(fileName, mnode.position)
							);
				}
			}
			if (blocks.size()) {
				String bnames;
				for (auto& block: blocks)
					bnames.appendBack(block.popBack() + ", ");
				throw Error::InvalidValue(
					toString("Missing closure for one or more blocks!"),
					toString("Blocks are: [", bnames.sliced(0, -3), "]"),
					CPP::SourceFile(fileName, nodes.back().position)
				);
			}
			if (tokens.empty())
				throw Error::FailedAction(
					"Failed to parse tree!",
					CPP::SourceFile(fileName, 0)
				);
		}

		/// @brief Creates an operation tree from a source file.
		/// @param src Source file to construct tree from.
		/// @param fname Source file. For error purposes.
		/// @return Operation tree.
		/// @throw Error::NonexistentValue if source file is empty.
		static OperationTree fromSource(String const& src, String const& fname = "unknown") {
			MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Tokenizer regex: ", RegexMatches::ALL_TOKENS);
			if (src.empty() || src.isNullOrSpaces())
				throw Error::NonexistentValue(
					"Source is empty!",
					CPP::SourceFile(fname, 0)
				);
			auto matches = Regex::find(src, RegexMatches::ALL_TOKENS);
			if (matches.empty())
				return OperationTree();
			MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("\nParsing tree...\n");
			return OperationTree(matches, fname);
		}

		virtual bool customKeyword(Regex::Match node, List<Regex::Match> const& nodes, usize& curNode) {return false;}

	private:
		StringList	blocks;
		String		entry;
		bool		isInScene	= false;
		bool		isInChoice	= false;
		bool		hasBlocks	= false;

		constexpr static bool isValidNameChar(char const c) {
			return
				isAlphanumericChar(c)
			||	c == '_'
			||	c == '-'
			;
		}

		void append(OperationTree const& other, usize const incline) {
			tokens.appendBack(other.tokens);
			for (auto& fun: other.functions.functions)
				if (functions.functions.contains(fun.key))
					throw Error::InvalidValue(
						"Function '" + fun.value.name
					+	"' (from file '" + other.fileName + "') already exists in '"
					+	fileName + "'!",
						CPP::SourceFile(fileName, incline)
					);
				else functions.functions[fun.key] = fun.value;
			for (auto& cho: other.choices)
				if (choices.contains(cho.key))
					throw Error::InvalidValue(
						"Choice '" + cho.value.name
					+	"' (from file '" + other.fileName + "') already exists in '"
					+	fileName + "'!",
						CPP::SourceFile(fileName, incline)
					);
				else choices[cho.key] = cho.value;
			choices.append(other.choices);
		}

		void processMacro(
			Regex::Match const& opmatch,
			Regex::Match const& valmatch,
			usize& curNode,
			List<Regex::Match> const& nodes
		) {
			assertHasAtLeast(nodes, curNode, 2, opmatch);
			switch (ConstHasher::hash(valmatch.match)) {
				case ConstHasher::hash("append"): {
					if (blocks.size())
						throw Error::InvalidValue(
							"Macro ![append] is only allowed in global scope!",
							CPP::SourceFile(fileName, curNode + 1)
						);
					++curNode;
					assertHasAtLeast(nodes, curNode, 2, opmatch);
					String const& file = nodes[curNode+1].match;
					String const root = std::filesystem::current_path().string();
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Current path: ", root);
					String const filePath = OS::FS::concatenate(
						OS::FS::directoryFromPath(fileName),
						file.front() == '\"' ? file.sliced(1, -2) : file
					);
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("File path: '", filePath, "'");
					if (OS::FS::exists(root + "/" + filePath) && !OS::FS::isDirectory(root + "/" + filePath))
						append(OperationTree::fromSource(File::getText(root + "/" + filePath), filePath), curNode+1);
					else throw Error::InvalidValue(
						"File '" + filePath + "' does not exist!",
						CPP::SourceFile(fileName, curNode + 1)
					);
					curNode += 2;
				} break;
			}
		}

		void addExtendedOperation(
			Regex::Match const& opmatch,
			Regex::Match const& valmatch,
			usize& curNode,
			List<Regex::Match> const& nodes,
			bool isNotNext = false
		) {
			constexpr uint16 CHOICE_JUMP_BIT = 0b1000u;
			auto const& [opi, op]	= opmatch;
			auto const& [vali, val]	= valmatch;
			if (
				tokens.size()
			&&	tokens.back().type == Operation::AVM_O_NO_OP
			&&	tokens.back().mode != 0
			)
				throw Error::InvalidValue(
					"Cannot apply '*' modifier on keywords!",
					CPP::SourceFile(fileName, opi)
				);
			auto const ophash = ConstHasher::hash(op);
			switch (ophash) {
				case (ConstHasher::hash("finish")):
				case (ConstHasher::hash("terminate")): {
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_HALT,
						.mode	= ophash == ConstHasher::hash("finish"),
						.pos	= opi,
						.valPos	= vali
					});
				} break;
				case (ConstHasher::hash("perform")):
				case (ConstHasher::hash("next")): {
					bool const performing = ophash == ConstHasher::hash("perform");
					if (val.empty())
						throw Error::InvalidValue(
							toString("Missing value for '", op, "'!"),
							CPP::SourceFile(fileName, opi)
						);
					if (val == "select") {
						assertHasAtLeast(nodes, curNode, 3, opmatch);
						++curNode;
						MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Select type: ", nodes[curNode+1].match);
						addExtendedOperation(valmatch, nodes[curNode+1], curNode, nodes, performing);
						return;
					}
					if (val == "choice") {
						assertHasAtLeast(nodes, curNode, 3, opmatch);
						curNode += 2;
						if (!nodes[curNode].match.validate(isValidNameChar))
							throw Error::InvalidValue(
								toString("Invalid choice name '", val, "'!"),
								CPP::SourceFile(fileName, vali)
							);
						MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Choice: ", nodes[curNode].match);
						MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Path: ", getChoicePath(nodes[curNode].match));
						auto const ppack = ParameterPack::fromString(nodes[curNode+1], fileName, functions, false);
						auto const path = ConstHasher::hash(getChoicePath(nodes[curNode].match));
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_GET_VALUE,
							.name	= getScopePath(nodes[curNode].match),
							.value	= path,
							.mode	= 3,
							.pos	= opi,
							.valPos	= vali
						});
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_JUMP,
							.range	= ppack.args.size(),
							.mode	= 2 | (isNotNext ? CHOICE_JUMP_BIT : 0u),
							.pos	= opi,
							.valPos	= vali
						});
						String const exit = getScopePath(nodes[curNode].match + "[choice:end]");
						processChoice(opi, vali, exit, ppack.args, curNode, nodes);
						++curNode;
						return;
					}
					++curNode;
					if (val == "none") return;
					if (val == "terminate" || val == "finish") {
						throw Error::InvalidValue(
							toString("Cannot have this keyword as a jump target!"),
							"Did you perhaps intend to do a ![choice] or ![select] jump?",
							CPP::SourceFile(fileName, vali)
						);
					}
					String const path = getScopePath(val);
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_JUMP,
						.name	= path,
						.mode	= performing,
						.pos	= opi,
						.valPos	= vali
					});
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Jump to: ", tokens.back().name);
					return;
				}
				case (ConstHasher::hash("function")):
				case (ConstHasher::hash("scene")):
				case (ConstHasher::hash("act")): {
					if (val.empty())
						throw Error::InvalidValue(
							toString("Missing block name!"),
							CPP::SourceFile(fileName, opi)
						);
					if (!val.validate(isValidNameChar))
						throw Error::InvalidValue(
							toString("Invalid block name '", val, "'!"),
							CPP::SourceFile(fileName, vali)
						);
					if (val == "none" || val == "terminate" || val == "finish") {
						throw Error::InvalidValue(
							toString("Cannot have this keyword as a block name!"),
							CPP::SourceFile(fileName, vali)
						);
					}
					if (
						blocks.size()
					&&	blocks.back().back() != ':'
					&&	blocks.back().back() != '*'
					) blocks.back().pushBack(isInScene ? ':' : '*');
					isInScene = ophash == ConstHasher::hash("scene");
					entry = val;
					if (ophash == ConstHasher::hash("function")) {
						auto const fpath = getScopePath(entry);
						functions.stack.pushBack({
							functions.stack.size(),
							ConstHasher::hash(fpath),
							blocks.size()
						});
						++curNode;
						if (curNode+1 >= nodes.size() || nodes[curNode+1].match[0] != '(')
							throw Error::InvalidValue(
								toString("Missing function arguments!"),
								CPP::SourceFile(fileName, opi)
							);
						if (functions.functions.contains(functions.stack.back().name))
							throw Error::InvalidValue(
								toString("Function '"+fpath+"' already exists!"),
								CPP::SourceFile(fileName, opi)
							);
						functions.functions[functions.stack.back().name].name = fpath;
						functions.functions[functions.stack.back().name].args = ParameterPack::fromString(
							nodes[curNode+1],
							fileName,
							functions
						).args;
					}
					blocks.pushBack(entry);
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Stack: ", blocks.size());
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Context: ", blocks.join());
					auto const end = blocks.join() + "[end]";
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_JUMP,
						.name	= end,
						.pos	= opi,
						.valPos	= vali
					});
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_NEXT,
						.entry	= blocks.join(),
						.pos	= opi,
						.valPos	= vali
					});
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Entrypoint: ", tokens.back().entry);
					hasBlocks = true;
					++curNode;
					return;
				}
				case (ConstHasher::hash("end")): {
					if (blocks.empty())
						throw Error::InvalidValue(
							toString("Missing block for 'end' statement!"),
							CPP::SourceFile(fileName, opi)
						);
					if (
						blocks.back().back() == ':'
					||	blocks.back().back() == '*'
					) blocks.back().popBack();
					auto const end = blocks.join() + "[end]";
					blocks.popBack();
					for (auto& fun: functions.stack)
						if (fun.scope == blocks.size()) {
							functions.stack.popBack();
							break;
						}
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Context: ", end);
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_HALT,
						.mode	= 1,
						.pos	= opi,
						.valPos	= vali
					});
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_NEXT,
						.entry	= end,
						.pos	= opi,
						.valPos	= vali
					});
					return;
				}
				case (ConstHasher::hash("select")): {
					assertHasAtLeast(nodes, curNode, 2, opmatch);
					auto const ppack = ParameterPack::fromString(nodes[curNode+2], fileName, functions, false);
					switch (val[0]) {
						case '$': {
							tokens.pushBack(Token{
								.type	= Operation::AVM_O_GET_VALUE,
								.name	= val.substring(1),
								.value	= 0,
								.range	= ppack.args.size()-1,
								.mode	= 1,
								.pos	= opi,
								.valPos	= vali
							});
							tokens.pushBack(Token{
								.type	= Operation::AVM_O_JUMP,
								.range	= ppack.args.size(),
								.mode	= 2 | (isNotNext ? CHOICE_JUMP_BIT : 0u),
								.pos	= opi,
								.valPos	= vali
							});
						} break;
						default:
						switch (ConstHasher::hash(val)) {
							case (ConstHasher::hash("random")): {
								tokens.pushBack(Token{
									.type	= Operation::AVM_O_JUMP,
									.range	= ppack.args.size(),
									.mode	= 3 | (isNotNext ? CHOICE_JUMP_BIT : 0u),
									.pos	= opi,
									.valPos	= vali
								});
							} break;
							default:
								throw Error::InvalidValue(
									toString("Invalid select mode '", val, "'!"),
									CPP::SourceFile(fileName, vali)
								);
						}
					}
					usize i = 0;
					String const exit = getScopePath(toString("*select", opmatch.position, "[end]"));
					processChoice(opi, vali, exit, ppack.args, curNode, nodes);
					++i;
					curNode += 2;
					return;
				}
				case (ConstHasher::hash("choice")): {
					assertHasAtLeast(nodes, curNode, 2, opmatch);
					MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Path: ", getChoicePath(val));
					if (!val.validate(isValidNameChar))
						throw Error::InvalidValue(
							toString("Invalid choice name '", val, "'!"),
							CPP::SourceFile(fileName, vali)
						);
					auto const ppack = ParameterPack::fromString(nodes[curNode+2], fileName, functions);
					auto const choice = ConstHasher::hash(getChoicePath(val));
					choices[choice].name	= getChoicePath(val);
					choices[choice].options	= ppack.args;
					curNode += 2;
				} break;
				case (ConstHasher::hash("call")): {
					assertHasAtLeast(nodes, curNode, 2, opmatch);
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_INVOKE,
						.name	= getScopePath(val),
						.pack	= ParameterPack::fromString(nodes[curNode+2], fileName, functions),
						.pos	= opi,
						.valPos	= vali
					});
					curNode += 2;
				} break;
				case (ConstHasher::hash("repeat")): {
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_JUMP,
						.name	= blocks.empty() ? String(GLOBAL_BLOCK) : blocks.join(),
						.pos	= opi,
						.valPos	= vali,
						.tags	= Token::CHOICE_BIT
					});
				} break;
				default:
				throw Error::InvalidValue(
					toString("Invalid keyword '", op, "'!"),
					CPP::SourceFile(fileName, opi)
				);
			}
		}

		constexpr void processChoice(
			ssize const opi,
			ssize const vali,
			String const& exit,
			StringList const& args,
			usize& curNode,
			List<Regex::Match> const& nodes
		) {
			for (auto const& param: args) {
				if (param == "...") {
					throw Error::InvalidValue(
						toString("Cannot have pack expansions in a jump list!"),
						CPP::SourceFile(fileName, nodes[curNode+2].position)
					);
				}
				if (Regex::count(param, RegexMatches::NON_NAME_CHAR) > 0) {
					throw Error::InvalidValue(
						toString("Invalid option '", param,"'!"),
						"Options must ONLY be block paths!",
						CPP::SourceFile(fileName, nodes[curNode+2].position)
					);
				}
				if (param == "repeat") {
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_JUMP,
						.name	= blocks.empty() ? String(GLOBAL_BLOCK) : blocks.join(),
						.pos	= opi,
						.valPos	= vali,
						.tags	= Token::CHOICE_BIT
					});
				} else if (param == "none") {
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_JUMP,
						.name	= exit,
						.pos	= opi,
						.valPos	= vali,
						.tags	= Token::CHOICE_BIT
					});
				} else if (param == "finish" || param == "terminate") {
					tokens.pushBack(Token{
						.type	= Operation::AVM_O_HALT,
						.mode	= param == "finish",
						.pos	= opi,
						.valPos	= vali,
						.tags	= Token::CHOICE_BIT
					});
					for (usize i = 0; i < 4; ++i)
						tokens.pushBack(Token{
							.type	= Operation::AVM_O_NEXT,
							.pos	= opi,
							.valPos	= vali,
							.tags	= Token::CHOICE_BIT
						});
				} else tokens.pushBack(Token{
					.type	= Operation::AVM_O_JUMP,
					.name	= getScopePath(param),
					.pos	= opi,
					.valPos	= vali,
					.tags	= Token::CHOICE_BIT
				});
			}
			tokens.pushBack(Token{
				.type	= Operation::AVM_O_NEXT,
				.entry	= exit,
				.pos	= opi,
				.valPos	= vali,
				.tags	= Token::CHOICE_BIT
			});
		}

		constexpr String getChoicePath(String const& choice) {
			return getScopePath(choice + "[choice]");
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

		constexpr String getScopePath(String const& val) {
			String path;
			switch (val.front()) {
				case ':': {
					path = val.substring(1);
				} break;
				case '~': {
					auto scope = blocks;
					if (scope.size()) {
						scope.popBack();
						path = scope.join();
					}
					path += val.substring(1);
				} break;
				default: {
					path = blocks.join() + val; break;
				} break;
			}
			return path;
		}

		void assertHasAtLeast(List<Regex::Match> const& nodes, usize const index, usize const size, Regex::Match const& node) {
			if (nodes.size() <= index + size)
				throw Error::InvalidValue(
					toString("Too few required arguments for '", node.match, "'!"),
					CPP::SourceFile(fileName, node.position)
				);
		}

		void assertValidNamedNode(Regex::Match const& node, usize const min = 2) {
			if (node.match.size() < min)
				throw Error::InvalidValue(
					toString("Invalid operation '", node.match, "'!"),
					"Name is too small!",
					CPP::SourceFile(fileName, node.position)
				);
		}
	};

	/// @brief Anima binary builder.
	struct BinaryBuilder: Anima {
		/// @brief Default constructor.
		constexpr BinaryBuilder(): Anima{
			.data	= {"false", "true"}
		} {
			jumps[ConstHasher::hash(OperationTree::GLOBAL_BLOCK)] = 0;
		}

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
			addOperand(data.size());
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
			addOperand(data.size());
			addOperand(params.size() - 1);
			data.appendBack(params);
			return *this;
		}

		struct ChoiceRef {
			uint64 start	= 0;
			uint64 size		= 0;
		};

		constexpr Map<uint64, ChoiceRef> processChoices(Map<usize, OperationTree::ChoiceEntry> const& choices) {
			Map<uint64, ChoiceRef> out;
			for (auto& [choice, options]: choices) {
				out[choice] = {options.options.empty() ? 0 : data.size(), options.options.size() - 1};
				data.appendBack(options.options);
			}
			return out;
		}

		/// @brief Creates a file header for the binary.
		/// @return File header.
		constexpr AnimaBinaryHeader header() const {
			AnimaBinaryHeader fh;
			fh.data		= {fh.headerSize, 0};
			for (String const& s: data) fh.data.size += s.size()+1;
			fh.jumps	= {fh.data.offset(), jumps.size() * sizeof(JumpEntry)};
			fh.code		= {fh.jumps.offset(), code.size() * sizeof(Operation)};
			return fh;
		}

		/// @brief Creates a binary from an operation tree.
		/// @param tree Tree to create binary from.
		/// @return Constructed binary.
		static BinaryBuilder fromTree(OperationTree const& tree) {
			MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("\nBuilding binary...\n");
			BinaryBuilder out;
			auto const choices = out.processChoices(tree.choices);
			#ifdef MAKAILIB_EX_ANIMA_COMPILER_DEBUG_ABSOLUTELY_EVERYTHING
			MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("<choices>");
			for (auto& choice: choices) {
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Choice: ", choice.key);
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("    Start: ", choice.value.start);
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("    Size: ", choice.value.size);
			}
			MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("</choices>");
			#endif
			for (auto& token: tree.tokens) {
				#ifdef MAKAILIB_EX_ANIMA_COMPILER_DEBUG_ABSOLUTELY_EVERYTHING
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("<token>");
				#define MKEX_ANIMAC_PRINT_NAME(NAME) case (NAME): DEBUGLN("Type: '", #NAME, "'"); break
				switch (token.type) {
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_NO_OP);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_NEXT);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_HALT);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_SYNC);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_USER_INPUT);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_LINE);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_ACTOR);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_EMOTION);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_WAIT);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_COLOR);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_ACTION);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_NAMED_CALL);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_JUMP);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_GET_VALUE);
					MKEX_ANIMAC_PRINT_NAME(Operation::AVM_O_INVOKE);
				}
				#undef MKEX_ANIMAC_PRINT_NAME
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Mode: ", token.mode);
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Name: '", token.name, "'");
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Value: ", token.value);
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Range: ", token.range);
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("Entry: ", token.entry);
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN(
					"Params (",
					token.pack.args.size(),
					"): ['",
					token.pack.args.join("', '"),
					"']"
				);
				MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("</token>");
				#endif
				if (token.entry.size()) {
					auto const njloc = ConstHasher::hash(token.entry);
					if (out.jumps.contains(njloc))
						throw Error::InvalidValue(
							toString("Named block '", token.entry, "' already exists!"),
							CPP::SourceFile(tree.fileName, 0)
						);
					out.jumps[njloc] = out.code.size();
				}
				if (token.type != asOperation(token.operation(SP_FLAG_MASK))) {
					throw Error::FailedAction(
						"Compiler error!",
						CTL_CPP_UNKNOWN_SOURCE
					);
				}
				switch (token.type) {
					case Operation::AVM_O_NO_OP:
					case Operation::AVM_O_NEXT:
					case Operation::AVM_O_HALT:
					case Operation::AVM_O_SYNC:
					case Operation::AVM_O_USER_INPUT:
						out.addOperation(token);
						break;
					case Operation::AVM_O_LINE:
						out.addOperation(token);
						out.addStringOperand(token.name);
						break;
					case Operation::AVM_O_ACTOR:
						for (usize i = 0; i < token.pack.args.size(); ++i) {
							auto const& arg = token.pack.args[i];
							if (arg == "...") {
								out.addOperation(token.operation(2));
								continue;
							}
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
						out.addNamedOperand(token.name);
						if (token.pack.args.size())
							out.addParameterPack(token.pack.args);
						break;
					case Operation::AVM_O_NAMED_CALL:
						out.addOperation(token.operation(token.pack.args.size() > 1));
						out.addNamedOperand(token.name);
						if (token.pack.args.size() > 1)
							out.addParameterPack(token.pack.args);
						else {
							String const& val = token.pack.args[0];
							if (val == "true")			out.addOperand(1);
							else if (val == "false")	out.addOperand(0);
							else						out.addStringOperand(val);
						}
						break;
					case Operation::AVM_O_JUMP:
						if (token.mode < 2) {
							out.addOperation(token);
							out.addNamedOperand(token.name);
						} else {
							out.addOperation(token);
							out.addOperand(token.range);
						}
						break;
					case Operation::AVM_O_GET_VALUE:
						out.addOperation(token);
						if (token.mode == 3) {
							out.addNamedOperand(token.name);
							out.addOperand(choices[token.value].start);
							out.addOperand(choices[token.value].size);
						} else {
							out.addNamedOperand(token.name);
							if (token.mode == 1) {
								out.addOperand(token.value);
								out.addOperand(token.range);
							}
						}
						break;
						case Operation::AVM_O_INVOKE: {
							auto const name = ConstHasher::hash(token.name);
							if (tree.functions.functions.contains(name)) {
								auto const argCount = token.pack.args.size();
								auto const paramCount = tree.functions.functions[name].args.size();
								if (argCount < paramCount) throw Error::InvalidAction(
									toString("Missing arguments in parameter pack!"),
									toString("Necessary argument count is [", paramCount, "], but recieved [", argCount, "] instead."),
									CPP::SourceFile(tree.fileName, token.pos)
								);
								out.addOperation(token.operation(!token.pack.args.empty()));
								if (token.pack.args.size()) {
									out.addNamedOperand(token.name);
									out.addParameterPack(token.pack.args.sliced(0, paramCount-1));
								}
							} else {
								throw Error::InvalidAction(
									toString("Function '", token.name, "' does not exist!"),
									toString("Did you perhaps miss a '~' or ':'?"),
									CPP::SourceFile(tree.fileName, token.pos)
								);
							}
						} break;
				}
			}
			out.addOperation(OperationTree::Token{Operation::AVM_O_HALT});
			#ifdef MAKAILIB_EX_ANIMA_COMPILER_DEBUG_ABSOLUTELY_EVERYTHING
			for (auto& name: out.data)
				MAKAILIB_EX_ANIMA_COMPILER_DEBUG("'", name, "', ");
			#endif
			MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("");
			MAKAILIB_EX_ANIMA_COMPILER_DEBUGLN("\nBinary built!\n");
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
	inline BinaryBuilder const compileSource(String const& source, String const& fname = "unknown") {
		return BinaryBuilder::fromTree(OperationTree::fromSource(source, fname));
	}
	
	/// @brief Compiles a anima source file.
	/// @param path Path to file to compile.
	/// @return Anima binary.
	inline BinaryBuilder const compileFile(String const& path) {
		return compileSource(File::getText(path), path);
	}

	/// @brief Compiles a anima source, then saves it to a file.
	/// @param source Source to compile.
	/// @param outpath Path to save binary to.
	inline void compileSourceToFile(String const& source, String const& outpath, String const& fname = "unknown") {
		File::saveBinary(outpath, compileSource(source, fname).toBytes());
	}

	/// @brief Compiles a anima source file, then saves it to a file.
	/// @param source Path to source to compile.
	/// @param outpath Path to save binary to.
	inline void compileFileToFile(String const& path, String const& outpath) {
		File::saveBinary(outpath, compileFile(path).toBytes());
	}
}

#undef MKEX_ANIMAC_SOURCE

#endif
