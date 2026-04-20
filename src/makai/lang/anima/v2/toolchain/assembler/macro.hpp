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

				Result match(Arguments const& args, Callback const& call = {}) const;

				Count fit(Arguments const& args) const {
					if (auto const result = match(args))
						return result.value().size();
					return null;
				}

				static Arguments solveExpression(Arguments const& args);
				static Arguments solveParameterPack(Arguments const& args, Tokenizer::Token::Type const end);

				static inline Functor<Result(Arguments const&)> expressionSolver =
					[] (Arguments const& args) -> Result {
						return solveExpression(args);
					}
				;

			private:
				Result matchGroup(Arguments const& args, Callback const& call = {}) const;
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

			void parse();
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

		static Instance<Macro> build(BaseContext& context);
	};
}

#endif
