#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using namespace Transformer;

using Type = BaseContext::Tokenizer::Token::Type;

using enum Type;

Namespace::Instance ITransformer::declare(Context& context, Node::Instance const& node) {
	auto const path = pathOf(node);
	if (auto const ns = resolve(context, path))
		return ns;
	stack = context.push(path);
	return context.scopeStack.back();
}

Makai::UTF8StringList ITransformer::pathOf(Node::Instance const& node) {
	if (node->content == Node::Content::AV2_TANC_NAME)
		return Makai::UTF8StringList::from(node->value.getString());
	Makai::UTF8StringList path;
	path.pushBack(node->lhs->value.getString());
	path.appendBack(pathOf(node->rhs));
	return path;
}

ITransformer::Instance FunctionDecl::transform(Context& context, Node::Instance const& node) {
	auto const path = pathOf(node->lhs);
	auto const scope = declare(context, path);
	if (!scope->function) {
		scope->function = scope->function.create();
		scope->function->name = path.back();
	}
	auto& fn = *scope->function;
	auto const proto = node->rhs;
	Function::OverloadRef ov = ov.create();
	if (proto->lhs)
		ov->result = resolve(context, path)->type;
}
