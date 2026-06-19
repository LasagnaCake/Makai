#include "node.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Makai::Data::Value Node::serialize() const {
	Makai::Data::Value out = out.object();
	out["id"]		= id();
	out["content"]	= asString(content);
	if (value)
		out["value"]	= value;
	if (leftSide)
		out["left"]		= leftSide->serialize();
	if (middle)
		out["middle"]	= middle->serialize();
	if (rightSide)
		out["right"]	= rightSide->serialize();
	if (children.size()) {
		Makai::Data::Value::ArrayType ch;
		ch.reserve(children.size());
		for (auto& child: children)
			if (child)
				ch.pushBack(child->serialize());
		out["children"] = ch;
	}
	if (interject.size()) {
		String ij;
		for (auto& tok: interject)
			ij += tok.text + " ";
		ij.popBack();
		out["asm"] = ij;
	}
	if (source) out["source"] = *source;
	out["base"] = base.text.toString();
	return out;
}

Makai::String Node::asString(Content const content) {
	switch (content) {
		case Content::AV2_TANC_EMPTY:				return "none";
		case Content::AV2_TANC_ARRAY:				return "array";
		case Content::AV2_TANC_ASSIGNMENT:			return "assign";
		case Content::AV2_TANC_PATH:				return "path";
		case Content::AV2_TANC_BLOCK:				return "block";
		case Content::AV2_TANC_DECLARATION:			return "decl";
		case Content::AV2_TANC_FN_CALL:				return "call";
		case Content::AV2_TANC_FN_PROTOTYPE:		return "proto";
		case Content::AV2_TANC_BRANCH:				return "branch";
		case Content::AV2_TANC_LOOP:				return "loop";
		case Content::AV2_TANC_VALUE:				return "value";
		case Content::AV2_TANC_NAME:				return "name";
		case Content::AV2_TANC_INLINE_MINIMA:		return "asm";
		case Content::AV2_TANC_INLINE_IF_ELSE:		return "if_else";
		case Content::AV2_TANC_DEFINITION:			return "def";
		case Content::AV2_TANC_IMPORT:				return "import";
		case Content::AV2_TANC_INFIX_OP:			return "infix";
		case Content::AV2_TANC_PREFIX_OP:			return "prefix";
		case Content::AV2_TANC_POSTFIX_OP:			return "postfix";
		case Content::AV2_TANC_SUBSCRIPT:			return "subscript";
		case Content::AV2_TANC_TEMPLATE:			return "template";
		case Content::AV2_TANC_TYPE_EXTENSION:		return "extend";
		case Content::AV2_TANC_ATTRIBUTE:			return "attrib";
		case Content::AV2_TANC_ALIAS:				return "alias";
		case Content::AV2_TANC_EMPTY_DECAY:			return "null_decay";
		case Content::AV2_TANC_NEW:					return "new";
		case Content::AV2_TANC_DROP:				return "drop";
		case Content::AV2_TANC_CUSTOM_PREFIX_OP:	return "ex_prefix";
		case Content::AV2_TANC_CUSTOM_INFIX_OP:		return "ex_infix";
		case Content::AV2_TANC_CUSTOM_POSTFIX_OP:	return "ex_postfix";
		case Content::AV2_TANC_UNSCOPING:			return "unscope";
		case Content::AV2_TANC_RANGE:				return "range";
		case Content::AV2_TANC_EXPANSION:			return "expand";
		case Content::AV2_TANC_ITERATION:			return "iter";
		case Content::AV2_TANC_NULLABLE_DECL:		return "null_decl";
		case Content::AV2_TANC_TRAIT:				return "trait";
		case Content::AV2_TANC_PROPERTY_GETTER:		return "get";
		case Content::AV2_TANC_PROPERTY_SETTER:		return "set";
		case Content::AV2_TANC_PROPERTY:			return "prop";
	}
	return "???";
}
