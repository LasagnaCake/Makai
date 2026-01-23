#include "semibreve.hpp"
#include "breve.hpp"
#include "context.hpp"
#include "core.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using namespace Makai::Error;

namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = AAssembler::TokenStream::Token::Type;
using enum Type;
using Value = Makai::Data::Value;

using Solution = Context::Scope::Value;

using NamespaceMember	= Makai::KeyValuePair<Makai::String, Makai::Instance<Context::Scope::Member>>;

#define SEMIBREVE_ASSEMBLE_FN(NAME) static void do##NAME (Context& context)
#define SEMIBREVE_TYPED_ASSEMBLE_FN(NAME) static Solution do##NAME (Context& context)
#define SEMIBREVE_SYMBOL_ASSEMBLE_FN(NAME) static Solution do##NAME (Context& context, Makai::Instance<Context::Scope::Member> const& sym)

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

SEMIBREVE_ASSEMBLE_FN(ModuleImport);
SEMIBREVE_ASSEMBLE_FN(Namespace);
SEMIBREVE_ASSEMBLE_FN(Scope);
SEMIBREVE_ASSEMBLE_FN(Expression);
SEMIBREVE_ASSEMBLE_FN(Return);
SEMIBREVE_ASSEMBLE_FN(Conditional);
SEMIBREVE_ASSEMBLE_FN(ForLoop);
SEMIBREVE_ASSEMBLE_FN(WhileLoop);
SEMIBREVE_ASSEMBLE_FN(RepeatLoop);
SEMIBREVE_ASSEMBLE_FN(DoLoop);
SEMIBREVE_ASSEMBLE_FN(Main);
SEMIBREVE_ASSEMBLE_FN(Terminate);
SEMIBREVE_ASSEMBLE_FN(Error);
SEMIBREVE_ASSEMBLE_FN(External);

SEMIBREVE_TYPED_ASSEMBLE_FN(ReservedValueResolution);
SEMIBREVE_TYPED_ASSEMBLE_FN(BinaryOperation);
SEMIBREVE_TYPED_ASSEMBLE_FN(UnaryOperation);
SEMIBREVE_TYPED_ASSEMBLE_FN(InternalPrint);
SEMIBREVE_TYPED_ASSEMBLE_FN(Internal);

SEMIBREVE_SYMBOL_ASSEMBLE_FN(MemberCall);
SEMIBREVE_SYMBOL_ASSEMBLE_FN(VariableAction);

static Solution doFunctionCall(Context& context, Makai::Instance<Context::Scope::Member> const& symbol, Makai::String const& self = "");

static void doMacroExpansion(Context& context, Makai::Instance<Context::Scope::Member> const& symbol, Makai::String const& self = "");

static Solution doValueResolution(Context& context, bool idCanBeValue = false);

static Makai::String doDefaultValue(Context& context, Makai::String const& var, Makai::String const& uname) {
	context.fetchNext();
	auto const dvloc = "__" + context.scopePath() + "_" + var + "_set_default" + uname;
	context.getSymbolByName(var).value["default_setter"] = dvloc;
	auto dv = dvloc + ":\n";
	auto const vr = doValueResolution(context);
	return dvloc + ":\npush" + vr.resolve();
}

constexpr auto const DVK_ANY = Context::DVK_ANY;

static Makai::Instance<Context::Scope::Member> getType(Context& context) {
	using enum Makai::Data::Value::Kind;
	auto const ret = context.currentToken();
	switch (ret.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = ret.value.get<Makai::String>();
			if (!context.hasType(id))
				return context.getBasicType("void");
			return context.getSymbolRefByName(id);
		}
		default: context.error<InvalidValue>("Invalid/Unsupported type!");
	}
	context.error<InvalidValue>("Invalid/Unsupported type!");
}

constexpr Makai::String toTypeName(Value::Kind t) {
	if (t < DVK_ANY) {
		t = Makai::Cast::as<Value::Kind>(Makai::Math::abs(Makai::enumcast(t)) - 2);
	}
	switch (t) {
		case DVK_ANY:						return "any";
		case Value::Kind::DVK_UNDEFINED:	return "v";
		case Value::Kind::DVK_NULL:			return "null";
		case Value::Kind::DVK_BOOLEAN:		return "b";
		case Value::Kind::DVK_UNSIGNED:		return "u";
		case Value::Kind::DVK_SIGNED:		return "i";
		case Value::Kind::DVK_REAL:			return "r";
		case Value::Kind::DVK_ARRAY:		return "a";
		case Value::Kind::DVK_OBJECT:		return "o";
		case Value::Kind::DVK_BYTES:		return "bin";
		case Value::Kind::DVK_VECTOR:		return "vec";
	}
	return "v";
}

constexpr Makai::String toTypeName(Makai::Instance<Context::Scope::Member> t) {
	return t->name;
}

struct Prototype {
	Makai::Instance<Context::Scope::Member>		returnType;
	Makai::String								name;
	Makai::String								fullName;
	Makai::String								resolution;
	Makai::Instance<Context::Scope::Member>		function;
};

struct Template {
	usize index;
};

using TemplateMap = Makai::Dictionary<Template>;

static TemplateMap doTemplates(Context& context) {
	TemplateMap tmap;
	context.fetchNext();
	while (!context.hasToken(Type{'>'})) {
		if (context.hasToken(Type{'>'})) break;
		if (!context.hasToken(LTS_TT_IDENTIFIER))
			context.error("Expected identifier here!");
		tmap[context.getValue<Makai::String>()] = {.index = tmap.size()};
		context.fetchNext();
		if (context.hasToken(Type{'>'})) break;
	}
	context.fetchNext();
	return tmap;
}

static Prototype doFunctionPrototype(Context& context, bool const isExtern = false, Makai::Handle<Context::Scope::Namespace> const ns = nullptr) {
	auto const fname = context.currentToken();
	if (fname.type != Type::LTS_TT_IDENTIFIER)
		context.error<InvalidValue>("Function name must be an identifier!");
	auto const fid = fname.value.get<Makai::String>();
	if (context.isReservedKeyword(fid))
		context.error<InvalidValue>("Function name cannot be a reserved keyword!");
	auto id = fid;
	auto args = Makai::Data::Value::array();
	context.fetchNext();
	TemplateMap templates;
	if (context.currentToken().type == Type{'<'})
		templates = doTemplates(context);
	else if (context.currentToken().type != Type{'('})
		context.error<NonexistentValue>("Expected '(' here!");
	auto retType = context.getBasicType("any");
	id += "_";
	Makai::String gpre = "";
	auto const signature = context.uniqueName();
	Makai::List<Makai::KeyValuePair<Makai::String, Value>> optionals;
	bool inOptionalRegion = false;
	while (context.nextToken() && context.currentToken().type != Type{')'}) {
		bool isOptional = false;
		auto const argn = context.currentToken();
		if (argn.type != Type::LTS_TT_IDENTIFIER)
			context.error<InvalidValue>("Argument name must be an identifier!");
		auto const argID = argn.value.get<Makai::String>();
		if (context.isReservedKeyword(argID))
			context.error<InvalidValue>("Argument name cannot be a reserved keyword!");
		if (context.currentScope().contains(argID))
			context.error<InvalidValue>("Argument with this name already exists!");
		auto const var = context.currentScope().addVariable(argID);
		context.fetchNext();
		if (context.currentToken().type != Type{':'})
			context.error<InvalidValue>("Expected ':' here!");
		context.fetchNext();
		Makai::Instance<Context::Scope::Member> argt;
		if (templates.contains(context.getValue<Makai::String>())) {
			// TODO: Template resolution
		} else argt = getType(context);
		DEBUGLN("Type: ", argt->name);
		if (context.isUndefined(argt))
			context.error<InvalidValue>("Invalid argument type!");
		context.fetchNext();
		var->base = argt;
		if (context.currentToken().type == Type{'='}) {
			isOptional = true;
			inOptionalRegion = true;
			gpre.appendBack(doDefaultValue(context, argID, signature));
			optionals.pushBack({argID});
			optionals.back().value["name"] = argID;
			optionals.back().value["type"] = argt->name;
		} else {
			id += "_" + argt->name.toString();
			auto& arg = args[args.size()];
			arg["name"]			= argID;
			var->value["type"]	= argt->name; 
			arg["type"]			= argt->name;
		}
		if (inOptionalRegion && !isOptional)
			context.error<NonexistentValue>("Missing value for optional argument!");
		if (context.currentToken().type == Type{')'})
			break;
		if (context.currentToken().type != Type{','})
			context.error<InvalidValue>("Expected ',' here!");
	}
	if (context.currentToken().type != Type{')'})
		context.error<InvalidValue>("Expected ')' here!");
	context.fetchNext();
	if (context.currentToken().type == Type{':'}) {
		context.fetchNext();
		retType = getType(context);
		context.fetchNext();
	}
	context.currentScope().result	= retType;
	context.currentScope().label	= fid;
	auto const baseName =
		context.scopePath()
	+	signature
	+	"_" + id
	;
	auto resolutionName = templates.size() ? "template:" + id : id;
	auto fullName = baseName;
	if (optionals.size()) {
		context.writeGlobalPreamble(gpre, "call", fullName, "()");
		context.writeGlobalPreamble("end");
	}
	for (auto& opt: optionals)
		fullName += "_" + opt.value["type"].get<Makai::String>();
	Prototype proto = {retType, fid, fullName};
	auto subName = baseName;
	for (auto& opt: Makai::Range::reverse(optionals)) {
		fullName = fullName.sliced(0, -(opt.value["type"].get<Makai::String>().size() + 2));
		opt.value["declname"] = fullName;
	}
	Makai::Instance<Context::Scope::Member> mem;
	if (ns && ns->members.contains(fid)) {
		if (ns->members[fid]->type != Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION)
			context.error("Non-function symbol with this name was already declared!");
		mem = context.currentScope().ns->members[fid];
	} else {
		mem = new Context::Scope::Member{
			.type = Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION,
			.name = fid
		};
	}
	proto.function = mem;
	proto.resolution = resolutionName;
	auto& overloads	= mem->value["overloads"];
	if (overloads.contains(resolutionName) && overloads[resolutionName]["decl"])
		context.error<InvalidValue>("Function with similar signature already exists!");
	auto& overload	= overloads[resolutionName];
	for (auto& opt: optionals) {
		fullName += "_" + opt.value["type"].get<Makai::String>();
	}
	overload["args"]		= args;
	overload["decl"]		= true;
	overload["full_name"]	= fullName;
	overload["return"]		= retType->name;
	overload["extern"]		= optionals.empty() ? isExtern : false;
	usize i = 0;
	for (auto& opt: optionals) {
		resolutionName += "_" + opt.key;
		if (overloads.contains(resolutionName) && overloads[resolutionName]["decl"])
			context.error<InvalidValue>("Function with similar signature already exists!");
		auto& overload	= overloads[resolutionName];
		args[args.size()]		= opt.value;
		overload["args"]		= args;
		overload["decl"]		= true;
		overload["full_name"]	= opt.value["declname"];
		overload["return"]		= retType->name;
		overload["extern"]		= ++i < optionals.size() ? false : isExtern;
	}
	context.functions.pushBack(mem);
	return proto;
}

SEMIBREVE_ASSEMBLE_FN(Function) {
	auto ns = context.currentNamespaceRef();
	if (context.inFunction()) ns = context.currentScope().ns;
	context.fetchNext();
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	auto const proto = doFunctionPrototype(context, false, ns);
	context.currentScope().ns->members[proto.name] = proto.function;
	context.writeLine(proto.fullName, ":");
	if (context.hasToken(Type{'{'})) {
		doScope(context);
	} else if (context.hasToken(LTS_TT_BIG_ARROW)) {
		auto const v = doValueResolution(context);
		if (!proto.returnType || proto.returnType == context.getBasicType("void"))
			context.writeLine("ret void");
		else if (proto.returnType != v.type && !(context.isCastable(proto.returnType) && context.isCastable(v.type)))
			context.error("Return types do not match!");
		if (proto.returnType != v.type) {
			context.writeLine("cast", v.resolve(), "as", toTypeName(proto.returnType), "-> .");
			context.writeLine("ret .");
		} else context.writeLine("ret", v.resolve());
	} else if (context.hasToken(Type{';'})) {
		proto.function->value["overloads"][proto.resolution] = false;
	} else context.error("Expected ';', '{' or '=>' here!");
	context.writeLine("end");
	context.endScope();
	if (!ns->members.contains(proto.name))
		ns->members[proto.name] = proto.function;
	else if (ns->members[proto.name]->type != Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION)
		context.error<InvalidValue>("Symbol with this name already exists!");
}

SEMIBREVE_ASSEMBLE_FN(ExternalFunction) {
	auto ns = context.currentNamespaceRef();
	if (context.inFunction()) ns = context.currentScope().ns;
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	context.fetchNext();
	auto const proto = doFunctionPrototype(context, true, ns);
	context.writeLine(proto.fullName, ":");
	Makai::String args;
	usize argc = 0;
	for (auto const& [name, overload]: context.currentScope().ns->members[proto.name]->value["overloads"].items()) {
		if (overload["extern"]) {
			argc = overload["args"].size();
			break;
		}
	}
	if (argc) for (auto const i: Makai::range(argc))
		args += Makai::toString(i, "= &[-", argc - (i + 1), "] ");
	auto const fname = toString("\"", context.namespacePath("."), ".", proto.name, "\"");
	if (proto.returnType->value["basic"])
		context.writeLine("call out", fname, toTypeName(proto.returnType), "(", args, ")");
	else context.error("External functions can only return basic types!");
	if (context.currentToken().type != Type{';'})
		context.error<InvalidValue>("Expected ';' here!");
	context.endScope();
}

SEMIBREVE_ASSEMBLE_FN(SharedFunction) {
	context.fetchNext().expectToken(Type{'['});
	auto const file		= context.fetchNext().fetchToken(LTS_TT_SINGLE_QUOTE_STRING).getString();
	context.fetchNext().expectToken(Type{':'});
	auto const function	= context.fetchNext().fetchToken(LTS_TT_SINGLE_QUOTE_STRING).getString();
	context.fetchNext().expectToken(Type{']'});
	context.program.ani.shared[file][function] = true;
	doExternalFunction(context);
}

SEMIBREVE_ASSEMBLE_FN(Scope) {
	while (context.nextToken()) {
		auto const current = context.currentToken();
		if (current.type == Type{'}'}) break; 
		else doExpression(context);
	}
	if (context.currentScope().varc)
		context.writeLine("clear ", context.currentScope().varc);
}

SEMIBREVE_ASSEMBLE_FN(ExternalValue) {
	auto const id = context.currentToken().value.get<Makai::String>();
	if (context.currentScope().contains(id))
		context.error<FailedAction>("Symbol with this name already exists in this scope!");
	// TODO: The rest of the owl
}

SEMIBREVE_ASSEMBLE_FN(External) {
	context.fetchNext();
	if (context.currentToken().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Expected keyword here!");
	auto const id = context.currentToken().value.get<Makai::String>();
	if (id == "function" || id == "func" || id == "fn") doExternalFunction(context);
	if (id == "shared") doSharedFunction(context);
	else if (!context.isReservedKeyword(id))
		doExternalValue(context);
	else context.error<NonexistentValue>("Invalid keyword!");
}

SEMIBREVE_TYPED_ASSEMBLE_FN(InternalPrint) {
	context.fetchNext();
	auto const v = doValueResolution(context);
	context.writeLine("push", v.resolve());
	context.writeLine("call in print");
	return {context.getBasicType("void"), context.resolveTo(".")};
}

SEMIBREVE_TYPED_ASSEMBLE_FN(InternalStringify) {
	context.fetchNext();
	auto const v = doValueResolution(context);
	context.writeLine("push", v.resolve());
	context.writeLine("call in stringify");
	return {context.getBasicType("void"), context.resolveTo(".")};
}

SEMIBREVE_TYPED_ASSEMBLE_FN(Internal) {
	context.fetchNext();
	if (context.currentToken().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Expected keyword here!");
	auto const id = context.currentToken().value.get<Makai::String>();
	if (id == "print") return doInternalPrint(context);
	if (id == "stringify") return doInternalStringify(context);
	else context.error<NonexistentValue>("Invalid keyword!");
}

NamespaceMember resolveNamespaceMember(Context& context, Context::Scope::Namespace& ns) {
	DEBUGLN("Namespace:", ns.name);
	context.fetchNext();
	if (context.currentToken().type != Type{'.'})
		context.error<NonexistentValue>("Expected '.' here!");
	context.fetchNext();
	if (context.currentToken().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.currentToken().value.get<Makai::String>();
	DEBUGLN("Looking for: ", id);
	if (ns.members.contains(id))
		return {id, ns.members[id]};
	else if (ns.children.contains(id))
		return resolveNamespaceMember(context, *ns.children[id]);
	else context.error<NonexistentValue>("Symbol does not exist!");
}

NamespaceMember resolveNamespaceMember(Context& context) {
	if (context.currentToken().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.currentToken().value.get<Makai::String>();
	return resolveNamespaceMember(context, context.getNamespaceByName(id));
}

Context::Scope::Namespace& resolveNamespace(Context& context, Context::Scope::Namespace& ns) {
	context.fetchNext();
	if (context.currentToken().type != Type{'.'})
		return ns;
	context.fetchNext();
	if (context.currentToken().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.currentToken().value.get<Makai::String>();
	if (ns.members.contains(id))
		context.error<NonexistentValue>("Not a namespace!");
	else if (ns.children.contains(id))
		return resolveNamespace(context, *ns.children[id]);
	else context.error<NonexistentValue>("Namespace does not exist!");
}

Context::Scope::Namespace& resolveNamespace(Context& context) {
	if (context.currentToken().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.currentToken().value.get<Makai::String>();
	return resolveNamespace(context, context.getNamespaceByName(id));
}

// TODO: Apply this solution to the rest of the assembler
static Solution resolveSymbol(Context& context, Makai::String const& id, Makai::Instance<Context::Scope::Member> const& sym) {
	if (sym->type == Context::Scope::Member::Type::AV2_TA_SMT_MACRO) {
		doMacroExpansion(context, sym);
		return doValueResolution(context);
	} else if (sym->type == Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION) {
		return doFunctionCall(context, sym);
	} else if (sym->type == Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE) {
		sym->value["use"] = true;
		if (!sym->base)
			context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing variable type!");
		auto const type = sym->base;
		DEBUGLN("Value type: ", type->name);
		return {type, context.varAccessor(sym)};
	} else context.error<InvalidValue>("Invalid symbol type for operation");
}

Makai::Instance<Context::Scope::Member> resolveSymbolPath(Context& context) {
	if (!context.hasToken(LTS_TT_IDENTIFIER))
		context.error("Type name must be an identifier!");
	auto const id = context.currentToken().value.get<Makai::String>();
	if (context.hasSymbol(id))
		return context.getSymbolRefByName(id);
	else if (context.hasNamespace(id))
		return resolveNamespaceMember(context).value;
	else context.error("Symbol with this name does not exist!");
}

static Solution doValueResolution(Context& context, bool idCanBeValue) {
	auto const current = context.currentToken();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			auto result = doReservedValueResolution(context);
			if (result.type != context.getBasicType("void")) return result;
			else if (context.hasSymbol(id)) 
				return resolveSymbol(context, id, context.getSymbolRefByName(id));
			else if (context.hasNamespace(id)) {
				auto const sym = resolveNamespaceMember(context);
				return resolveSymbol(context, sym.key, sym.value);
			} else if (id == "sizeof") {
				context.fetchNext();
				auto result = doValueResolution(context);
				context.writeLine("push", result.resolve());
				context.writeLine("call in sizeof");
				context.writeLine("pop .");
				return {context.getBasicType("uint"), context.resolveTo(".")};
			} else if (idCanBeValue) return {context.getBasicType("string"), context.resolveTo("\"" + id + "\"")};
			else context.error<InvalidValue>("Identifier does not match any reserved value or member name!");
		} break;
		case Type{'('}: {
			return doBinaryOperation(context);
		} break;
		case Type{'-'}:
		case Type{'+'}:
		case LTS_TT_DECREMENT:
		case LTS_TT_INCREMENT: {
			return doUnaryOperation(context);
		} break;
		case LTS_TT_SINGLE_QUOTE_STRING: 
		case LTS_TT_DOUBLE_QUOTE_STRING:	return {context.getBasicType("string"),	context.resolveTo(current.value.toString())								};
		case LTS_TT_CHARACTER: 				return {context.getBasicType("string"),	context.resolveTo(Makai::toString("'", current.value.get<char>(), "'"))	};
		case LTS_TT_INTEGER:				return {context.getBasicType("uint"),	context.resolveTo(current.value.toString())								};
		case LTS_TT_REAL:					return {context.getBasicType("real"),	context.resolveTo(current.value.toString())								};
		default: context.error<InvalidValue>("Invalid expression!");
	}
}

constexpr Value::Kind stronger(Value::Kind const a, Value::Kind const b) {
	if (a == b) return a;
	return a > b ? a : b;
}

constexpr Makai::Instance<Context::Scope::Member> stronger(Context& context, Makai::Instance<Context::Scope::Member> const& a, Makai::Instance<Context::Scope::Member> const& b) {
	if (a == b) return a;
	if (!a || !b) context.error("Value types mysteriously disappeared!");
	if (!a->value["basic"]) return a;
	if (!b->value["basic"]) return b;
	auto const res = stronger(Makai::Cast::as<Value::Kind>(a->value["type"]), Makai::Cast::as<Value::Kind>(b->value["type"]));
	return res == Makai::Cast::as<Value::Kind>(a->value["type"]) ? a : b;
}

static auto handleTernary(Context& context, Solution const& cond, Solution const& ifTrue, Solution const& ifFalse) {
	auto const result = stronger(context, ifTrue.type, ifFalse.type);
	if (context.isNumber(result) && ifTrue.type != ifFalse.type)
		context.error<InvalidValue>("Types must match, or be similar!");
	if (context.isUndefined(cond.type))
		context.error<InvalidValue>("Invalid condition type!");
	if (!context.isVerifiable(cond.type))
		context.error<InvalidValue>("Condition must be a verifiable type!");
	auto const trueJump		= context.scopePath() + "_ternary_true"		+ context.uniqueName();
	auto const falseJump	= context.scopePath() + "_ternary_false"	+ context.uniqueName();
	auto const endJump		= context.scopePath() + "_ternary_end"		+ context.uniqueName();
	context.writeLine("jump if true", cond.resolve(), trueJump);
	context.writeLine("jump if false", cond.resolve(), falseJump);
	context.writeLine(trueJump + ":");
	context.writeLine("copy", ifTrue.resolve(), "-> .");
	context.writeLine("jump", endJump);
	context.writeLine(falseJump + ":");
	context.writeLine("copy", ifFalse.resolve(), "-> .");
	context.writeLine("jump", endJump);
	context.writeLine(endJump + ":");
	return result;
}

static auto handleNullCoalescence(Context& context, Solution const& value, Solution const& elseValue) {
	auto const result = stronger(context, value.type, elseValue.type);
	if (context.isNumber(result) && value.type != elseValue.type)
		context.error<InvalidValue>("Types must match, or be similar!");
	auto const falseJump	= context.scopePath() + "_nc_false"	+ context.uniqueName();
	auto const endJump		= context.scopePath() + "_nc_end"	+ context.uniqueName();
	context.writeLine("jump if false", elseValue.resolve(), falseJump);
	context.writeLine("copy", value.resolve(), "-> .");
	context.writeLine("jump", endJump);
	context.writeLine(falseJump + ":");
	context.writeLine("copy", elseValue.resolve(), "-> .");
	context.writeLine("jump", endJump);
	context.writeLine(endJump + ":");
	return result;
}

SEMIBREVE_TYPED_ASSEMBLE_FN(BinaryOperation) {
	context.fetchNext();
	auto lhs = doValueResolution(context);
	usize stackUsage = 0;
	if (lhs.resolve() == ".") {
		context.writeLine("push .");
		lhs.resolver = context.resolveTo("&[-0]");
		++stackUsage;
	}
	context.fetchNext();
	auto const opname = context.currentToken();
	if (
		opname.type == LTS_TT_INCREMENT
	||	opname.type == LTS_TT_DECREMENT
	) {
		Makai::String const op = opname.type == LTS_TT_INCREMENT ? "inc" : "dec";
		context.writeLine("copy", lhs.resolve(), "-> .");
		context.writeLine("uop inc", lhs.resolve(), "->", lhs.resolve());
		if (stackUsage)
			context.writeLine("clear", stackUsage);
		return {lhs.type, lhs.resolver, context.resolveTo(".")};
	}
	if (opname.type == LTS_TT_IDENTIFIER) {
		auto const id = context.getValue<Makai::String>();
		if (id == "is") {
			context.fetchNext();
			if (!context.hasToken(LTS_TT_IDENTIFIER))
				context.error("Expected type name here!");
			auto const type = resolveSymbolPath(context);
			if (type->type != Context::Scope::Member::Type::AV2_TA_SMT_TYPE)
				context.error("Symbol is not a type!");
			context.writeLine("push", lhs.resolve());
			context.writeLine("call in tname");
			context.writeLine("comp ( &[-0] = \"", type->name, "\") -> .");
			context.writeLine("pop void");
			return {context.getBasicType("bool"), context.resolveTo(".")};
		}
	}
	context.fetchNext();
	auto rhs = doValueResolution(context);
	if (rhs.resolve() == ".") {
		context.writeLine("push .");
		rhs.resolver = context.resolveTo("&[-0]");
		if (stackUsage++) rhs.resolver = context.resolveTo("&[-1]");
	}
	auto result = stronger(context, lhs.type, rhs.type);
	if (
		opname.type != Type{','}
	&&	lhs.type->value["basic"]
	&&	rhs.type->value["basic"]
	&& (
		Value::isUndefined(lhs.type->value["type"])
	||	Value::isUndefined(rhs.type->value["type"])
	)
	)
		context.error<InvalidValue>("Invalid operand types!");
	switch (opname.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = opname.value.get<Makai::String>();
			if (id == "as") {
				if (!context.isCastable(rhs.type))
					context.error<InvalidValue>("Casts can only happen between scalar types, strings, and [any]!");
				if (rhs.type != context.getBasicType("any")) {
					context.writeLine("cast", lhs.resolve(), ":", toTypeName(rhs.type), "-> .");
					result = rhs.type;
				}
			} else if (id == "if") {
				context.fetchNext();
				if (
					context.currentToken().type != LTS_TT_IDENTIFIER
				&&	context.currentToken().value.get<Makai::String>() != "else"
				) context.error<InvalidValue>("Expected 'else' here!");
				context.fetchNext();
				auto const elseVal = doValueResolution(context);
				result = handleTernary(context, lhs, rhs, elseVal);
			} else if (id == "else" || id == "or") {
				result = handleNullCoalescence(context, lhs, rhs);
			} else context.error<InvalidValue>("Invalid/Unsupported operation!");
		} break;
		case Type{'+'}: {
			if (context.isNumber(result)) context.writeLine("bop", lhs.resolve(), "+", rhs.resolve(), "-> .");
			else if (context.isString(lhs.type) && context.isString(rhs.type))
				context.writeLine("str cat", lhs.resolve(), "(", rhs.resolve(), ") -> .");
			else context.error<InvalidValue>("Invalid expression type(s) for operation!");
		} break;
		case Type{'/'}: {
			if (context.isNumber(result)) context.writeLine("bop", lhs.resolve(), "/", rhs.resolve(), "-> .");
			else if (context.isString(result)) 
				context.writeLine("str sep", lhs.resolve(), "(", rhs.resolve(), ") -> .");
			else context.error<InvalidValue>("Invalid expression type(s) for operation!");
		} break;
		case Type{','}: {
			return rhs;
		} break;
		case Type{'-'}:
		case Type{'*'}:
		case Type{'%'}: {
			auto const opstr = Makai::toString(Makai::Cast::as<char>(opname.type));
			if (context.isNumber(result)) context.writeLine("bop", lhs.resolve(), opstr, "-> .");
			else context.error<InvalidValue>("Invalid expression type(s) for operation!");
		} break;
		case Type::LTS_TT_COMPARE_EQUALS:
		case Type::LTS_TT_COMPARE_LESS_EQUALS:
		case Type::LTS_TT_COMPARE_GREATER_EQUALS:
		case Type{'<'}:
		case Type{'>'}:
		case Type{':'}: {
			Makai::String opstr;
			switch (opname.type) {
				case Type::LTS_TT_COMPARE_EQUALS:			opstr = "=";	break;
				case Type::LTS_TT_COMPARE_NOT_EQUALS:		opstr = "!";	break;
				case Type::LTS_TT_COMPARE_LESS_EQUALS:		opstr = "le";	break;
				case Type::LTS_TT_COMPARE_GREATER_EQUALS:	opstr = "ge";	break;
				case Type{'<'}:								opstr = "<";	break;
				case Type{'>'}:								opstr = ">";	break;
				case Type{':'}:								opstr = ":";	break;
			}
			context.writeLine("comp (", lhs.resolve(), opstr, rhs.resolve(), ") -> .");
		} break;
		case Type{'['}: {
			if (context.isObject(lhs.type)) {
				if (!context.isString(rhs.type))
					context.error<InvalidValue>("Right-hand side MUST be a string!");
			} else if (context.isArray(lhs.type)) {
				if (!context.isInteger(rhs.type))
					context.error<InvalidValue>("Right-hand side MUST be an integer!");
			} else context.error<InvalidValue>("Left-hand side MUST be an object or an array!");
			context.writeLine("get ", lhs.resolve(), "[", rhs.resolve(), "] -> .");
			result = context.getBasicType("any");
			context.fetchNext();
			if (context.currentToken().type != Type{']'})
				context.error<InvalidValue>("Expected ']' here!");
			context.fetchNext();
		} break;
		case Type{'='}: {
			if (lhs.type != rhs.type) {
				if (context.isCastable(result)) {
					context.writeLine("cast", rhs.resolve(), ":", toTypeName(lhs.type), "-> .");
					context.writeLine("copy . ->", lhs.resolve());
				} else context.error<InvalidValue>("Types are not convertible to each other!");
			}
			context.writeLine("copy", rhs.resolve(), "->", lhs.resolve());
		}
		default: context.error<InvalidValue>("Invalid/Unsupported operation!");
	}
	if (context.currentToken().type != Type{')'})
		context.error<InvalidValue>("Expected ')' here!");
	if (stackUsage)
		context.writeLine("clear", stackUsage);
	return {result, context.resolveTo(".")};
}

SEMIBREVE_TYPED_ASSEMBLE_FN(ReservedValueResolution) {
	auto const id = context.currentToken().value.get<Makai::String>();
	auto t = getType(context);
	return {t, context.resolveTo(id)};
}

using PreAssignFunction = Makai::Functor<void(Context&, Solution&)>;

void doVarAssign(
	Context& context,
	Makai::Instance<Context::Scope::Member> const& sym,
	Makai::Instance<Context::Scope::Member> const& type,
	bool const isGlobalVar = false,
	bool const isNewVar = false,
	PreAssignFunction const& preassign = {},
	PreAssignFunction const& postassign = {}
) {
	if (context.currentNamespace().hasChild(sym->name))
		context.error<InvalidValue>("Symbol name is also a namespace name!");
	auto result = doValueResolution(context);
	if (result.type != type) {
		if (!(context.isCastable(result.type) && context.isCastable(type)))
			context.error<InvalidValue>("Invalid expression type for assignment!");
		context.writeAdaptive("cast", result.resolve(), ":", toTypeName(type), "-> .");
		result.resolver = context.resolveTo(".");
	}
	if (isNewVar) {
		if (isGlobalVar) {
			if (sym->type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
				context.error<InvalidValue>("Symbol has already been defined as a different type in a previous scope!");
			else if (!sym->value.contains("type"))
				context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing global variable type!");
			else if (isGlobalVar && sym->value["global"] && sym->base != type)
				context.error<InvalidValue>("Global variable expression does not match its prevoius type!");
		}
	} else {
		if (!context.hasSymbol(sym->name))
			context.error<InvalidValue>("Variable does not exist in the current scope!");
		if (sym->type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
			context.error<InvalidValue>("Symbol has already been defined as a different type in a previous scope!");
	}
	preassign(context, result);
	if (isGlobalVar)
		context.writeAdaptive("copy", result.resolve(), "-> :", sym->name);
	else context.writeAdaptive("copy", result.resolve(), "->", context.varAccessor(sym).invoke());
	sym->value["init"] = true;
	postassign(context, result);
}

void doVarDecl(Context& context, Makai::Instance<Context::Scope::Member> const& sym, bool const isGlobalVar = false) {
	if (context.currentToken().type != Type{':'})
		context.error<InvalidValue>("Expected ':' here!");
	if (sym->declared())
		context.error<InvalidValue>("Redeclaration of already-declared symbol!");
	else sym->declare();
	auto type = context.getBasicType("any");
	sym->base = type;
	switch (context.currentToken().type) {
		case Type{':'}: {
			context.fetchNext();
			type = getType(context); 
		} break;
	}
	if (!context.nextToken()) {
		if (type == context.getBasicType("void"))
			context.error<NonexistentValue>("Malformed variable!");
	}
	sym->base = type;
	if (context.currentToken().type == Type{'='}) {
		context.fetchNext();
		doVarAssign(context, sym, type, isGlobalVar, true);
	}
}

static void doVarDecl(Context& context, bool const overrideAsLocal = false) {
	bool const isGlobalVar = overrideAsLocal ? false : context.currentToken().value.get<Makai::String>() == "global";
	if (!overrideAsLocal)
		context.fetchNext();
	auto const varname = context.currentToken();
	if (varname.type != LTS_TT_IDENTIFIER)
		context.error<InvalidValue>("Variable name must be an identifier!");
	auto const id = varname.value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		context.error<InvalidValue>("Variable name cannot be a reserved keyword!");
	if (!isGlobalVar) {
		context.writeAdaptive("push null");
	}
	if (context.currentScope().contains(id))
		context.error("Symbol with this name already exists in the current scope!");
	auto const sym = context.currentScope().addVariable(id, isGlobalVar);
	context.fetchNext();
	doVarDecl(context, sym, isGlobalVar);
}

#define ASSIGN_FN [=] (Context& context, Solution& result) -> void

SEMIBREVE_SYMBOL_ASSEMBLE_FN(SubscriptAssignment) {
	auto const accessor = context.varAccessor(sym);
	context.fetchNext();
	auto nameOrID = doValueResolution(context);
	usize stackUsage = 0;
	if (nameOrID.resolve() == ".") {
		context.writeLine("push .");
		nameOrID.resolver = context.resolveTo("&[-0]");
		++stackUsage;
	}
	if (!context.isBasicType(sym->base))
		context.error<InvalidValue>("Subscription is only allowed in basic objects and arrays!");
	auto const type = Makai::Cast::as<Makai::Data::Value::Kind, int16>(sym->base->value["type"]);
	switch (type) {
		case Value::Kind::DVK_OBJECT: {
			if (nameOrID.type != context.getBasicType("text"))
				context.error<InvalidValue>("Object subscription location must be a string!");
		} break;
		case Value::Kind::DVK_ARRAY: {
			if (!context.isInteger(nameOrID.type))
				context.error<InvalidValue>("Array subscription location must be an integer!");
		} break;
		default: context.error<InvalidValue>("Subscription is only allowed in basic objects and arrays!");
	}
	context.fetchNext();
	if (!context.hasToken(Type{'='}))
		context.error<InvalidValue>("Expected '=' here!");
	context.fetchNext();
	auto v = doValueResolution(context);
	if (nameOrID.resolve() == ".") {
		context.writeLine("push .");
		v.resolver = context.resolveTo("&[-0]");
		if (stackUsage++) nameOrID.resolver = context.resolveTo("&[-1]");
	}
	context.writeLine("set", v.resolve(), "->", accessor.invoke(), "[", nameOrID.resolve(), "]");
	context.writeLine("copy", v.resolve(), "-> .");
	if (stackUsage) context.writeLine("clear", stackUsage);
	return {sym->base, context.resolveTo(".")};
}

SEMIBREVE_SYMBOL_ASSEMBLE_FN(VariableAction) {
	context.fetchNext();
	auto const current = context.currentToken();
	PreAssignFunction pre, post;
	switch (current.type) {
		case Type{':'}: {
			doVarDecl(context, sym, false);
			if (sym->base) {
				auto const varType	= sym->base;
				return {sym->base, context.varAccessor(sym)};
			} else context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing variable type!");
		}
		case Type{'['}: {
			return doSubscriptAssignment(context, sym);
		} break;
		case Type{'.'}: {
			return doMemberCall(context, sym);
		}
		case Type{'='}: break;
		case LTS_TT_ADD_ASSIGN:
		case LTS_TT_SUB_ASSIGN:
		case LTS_TT_MUL_ASSIGN:
		case LTS_TT_DIV_ASSIGN:
		case LTS_TT_MOD_ASSIGN: {
			Makai::String const accessor = context.varAccessor(sym).invoke();
			Makai::String operation;
			switch (current.type) {
				case LTS_TT_ADD_ASSIGN: operation = "+"; break;
				case LTS_TT_SUB_ASSIGN: operation = "-"; break;
				case LTS_TT_MUL_ASSIGN: operation = "*"; break;
				case LTS_TT_DIV_ASSIGN: operation = "/"; break;
				case LTS_TT_MOD_ASSIGN: operation = "%"; break;
			}
			pre = ASSIGN_FN {
				context.writeLine("bop", accessor, operation, result.resolve(), "-> .");
				result.resolver = context.resolveTo(".");
			};
		} break;
		default: context.error<InvalidValue>("Invalid assignment operation!");
	}
	context.fetchNext();
	if (sym->base) {
		doVarAssign(context, sym, sym->base, false, false, pre);
		return {.type = sym->base, .resolver = context.varAccessor(sym)};
	} else context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing variable type!");
}

static Solution doFunctionCall(Context& context, Makai::Instance<Context::Scope::Member> const& sym, Makai::String const& self) {
	auto const id = sym->name;
	context.fetchNext();
	if (context.currentToken().type != Type{'('})
		context.error<InvalidValue>("Expected '(' here!");
	usize pushes = 0;
	Makai::List<Solution> args;
	auto const start = context.currentScope().stackc + context.currentScope().varc;
	auto legalName = id + "_";
	while (context.nextToken()) {
		if (context.currentToken().type == Type{')'}) break;
		args.pushBack(doValueResolution(context));
		DEBUGLN("Argument type: ", args.back().type->name);
		legalName += "_" + args.back().type->name;
		if (args.back().resolve() == ".") {
			context.writeLine("push .");
			args.back().resolver = context.resolveTo(Makai::toString("&[", start + pushes, "]"));
			++pushes;
		}
		context.fetchNext();
		if (context.currentToken().type == Type{')'}) break;
		else if (context.currentToken().type != Type{','})
			context.error<InvalidValue>("Expected ',' here!");
	}
	if (context.currentToken().type != Type{')'})
		context.error<InvalidValue>("Expected ')' here!");
	Makai::String call = "( ";
	usize index = 0;
	if (self.size())
		call += Makai::toString(index++, "=", self) + " ";
	for (auto const& arg: args)
		call += Makai::toString(index++, "=", arg.resolve()) + " ";
	call += ")";
	DEBUGLN("Overloads: [", sym->value["overloads"].get<Value::ObjectType>().keys().join("], ["), "]");
	DEBUGLN("Looking for: [", legalName, "]");
	if (!sym->value["overloads"].contains(legalName))
		context.error<InvalidValue>("Function overload does not exist!");
	auto const overload = sym->value["overloads"][legalName];
	context.writeLine("call", overload["full_name"].get<Makai::String>(), call);
	if (pushes)
		context.writeLine("clear", pushes);
	if (overload.contains("return"))
		return {
			context.resolveSymbol(overload["return"]),
			context.resolveTo(".")
		};
	else context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing return type!");
}

SEMIBREVE_ASSEMBLE_FN(Assembly) {
	if (context.currentScope().secure)
		context.error<NonexistentValue>("Assembly is only allowed in a [fatal] context!");
	context.fetchNext();
	if (context.currentToken().type != Type{'{'})
		context.error<NonexistentValue>("Expected '{' here!");
	context.fetchNext();
	while (context.currentToken().type != Type{'}'}) {
		context.writeLine(context.currentToken().token);
		context.fetchNext();
	}
	context.fetchNext();
}

SEMIBREVE_ASSEMBLE_FN(LooseContext) {
	context.fetchNext();
	context.startScope();
	context.currentScope().secure = false;
	doExpression(context);
	context.currentScope().secure = true;
	context.endScope();
}

SEMIBREVE_ASSEMBLE_FN(Return) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have returns outside of functions!");
	context.fetchNext();
	Solution result = {context.getBasicType("void")};
	auto const expectedType = context.functionScope().result;
	if (context.currentToken().type == Type{';'}) {
		if (expectedType != context.getBasicType("void"))
			context.error<NonexistentValue>("Missing return value!");
	} else {
		if (expectedType == context.getBasicType("void"))
			context.error<InvalidValue>("Function does not return a value!");
		result = doValueResolution(context);
		if (
			result.type != expectedType
		&&	!context.isNumber(stronger(context, result.type, expectedType))
		) context.error<InvalidValue>("Return type does not match!");
	}
	context.addFunctionExit();
	if (expectedType == context.getBasicType("void"))
		context.writeLine("end");
	else context.writeLine("ret", result.resolve());
}

SEMIBREVE_ASSEMBLE_FN(Main) {
	context.fetchNext();
	if (context.hasMain)
		context.error<NonexistentValue>("Only one entrypoint is allowed!");
	if (!context.inGlobalScope())
		context.error<NonexistentValue>("Main can only be declared on the global scope!");
	context.hasMain = true;
	if (context.currentToken().type != Type{'{'})
		context.error<InvalidValue>("Expected '{' here!");
	context.writeLine(context.main.entryPoint, ":");
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	doScope(context);
	context.endScope();
	context.writeLine("end");
	if (context.currentToken().type != Type{'}'})
		context.error<InvalidValue>("Expected '}' here!");
}

SEMIBREVE_ASSEMBLE_FN(Conditional) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have branches outside of functions!");
	context.fetchNext();
	auto const scopeName = context.scopePath() + context.uniqueName() + "_if";
	auto const ifTrue	= scopeName + "_true";
	auto const ifFalse	= scopeName + "_false";
	auto const endIf	= scopeName + "_end";
	auto const val = doValueResolution(context);
	context.fetchNext();
	context.writeLine("jump if true", val.resolve(), ifTrue);
	context.writeLine("jump if false", val.resolve(), ifFalse);
	context.writeLine(ifTrue, ":");
	context.startScope();
	doExpression(context);
	context.endScope();
	context.writeLine("jump", endIf);
	context.fetchNext();
	if (context.currentToken().type == LTS_TT_IDENTIFIER) {
		auto const id = context.currentToken().value.get<Makai::String>();
		if (id == "else") {
			context.fetchNext();
			context.writeLine(ifFalse, ":");
			context.startScope();
			doExpression(context);
			context.endScope();
			context.writeLine("jump", endIf);
		} else doExpression(context);
	}
	context.writeLine(endIf, ":");
	context.writeLine("next");
}

SEMIBREVE_ASSEMBLE_FN(ForLoop) {
	// TODO: This
}

SEMIBREVE_ASSEMBLE_FN(RepeatLoop) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have loops outside of functions!");
	auto const loopStart	= context.scopePath() + context.uniqueName() + "_repeat";
	auto const loopEnd		= loopStart + "_end";
	auto const tmpVar		= loopStart + "_tmpvar";
	context.writeLine(loopStart, ":");
	context.fetchNext();
	if (context.hasToken(Type{'{'})) {
		context.startScope(Context::Scope::Type::AV2_TA_ST_LOOP);
		doExpression(context);
		context.writeLine("jump", loopStart);
		context.endScope();
	} else if (context.hasToken(LTS_TT_IDENTIFIER)) {
		auto const id = context.currentToken().value.get<Makai::String>();
		if (context.currentScope().contains(id))
			context.error("Symbol with this name was already declared in this scope!");
		context.fetchNext();
		if (!context.hasToken(Type{':'}))
			context.error("Expected ':' here!");
		context.fetchNext();
		auto const times = doValueResolution(context);
		if (!context.isUnsigned(times.type))
			context.error("Loop count must be an unsigned integer!");
		context.startScope();
		auto const var = context.currentScope().addVariable(id);
		var->base = context.getBasicType("uint");
		context.writeLine("jump if zero", context.varAccessor(var).invoke(), loopEnd);
		context.writeLine("push", times.resolve());
		if (!context.isUnsigned(times.type))
			context.writeLine("cast &[-0]: uint -> &[-0]");
		context.fetchNext();
		doExpression(context);
		context.writeLine("uop dec ", context.varAccessor(var).invoke(), " -> ", context.varAccessor(var).invoke());
		context.writeLine("jump if pos", context.varAccessor(var).invoke(), loopStart);
		context.writeLine("pop void");
		context.endScope();
	} else context.error("Invalid expression!");
	context.writeLine(loopEnd, ":");
}

SEMIBREVE_ASSEMBLE_FN(DoLoop) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have loops outside of functions!");
	auto const uname = context.scopePath() + context.uniqueName() + "_do";
	context.fetchNext();
	context.writeLine(uname, ":");
	context.startScope(Context::Scope::Type::AV2_TA_ST_LOOP);
	doExpression(context);
	context.endScope();
	context.fetchNext();
	if (
		context.currentToken().type != LTS_TT_IDENTIFIER
	||	context.currentToken().value.get<Makai::String>() != "while"
	) context.error<InvalidValue>("Expected 'while' here!");
	auto const cond = doValueResolution(context);
	if (!context.isVerifiable(cond.type))
		context.error<InvalidValue>("Condition result must be a verifiable type!");
	context.writeLine("jump if true", cond.resolve(), uname);
}

SEMIBREVE_ASSEMBLE_FN(WhileLoop) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have loops outside of functions!");
	auto const uname = context.scopePath() + context.uniqueName() + "_do";
	auto const loopend = uname + "_end";
	context.fetchNext();
	auto const cond = doValueResolution(context);
	if (!context.isVerifiable(cond.type))
		context.error<InvalidValue>("Condition result must be a verifiable type!");
	context.writeLine(uname, ":");
	context.writeLine("jump if false", cond.resolve(), loopend);
	context.fetchNext();
	context.startScope(Context::Scope::Type::AV2_TA_ST_LOOP);
	doExpression(context);
	context.endScope();
	context.writeLine("jump if true", cond.resolve(), uname);
	context.writeLine(loopend, ":");
}

SEMIBREVE_ASSEMBLE_FN(Terminate) {
	context.writeLine("halt");
}

SEMIBREVE_ASSEMBLE_FN(Error) {
	if (context.inGlobalScope())
		context.error<InvalidValue>("Errors cannot be thrown in the global scope!");
	context.fetchNext();
	auto const err = doValueResolution(context);
	context.writeLine("error", err.resolve());
}

SEMIBREVE_TYPED_ASSEMBLE_FN(UnaryOperation) {
	auto const current = context.currentToken();
	context.fetchNext();
	auto result = doValueResolution(context);
	switch (current.type) {
		case Type{'-'}: {
			if (!context.isNumber(result.type))
				context.error<NonexistentValue>("Negation can only happen on numbers!");
			context.writeLine("uop -", result.resolve(), "-> .");
			//result.source = result.value;
			result.resolver = context.resolveTo(".");
			result.type = context.getBasicType("int");
		} break;
		case Type{'+'}: {
			if (!context.isNumber(result.type))
				context.error<NonexistentValue>("Positration can only happen on numbers!");
			context.writeLine("copy", result.resolve(), "-> .");
			//result.source = result.value;
			result.resolver = context.resolveTo(".");
		} break;
		case LTS_TT_DECREMENT: {
			if (!context.isNumber(result.type))
				context.error<NonexistentValue>("Incrementation can only happen on numbers!");
			context.writeLine("uop inc", result.resolve(), "->", result.resolve());
		} break;
		case LTS_TT_INCREMENT: {
			if (!context.isNumber(result.type))
				context.error<NonexistentValue>("Decrementation can only happen on numbers!");
			context.writeLine("uop dec", result.resolve(), "->", result.resolve());
		} break;
	}
	return result;
}

struct ModuleResolution {
	Makai::String path;
	Makai::String fullName;
	Makai::String sourceName;
	Makai::String head;
};

ModuleResolution resolveModuleName(Context& context) {
	Makai::String path			= "";
	Makai::String fullName		= "";
	Makai::String sourceName	= "";
	Makai::String head			= "";
	while (true) {
		context.fetchNext();
		if (context.currentToken().type != LTS_TT_IDENTIFIER)
			context.error<InvalidValue>("Expected module name here!");
		auto const node = context.currentToken().value.get<Makai::String>();
		path		+= "/" + node;
		fullName	+= "_" + node;
		sourceName	+= "." + node;
		if (head.empty())
			head = node;
		context.fetchNext();
		if (context.currentToken().type != Type{'.'}) break;
	}
	return {path, fullName, sourceName, head};
}

SEMIBREVE_ASSEMBLE_FN(ModuleImport) {
	if (!context.inGlobalScope())
		context.error<InvalidValue>("Module imports/exports can only be declared in the global scope!");
	Context submodule;
	ModuleResolution mod = resolveModuleName(context);
	if (context.hasModule(mod.sourceName)) {
		context.out.writeLine("Module '", mod.sourceName, "' already loaded - importing not needed...");
		return;
	}
	context.registerModule(mod.sourceName);
	submodule.fileName		= mod.path;
	submodule.isModule		= true;
	submodule.sourcePaths	= context.sourcePaths;
	submodule.stream.open(context.getModuleFile(mod.path));
	submodule.main.preEntryPoint	+= "_" + mod.fullName;
	submodule.main.entryPoint		+= "_" + mod.fullName;
	submodule.main.postEntryPoint	+= "_" + mod.fullName;
	submodule.global.stackc = submodule.global.stackc + submodule.global.varc;
	Breve assembler(submodule);
	submodule.modules.append(context.modules);
	assembler.assemble();
	context.writeFinale(submodule.intermediate());
	context.writeMainPreamble("call", submodule.main.preEntryPoint, "()");
	context.writeMainPostscript("call", submodule.main.postEntryPoint, "()");
	submodule.global.ns->name = mod.head;
	if (submodule.global.ns->hasChild(mod.head))
		context.importModule(submodule.global.ns->children[mod.head]);
	else context.importModule(submodule.global.ns);
	context.modules.append(submodule.modules);
}

SEMIBREVE_ASSEMBLE_FN(UsingDeclaration) {
	context.fetchNext();
	auto ns = resolveNamespace(context);
	context.currentNamespace().append(ns);
}

SEMIBREVE_ASSEMBLE_FN(Namespace) {
	if (!context.inNamespace())
		context.error<InvalidValue>("You can only declare sub-namespaces inside other namespaces!");
	usize scopeCount = 0;
	auto ns = context.currentNamespaceRef();
	while (
		context.currentToken().type == Type{'.'}
	||	context.currentToken().type == LTS_TT_IDENTIFIER
	) {
		context.fetchNext();
		if (context.currentToken().type != LTS_TT_IDENTIFIER)
			context.error<NonexistentValue>("Expected identifier for namespace name!");
		auto const id = context.currentToken().value.get<Makai::String>();
		if (context.isReservedKeyword(id))
			context.error<InvalidValue>("Namespace name cannot be a reserved keyword!");
		if (context.currentScope().contains(id))
			context.error<InvalidValue>("Namespace name is also a symbol name!");
		context.startScope(Context::Scope::Type::AV2_TA_ST_NAMESPACE);
		auto& scope = context.currentScope();
		scope.name		=
		scope.ns->name	= id;
		++scopeCount;
		context.fetchNext();
		if (context.currentNamespace().hasChild(id))
			ns = context.currentNamespace().children[id];
		else {
			ns->addChild(context.currentScope().ns);
			ns = context.currentNamespaceRef();
		}
		if (context.currentToken().type == Type {'{'})
			break;
		if (context.currentToken().type != Type{'.'})
			context.error<NonexistentValue>("Expected '.' here!!");
	}
	if (context.currentToken().type != Type {'{'})
		context.error<NonexistentValue>("Expected '{' here!");
	doScope(context);
	if (context.currentToken().type != Type {'}'})
		context.fetchNext();
	if (context.currentToken().type != Type {'}'})
		context.error<NonexistentValue>("Expected '}' here!");
	while (scopeCount--)
		context.endScope();
}

SEMIBREVE_ASSEMBLE_FN(Signal) {
	context.fetchNext();
	if (!context.hasToken(LTS_TT_IDENTIFIER))
		context.error<NonexistentValue>("Signal name must be an identifier!");
	auto const name = context.currentToken().value.get<Makai::String>();
	if (context.isReservedKeyword(name))
		context.error<NonexistentValue>("Signal name cannot be a reserved keyword!");
	auto const fullName = context.namespacePath("_") + "_" + name;
	context.currentScope().addFunction(name);
	auto& overloads = context.getSymbolByName(name).value["overloads"];
	auto& overload = overloads[fullName];
	overload["args"]		= Value::array();
	overload["full_name"]	= "_signal" + fullName;
	overload["return"]		= "void";
	overload["extern"]		= false;
	context.writeLine("hook _signal" + fullName, ":");
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	doExpression(context);
	context.endScope();
	context.writeLine("end");
}

SEMIBREVE_ASSEMBLE_FN(Yield) {
	if (!context.inFunction())
		context.error<InvalidValue>("Can only yield inside functions!");
	else context.writeLine("yield");
}

SEMIBREVE_SYMBOL_ASSEMBLE_FN(MemberCall) {
	auto const ns = sym->base->ns;
	context.fetchNext();
	if (!context.hasToken(LTS_TT_IDENTIFIER))
		context.error("Member name must be an identifier!");
	auto const id = context.currentToken().value.get<Makai::String>();
	if (!ns->members.contains(id))
		context.error<NonexistentValue>("Member call does not exist!");
	auto const memcall = ns->members[id];
	auto const ret = doFunctionCall(context, memcall, memcall->value["static"] ? "" : context.varAccessor(sym).invoke());
	return ret;
}

SEMIBREVE_ASSEMBLE_FN(TypeDefinition) {
	context.fetchNext();
	if (!context.hasToken(LTS_TT_IDENTIFIER))
		context.error("Type name must be an identifier!");
	auto const name = context.currentToken().value.get<Makai::String>();
	if (context.currentScope().contains(name))
		context.error("Symbol with this name already exists in the current scope!");
	context.fetchNext();
	if (!context.hasToken(Type{'='}))
		context.error("Expected '=' here!");
	context.fetchNext();
	auto const sym = resolveSymbolPath(context);
	if (sym->type != Context::Scope::Member::Type::AV2_TA_SMT_TYPE)
		context.error("Type definition must be another type!");
	context.currentScope().ns->members[name] = sym;
}

SEMIBREVE_ASSEMBLE_FN(TypeExtension) {
	// TODO: This
}

static void doMacroRuleType(Context& context, Context::Macro::Rule& rule, Context::Macro::Rule::Match& base) {
	auto const varType = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "rule type").getString();
	if (varType == "expr") {
		base.type = decltype(base.type)::AV2_TA_SM_RMT_EXPRESSION;
	} else {
		context.error("Invalid rule type!");
	}
}

static void doMacroRule(Context& context, Context::Macro::Rule& rule, Context::Macro::Rule::Match& base);

static void doMacroRuleGroup(Context& context, Context::Macro::Rule& rule, Context::Macro::Rule::Match& base) {
	context.expectToken(Type{'{'});
	while (true) {
		if (context.fetchNext().hasToken(Type{'}'})) break;
		doMacroRule(context, rule, *base.addSubMatch());
	}
	context.expectToken(Type{'}'});
}

static void doMacroRule(Context& context, Context::Macro::Rule& rule, Context::Macro::Rule::Match& base) {
	switch (context.currentToken().type) {
		case Type{'$'}: {
			context.fetchNext();
			switch (context.currentToken().type) {
				case LTS_TT_IDENTIFIER: {
					auto const varName = context.getValue<Makai::String>();
					context.fetchNext().expectToken(Type{':'});
					doMacroRuleType(context, rule, base);
					rule.variables[base.id()] = varName;
				} break;
				case Type{'?'}:
				case Type{'$'}:
				case Type{'*'}:
				case Type{'{'}:
				case Type{'}'}: base.tokens.pushBack(context.currentToken()); break;
				default: break;
			}
		} break;
		case Type{'*'}: {
			base.variadic	= true;
			base.count		= -1;
			context.fetchNext();
			doMacroRule(context, rule, *base.addSubMatch());
		} break;
		case Type{'?'}: {
			base.variadic	= true;
			base.count		= 1;
			context.fetchNext();
			doMacroRule(context, rule, *base.addSubMatch());
		} break;
		case Type{'{'}: {
			doMacroRuleGroup(context, rule, *base.addSubMatch());
		} break;
		case Type{'#'}: {
			doMacroRuleType(context, rule, base);
		} break;
		default: {
			base.tokens.pushBack(context.currentToken());
		} break;
	}
}

Makai::Instance<Context::Macro::Transformation> macroApply(Context::Macro::Arguments const& values) {
	return new Context::Macro::Transformation{
		.pre = [=] (Context::Macro::Context& context) {
			context.result.value.appendBack(values);
		}
	};
}

static void doMacroTransform(
	Context& context,
	Context::Macro::Rule& rule,
	Context::Macro::Transformation& base
) {
	Context::Macro::Arguments result;
	while (true) {
		if (context.fetchNext().hasToken(Type{'}'})) break;
		switch (context.currentToken().type) {
			case Type{'$'}: {
				if (result.size())
					base.sub.pushBack(macroApply(result));
				result.clear();
				context.fetchNext();
				switch (context.currentToken().type) {
					case LTS_TT_IDENTIFIER: {
						auto const varName = context.getValue<Makai::String>();
						if (rule.variables.values().find(varName) == -1)
							context.error("Macro variable does not exist!");
						DEBUGLN("--- Transform::Variable: [", varName, "]");
						base.newTransform()->pre = [varName = Makai::copy(varName)] (Context::Macro::Context& context) {
							DEBUGLN("--- SIMPLE VARIABLE EXPANSION");
							DEBUGLN("--- Apply::Variable: [", varName, "]");
							auto toks = context.variables[varName].tokens;
							DEBUGLN("--- Apply::Argc: [", toks.size(), "]");
							for (auto& tok: toks)
								context.result.value.appendBack(tok);
						};
					} break;
					case Type{'*'}: {
						auto const varName = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "macro variable name").getString();
						if (rule.variables.values().find(varName) == -1)
							context.error("Macro variable does not exist!");
						DEBUGLN("--- Transform::Variable: [", varName, "]");
						context.fetchNext().expectToken(Type{'{'});
						Context::Macro::Transformation tf;
						doMacroTransform(context, rule, tf);
						context.expectToken(Type{'}'});
						base.newTransform()->pre = 
							[varName = Makai::copy(varName), tf] (Context::Macro::Context& ctx) {
								DEBUGLN("--- COMPLEX VARIABLE EXPANSION");
								Context::Macro::Context subctx = ctx;
								tf.apply(subctx);
								DEBUGLN("--- Apply::Variable: [", varName, "]");
								auto toks = ctx.variables[varName].tokens;
								DEBUGLN("--- Apply::Argc: [", toks.size(), "]");
								usize i = 0;
								for (auto& tok: toks) {
									ctx.result.value.appendBack(tok);
									if (i++) ctx.result.value.appendBack(subctx.result.match);
								}
							}
						;
					} break;
					case Type{'!'}: {
						auto const msgt = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "message type").getString();
						auto const msgv = context.fetchNext().fetchToken(LTS_TT_DOUBLE_QUOTE_STRING).getString();
						if (msgt == "error" || msgt == "err")
							base.newTransform()->pre = [msgv, &context] (auto&) {
								context.error<Context::MacroError>(msgv);
							};
						else if (msgt == "warning" || msgt == "warn")
							base.newTransform()->pre = [msgv, &context] (auto&) {
								context.out.writeLine("Warning: ", msgv);
								context.out.writeLine("At: ", context.currentToken().position.line);
								context.out.writeLine("Column: ", context.currentToken().position.column);
							};
						else context.error("Invalid message type!");
					} break;
					default: context.error("Invalid macro expansion!");
				}
			} break;
			default: result.pushBack(context.currentToken());
		}
	}
	if (result.size())
		base.sub.pushBack(macroApply(result));
}

static Context::Macro::Expression doMacroExpression(Context& context, Context::Macro& macro) {
	Context::Macro::Expression expr;
	doMacroRule(context, expr.rule, *expr.rule.root);
	context.fetchNext().expectToken(LTS_TT_BIG_ARROW);
	context.fetchNext().expectToken(Type{'{'});
	doMacroTransform(context, expr.rule, expr.transform);
	context.expectToken(Type{'}'});
	return expr;
}

SEMIBREVE_ASSEMBLE_FN(Macro) {
	context.fetchNext();
	auto const name = context.fetchToken(LTS_TT_IDENTIFIER, "macro name").getString();
	auto const macro = (context.currentScope().addMacro(name)->macro = new Context::Macro());
	switch (context.fetchNext().currentToken().type) {
		case LTS_TT_BIG_ARROW: {
			context.fetchNext().expectToken(Type{'{'});
			macro->exprs.pushBack(doMacroExpression(context, *macro));
		} break;
		case Type{'{'}: {
			while (true) {
				if (context.fetchNext().hasToken(Type{'}'})) break;
				macro->exprs.pushBack(doMacroExpression(context, *macro));
			}
			if (macro->exprs.empty())
				context.error<NonexistentValue>("Macro is empty!");
		} break;
		case Type{'='}: {
			context.fetchNext();
			macro->simple = true;
			Context::Macro::Expression expr;
			doMacroTransform(context, expr.rule, expr.transform);
			macro->exprs.pushBack(expr);
		} break;
	}
}

static void doMacroExpansion(Context& context, Makai::Instance<Context::Scope::Member> const& symbol, Makai::String const& self) {
	context.fetchNext();
	auto const result = symbol->macro->resolve(context.append.cache, context);
	if (!result)
		context.error("No viable macro rules match the given expression!");
	auto rv = result.value();
	DEBUGLN("Match: ", rv.match.toList<Makai::String>([] (auto const& elem) -> Makai::String {return elem.type == LTS_TT_IDENTIFIER ? (" " + elem.token) : elem.token;}).join());
	DEBUGLN("Result: ", rv.value.toList<Makai::String>([] (auto const& elem) -> Makai::String {return elem.type == LTS_TT_IDENTIFIER ? (" " + elem.token) : elem.token;}).join());
	auto const pc = context.append.cache.sliced(rv.match.size());
	context.append.cache.clear().appendBack(rv.value).appendBack(pc);
}

SEMIBREVE_ASSEMBLE_FN(Expression) {
	auto const current = context.currentToken();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			if (id == "function" || id == "func" || id == "fn")	doFunction(context);
			else if (id == "signal")							doSignal(context);
			else if (id == "external" || id == "out")			doExternal(context);
			else if (id == "internal" || id == "in")			doInternal(context);
			else if (id == "namespace" || id == "module")		doNamespace(context);
			else if (id == "import")							doModuleImport(context);
			else if (id == "using")								doUsingDeclaration(context);
			else if (id == "global" || id == "local")			doVarDecl(context);
			else if (id == "minima" || id == "asm")				doAssembly(context);
			else if (id == "fatal")								doLooseContext(context);
			else if (id == "return")							doReturn(context);
			else if (id == "if")								doConditional(context);
			else if (id == "do")								doDoLoop(context);
			else if (id == "while")								doWhileLoop(context);
			else if (id == "for")								doForLoop(context);
			else if (id == "repeat")							doRepeatLoop(context);
			else if (id == "main")								doMain(context);
			else if (id == "terminate")							doTerminate(context);
			else if (id == "yield")								doYield(context);
			else if (id == "error")								doError(context);
			else if (id == "type")								doTypeDefinition(context);
			else if (id == "extend")							doTypeExtension(context);
			else if (id == "macro")								doMacro(context);
			else if (context.hasSymbol(id)) {
				auto sym = context.getSymbolRefByName(id);
				switch (sym->type) {
					case Context::Scope::Member::Type::AV2_TA_SMT_MACRO:	doMacroExpansion(context, sym); doExpression(context);	break;
					case Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION:	doFunctionCall(context, sym);							break;
					case Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE:	doVariableAction(context, sym);							break;
					default: context.error<InvalidValue>("Invalid/Unsupported expression!");
				}
			} else if (context.hasNamespace(id)) {
				auto const sym = resolveNamespaceMember(context);
				switch (sym.value->type) {
					case Context::Scope::Member::Type::AV2_TA_SMT_MACRO:	doMacroExpansion(context, sym.value); doExpression(context);	break;
					case Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION: doFunctionCall(context, sym.value);								break;
					case Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE: doVariableAction(context, sym.value);							break;
					default: context.error<InvalidValue>("Invalid/Unsupported expression!");
				}
			} else doVarDecl(context, true);
		} break;
		case Type{'('}: {
			doBinaryOperation(context);
		} break;
		case Type{'-'}:
		case Type{'+'}:
		case LTS_TT_DECREMENT:
		case LTS_TT_INCREMENT: {
			auto const result = doUnaryOperation(context);
		} break;
		case Type{'{'}: {
			context.startScope();
			doScope(context);
			context.endScope();
		}
		case Type{'}'}:
		case Type{';'}: break;
		default: context.error<InvalidValue>("Invalid expression!");
	}
}

void Semibreve::assemble() {
	if (!context.isModule) {
		context.writeGlobalPreamble("call", context.main.preEntryPoint, "()");
		context.writeGlobalPreamble("call", context.main.entryPoint, "()");
		context.writeGlobalPreamble("call", context.main.postEntryPoint, "()");
		context.writeGlobalPreamble("flush");
		context.writeGlobalPreamble("halt");
	}
	context.writeMainPreamble(context.main.preEntryPoint, ":");
	context.writeMainPostscript(context.main.postEntryPoint, ":");
	context.cache();
	while (context.nextToken()) doExpression(context);
	context.writeMainPreamble("end");
	context.writeMainPostscript("end");
	if (!context.isModule && !context.hasMain)
		context.error<NonexistentValue>("Missing main entrypoint!");
}

CTL_DIAGBLOCK_END