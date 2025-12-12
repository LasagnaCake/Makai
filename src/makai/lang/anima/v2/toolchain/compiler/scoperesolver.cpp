#include "scoperesolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler;

usize ScopeResolver::resolve(usize const start, ParseTree::Node& root) {
	Context::Scope scope(context);
	usize current = start;
	auto node = context.tree.create(root).node();
	while (context.tokens[current].type != TokenStream::Token::Type{'}'}) {
		auto& token = context.tokens[current];
		if (token.type == TokenStream::Token::Type{'{'}) {
			ScopeResolver resolver(context);
			current += resolver.resolve(current+1, node);
		} else if (token.type == TokenStream::Token::Type::LTS_TT_IDENTIFIER) {
			auto const name = token.value.get<String>();
			if (
				name == "global"
			||	name == "local"
			) {
				++current;
				auto& varName = context.tokens[current];
				if (varName.type == TokenStream::Token::Type::LTS_TT_IDENTIFIER) {
					scope.addName(varName.value.get<String>(), Context::NameType::AV2_TC_CNT_VARIABLE);
					VariableResolver var(context);
					var.resolve(current, node);
				}
			}
		}
		++current;
	}
	return current+1;
}