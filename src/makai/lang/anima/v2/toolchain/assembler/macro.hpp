#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MACRO_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MACRO_H

#include "../../../../../lexer/lexer.hpp"
#include "../../core/core.hpp"
#include "context.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Macro {
		using Axiom		= BaseContext::Axiom;
		using Tokenizer	= BaseContext::Tokenizer;

		DEFINE_ERROR_TYPE_EX(MacroError, InvalidValue);

		using Stack			= List<Axiom>;
		using Arguments		= List<Axiom>;

		struct Result {
			Arguments	match;
			List<Axiom>	value;
		};

		struct Rule: ID::Identifiable<Rule> {
			struct Match: Identifiable<Match> {
				enum class Type {
					AV2_TA_SM_RMT_WHATEVER,
					AV2_TA_SM_RMT_ANY_OF,
					AV2_TA_SM_RMT_EXPRESSION
				}	type = Type::AV2_TA_SM_RMT_ANY_OF;
				bool					variadic	= false;
				usize					minimum		= 1;
				usize					count		= 1;
				List<Axiom>				tokens;
				List<Instance<Match>>	matches;

				using Count		= Nullable<usize>;
				using Result	= Nullable<Arguments>;

				constexpr Instance<Match> addSubMatch() {
					return matches.pushBack(new Match()).back();
				}

				using Callback = Functor<void(Match const&, Arguments const&)>;

				constexpr Result match(Arguments const& args, Callback const& call = {}) const {
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

				constexpr Count fit(Arguments const& args) const {
					if (auto const result = match(args))
						return result.value().size();
					return null;
				}

				constexpr static Arguments solveExpression(Arguments const& args) {
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

				constexpr static Arguments solveParameterPack(Arguments const& args, Tokenizer::Token::Type const end) {
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

				static inline Functor<Result(Arguments const&)> expressionSolver =
					[] (Arguments const& args) -> Result {
						return solveExpression(args);
					}
				;

			private:
				constexpr Result matchGroup(Arguments const& args, Callback const& call = {}) const {
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
			};

			template <class T>
			using Bank = Map<Match::IdentifierType, T>;

			Bank<String>	variables;
			Instance<Match>	root		= new Match();

			constexpr Match::Count fit(Arguments const& args) const {
				return root ? root->fit(args) : null;
			}

			constexpr Match::Result match(Arguments const& args, Match::Callback const& call = {}) const {
				return root ? root->match(args, call) : null;
			}
		};

		struct Context {
			Arguments		input;
			Stack			stack;
			Result			result;
			Rule			rule;
			BaseContext&	baseContext;

			struct Variable {
				List<Arguments>	tokens;
			};

			Dictionary<Variable> variables;

			void parse() {
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
				else baseContext.error<Error::FailedAction>("Macro expansion failure!");
			}
		};

		struct Transformation {
			using Action = Functor<void(Context&)>;

			Action							pre;
			List<Instance<Transformation>>	sub;
			Action							post;

			constexpr Instance<Transformation> newTransform() {
				return sub.pushBack(new Transformation()).back();
			}

			constexpr Transformation& apply(Context& ctx) {
				pre(ctx);
				for (auto& action: sub)
					action->apply(ctx);
				post(ctx);
				return *this;
			}

			constexpr void apply(Context& ctx) const {
				pre(ctx);
				for (auto& action: sub)
					action->apply(ctx);
				post(ctx);
			}

			constexpr Result result(Context& ctx) const {
				return ctx.result;
			}
		};

		struct Expression {
			Rule			rule;
			Transformation	transform;
		};

		using Expressions	= List<Expression>;

		Expressions	exprs;

		bool simple = false;

		Nullable<Result> resolve(Arguments const& args, BaseContext& context) {
			if (simple) {
				if (exprs.empty()) return null;
				Context ctx{.input = args, .baseContext = context};
				return exprs.front().transform.apply(ctx).result(ctx);
			} else {
				usize i = 0;
				for (auto& expr: exprs)
					if (expr.rule.fit(args)) {
						DEBUGLN("Found matching rule: ", i);
						Context ctx{.input = args, .rule = expr.rule, .baseContext = context};
						ctx.parse();
						return expr.transform.apply(ctx).result(ctx);
					} else ++i;
			}
			return null;
		}

		constexpr static bool isExpressionToken(Tokenizer::Token::Type const& type) {
			using Type = Tokenizer::Token::Type;
			switch (type) {
				case (Type::LTS_TT_IDENTIFIER):
				case (Type::LTS_TT_SINGLE_QUOTE_STRING):
				case (Type::LTS_TT_DOUBLE_QUOTE_STRING):
				case (Type::LTS_TT_INTEGER):
				case (Type::LTS_TT_REAL):
				case (Type{'.'}): return true;
				default: return false;
			}
		}

		constexpr static bool isScopeStarter(Tokenizer::Token::Type const& type) {
			using Type = Tokenizer::Token::Type;
			switch (type) {
				case (Type{'{'}):
				case (Type{'('}):
				case (Type{'['}):
				return true;
				default: return false;
			}
		}
	};
}

#endif
