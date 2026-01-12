#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H

#include "../../../../../lexer/lexer.hpp"
#include "../../runtime/program.hpp"
#include "../../instruction.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Context {
		using Tokenizer	= Lexer::CStyle::TokenStream;
		using Program	= Runtime::Program;

		struct Macro {
			struct Axiom: Tokenizer::Token {
				bool strict = false;

				constexpr Ordered::OrderType operator<=>(Axiom const& other) const {
					if (!strict) return type <=> other.type;
					Ordered::OrderType order = type <=> other.type;
					if (order == Ordered::Order::EQUAL) return value <=> other.value;
					else return order;
				}

				constexpr bool operator==(Axiom const& other) const {
					if (!strict) return type == other.type;
					if (type == other.type) return true;
					return value == other.value;
				}

				constexpr Ordered::OrderType operator<=>(Tokenizer::Token const& other) const {
					return operator<=>(Axiom{other});
				}

				constexpr bool operator==(Tokenizer::Token const& other) const {
					return operator==(Axiom{other});
				}
			};

			using Stack			= Tokenizer::TokenList;
			using Arguments		= Tokenizer::TokenList;
			using Result		= Tokenizer::TokenList;

			struct Rule: ID::Identifiable<Rule> {
				struct VariadicRegion {
					usize begin;
					usize end;

					constexpr usize size() const {return end - begin;}
				};

				struct Match {
					using ID = ID::VLUID;
					enum class Type {
						AV2_TA_SM_RMT_COUNT,
						AV2_TA_SM_RMT_ALL_OF,
						AV2_TA_SM_RMT_ANY_OF,
						AV2_TA_SM_RMT_EXPRESSION,
						AV2_TA_SM_RMT_VA_BEGIN,
						AV2_TA_SM_RMT_VA_END,
					}	type = Type::AV2_TA_SM_RMT_ALL_OF;
					ID	id		= all++;
					bool atMost	= false;
				private:
					static inline ID all = ID::create(0);
				};

				template <class T>
				using Bank = Map<Match::ID, T>;

				struct Section {
					usize		count;
					List<Axiom>	match;
				};

				Bank<String>			variables;
				List<Instance<Match>>	matches;
				Bank<Instance<Section>>	sections;

				constexpr Instance<Section> addSection(Match const& match) {
					return sections[match.id];
				}

				constexpr Instance<Match> createMatch() {
					return matches.pushBack({}).back();
				}

				using MatchResult = Nullable<Arguments>;

				constexpr MatchResult match(Arguments const& args, usize const match) {
					if (match >= matches.size()) return null;
					auto matchInfo = matches[match];
					if (
						matchInfo->type == Match::Type::AV2_TA_SM_RMT_VA_BEGIN
					||	matchInfo->type == Match::Type::AV2_TA_SM_RMT_VA_END
					) return null;
					Arguments result;
					auto rule = sections[matchInfo->id];
					if (!rule->count) return Arguments();
					if (!matchInfo->atMost && rule->count < args.size()) return null;
					auto const count = matchInfo->atMost ? rule->count : Math::min(rule->count, args.size());
					switch (matchInfo->type) {
						case Match::Type::AV2_TA_SM_RMT_COUNT: {
							return args.sliced(0, count);
						} break;
						case Match::Type::AV2_TA_SM_RMT_ALL_OF: {
							for (usize i = 0; i < count; ++i) {
								if (args[i] != rule->match[i]) return matchInfo->atMost ? MatchResult{result} : MatchResult{null};
								result.pushBack(args[i]);
							}
						} break;
						case Match::Type::AV2_TA_SM_RMT_ANY_OF: {
							for (usize i = 0; i < count; ++i) {
								if (rule->match.find({args[i]}) != -1) return matchInfo->atMost ? MatchResult{result} : MatchResult{null};
								result.pushBack(args[i]);
							}
						} break;
						default: break;
					}
					return result;
				}

				using MatchCount = Nullable<usize>;

				constexpr MatchCount fits(Arguments const& args, usize const match) const {
					if (match >= matches.size()) return null;
					auto& matchInfo = matches[match];
					if (
						matchInfo->type == Match::Type::AV2_TA_SM_RMT_VA_BEGIN
					||	matchInfo->type == Match::Type::AV2_TA_SM_RMT_VA_END
					) return null;
					auto rule = sections[matchInfo->id];
					if (!rule->count) return MatchCount{0ull};
					if (!matchInfo->atMost && rule->count < args.size()) return null;
					auto const count = matchInfo->atMost ? rule->count : Math::min(rule->count, args.size());
					switch (matchInfo->type) {
						case Match::Type::AV2_TA_SM_RMT_COUNT:return null; break;
						case Match::Type::AV2_TA_SM_RMT_ALL_OF: {
							for (usize i = 0; i < count; ++i) {
								if (args[i] != rule->match[i]) return matchInfo->atMost ? MatchCount{i} : null;
							}
						} break;
						case Match::Type::AV2_TA_SM_RMT_ANY_OF: {
							for (usize i = 0; i < count; ++i)
								if (rule->match.find({args[i]}) != -1) return matchInfo->atMost ? MatchCount{i} : null;
						} break;
						default: break;
					}
					return true;
				}

				constexpr bool fits(Arguments const& args) const {
					bool vaRegion	= false;
					bool doVariadic	= false;
					VariadicRegion	va;
					usize		count	= 0;
					MatchCount	mc		= null;
					for (usize i = 0; i < matches.size(); ++i) {
						if (count >= args.size()) break;
						if (matches[i]->type == Match::Type::AV2_TA_SM_RMT_VA_BEGIN) {
							va.begin = i+1;
							vaRegion = true;
						} else if (matches[i]->type == Match::Type::AV2_TA_SM_RMT_VA_END) {
							va.end = i-1;
							vaRegion = false;
							doVariadic = true;
						}
						if (vaRegion) continue;
						else if (doVariadic) {
							while (true) {
								for (usize i = va.begin; i < va.end; ++i) {
									if (!(mc = fits(args.sliced(count), i))) break;
									else count += mc.value();
								}
								if (!mc) break;
							}	
						} else if (!(mc = fits(args.sliced(count), i))) return false;
						else count += mc.value();
					}
					return true;
				}
			};

			struct Context {
				Arguments		input;
				Stack			stack;
				Result			result;
				Rule			rule;

				struct Variable {
					List<Arguments>	tokens;
				};

				Dictionary<Variable> variables;
			
				Rule::MatchResult consume(usize const match) {
					auto const result = rule.match(input, match);
					if (result)
						input.removeRange(result.value().size());
					return result;
				}

				void parseVariadic(Rule::VariadicRegion const& va) {
					while (true) {
						Rule::MatchResult toks;
						for (usize a = va.begin; a < va.end; ++a) {
							toks = consume(a);
							if (!toks) break;
							if (rule.variables.contains(rule.matches[a]->id))
								variables[rule.variables[rule.matches[a]->id]].tokens.pushBack(toks.value());
						}
						if (!toks) break;
					}	
				}

				void parse() {
					bool vaRegion	= false;
					Rule::VariadicRegion va;
					for (usize i = 0; i < rule.matches.size(); ++i) {
						auto& match = rule.matches[i];
						if (input.empty()) break;
						if (match->type == Rule::Match::Type::AV2_TA_SM_RMT_VA_BEGIN) {
							va.begin = i+1;
							vaRegion = true;
						} else if (match->type == Rule::Match::Type::AV2_TA_SM_RMT_VA_END) {
							va.end = i-1;
							vaRegion = false;
							parseVariadic(va);
						}
						if (vaRegion) continue;
						else if (rule.variables.contains(match->id)) {
							auto const toks = consume(i);
							if (toks)
								variables[rule.variables[match->id]].tokens.pushBack(toks.value());
							else break;
						} else if (!consume(i)) break;
					}
				}
			};

			struct Transformation {
				using Action = Function<void(Context&)>;
				
				List<Action> actions;

				constexpr Transformation& apply(Context& ctx) {
					for (auto& action: actions)
						action(ctx);
					return *this;
				}

				constexpr Result result(Context& ctx) const {
					return ctx.result;
				}
			};

			using Expressions	= Map<typename Rule::IdentifierType, Transformation>;

			struct Expression {
				Rule			rule;
				Transformation	transform;
			};

			Expressions	exprs;
			List<Rule>	rules;

			struct Entry {
				Tokenizer::TokenList pre;
				Tokenizer::TokenList main;
				Tokenizer::TokenList post;
				bool variadic = false;
			};

			Makai::Dictionary<Instance<Entry>>	variables;
			List<Instance<Entry>>				entries;

			Nullable<Result> resolve(Arguments const& args) {
				for (auto& rule: rules) if (rule.fits(args)) {
					Context ctx{.input = args, .rule = rule};
					ctx.parse();
					return exprs[rule.id()].apply(ctx).result(ctx);
				}
				return null;
			}
		};

		struct Scope {
			enum class Type {
				AV2_TA_ST_NORMAL,
				AV2_TA_ST_FUNCTION,
				AV2_TA_ST_SWITCH,
				AV2_TA_ST_LOOP,
				AV2_TA_ST_NAMESPACE,
				AV2_TA_ST_CLASS,
			};

			struct Namespace;

			struct Member {
				enum class Type {
					AV2_TA_SMT_UNKNOWN,
					AV2_TA_SMT_MACRO,
					AV2_TA_SMT_VARIABLE,
					AV2_TA_SMT_FUNCTION,
					AV2_TA_SMT_CLASS,
					AV2_TA_SMT_TYPE,
				};

				enum class Declaration {
					AV2_TA_SMD_UNDECLARED,
					AV2_TA_SMD_DECLARED,
					AV2_TA_SMD_INTERNAL,
					AV2_TA_SMD_EXTERNAL,
				};

				Type				type	= Type::AV2_TA_SMT_UNKNOWN;
				String				name;
				Data::Value			value	= Data::Value::object();
				Declaration			decl	= Declaration::AV2_TA_SMD_UNDECLARED;
				Instance<Member>	base	= nullptr;
				ID::VLUID			id		= all++;

				Instance<Namespace> ns;
				Instance<Macro>		macro;

				constexpr bool declared() const {return decl != Declaration::AV2_TA_SMD_UNDECLARED;}

				constexpr void declare() {
					decl = Declaration::AV2_TA_SMD_DECLARED;
				}

				constexpr void declareSpecial(bool const external) {
					if (external)
						decl = Declaration::AV2_TA_SMD_EXTERNAL;
					else decl = Declaration::AV2_TA_SMD_INTERNAL;
				}
			private:
				static inline ID::VLUID all = ID::VLUID::create(0);
			};

			constexpr bool contains(String const& name) const {
				return ns->members.contains(name);
			}

			constexpr Instance<Member> addMember(String const& name) {
				if (ns->members.contains(name)) return ns->members[name];
				Instance<Member> mem	= new Member{.name = name, .ns = new Namespace()};
				auto& sym				= mem->value;
				sym["name"]				= name;
				ns->members[name]		= mem;
				return mem;
			}

			constexpr Instance<Member> addVariable(String const& name, bool const global = false) {
				if (ns->members.contains(name)) return ns->members[name];
				auto mem = addMember(name);
				mem->type = Member::Type::AV2_TA_SMT_VARIABLE;
				auto& sym = mem->value;
				sym["global"]	= global;
				sym["init"]		= false;
				sym["use"]		= false;
				if (!global)
					sym["stack_id"] = stackc + varc++;
				return mem;
			}

			constexpr void addFunction(String const& name) {
				if (ns->members.contains(name)) return;
				auto mem = addMember(name);
				mem->type = Member::Type::AV2_TA_SMT_FUNCTION;
				auto& sym = mem->value;
				sym["overloads"] = sym.object();
				return;
			}

			constexpr Instance<Scope::Member> addMacro(String const& name) {
				if (ns->members.contains(name)) return ns->members[name];
				auto mem = addMember(name);
				mem->type = Member::Type::AV2_TA_SMT_MACRO;
				mem->value["id"] = toString("ID_", mem->id[0], "i", mem->id[1], "i", mem->id[2], "i", mem->id[3]);
				return mem;
			}

			constexpr Instance<Scope::Member> addTypeDefinition(String const& name, Nullable<Data::Value::Kind> const type = null) {
				if (ns->members.contains(name)) return ns->members[name];
				auto mem = addMember(name);
				mem->type = Member::Type::AV2_TA_SMT_TYPE;
				auto& sym = mem->value;
				if (type) {
					sym["basic"]	= true;
					sym["type"]		= type.value();
				}
				return mem;
			}

			constexpr String compose() const {
				return pre + "\n" + code + "\n" + post + "\n";
			}

			struct Namespace {
				String								name;
				Dictionary<Instance<Namespace>>		children;
				Dictionary<Instance<Scope::Member>>	members;

				constexpr void addChild(Instance<Namespace> const& ns) {
					if (!ns) return;
					if (hasChild(ns->name))
							for (auto [name, child]: ns->children)
								children[ns->name]->addChild(child);
					else children[ns->name] = ns;
				}

				constexpr bool hasChild(String const& name) {
					return children.contains(name);
				}

				constexpr bool append(Namespace const& ns) {
					for (auto const& [name, child]: ns.children)
						if (!hasChild(name))
							children[name] = child;
						else if (child && child != children[name])
							for (auto [gname, gchild]: child->children)
								children[name]->addChild(gchild);
					for (auto const& [name, child]: ns.members)
						if (!members.contains(name))
							members[name] = child;
					return true;
				}
			};

			struct Value {
				using Resolver	= Function<String()>;
				Instance<Scope::Member>		type;
				Resolver					resolver;
				Resolver					source		= resolveTo(".");

				constexpr Makai::String resolve() const {
					return resolver.invoke();
				}

				constexpr Makai::String resolveSource() const {
					return source.invoke();
				}
			};
			
			uint64				entry	= 0;
			Instance<Member>	result	= nullptr;
			bool				secure	= true;
			Type				type	= Type::AV2_TA_ST_NORMAL;
			String				name;
			String				label;
			uint64				varc	= 0;
			uint64				stackc	= 0;
			Instance<Namespace>	ns = new Namespace();
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

		constexpr static Data::Value::Kind DVK_ANY	= Data::Value::Kind{-2};
		constexpr static Data::Value::Kind DVK_DECL	= Data::Value::Kind{-3};

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

		constexpr void startScope(Scope::Type const type = Scope::Type::AV2_TA_ST_NORMAL) {
			if (scope.size())
				scope.pushBack({.stackc = currentScope().varc});
			else scope.pushBack({});
			scope.back().type = type;
		}

		constexpr Scope& currentScope() {
			if (scope.empty()) return global;
			return scope.back();
		}

		constexpr Scope const& currentScope() const {
			if (scope.empty()) return global;
			return scope.back();
		}

		constexpr void addStackEntry(Instruction::StackPush const& entry) {
			if (scope.empty()) return;
			addInstructionType(addNamedInstruction(Instruction::Name::AV2_IN_STACK_PUSH), entry);
		}

		constexpr void endScope() {
			if (scope.empty()) return;
			writeLine(scope.popBack().compose(), "");
		}

		constexpr void addFunctionExit() {
			uint64 varc = 0;
			for (auto& sc: Range::reverse(scope))
				if (sc.type == Scope::Type::AV2_TA_ST_FUNCTION) {
					varc = sc.varc;
					break;
				}
			if (varc)
				writeLine("clear", varc);
		}

		constexpr bool inFunction() const {
			for (auto& sc: Range::reverse(scope))
				if (sc.type == Scope::Type::AV2_TA_ST_FUNCTION) return true;
			return false;
		}

		constexpr bool inClass() const {
			for (auto& sc: Range::reverse(scope))
				if (sc.type == Scope::Type::AV2_TA_ST_CLASS) return true;
			return false;
		}

		constexpr bool inNamespace() const {
			if (inGlobalScope()) return true;
			else return currentScope().type == Scope::Type::AV2_TA_ST_NAMESPACE;
		}

		constexpr bool inGlobalScope() const {
			return scope.empty();
		}

		constexpr Scope& functionScope() {
			for (auto& sc: Range::reverse(scope))
				if (sc.type == Scope::Type::AV2_TA_ST_FUNCTION) return sc;
			throw Error::FailedAction("Not in function scope!");
		}

		constexpr bool hasSymbol(String const& name) const {
			for (auto const& sc: Range::reverse(scope)) {
				if (sc.contains(name)) return true;
			}
			if (global.contains(name)) return true;
			return false;
		}

		constexpr bool hasType(String const& name) const {
			for (auto const& sc: Range::reverse(scope)) {
				if (sc.contains(name) && sc.ns->members[name]->type == Scope::Member::Type::AV2_TA_SMT_TYPE) return true;
			}
			if (global.contains(name) && global.ns->members[name]->type == Scope::Member::Type::AV2_TA_SMT_TYPE) return true;
			return false;
		}

		constexpr bool hasNamespace(String const& name) const {
			for (auto const& sc: Range::reverse(scope))
				if (sc.ns->name == name) return true;
			for (auto& ns: global.ns->children)
				if (ns.value->name == name) return true;
			return false;
		}

		constexpr Scope::Member& getSymbolByName(String const& name) {
			return *getSymbolRefByName(name);
		}

		constexpr Instance<Scope::Member> getSymbolRefByName(String const& name) {
			for (auto& sc: Range::reverse(scope))
				if (sc.contains(name)) return sc.ns->members[name];
			if (global.contains(name)) return global.ns->members[name];
			throw Error::FailedAction("Context does not contain symbol '"+name+"'!");
		}

		constexpr Instance<Scope::Member> resolveSymbol(String const& name) {
			if (name.empty()) return nullptr;
			auto path = name.split('.').reverse();
			if (path.size() == 1) return getSymbolRefByName(path.back());
			auto ns = getNamespaceRefByName(path.popBack());
			while (path.size() > 1) {
				auto const next = path.popBack();
				if (ns->hasChild(next))
					ns = ns->children[next];
				else error<Error::NonexistentValue>("Namespace ["+next+"] does not exist!");
			}
			if (!ns->members.contains(path.back()))
				error<Error::NonexistentValue>("Symbol ["+path.back()+"] on namespace [" + ns->name + "] does not exist!");
			return ns->members[path.back()];
		}

		constexpr Scope::Namespace& getNamespaceByName(String const& name) {
			return *getNamespaceRefByName(name);
		}

		constexpr Instance<Scope::Namespace> getNamespaceRefByName(String const& name) {
			for (auto& sc: Range::reverse(scope))
				if (sc.ns->name == name) return sc.ns;
			for (auto& ns: global.ns->children)
				if (ns.value->name == name) return ns.value;
			throw Error::FailedAction("Context does not contain namespace '"+name+"'!");
		}

		constexpr String scopePath() const {
			return "_" + namespacePath("_");
		}

		template <class... Args>
		constexpr void write(Args const&... args) {
			auto& content = global.code;
			content += toString(toString(args, " ")...);
		}
		
		constexpr String namespacePath(String const& sep = ".") const {
			String path;
			for (auto& sc: scope)
				if (path.empty()) path = sc.name;
				else if (sc.name.size()) path += sep + sc.name;
			return path;
		}

		template <class... Args>
		constexpr void writeMainPreamble(Args const&... args) {
			auto& content = main.pre;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeMainPostscript(Args const&... args) {
			auto& content = main.post;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeGlobalLine(Args const&... args) {
			auto& content = global.code;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeGlobalPreamble(Args const&... args) {
			auto& content = global.pre;
			content += toString(toString(args, " ")..., "\n");
		}

		template <class... Args>
		constexpr void writeGlobalPostscript(Args const&... args) {
			auto& content = global.post;
			content += toString(toString(args, " ")..., "\n");
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
			content += toString(toString(args, " ")..., "\n");
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
			if (inGlobalScope() || inNamespace())	writeMainPreamble(args...);
			else									writeLine(args...);
		}

		template <class... Args>
		constexpr void writeFinale(Args const&... args) {
			auto& content = finale;
			content += toString(toString(args, " ")..., "\n");
		}

		constexpr static bool isCastable(Data::Value::Kind const type) {
			return Data::Value::isScalar(type) || Data::Value::isString(type) || type == DVK_ANY;
		}

		constexpr static bool isCastable(Instance<Scope::Member> const type) {
			if (!type) return false;
			if (isBasicType(type)) {
				auto const t = Cast::as<Data::Value::Kind>(type->value["type"].get<int64>());
				return Data::Value::isScalar(t) || Data::Value::isString(t) || t == DVK_ANY;
			}
			return false;
		}

		constexpr static bool isBasicType(Instance<Context::Scope::Member> const& type) {
			if (!type) return false;
			return type->value["basic"];
		}

		constexpr static bool isUndefined(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isUndefined(type->value["type"]);
		}

		constexpr static bool isNumber(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isNumber(type->value["type"]);
		}

		constexpr static bool isString(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isString(type->value["type"]);
		}

		constexpr static bool isObject(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isObject(type->value["type"]);
		}

		constexpr static bool isArray(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isArray(type->value["type"]);
		}

		constexpr static bool isInteger(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isInteger(type->value["type"]);
		}

		constexpr static bool isUnsigned(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isUnsigned(type->value["type"]);
		}

		constexpr static bool isVerifiable(Instance<Context::Scope::Member> const& type) {
			if (!isBasicType(type)) return false;
			return Data::Value::isVerifiable(type->value["type"]);
		}

		inline static String uniqueName() {
			uuid++;
			return Makai::toString("_i", uuid[3], "i", uuid[2], "i", uuid[1], "i", uuid[0]);
		}

		constexpr String compose() const {
			return global.compose() + "\n" + main.pre + "\n" + main.post + "\n" + finale;
		}

		constexpr Scope::Namespace& currentNamespace() {
			for (auto& sc: Range::reverse(scope))
				if (sc.type == Scope::Type::AV2_TA_ST_NAMESPACE)
					return *sc.ns;
			return *global.ns;
		}
		

		constexpr Handle<Scope::Namespace> currentNamespaceRef() {
			for (auto& sc: Range::reverse(scope))
				if (sc.type == Scope::Type::AV2_TA_ST_NAMESPACE)
					return sc.ns;
			return global.ns;
		}

		void fetchNext() {
			if (!nextToken())
				error<Error::NonexistentValue>("Unexpected end-of-file!");
		}

		bool nextToken() {
			if (append.hasTokens()) {
				append.next();
				return true;
			}
			return stream.next();
		}

		bool hasToken(Tokenizer::Token::Type const type) {
			return currentToken().type == type;
		}

		constexpr bool hasModule(String const& fullName) {
			return modules.contains(fullName);
		}

		constexpr void registerModule(String const& fullName) {
			modules[fullName] = true;	
		}

		Tokenizer::Token currentToken() const {
			if (append.hasTokens())
				return append.current();
			return stream.current();
		}

		template <class T>
		T getValue() const {
			return currentToken().value.template get<T>();
		}

		String getModuleFile(String const& path) const {
			DEBUGLN("Locating module '"+path+"'...");
			for (auto const& source: sourcePaths) {
				auto const fullName = source + "/" + path + ".bv";
				DEBUGLN("  Searching for: '"+fullName+"'");
				if (OS::FS::exists(source) && OS::FS::exists(fullName)) {
					DEBUGLN("Found!");
					return Makai::File::loadText(fullName);
				} else if (File::isArchiveAttached()) try {
					auto const f = Makai::File::loadTextFromArchive(fullName);
					DEBUGLN("Found!");
					return f;
				} catch (...) {}
			}
			DEBUGLN("Not found");
			error<Error::NonexistentValue>("Module file '"+path+"' does not exist or could not be found!");
		}

		template <Type::Derived<Error::Generic> E = Error::InvalidValue>
		[[noreturn]]
		void error(String const& what) const {
			auto const pos = stream.position();
			throw E(
				Makai::toString(
					"At:\nLINE: ", pos.line,
					"\nCOLUMN: ", pos.column,
					"\n--> [", stream.tokenText(), "]"
				),
				what,
				Makai::CPP::SourceFile{"n/a", Cast::as<int>(pos.line), fileName}
			);
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
			if (name == "string" || name == "text")							return true;
			if (name == "binary" || name == "bytes")						return true;
			if (name == "array" || name == "list")							return true;
			if (name == "object" || name == "data")							return true;
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
			if (name == "export" || name == "import")						return true;
			if (name == "signal")											return true;
			if (name == "main")												return true;
			return false;
		}

		constexpr uint64 stackSize() const {
			return currentScope().stackc + currentScope().varc;
		}

		constexpr uint64 relativeStackOffset(Instance<Scope::Member> const& sym) {
			auto const sid = stackSize() - (sym->value["stack_id"].get<uint64>() + 1);
			return sid;
		}

		constexpr String stackIndex(Instance<Scope::Member> const& sym) {
			return "-" + toString(relativeStackOffset(sym));
		}

		constexpr Scope::Value::Resolver varAccessor(Instance<Scope::Member> const& sym) {
			return [&, sym] {
				if (sym->value["extern"])
					return "@" + sym->value["name"].get<String>();
				if (sym->value["global"])
					return ":" + sym->value["name"].get<String>();
				return "&[" + stackIndex(sym) + "]";
			};
		}

		String intermediate() const {
			auto prg = Makai::Regex::replace(compose(), "([\\n\\r\\f][\\t\\ ]*)([\\n\\r\\f][\\t\\ ]*)+", "\n\n");
			prg = Makai::Regex::replace(prg, "[\\t\\ ]+", " ");
			return prg;
		}

		constexpr Instance<Scope::Member> getBasicType(String const& name) {
			return global.ns->members[name];
		}

		Context() {
			auto const voidT	= global.addTypeDefinition("void",		Data::Value::Kind::DVK_VOID		);
			auto const nullT	= global.addTypeDefinition("null",		Data::Value::Kind::DVK_NULL		);
			auto const intT		= global.addTypeDefinition("int",		Data::Value::Kind::DVK_SIGNED	);
			auto const uintT	= global.addTypeDefinition("uint",		Data::Value::Kind::DVK_UNSIGNED	);
			auto const floatT	= global.addTypeDefinition("float",		Data::Value::Kind::DVK_REAL		);
			auto const stringT	= global.addTypeDefinition("string",	Data::Value::Kind::DVK_STRING	);
			auto const bytesT	= global.addTypeDefinition("bytes",		Data::Value::Kind::DVK_BYTES	);
			auto const arrayT	= global.addTypeDefinition("array",		Data::Value::Kind::DVK_ARRAY	);
			auto const objectT	= global.addTypeDefinition("object",	Data::Value::Kind::DVK_OBJECT	);
			auto const anyT		= global.addTypeDefinition("any",		DVK_ANY							);
			global.ns->members["unsigned"]	= uintT;
			global.ns->members["signed"]	= intT;
			global.ns->members["real"]		= floatT;
			global.ns->members["text"]		= stringT;
			global.ns->members["str"]		= stringT;
			global.ns->members["binary"]	= bytesT;
			global.ns->members["list"]		= arrayT;
			global.ns->members["data"]		= objectT;
			global.ns->members["nil"]		= nullT;
		}

		struct Appendix {
			Tokenizer::TokenList	cache;
			
			constexpr void add(Tokenizer::Token const& tok)			{cache.pushBack(tok);								}
			constexpr void add(Tokenizer::TokenList const& toks)	{cache.appendBack(toks);							}
			constexpr bool hasTokens() const						{return ct < cache.size();							}
			constexpr bool next()									{if (ct < cache.size()) return ++ct; return false;	}
			constexpr Tokenizer::Token current() const				{return cache[ct-1];								}
		
		private:
			usize					ct = 0;
		};

		StringList				sourcePaths;

		Scope					global;
		List<Scope>				scope;
		Jumps					jumps;
		Tokenizer				stream;
		Appendix				append;
		Program					program;
		String					fileName;
		Random::SecureGenerator	rng;

		Dictionary<bool>		modules;

		List<Instance<Scope::Member>> functions;

		Dictionary<Instance<Macro>> macros;

		constexpr Instance<Macro> getMacro(Instance<Scope::Member> const& macro) const {
			if (macro->type != Scope::Member::Type::AV2_TA_SMT_MACRO) return nullptr;
			if (macro->macro) return macro->macro;
			auto const macroID = macro->value["id"].get<Makai::String>();
			if (!macros.contains(macroID)) return nullptr;
			return macros[macroID];
		}
		
		bool					hasMain		= false;
		bool					isModule	= false;

		struct SegmentedScope {
			String	preEntryPoint;
			String	entryPoint;
			String	postEntryPoint;
			String	pre;
			String	post;
		};

		void importModule(Instance<Scope::Namespace> const& ns) {
			if (!ns) return;
			if (ns->name.empty())
				error<Error::FailedAction>("INTERNAL ERROR: Missing namespace name!");
			global.ns->addChild(ns);
			global.ns->addChild(new Scope::Namespace{"__imports"});
			global.ns->children["__imports"]->addChild(ns);
		}

		SegmentedScope main {
			"__pre"		+ uniqueName(),
			"__main"	+ uniqueName(),
			"__post"	+ uniqueName()
		};

		String finale;

		inline static ID::VLUID uuid = ID::VLUID::create(0);

		constexpr static Scope::Value::Resolver resolveTo(String const& value) {
			return [=] {return value;};
		}
	};
}

#endif