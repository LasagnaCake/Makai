#include "transform.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using namespace Transformer;

using Type = BaseContext::Tokenizer::Token::Type;

using enum Type;

Namespace::Instance ITransformer::declare(Context& context, Node::Instance const& node) {
	auto const path = pathOf(node);
	if (auto const ns = resolve(context, path))
		return ns;
	return context.push(path);
}

Makai::UTF8StringList ITransformer::pathOf(Node::Instance const& node) {
	if (node->content == Node::Content::AV2_TANC_NAME)
		return Makai::UTF8StringList::from(node->value.getString());
	Makai::UTF8StringList path;
	path.pushBack(node->lhs->value.getString());
	path.appendBack(pathOf(node->rhs));
	return path;
}
