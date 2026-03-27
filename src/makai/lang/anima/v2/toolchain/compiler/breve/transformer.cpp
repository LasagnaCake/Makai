#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using namespace Transformer;

using Type = BaseContext::Tokenizer::Token::Type;

using enum BaseContext::Tokenizer::Token::Type;

Namespace::Instance ATransformer::declare(Context& context, UTF8StringList const& path) {
	if (auto const ns = resolve(context, path))
		return ns;
	stack = context.push(path);
	return context.scopeStack.back();
}

Namespace::Instance ATransformer::fetch(Context& context, UTF8StringList const& path, Node::Instance const& base) {
	if (auto const ns = resolve(context, path))
		return ns;
	context.error("Symbol does not exist!", base);
}

Makai::UTF8StringList ATransformer::pathOf(Node::Instance const& node) {
	if (!node || node->content != Node::Content::AV2_TANC_NAME)
		return {};
	if (node->content == Node::Content::AV2_TANC_NAME)
		return Makai::UTF8StringList::from(node->value.getString());
	else if (!node->isNameOrPath())
		Context::error("This is not a valid path!", node);
	Makai::UTF8StringList path;
	path.pushBack(node->lhs->value.getString());
	path.appendBack(pathOf(node->rhs));
	return path;
}

Namespace::Instance VariableDecl::transform(Context& context, Node::Instance const& node) {

}


Namespace::Instance StructureDecl::transform(Context& context, Node::Instance const& node) {

}

Namespace::Instance FunctionDecl::transform(Context& context, Node::Instance const& node) {
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
		ov->result = fetch(context, path, node->lhs)->type;
	VariableDecl vd;
	for (auto const& arg: proto->children)
		ov->arguments.pushBack(vd.transform(context, arg)->variable);
	if (fn.overload(ov->arguments))
		context.error("Redeclaration of function overload!", node);
	return scope;
}
