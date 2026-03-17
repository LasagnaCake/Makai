#include "macro.hpp"
#include "../../../../../data/data.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;

using Type = BaseContext::Tokenizer::Token::Type;
using enum Type;

using Context = BaseContext;

Macro::Rule::Match::Result Macro::Rule::Match::match(Macro::Arguments const& args, Macro::Rule::Match::Callback const& call) const {
	Arguments result;
	if (args.empty()) return variadic ? Result{Arguments()} : null;
	if (!count) return Arguments();
	if (matches.size()) return matchGroup(args, call);
	if (!variadic && count >= args.size()) return null;
	auto const sz = (!variadic) ? count : Math::min(count, args.size());
	// DEBUGLN(">>> $--- Variadic match? ", variadic);
	switch (type) {
		case Type::AV2_TA_SM_RMT_WHATEVER: {
			//if (inRunTime()) DEBUGLN("::: WHATEVER");
			if (sz < minimum)
				return null;
			return args.sliced(0, sz);
		} break;
		case Type::AV2_TA_SM_RMT_ANY_OF: {
			//if (inRunTime()) DEBUGLN("::: TOKEN");
			//if (inRunTime()) DEBUGLN("Tokens: [", tokens.toList<String>([] (auto const& elem) {return Tokenizer::Token::asName(elem.type);}).join(", "), "]");
			bool next = false;
			for (usize i = 0; i < sz; ++i) {
				//DEBUGLN("[", Tokenizer::Token::asName(args[i].type), "]");
				next = false;
				for (auto& tok : tokens) {
					if (tok == args[i]) {
						result.pushBack(args[i]);
						next = true;
						continue;
					}
				}
				if (next)							continue;
				else if (variadic && i >= minimum)	break;
				else								return null;
			}
		} break;
		case Type::AV2_TA_SM_RMT_EXPRESSION: {
			//if (inRunTime()) DEBUGLN("::: EXPRESSION");
			if (expressionSolver)
				result = expressionSolver(args).value();
			//if (inRunTime()) DEBUGLN("Expression: [", result.toList<String>([] (auto const& elem) {return Tokenizer::Token::asName(elem.type);}).join(""), "]");
		} break;
	}
	// DEBUGLN("$--- Variadic match? ", variadic);
	// DEBUGLN("$--- Match size: ", sz);
	// DEBUGLN("$--- Total: ", result.size());
	call.invoke(*this, result);
	return result;
}

Macro::Arguments Macro::Rule::Match::solveExpression(Macro::Arguments const& args) {
	Arguments result;
	usize prev = 0;
	for (usize i = 0; i < args.size();) {
		using Type = Tokenizer::Token::Type;
		if (isScopeStarter(args[i].type)) {
			switch (args[i].type) {
				case Type{'('}: result.appendBack(solveParameterPack(args.sliced(i), Type{')'})); break;
				case Type{'{'}: result.appendBack(solveParameterPack(args.sliced(i), Type{'}'})); break;
				case Type{'['}: result.appendBack(solveParameterPack(args.sliced(i), Type{']'})); break;
				default: break;
			}
		} else if (isExpressionToken(args[i].type)) {
			result.pushBack(args[i]);
			DEBUGLN("$ -> ", i, ":", args[i].token);
		}
		else break;
		i += result.size() - prev;
		prev = result.size();
	}
	return result;
}

Macro::Arguments Macro::Rule::Match::solveParameterPack(Macro::Arguments const& args, Tokenizer::Token::Type const end) {
	Arguments result;
	usize prev	= 1;
	usize i		= 1;
	result.pushBack(args.front());
	while (i < args.size()) {
		using Type = Tokenizer::Token::Type;
		if (args[i].type == end) break;
		switch (args[i].type) {
			case Type{'('}: result.appendBack(solveParameterPack(args.sliced(i), Type{')'})); break;
			case Type{'{'}: result.appendBack(solveParameterPack(args.sliced(i), Type{'}'})); break;
			case Type{'['}: result.appendBack(solveParameterPack(args.sliced(i), Type{']'})); break;
			default: DEBUGLN(". -> ", i, ":", args[i].token); result.pushBack(args[i]); break;
		}
		i += result.size() - prev;
		prev = result.size();
	}
	if (i < args.size())
		result.pushBack(args[i]);
	return result;
}

Macro::Rule::Match::Result Macro::Rule::Match::matchGroup(Macro::Arguments const& args, Macro::Rule::Match::Callback const& call) const {
	//if (inRunTime()) DEBUGLN("::: GROUP");
	if (matches.empty()) return null;
	Arguments result;
	if (!count) return Arguments();
	auto const sz = (!variadic) ? count : Math::min(count, args.size());
	usize tokenStart	= 0;
	usize matchCount	= 0;
	Result mr;
	// DEBUGLN(">>> .--- Variadic match? ", variadic);
	do {
		//if (inRunTime()) DEBUGLN("<match>");
		for (auto& match: matches) {
			//if (inRunTime()) DEBUGLN("<sub-match>");
			if (tokenStart >= args.size()) {
				if (result.empty() || !variadic) return null;
				mr = Result{result};
				break;
			}
			else mr = match->match(args.sliced(tokenStart), call);
			//if (inRunTime()) DEBUGLN("</sub-match>");
			if (!mr) break;
			auto const v = mr.value();
			//if (inRunTime()) DEBUGLN("Total match count: ", v.size());
			if (v.empty()) break;
			tokenStart += v.size();
			result.appendBack(v);
		}
		//if (inRunTime()) DEBUGLN("</match>");
		if (!mr || mr.value().empty()) break;
		if (++matchCount >= sz) break;
	} while (true);
	//if (inRunTime()) DEBUGLN("Matched: [", result.toList<String>([] (auto const& elem) {return Tokenizer::Token::asName(elem.type);}).join(""), "]");
	if (matchCount < minimum)
		return null;
	// DEBUGLN(".--- Variadic match? ", variadic);
	// DEBUGLN(".--- Match size: ", sz);
	// DEBUGLN(".--- Total: ", matchCount);
	if (variadic || matchCount >= sz)
		return result;
	return null;
}

void Macro::Context::parse() {
	auto const match = rule.match(
		input,
		[&] (Rule::Match const& match, Arguments const& result) {
			if (rule.variables.contains(match.id())) {
				//DEBUGLN("--- Variable: [", rule.variables[match.id()], "]");
				//DEBUGLN("--- Match: [", result.toList<Makai::String>([] (auto const& elem) -> Makai::String {return elem.token;}).join(), "]");
				variables[rule.variables[match.id()]].tokens.pushBack(result);
			}
		}
	);
	if (match)
		result.match = match.value();
	else baseContext.error("Macro expansion failure!");
}

static void doMacroRuleType(Context& context, Macro::Rule& rule, Macro::Rule::Match& base) {
	context.next();
	switch (context.type()) {
		case LTS_TT_IDENTIFIER: {
			auto const varType = context.value().getString();
			if (varType == "expr" || varType == "expression") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_EXPRESSION;
			} else if (varType == "str" || varType == "string") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_SINGLE_QUOTE_STRING}});
				base.tokens.pushBack({{.type = LTS_TT_DOUBLE_QUOTE_STRING}});
			} else if (varType == "id" || varType == "identifier") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_IDENTIFIER}});
			} else if (varType == "name") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_IDENTIFIER}});
				base.tokens.pushBack({{.type = LTS_TT_SINGLE_QUOTE_STRING}});
				base.tokens.pushBack({{.type = LTS_TT_DOUBLE_QUOTE_STRING}});
			} else if (varType == "sqstr" || varType == "sqstring") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_SINGLE_QUOTE_STRING}});
			} else if (varType == "dqstr" || varType == "dqstring") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_DOUBLE_QUOTE_STRING}});
			} else if (varType == "int" || varType == "integer") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_INTEGER}});
			} else if (varType == "real") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_REAL}});
			} else if (varType == "number" || varType == "num") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_INTEGER}});
				base.tokens.pushBack({{.type = LTS_TT_REAL}});
			} else if (varType == "char" || varType == "character") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_CHARACTER}});
			} else if (varType == "value") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_IDENTIFIER}});
				base.tokens.pushBack({{.type = LTS_TT_SINGLE_QUOTE_STRING}});
				base.tokens.pushBack({{.type = LTS_TT_DOUBLE_QUOTE_STRING}});
				base.tokens.pushBack({{.type = LTS_TT_INTEGER}});
				base.tokens.pushBack({{.type = LTS_TT_REAL}});
				base.tokens.pushBack({{.type = LTS_TT_CHARACTER}});
			} else if (varType == "mathop") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = Type{'+'}}});
				base.tokens.pushBack({{.type = Type{'-'}}});
				base.tokens.pushBack({{.type = Type{'*'}}});
				base.tokens.pushBack({{.type = Type{'/'}}});
				base.tokens.pushBack({{.type = Type{'%'}}});
			} else if (varType == "binop") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = Type{'^'}}});
				base.tokens.pushBack({{.type = Type{'|'}}});
				base.tokens.pushBack({{.type = Type{'&'}}});
				base.tokens.pushBack({{.type = Type{'~'}}});
			} else if (varType == "logicop") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = Type{'!'}}});
				base.tokens.pushBack({{.type = LTS_TT_LOGIC_AND}});
				base.tokens.pushBack({{.type = LTS_TT_LOGIC_OR}});
			} else if (varType == "arrow") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_LITTLE_ARROW}});
				base.tokens.pushBack({{.type = LTS_TT_BIG_ARROW}});
			} else if (varType == "incdec") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_INCREMENT}});
				base.tokens.pushBack({{.type = LTS_TT_DECREMENT}});
			} else if (varType == "compare") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = LTS_TT_COMPARE_LESS_EQUALS}});
				base.tokens.pushBack({{.type = LTS_TT_COMPARE_GREATER_EQUALS}});
				base.tokens.pushBack({{.type = LTS_TT_COMPARE_EQUALS}});
				base.tokens.pushBack({{.type = LTS_TT_COMPARE_NOT_EQUALS}});
				base.tokens.pushBack({{.type = Type{'>'}}});
				base.tokens.pushBack({{.type = Type{'<'}}});
			} else if (varType == "assignop") {
				base.tokens.pushBack({{.type = Type{'='}}});
				// TODO: The rest
			} else if (varType == "otherop") {
				base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
				base.tokens.pushBack({{.type = Type{'@'}}});
				base.tokens.pushBack({{.type = Type{':'}}});
			} else {
				context.error("Invalid rule type!");
			}
		} break;
		case Type{'['}: {
			base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
			while (true) {
				if (context.next().has(Type{']'}))
					break;
				switch (context.type()) {
					case Type{'$'}: {
						context.next();
						base.tokens.pushBack(context.token());
					} break;
					case Type{'@'}: {
						doMacroRuleType(context, rule, base);
					} break;
					default: {
						base.tokens.pushBack(context.token());
					} break;
				}
			}
		} break;
		default: {
			base.type = decltype(base.type)::AV2_TA_SM_RMT_ANY_OF;
			base.tokens.pushBack(context.token());
		} break;
	}
}

static void doMacroRule(Context& context, Macro::Rule& rule, Macro::Rule::Match& base);

static void doMacroRuleGroup(Context& context, Macro::Rule& rule, Macro::Rule::Match& base) {
	context.expect(Type{'{'});
	while (true) {
		if (context.next().has(Type{'}'})) break;
		doMacroRule(context, rule, *base.addSubMatch());
	}
	context.expect(Type{'}'});
}

static void doMacroRule(Context& context, Macro::Rule& rule, Macro::Rule::Match& base) {
	switch (context.type()) {
		case Type{'$'}: {
			context.next();
			switch (context.type()) {
				case LTS_TT_IDENTIFIER: {
					auto const varName = context.value().getString();
					context.expectNext(Type{':'});
					doMacroRuleType(context, rule, base);
					rule.variables[base.id()] = varName;
				} break;
				case Type{'+'}:
				case Type{'@'}:
				case Type{'?'}:
				case Type{'#'}:
				case Type{'$'}:
				case Type{'*'}:
				case Type{'{'}:
				case Type{'}'}:
				case Type{'['}:
				case Type{']'}:
				case Type{'('}:
				case Type{')'}: base.tokens.pushBack(context.token()); break;
				default: break;
			}
		} break;
		case Type{'*'}: {
			base.variadic	= true;
			base.minimum	= 0;
			base.count		= -1;
			context.next();
			doMacroRule(context, rule, *base.addSubMatch());
		} break;
		case Type{'?'}: {
			base.variadic	= true;
			base.minimum	= 0;
			base.count		= 1;
			context.next();
			doMacroRule(context, rule, *base.addSubMatch());
		} break;
		case Type{'+'}: {
			base.variadic	= true;
			base.minimum	= 1;
			base.count		= -1;
			context.next();
			doMacroRule(context, rule, *base.addSubMatch());
		}
		case Type{'#'}: {
			base.variadic	= true;
			context.expectNext(Type{'['});
			base.minimum	= context.getNext(LTS_TT_INTEGER, "minimum match count").getUnsigned();
			context.expectNext(Type{':'});
			base.count		= -1;
			if (!context.next().has(Type{']'}))
				base.count = context.getNext(LTS_TT_INTEGER, "maximum match count").getUnsigned();
			context.expect(Type{']'});
			doMacroRule(context, rule, *base.addSubMatch());
		} break;
		case Type{'{'}: {
			doMacroRuleGroup(context, rule, *base.addSubMatch());
		} break;
		case Type{'['}: {
			while (true) {
				if (context.next().has(Type{']'}))
					break;
				switch (context.type()) {
					case Type{'$'}: {
						base.tokens.pushBack(context.next().token());
					} break;
					case Type{'@'}: {
						doMacroRuleType(context, rule, base);
					} break;
					default: {
						base.tokens.pushBack(context.token());
					} break;
				}
			}
		} break;
		case Type{'@'}: {
			doMacroRuleType(context, rule, base);
		} break;
		default: {
			base.tokens.pushBack(context.token());
		} break;
	}
}

Makai::Instance<Macro::Transformation> macroApply(Macro::Arguments const& values) {
	return new Macro::Transformation{
		.pre = [=] (Macro::Context& context) {
			context.result.value.appendBack(values);
		}
	};
}

namespace {
	static Makai::Random::SecureGenerator rng;
	static inline Makai::ID::VLUID uuid = uuid.create(0);

	static Makai::String giveMeAName() {
		++uuid;
		return Makai::toString("_u", uuid[0], "_u", uuid[1], "_u", uuid[2], "_u", uuid[3]);
	}

	struct IExpandAfter {
		using Instance = Makai::Instance<IExpandAfter>;
		virtual Makai::String expand(Macro::Context& context) const = 0;

		virtual ~IExpandAfter() {}
	};

	struct ExpandToValue: IExpandAfter {
		Makai::String expand(Macro::Context& context) const {
			return value;
		}

		ExpandToValue(Makai::String const& value): value(value) {}

	private:
		Makai::String value;
	};

	struct ExpandToVariable: IExpandAfter {
		Makai::String expand(Macro::Context& context) const {
			if (!context.variables.contains(var)) return "";
			Makai::String result;
			for (auto& toks: context.variables[var].tokens) {
				for (auto& tok: toks) {
					switch (tok.type) {
						case LTS_TT_SINGLE_QUOTE_STRING:
						case LTS_TT_DOUBLE_QUOTE_STRING:
						case LTS_TT_IDENTIFIER:
							result += tok.value.getString();
						default: result += tok.token;
					}
				}
			}
			return result;
		}

		ExpandToVariable(Makai::String const& var): var(var) {}

	private:
		Makai::String var;
	};

	struct ExpansionGroup: IExpandAfter {
		Makai::List<Instance> sub;

		Makai::String expand(Macro::Context& context) const {
			Makai::String result = "";
			for (auto& s: sub)
				result += s->expand(context);
			return result;
		}
	};

	struct ExpandToEncryption: IExpandAfter {
		Instance base;

		Makai::String expand(Macro::Context& context) const {
			Makai::String result = base->expand(context);
			return Makai::Data::encode(
				Makai::Data::hashed(result.toBytes()),
				Makai::Data::EncodingType::ET_BASE64
			);
		}
	};

	static ExpansionGroup::Instance doExpansionGroup(Context& context, Macro::Rule& rule) {
		auto content = new ExpansionGroup();
		switch(context.type()) {
			case LTS_TT_SINGLE_QUOTE_STRING:
			case LTS_TT_DOUBLE_QUOTE_STRING:
				content->sub.pushBack(new ExpandToValue(context.value().getString()));
			break;
			case Type{'$'}: {
				switch (context.next().type()) {
					case LTS_TT_IDENTIFIER: {
						auto const varID = context.value().getString();
						if (rule.variables.values().find(varID) == -1)
							context.error("Macro variable does not exist!");
						content->sub.pushBack(new ExpandToVariable(varID));
					} break;
					default: {

					} break;
				}
			} break;
			case Type{'%'}: {
				context.expectNext(Type{'('});
				auto pre = new ExpansionGroup();
				while (!context.has(Type{')'})) {
					if (context.next().has(Type{')'}))
						break;
					pre->sub.pushBack(doExpansionGroup(context, rule));
				}
				auto const enc = new ExpandToEncryption();
				enc->base = pre;
				content->sub.pushBack(enc);
				context.expect(Type{')'});
			} break;
			case Type{'*'}: {
				context.expectNext(Type{'('});
				while (!context.has(Type{')'})) {
					if (context.next().has(Type{')'}))
						break;
					content->sub.pushBack(doExpansionGroup(context, rule));
				}
				context.expect(Type{')'});
			} break;
			case Type{'+'}: {
				content->sub.pushBack(new ExpandToValue(Makai::toString(rng.integer<uint64>())));
			} break;
			case Type{'-'}: {
				content->sub.pushBack(new ExpandToValue(Makai::toString(rng.integer<int64>())));
			} break;
			case Type{'~'}: {
				content->sub.pushBack(new ExpandToValue(Makai::toString(rng.real<double>())));
			} break;
			case Type{'@'}: {
				content->sub.pushBack(new ExpandToValue(giveMeAName()));
			} break;
			default: context.error("Invalid tokenization!");
		}
		return content;
	}
}

static void doMacroTransform(
	Context& context,
	Macro::Rule& rule,
	Macro::Transformation& base
) {
	Macro::Arguments result;
	while (true) {
		if (context.next().has(Type{'}'})) break;
		switch (context.type()) {
			case Type{'$'}: {
				if (result.size())
					base.sub.pushBack(macroApply(result));
				result.clear();
				context.next();
				switch (context.type()) {
					case LTS_TT_IDENTIFIER: {
						auto const varName = context.value().getString();
						if (rule.variables.values().find(varName) == -1)
							context.error("Macro variable does not exist!");
						//DEBUGLN("--- Transform::Variable: [", varName, "]");
						base.newTransform()->pre = [varName = Makai::copy(varName)] (Macro::Context& context) {
							//DEBUGLN("--- SIMPLE VARIABLE EXPANSION");
							//DEBUGLN("--- Apply::Variable: [", varName, "]");
							auto toks = context.variables[varName].tokens;
							//DEBUGLN("--- Apply::Argc: [", toks.size(), "]");
							for (auto& tok: toks)
								context.result.value.appendBack(tok);
						};
					} break;
					case Type{'*'}: {
						auto const varName = context.getNext(LTS_TT_IDENTIFIER, "macro variable name").getString();
						if (rule.variables.values().find(varName) == -1)
							context.error("Macro variable does not exist!");
						//DEBUGLN("--- Transform::Variable: [", varName, "]");
						context.expectNext(Type{'{'});
						Makai::Instance<Macro::Transformation> tf = tf.create();
						doMacroTransform(context, rule, *tf);
						context.expect(Type{'}'});
						base.newTransform()->pre =
							[varName = Makai::copy(varName), tf = Makai::copy(tf)] (Macro::Context& ctx) {
								//DEBUGLN("--- COMPLEX VARIABLE EXPANSION");
								Macro::Context subctx = {ctx};
								subctx.result = {};
								tf->apply(subctx);
								//DEBUG("Separator: [ ");
								for (auto& tok: subctx.result.value)
									DEBUG(tok.token, " ");
								//DEBUGLN("]");
								//DEBUGLN("--- Apply::Variable: [", varName, "]");
								auto toks = ctx.variables[varName].tokens;
								//DEBUGLN("--- Apply::Argc: [", toks.size(), "]");
								usize i = 0;
								for (auto& tok: toks) {
									//DEBUGLN(i);
									if (i) ctx.result.value.appendBack(subctx.result.value);
									ctx.result.value.appendBack(tok);
									++i;
								}
							}
						;
					} break;
					case Type{'='}: {
						auto const toks = doExpansionGroup(context.next(), rule);
						base.newTransform()->pre = [toks] (auto& ctx) {
							auto const stream = toks->expand(ctx);
							Tokenizer tz;
							Makai::List<Macro::Axiom> appendix;
							tz.open(stream);
							while (tz.next())
								appendix.pushBack({{tz.current()}, true, tz.tokenText(), tz.position()});
							ctx.result.value.appendBack(appendix);
						};
					} break;
					case Type{':'}: {
						auto const toks = doExpansionGroup(context.next(), rule);
						base.newTransform()->pre = [toks] (auto& ctx) {
							auto const v = toks->expand(ctx);
							ctx.result.value.pushBack({{.type = LTS_TT_DOUBLE_QUOTE_STRING, .value = v}});
						};
					} break;
					case Type{'!'}: {
						auto const msgt = context.getNext(LTS_TT_IDENTIFIER, "message type").getString();
						if (msgt == "error" || msgt == "err"){
							auto const msgv = doExpansionGroup(context.next(), rule);
							base.newTransform()->pre = [msgv] (auto& ctx) {
								ctx.baseContext.template error<MacroError>(msgv->expand(ctx));
							};
						} else if (msgt == "warning" || msgt == "warn") {
							auto const msgv = doExpansionGroup(context.next(), rule);
							base.newTransform()->pre = [msgv] (auto& ctx) {
								ctx.baseContext.out.writeLine("Warning: ", msgv->expand(ctx));
								ctx.baseContext.out.writeLine("At: ", ctx.baseContext.token().position.line);
								ctx.baseContext.out.writeLine("Column: ", ctx.baseContext.token().position.column);
							};
						} else if (msgt == "message" || msgt == "msg") {
							auto const msgv = doExpansionGroup(context.next(), rule);
							base.newTransform()->pre = [msgv] (auto& ctx) {
								auto content = msgv->expand(ctx);
								content = Makai::Regex::replace(content, "\\$\\{LINE\\}", Makai::toString(ctx.baseContext.token().position.line));
								content = Makai::Regex::replace(content, "\\$\\{FILE\\}", Makai::toString(ctx.baseContext.fileName));
								ctx.baseContext.out.writeLine("Message: ", content);
							};
						}
						else context.error("Invalid message type!");
					} break;
					case Type{'$'}:
					case Type{'{'}:
					case Type{'}'}: result.pushBack(context.token()); break;
					default: context.error("Invalid macro expansion!");
				}
			} break;
			default: result.pushBack(context.token());
		}
	}
	if (result.size())
		base.sub.pushBack(macroApply(result));
}

static Macro::Expression doMacroExpression(Context& context, Macro& macro) {
	Macro::Expression expr;
	doMacroRule(context, expr.rule, *expr.rule.root);
	context.expectNext(LTS_TT_BIG_ARROW);
	context.expectNext(Type{'{'});
	doMacroTransform(context, expr.rule, expr.transform);
	context.expect(Type{'}'});
	return expr;
}

Makai::Instance<Macro> Macro::build(BaseContext& context) {
	Makai::Instance<Macro> macro = macro.create();
	doMacroExpression(context, *macro);
}
