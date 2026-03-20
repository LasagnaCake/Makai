#include "parser.hpp"
#include "resolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Parser::Precedence Parser::precedenceOf(BaseContext::Axiom const& tok) {
	switch (tok.type) {
		using enum Parser::Precedence;
		case LTS_TT_IDENTIFIER: {
			if (!tok.strict) return AV2_TAPP_NONE;
			auto const id = tok.token;
			if (id == "else") return AV2_TAPP_NULL_DECAY;
			else if (id == "and") return AV2_TAPP_LAND;
			else if (id == "or") return AV2_TAPP_LOR;
			else if (id == "xor") return AV2_TAPP_LXOR;
			else if (id == "pow") return AV2_TAPP_POW_ROOT;
			else if (id == "cross" || id == "fcross")
				return AV2_TAPP_CROSS_FCROSS;
			else if (id == "atan") return AV2_TAPP_ATAN2;
			else if (id == "as") return AV2_TAPP_CAST;
			else if (id == "is") return AV2_TAPP_TYPE_CHECK;
		}
		default: break;
		case LTS_TT_ASSIGN:
		case LTS_TT_ADD_ASSIGN:
		case LTS_TT_MUL_ASSIGN:
		case LTS_TT_SUB_ASSIGN:
		case LTS_TT_DIV_ASSIGN:
		case LTS_TT_MOD_ASSIGN:
		case LTS_TT_BIT_AND_ASSIGN:
		case LTS_TT_BIT_OR_ASSIGN:
		case LTS_TT_BIT_XOR_ASSIGN:
		case LTS_TT_BIT_SHIFT_LEFT_ASSIGN:
		case LTS_TT_BIT_SHIFT_RIGHT_ASSIGN: return AV2_TAPP_ASSIGN;
		case LTS_TT_MINUS:
		case LTS_TT_PLUS: return AV2_TAPP_ADD_SUB;
		case LTS_TT_STAR:
		case LTS_TT_FWD_SLASH:
		case LTS_TT_PERCENT: return AV2_TAPP_MUL_DIV_REM;
		case LTS_TT_BIT_SHIFT_RIGHT:
		case LTS_TT_BIT_SHIFT_LEFT: return AV2_TAPP_BIT_SHIFT;
		case LTS_TT_TILDE: return AV2_TAPP_ORDER;
		case LTS_TT_LESS_THAN:
		case LTS_TT_GREATER_THAN:
		case LTS_TT_COMPARE_GREATER_EQUALS:
		case LTS_TT_COMPARE_LESS_EQUALS: return AV2_TAPP_COMPARE;
		case LTS_TT_COMPARE_EQUALS:
		case LTS_TT_COMPARE_NOT_EQUALS: return AV2_TAPP_EQ_INEQ;
		case LTS_TT_RAISE: return AV2_TAPP_BXOR;
		case LTS_TT_BIT_AND: return AV2_TAPP_BAND;
		case LTS_TT_BIT_OR: return AV2_TAPP_BOR;
		case LTS_TT_LOGIC_AND: return AV2_TAPP_LAND;
		case LTS_TT_LOGIC_OR: return AV2_TAPP_LOR;
		case LTS_TT_INCREMENT:
		case LTS_TT_DECREMENT:
		case LTS_TT_DOT: return AV2_TAPP_PATH;
		case LTS_TT_COMMA: return AV2_TAPP_RHS_DECAY;
		case LTS_TT_QUESTION: return AV2_TAPP_NULL_DECAY;
	}
	return Parser::Precedence::AV2_TAPP_NONE;
}

Parser::Parser(BaseContext& context): context(context) {
	// Direct resolutions
	DEBUGLN("Direct parsers");
	direct(
		LTS_TT_IDENTIFIER,
		LTS_TT_INTEGER,
		LTS_TT_DOUBLE_QUOTE_STRING,
		LTS_TT_SINGLE_QUOTE_STRING,
		LTS_TT_REAL,
		LTS_TT_CHARACTER
	);
	// Basic prefixes
	DEBUGLN("Prefix parsers");
	prefix(
		"sizeof",
		"countof",
		"typeof",
		"sin",
		"cos",
		"tan",
		"asin",
		"acos",
		"atan",
		"sinh",
		"cosh",
		"tanh",
		"log2",
		"log10",
		"ln",
		"sqrt",
		"not",
		"return",
		"ref",
		"move",
		"copy",
		"is",
		"as",
		"drop",
		"local",
		LTS_TT_PLUS,
		LTS_TT_MINUS,
		LTS_TT_LOGIC_NOT,
		LTS_TT_BIT_NOT,
		LTS_TT_RAISE,
		LTS_TT_DOLLAR,
		LTS_TT_SHARP,
		LTS_TT_AMP,
		LTS_TT_STAR
	);
	// Basic infixes
	DEBUGLN("Infix parsers");
	infix(LTS_TT_PLUS, false);
	infix(LTS_TT_MINUS, false);
	infix(LTS_TT_DIVIDE, false);
	infix(LTS_TT_STAR, false);
	infix(LTS_TT_MODULO, false);
	infix(LTS_TT_COMMA, false);
	infix(LTS_TT_AMP, false);
	infix(LTS_TT_PIPE, false);
	infix(LTS_TT_RAISE, false);
	infix(LTS_TT_LOGIC_AND, false);
	infix(LTS_TT_LOGIC_OR, false);
	infix(LTS_TT_LESS_THAN, false);
	infix(LTS_TT_GREATER_THAN, false);
	infix(LTS_TT_COMPARE_LESS_EQUALS, false);
	infix(LTS_TT_COMPARE_GREATER_EQUALS, false);
	infix(LTS_TT_COMPARE_EQUALS, false);
	infix(LTS_TT_COMPARE_NOT_EQUALS, false);
	infix("xor", false);
	infix("atan", false);
	infix("cross", false);
	infix("fcross", false);
	infix("is", false);
	infix("as", false);
	infix("pow", false);
	// Basic postfixes
	DEBUGLN("Postfix parsers");
	postfix(
		LTS_TT_INCREMENT,
		LTS_TT_DECREMENT
	);
	// Advanced prefixes
	DEBUGLN("Advanced prefix parsers");
	add(LTS_TT_OPEN_PAREN, prefixes, new SubExpressionResolver());
	add(LTS_TT_OPEN_CURLY, prefixes, new BlockResolver());
	add(LTS_TT_OPEN_BRACKET, prefixes, new ArrayResolver());
	add("asm", prefixes, new InlineMinimaResolver());
	add("if", prefixes, new BranchResolver());
	add("unless", prefixes, new BranchResolver());
	add("switch", prefixes, new BranchResolver());
	add("repeat", prefixes, new LoopResolver());
	add("do", prefixes, new LoopResolver());
	add("while", prefixes, new LoopResolver());
	add("for", prefixes, new LoopResolver());
	add("module", prefixes, new ModuleDeclResolver());
	add("global", prefixes, new SpecialVarDeclResolver());
	add("out", prefixes, new SpecialVarDeclResolver());
	add("func", prefixes, new FunctionDeclResolver());
	add("extend", prefixes, new ExtensionResolver());
	add("import", prefixes, new ImportResolver());
	add("trait", prefixes, new TraitDeclResolver());
	add("with", prefixes, new TemplateDeclResolver());
	add("struct", prefixes, new StructureDeclResolver());
	add("prop", prefixes, new PropertyDeclResolver());
	add("prefix", prefixes, new DynamicOperatorDeclResolver());
	add("postfix", prefixes, new DynamicOperatorDeclResolver());
	add("infix", prefixes, new DynamicOperatorDeclResolver());
	// Advanced infixes
	DEBUGLN("Advanced infix parsers");
	add("if", infixes, new InlineIfElseResolver());
	add("unless", infixes, new InlineIfElseResolver());
	add(LTS_TT_DOT, infixes, new PathResolver());
	add(LTS_TT_COLON, infixes, new VariableDeclResolver());
	add(LTS_TT_OPEN_PAREN, infixes, new FunctionCallResolver());
	add(LTS_TT_EXCLAMATION, infixes, new FunctionCallResolver());
	add(LTS_TT_OPEN_BRACKET, infixes, new ArrayResolver());
	add(LTS_TT_EQUALS, infixes, new AssignmentResolver());
	add(LTS_TT_ADD_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_SUB_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_MUL_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_DIV_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_MOD_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_BIT_AND_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_BIT_OR_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_BIT_XOR_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_BIT_SHIFT_LEFT_ASSIGN, infixes, new AssignmentResolver());
	add(LTS_TT_BIT_SHIFT_RIGHT_ASSIGN, infixes, new AssignmentResolver());
	DEBUGLN("Done!");
}

AResolver::AResolver(Parser::Precedence const precedence, bool const rightToLeft):
	precedence(Cast::as<Parser::Precedence>(enumcast(precedence) - rightToLeft)),
	rightToLeft(rightToLeft) {
}

Node::Instance Parser::nextExpression(Parser::Precedence precedence) {
	auto tok = context.next().token();
	if (!prefixes.contains(tok))
		context.error("Invalid expression!");
	Node::Instance lhs = prefixes[tok]->resolve(*this, null, tok);
	if (!infixes.contains(tok))
		return lhs;
	while (precedence < currentPrecedence()) {
		tok = context.next().token();
		lhs = infixes[tok]->resolve(*this, lhs, tok);
	}
	return lhs;
}

Node::Instance Parser::parse() {
	Node::Instance root = Node::Instance::create();
	while (!context.empty())
		root->children.pushBack(nextExpression());
	return root;
}

Parser::Precedence Parser::currentPrecedence() {
	auto const tok = context.peek();
	if (!infixes.contains(tok))
		return Parser::Precedence::AV2_TAPP_NONE;
	return infixes[tok]->precedence;
}

void Parser::direct(BaseContext::Axiom::Type const op) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	add(ax, prefixes, new DirectResolver());
}

void Parser::prefix(BaseContext::Axiom::Type const op) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	add(ax, prefixes, new PrefixResolver());
}

void Parser::infix(BaseContext::Axiom::Type const op, bool const rightToLeft) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	add(ax, infixes, new InfixResolver(precedenceOf(ax), rightToLeft));
}

void Parser::postfix(BaseContext::Axiom::Type const op) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	add(ax, prefixes, new PostfixResolver());
}

void Parser::prefix(String const& op) {
	BaseContext::Axiom ax;
	ax.type = LTS_TT_IDENTIFIER;
	ax.strict = true;
	ax.token = op;
	add(ax, prefixes, new PrefixResolver());
}

void Parser::infix(String const& op, bool const rightToLeft) {
	BaseContext::Axiom ax;
	ax.type = LTS_TT_IDENTIFIER;
	ax.strict = true;
	ax.token = op;
	add(ax, infixes, new InfixResolver(precedenceOf(ax), rightToLeft));
}

void Parser::postfix(String const& op) {
	BaseContext::Axiom ax;
	ax.type = LTS_TT_IDENTIFIER;
	ax.strict = true;
	ax.token = op;
	add(ax, infixes, new PostfixResolver());
}

void Parser::add(BaseContext::Axiom const op, OperatorBank& bank, Instance<AResolver> const& resolver) {
	bank[op] = resolver;
}

void Parser::add(BaseContext::Axiom::Type const op, OperatorBank& bank, Instance<AResolver> const& resolver) {
	BaseContext::Axiom ax;
	ax.type = op;
	ax.strict = false;
	add(ax, bank, resolver);
}

void Parser::add(Makai::String const op, OperatorBank& bank, Instance<AResolver> const& resolver) {
	BaseContext::Axiom ax;
	ax.type = LTS_TT_IDENTIFIER;
	ax.strict = true;
	ax.value = op;
	ax.token = op;
	add(ax, bank, resolver);
}

Parser::~Parser() {
}
