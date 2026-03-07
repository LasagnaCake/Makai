#include "resolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

IValueResolver::Result BinaryResolver::resolve(BaseContext& context) {
	ExpressionResolver lhs, rhs;
	Result out;
	auto const lv = lhs.resolve(context);
	auto const op = context.next().token();
	auto const rv = rhs.resolve(context.next());
	if ((lv.source & Core::DataLocation::AV2_DL_STACK) != Core::DataLocation::AV2_DL_STACK)
		out.action += "push " + lv.value;
	if ((rv.source & Core::DataLocation::AV2_DL_STACK) != Core::DataLocation::AV2_DL_STACK)
		out.action += "push " + rv.value;
	out.action += lv.action + rv.action;
	switch (op.type) {
		case LTS_TT_IDENTIFIER: {

		};
		default: context.error("Unsupported binary expression!");
	}
	return out;
}
