#include "breve.hpp"
#include "context.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using namespace Makai::Error;

namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = Breve::TokenStream::Token::Type;
using enum Type;
using Value = Makai::Data::Value;

struct Solution {
	Value::Kind		type;
	Makai::String	value;
	Makai::String	source;
};

using NamespaceMember	= Makai::KeyValuePair<Makai::String, Context::Scope::Member&>;

#define BREVE_ASSEMBLE_FN(NAME) static void do##NAME (Breve::Context& context)
#define BREVE_TYPED_ASSEMBLE_FN(NAME) static Solution do##NAME (Breve::Context& context)
#define BREVE_SYMBOL_ASSEMBLE_FN(NAME) static Solution do##NAME (Breve::Context& context, Context::Scope::Member& sym)

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

BREVE_ASSEMBLE_FN(ModuleImport);
BREVE_ASSEMBLE_FN(Namespace);
BREVE_ASSEMBLE_FN(Scope);
BREVE_ASSEMBLE_FN(Expression);
BREVE_ASSEMBLE_FN(Return);
BREVE_ASSEMBLE_FN(Conditional);
BREVE_ASSEMBLE_FN(ForLoop);
BREVE_ASSEMBLE_FN(WhileLoop);
BREVE_ASSEMBLE_FN(RepeatLoop);
BREVE_ASSEMBLE_FN(DoLoop);
BREVE_ASSEMBLE_FN(Main);
BREVE_ASSEMBLE_FN(Terminate);
BREVE_ASSEMBLE_FN(Error);
BREVE_ASSEMBLE_FN(External);

BREVE_TYPED_ASSEMBLE_FN(ReservedValueResolution);
BREVE_TYPED_ASSEMBLE_FN(BinaryOperation);
BREVE_TYPED_ASSEMBLE_FN(UnaryOperation);
BREVE_TYPED_ASSEMBLE_FN(InternalPrint);
BREVE_TYPED_ASSEMBLE_FN(Internal);

BREVE_SYMBOL_ASSEMBLE_FN(Assignment);

static Solution doFunctionCall(Context& context, Context::Scope::Member& symbol);

static Solution doValueResolution(Context& context, bool idCanBeValue = false);

static Makai::String doDefaultValue(Breve::Context& context, Makai::String const& var, Makai::String const& uname) {
	context.fetchNext();
	auto const dvloc = "__" + context.scopePath() + "_" + var + "_set_default" + uname;
	context.getSymbolByName(var).value["default_setter"] = dvloc;
	auto dv = dvloc + ":\n";
	auto const vr = doValueResolution(context);
	return dvloc + ":\npush" + vr.value;
}

constexpr auto const DVK_ANY = Context::DVK_ANY;

static Makai::Data::Value::Kind getType(Context& context) {
	using enum Makai::Data::Value::Kind;
	auto const ret = context.stream.current();
	switch (ret.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = ret.value.get<Makai::String>();
			if (id == "any")							return DVK_ANY;
			else if (id == "undefined" || id == "void")	return DVK_VOID;
			else if (id == "signed" || id == "int")		return DVK_SIGNED;
			else if (id == "unsigned" || id == "uint")	return DVK_UNSIGNED;
			else if (id == "float" || id == "real")		return DVK_REAL;
			else if (id == "string" || id == "str")		return DVK_STRING;
			else if (id == "array" || id == "arr")		return DVK_ARRAY;
			else if (id == "binary" || id == "bytes")	return DVK_BYTES;
			else if (id == "object" || id == "struct")	return DVK_OBJECT;
			else return DVK_VOID;
		}
		default: context.error<InvalidValue>("Invalid/Unsupported type!");
	}
	context.error<InvalidValue>("Invalid/Unsupported type!");
}

static Makai::String argname(Value::Kind const& type) {
	if (type == DVK_ANY)			return "any";
	if (Value::isScalar(type))		return "val";
	if (Value::isString(type))		return "str";
	if (Value::isArray(type))		return "arr";
	if (Value::isBytes(type))		return "bin";
	if (Value::isObject(type))		return "obj";
	if (Value::isNull(type))		return "null";
	if (Value::isUndefined(type))	return "void";
	return "none";
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

struct Prototype {
	Value::Kind									returnType;
	Makai::String								name;
	Makai::String								fullName;
	Makai::Instance<Context::Scope::Member>		function;
};

static Prototype doFunctionPrototype(Context& context, bool const isExtern = false, bool const declared = false) {
	auto const fname = context.stream.current();
	if (fname.type != Type::LTS_TT_IDENTIFIER)
		context.error<InvalidValue>("Function name must be an identifier!");
	auto const fid = fname.value.get<Makai::String>();
	if (context.isReservedKeyword(fid))
		context.error<InvalidValue>("Function name cannot be a reserved keyword!");
	auto id = fid;
	auto args = Makai::Data::Value::array();
	context.fetchNext();
	if (context.stream.current().type != Type{'('})
		context.error<NonexistentValue>("Expected '(' here!");
	CTL::Ex::Data::Value::Kind retType = DVK_ANY;
	id += "_";
	Makai::String gpre = "";
	auto const signature = context.uniqueName();
	Makai::List<Makai::KeyValuePair<Makai::String, Value>> optionals;
	bool inOptionalRegion = false;
	while (context.stream.next() && context.stream.current().type != Type{')'}) {
		bool isOptional = false;
		auto const argn = context.stream.current();
		if (argn.type != Type::LTS_TT_IDENTIFIER)
			context.error<InvalidValue>("Argument name must be an identifier!");
		auto const argID = argn.value.get<Makai::String>();
		if (context.isReservedKeyword(argID))
			context.error<InvalidValue>("Argument name cannot be a reserved keyword!");
		if (!context.currentScope().contains(argID))
			context.currentScope().addVariable(argID);
		else context.error<InvalidValue>("Argument with this name already exists!");
		context.fetchNext();
		if (context.stream.current().type != Type{':'})
			context.error<InvalidValue>("Expected ':' here!");
		context.fetchNext();
		auto const argt = getType(context);
		if (argt == CTL::Ex::Data::Value::Kind::DVK_UNDEFINED)
			context.error<InvalidValue>("Invalid argument type!");
		auto& var = context.currentScope().ns->members[argID]->value;
		context.fetchNext();
		if (context.stream.current().type == Type{'='}) {
			isOptional = true;
			inOptionalRegion = true;
			gpre.appendBack(doDefaultValue(context, argID, signature));
			optionals.pushBack({argID});
			optionals.back().value["name"] = argID;
			optionals.back().value["type"] = Makai::enumcast(argt);
		} else {
			id += "_" + argname(argt);
			auto& arg = args[args.size()];
			arg["name"] = argID;
			var["type"] = Makai::enumcast(argt); 
			arg["type"] = Makai::enumcast(argt);
		}
		if (inOptionalRegion && !isOptional)
			context.error<NonexistentValue>("Missing value for optional argument!");
		if (context.stream.current().type == Type{')'})
			break;
		if (context.stream.current().type != Type{','})
			context.error<InvalidValue>("Expected ',' here!");
	}
	if (context.stream.current().type != Type{')'})
		context.error<InvalidValue>("Expected ')' here!");
	context.fetchNext();
	if (context.stream.current().type == Type{':'}) {
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
	auto resolutionName = id;
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
	if (!context.currentScope().contains(fid))
		context.currentScope().addFunction(fid);
	else if (context.currentScope().ns->members[fid]->type != Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION)
		context.error<InvalidValue>("Symbol with this name already exists!");
	auto mem = context.currentScope().ns->members[fid];
	proto.function = mem;
	auto& overloads	= mem->value["overloads"];
	if (overloads.contains(resolutionName))
		context.error<InvalidValue>("Function with similar signature already exists!");
	auto& overload	= overloads[resolutionName];
	for (auto& opt: optionals) {
		fullName += "_" + opt.value["type"].get<Makai::String>();
	}
	overload["args"]		= args;
	overload["decl"]		= declared;
	overload["full_name"]	= fullName;
	overload["return"]		= Makai::enumcast(retType);
	overload["extern"]		= optionals.empty() ? isExtern : false;
	usize i = 0;
	for (auto& opt: optionals) {
		resolutionName += "_" + opt.key;
		if (overloads.contains(resolutionName))
			context.error<InvalidValue>("Function with similar signature already exists!");
		auto& overload	= overloads[resolutionName];
		args[args.size()]		= opt.value;
		overload["args"]		= args;
		overload["decl"]		= true;
		overload["full_name"]	= opt.value["declname"];
		overload["return"]		= Makai::enumcast(retType);
		overload["extern"]		= ++i < optionals.size() ? false : isExtern;
	}
	context.functions.pushBack(mem);
	return proto;
}

BREVE_ASSEMBLE_FN(Function) {
	context.fetchNext();
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	auto const proto = doFunctionPrototype(context, false, true);
	context.writeLine(proto.fullName, ":");
	if (context.stream.current().type == Type{'{'}) {
		doScope(context);
	} else if (context.stream.current().type == LTS_TT_BIG_ARROW) {
		auto const v = doValueResolution(context);
		if (proto.returnType != v.type && !(context.isCastable(proto.returnType) && context.isCastable(v.type)))
			context.error("Return types do not match!");
		if (proto.returnType != v.type) {
			context.writeLine("cast", v.value, "as", toTypeName(proto.returnType), "-> .");
			context.writeLine("ret .");
		}
		else context.writeLine("ret", v.value);
	} else context.error("Expected '{' or '=>' here!");
	context.writeLine("end");
	context.endScope();
	auto ns = context.currentNamespaceRef();
	if (context.inFunction()) ns = context.currentScope().ns;
	if (!ns->members.contains(proto.name))
		ns->members[proto.name] = proto.function;
	else if (ns->members[proto.name]->type != Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION)
		context.error<InvalidValue>("Symbol with this name already exists!");
}

BREVE_ASSEMBLE_FN(ExternalFunction) {
	context.startScope();
	context.fetchNext();
	auto const proto = doFunctionPrototype(context, true, true);
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
	context.writeLine("call out", fname, toTypeName(proto.returnType), "(", args, ")");
	if (context.stream.current().type != Type{';'})
		context.error<InvalidValue>("Expected ';' here!");
	context.endScope();
}

BREVE_ASSEMBLE_FN(Scope) {
	while (context.stream.next()) {
		auto const current = context.stream.current();
		if (current.type == Type{'}'}) break; 
		else doExpression(context);
	}
	if (context.currentScope().varc)
		context.writeLine("clear ", context.currentScope().varc);
}

BREVE_ASSEMBLE_FN(ExternalValue) {
	auto const id = context.stream.current().value.get<Makai::String>();
	if (context.currentScope().contains(id))
		context.error<FailedAction>("Symbol with this name already exists in this scope!");
	// TODO: The rest of the owl
}

BREVE_ASSEMBLE_FN(External) {
	context.fetchNext();
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Expected keyword here!");
	auto const id = context.stream.current().value.get<Makai::String>();
	if (id == "function" || id == "func" || id == "fn") doExternalFunction(context);
	else if (!context.isReservedKeyword(id))
		doExternalValue(context);
	else context.error<NonexistentValue>("Invalid keyword!");
}

BREVE_TYPED_ASSEMBLE_FN(InternalPrint) {
	context.fetchNext();
	auto const v = doValueResolution(context);
	context.writeLine("push", v.value);
	context.writeLine("call in print");
	return {Value::Kind::DVK_VOID, "."};
}

BREVE_TYPED_ASSEMBLE_FN(Internal) {
	context.fetchNext();
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Expected keyword here!");
	auto const id = context.stream.current().value.get<Makai::String>();
	if (id == "print") return doInternalPrint(context);
	else context.error<NonexistentValue>("Invalid keyword!");
}

NamespaceMember resolveNamespaceMember(Context& context, Context::Scope::Namespace& ns) {
	DEBUGLN("Namespace:", ns.name);
	for (auto const& mem : ns.children.keys())
		DEBUGLN("  - Namespace: ", mem);
	for (auto const& mem : ns.members.keys())
		DEBUGLN("  - Member: ", mem);
	context.fetchNext();
	if (context.stream.current().type != Type{'.'})
		context.error<NonexistentValue>("Expected '.' here!");
	context.fetchNext();
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.stream.current().value.get<Makai::String>();
	DEBUGLN("Looking for: ", id);
	if (ns.members.contains(id))
		return {id, *ns.members[id]};
	else if (ns.children.contains(id))
		return resolveNamespaceMember(context, *ns.children[id]);
	else context.error<NonexistentValue>("Symbol does not exist!");
}

NamespaceMember resolveNamespaceMember(Context& context) {
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.stream.current().value.get<Makai::String>();
	return resolveNamespaceMember(context, context.getNamespaceByName(id));
}

Context::Scope::Namespace& resolveNamespace(Context& context, Context::Scope::Namespace& ns) {
	context.fetchNext();
	if (context.stream.current().type != Type{'.'})
		return ns;
	context.fetchNext();
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.stream.current().value.get<Makai::String>();
	if (ns.members.contains(id))
		context.error<NonexistentValue>("Not a namespace!");
	else if (ns.children.contains(id))
		return resolveNamespace(context, *ns.children[id]);
	else context.error<NonexistentValue>("Namespace does not exist!");
}

Context::Scope::Namespace& resolveNamespace(Context& context) {
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		context.error<NonexistentValue>("Namespace name must be an identifier!");
	auto const id = context.stream.current().value.get<Makai::String>();
	return resolveNamespace(context, context.getNamespaceByName(id));
}

// TODO: Apply this solution to the rest of the assembler
static Solution resolveSymbol(Context& context, Makai::String const& id, Context::Scope::Member& sym) {
	// TODO: Add value resolution to account for symbol
	// TODO: Ensure symbol reference does not get borked
		// NOTE: Might be moot point since namespace data is stored as shared pointer,
		// and members only exist in namespaces
		// And these functions do not directly modify the namespace
	if (sym.type == Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION) {
		return doFunctionCall(context, sym);
	} else if (sym.type == Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE) {
		sym.value["use"] = true;
		if (!sym.value.contains("type"))
			context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing variable type!");
		auto const type = Makai::Cast::as<Makai::Data::Value::Kind, int16>(sym.value["type"]);
		return {type, context.varAccessor(sym)};
	} else context.error<InvalidValue>("Invalid symbol type for operation");
}

static Solution doValueResolution(Context& context, bool idCanBeValue) {
	auto const current = context.stream.current();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			auto result = doReservedValueResolution(context);
			if (result.type != Value::Kind::DVK_VOID) return result;
			else if (context.hasSymbol(id)) 
				return resolveSymbol(context, id, context.getSymbolByName(id));
			else if (context.hasNamespace(id)) {
				auto const sym = resolveNamespaceMember(context);
				return resolveSymbol(context, sym.key, sym.value);
			} else if (id == "sizeof") {
				context.fetchNext();
				auto result = doValueResolution(context);
				context.writeLine("push", result.source);
				context.writeLine("call in sizeof");
				context.writeLine("pop .");
				return {Value::Kind::DVK_UNSIGNED, "."};
			} else if (idCanBeValue) return {Value::Kind::DVK_STRING, "\"" + id + "\""};
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
		case LTS_TT_DOUBLE_QUOTE_STRING:	return {Value::Kind::DVK_STRING,	current.value.toString()								};
		case LTS_TT_CHARACTER: 				return {Value::Kind::DVK_STRING,	Makai::toString("'", current.value.get<char>(), "'")	};
		case LTS_TT_INTEGER:				return {Value::Kind::DVK_UNSIGNED,	current.value.toString()								};
		case LTS_TT_REAL:					return {Value::Kind::DVK_REAL,		current.value.toString()								};
		default: context.error<InvalidValue>("Invalid expression!");
	}
}

constexpr Value::Kind stronger(Value::Kind const a, Value::Kind const b) {
	if (a == b) return a;
	return a > b ? a : b;
}

static Value::Kind handleTernary(Breve::Context& context, Solution const& cond, Solution const& ifTrue, Solution const& ifFalse) {
	auto const result = stronger(ifTrue.type, ifFalse.type);
	if (!Value::isNumber(result) && ifTrue.type != ifFalse.type)
		context.error<InvalidValue>("Types must match, or be similar!");
	if (Value::isUndefined(cond.type))
		context.error<InvalidValue>("Invalid condition type!");
	if (!Value::isVerifiable(cond.type))
		context.error<InvalidValue>("Condition must be a verifiable type!");
	auto const trueJump		= context.scopePath() + "_ternary_true"		+ context.uniqueName();
	auto const falseJump	= context.scopePath() + "_ternary_false"	+ context.uniqueName();
	auto const endJump		= context.scopePath() + "_ternary_end"		+ context.uniqueName();
	context.writeLine("jump if true", cond.value, trueJump);
	context.writeLine("jump if false", cond.value, falseJump);
	context.writeLine(trueJump + ":");
	context.writeLine("copy", ifTrue.value, "-> .");
	context.writeLine("jump", endJump);
	context.writeLine(falseJump + ":");
	context.writeLine("copy", ifFalse.value, "-> .");
	context.writeLine("jump", endJump);
	return result;
}

BREVE_TYPED_ASSEMBLE_FN(BinaryOperation) {
	context.fetchNext();
	auto lhs = doValueResolution(context);
	usize stackUsage = 0;
	if (lhs.value == ".") {
		context.writeLine("push .");
		lhs.value = "&[-0]";
		++stackUsage;
	}
	context.fetchNext();
	auto const opname = context.stream.current();
	if (
		opname.type == LTS_TT_INCREMENT
	||	opname.type == LTS_TT_DECREMENT
	) {
		Makai::String const op = opname.type == LTS_TT_INCREMENT ? "inc" : "dec";
		context.writeLine("copy", lhs.value, "-> .");
		context.writeLine("uop inc", lhs.value, "->", lhs.value);
		if (stackUsage)
			context.writeLine("clear", stackUsage);
		return {lhs.type, ".", lhs.value};
	}
	context.fetchNext();
	auto rhs = doValueResolution(context);
	if (rhs.value == ".") {
		context.writeLine("push .");
		rhs.value = "&[-0]";
		if (stackUsage++) lhs.value = "&[-1]";
	}
	auto result = stronger(lhs.type, rhs.type);
	if (Value::isUndefined(lhs.type) || Value::isUndefined(rhs.type))
		context.error<InvalidValue>("Invalid operand types!");
	switch (opname.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = opname.value.get<Makai::String>();
			if (id == "as") {
				if (!context.isCastable(rhs.type))
					context.error<InvalidValue>("Casts can only happen between scalar types, strings, and [any]!");
				if (rhs.type != context.DVK_ANY) {
					context.writeLine("cast", lhs.value, ":", toTypeName(rhs.type), "-> .");
					result = rhs.type;
				}
			} else if (id == "if") {
				context.fetchNext();
				if (
					context.stream.current().type != LTS_TT_IDENTIFIER
				&&	context.stream.current().value.get<Makai::String>() != "else"
				) context.error<InvalidValue>("Expected 'else' here!");
				context.fetchNext();
				auto const elseVal = doValueResolution(context);
				result = handleTernary(context, lhs, rhs, elseVal);
			} else context.error<InvalidValue>("Invalid/Unsupported operation!");
		} break;
		case Type{'+'}: {
			if (Value::isNumber(result)) context.writeLine("bop", lhs.value, "+", rhs.value, "-> .");
			else if (Value::isString(lhs.type) && Value::isString(rhs.type))
				context.writeLine("str cat", lhs.value, "(", rhs.value, ") -> .");
			else context.error<InvalidValue>("Invalid expression type(s) for operation!");
		} break;
		case Type{'/'}: {
			if (Value::isNumber(result)) context.writeLine("bop", lhs.value, "/", rhs.value, "-> .");
			else if (Value::isString(result)) 
				context.writeLine("str sep", lhs.value, "(", rhs.value, ") -> .");
			else context.error<InvalidValue>("Invalid expression type(s) for operation!");
		};
		case Type{'-'}:
		case Type{'*'}:
		case Type{'%'}: {
			auto const opstr = Makai::toString(Makai::Cast::as<char>(opname.type));
			if (Value::isNumber(result)) context.writeLine("bop", lhs.value, opstr, "-> .");
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
				case Type::LTS_TT_COMPARE_EQUALS:			opstr = " = ";	break;
				case Type::LTS_TT_COMPARE_NOT_EQUALS:		opstr = " ! ";	break;
				case Type::LTS_TT_COMPARE_LESS_EQUALS:		opstr = " le ";	break;
				case Type::LTS_TT_COMPARE_GREATER_EQUALS:	opstr = " ge ";	break;
				case Type{'<'}:								opstr = " < ";	break;
				case Type{'>'}:								opstr = " > ";	break;
				case Type{':'}:								opstr = " : ";	break;
			}
			context.writeLine("comp (", lhs.value, opstr, rhs.value, ") -> .");
		} break;
		case Type{'['}: {
			if (Value::isObject(lhs.type)) {
				if (!Value::isString(rhs.type))
					context.error<InvalidValue>("Right-hand side MUST be a string!");
			} else if (Value::isArray(lhs.type)) {
				if (!Value::isInteger(rhs.type))
					context.error<InvalidValue>("Right-hand side MUST be an integer!");
			} else context.error<InvalidValue>("Left-hand side MUST be an object or an array!");
			context.writeLine("get ", lhs.value, "[", rhs.value, "] -> .");
			result = DVK_ANY;
			context.fetchNext();
			if (context.stream.current().type != Type{']'})
				context.error<InvalidValue>("Expected ']' here!");
			context.fetchNext();
		} break;
		case Type{'='}: {
			if (lhs.type != rhs.type) {
				if (context.isCastable(result)) {
					context.writeLine("cast", rhs.value, ":", toTypeName(lhs.type), "-> .");
					context.writeLine("copy . ->", lhs.value);
				} else context.error<InvalidValue>("Types are not convertible to each other!");
			}
			context.writeLine("copy", rhs.value, "->", lhs.value);
		}
		default: context.error<InvalidValue>("Invalid/Unsupported operation!");
	}
	if (context.stream.current().type != Type{')'})
		context.error<InvalidValue>("Expected ')' here!");
	if (stackUsage)
		context.writeLine("clear", stackUsage);
	return {result, "."};
}

BREVE_TYPED_ASSEMBLE_FN(ReservedValueResolution) {
	auto const id = context.stream.current().value.get<Makai::String>();
	auto t = getType(context);
	if (t != Value::Kind::DVK_VOID) return {t, ""};
	if (id == "true" || id == "false")		t = Value::Kind::DVK_BOOLEAN;
	else if (id == "null")					t = Value::Kind::DVK_NULL;
	else if (id == "nan")					t = Value::Kind::DVK_NAN;
	else if (id == "array" || id == "arr")	t = Value::Kind::DVK_ARRAY;
	else if (id == "object" || id == "obj")	t = Value::Kind::DVK_OBJECT;
	else return {Value::Kind::DVK_VOID, ""};
	return {t, id};
}

using PreAssignFunction = Makai::Functor<void(Breve::Context&, Solution&)>;

void doVarAssign(
	Breve::Context& context,
	Context::Scope::Member& sym,
	Value::Kind const& type,
	bool const isGlobalVar = false,
	bool const isNewVar = false,
	PreAssignFunction const& preassign = {},
	PreAssignFunction const& postassign = {}
) {
	if (context.currentNamespace().hasChild(sym.name))
		context.error<InvalidValue>("Symbol name is also a namespace name!");
	auto result = doValueResolution(context);
	if (result.type != type) {
		if (context.isCastable(result.type))
			context.error<InvalidValue>("Invalid expression type for assignment!");
		context.writeAdaptive("cast", result.value, ":", toTypeName(type), "-> .");
		result.value = ".";
	}
	if (isNewVar) {
		if (isGlobalVar) {
			if (sym.type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
				context.error<InvalidValue>("Symbol has already been defined as a different type in a previous scope!");
			else if (!sym.value.contains("type"))
				context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing global variable type!");
			else if (isGlobalVar && sym.value["global"] && Makai::Cast::as<Value::Kind, int16>(sym.value["type"]) != type)
				context.error<InvalidValue>("Global variable expression does not match its prevoius type!");
		} else {
			if (context.currentScope().contains(sym.name))
				context.error<InvalidValue>("Variable already exists in current scope!");
			else context.currentScope().addVariable(sym.name, isGlobalVar);
		}
	} else {
		if (!context.hasSymbol(sym.name))
			context.error<InvalidValue>("Variable does not exist in the current scope!");
		if (sym.type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
			context.error<InvalidValue>("Symbol has already been defined as a different type in a previous scope!");
	}
	preassign(context, result);
	if (isGlobalVar)
		context.writeAdaptive("copy", result.value, "-> :", sym.name);
	else context.writeAdaptive("copy", result.value, "-> &[", context.stackIndex(sym), "]");
	sym.value["init"] = true;
	postassign(context, result);
}

void doVarDecl(Breve::Context& context, Context::Scope::Member& sym, bool const isGlobalVar = false) {
	if (context.stream.current().type != Type{':'})
		context.error<InvalidValue>("Expected ':' here!");
	if (sym.declared())
		context.error<InvalidValue>("Redeclaration of already-declared symbol!");
	else sym.declare();
	auto type = DVK_ANY;
	switch (context.stream.current().type) {
		case Type{':'}: {
			type = getType(context); 
		} break;
	}
	sym.value["type"] = Makai::enumcast(type);
	if (
		!context.stream.next()
	) {
		if (type == Value::Kind::DVK_VOID)
			context.error<NonexistentValue>("Malformed variable!");
	}
	if (context.stream.current().type == Type{'='}) {
		context.fetchNext();
		doVarAssign(context, sym, type, isGlobalVar, true);
	}
}

BREVE_ASSEMBLE_FN(VarDecl) {
	bool const isGlobalVar = context.stream.current().value.get<Makai::String>() == "global";
	context.fetchNext();
	auto const varname = context.stream.current();
	if (varname.type != LTS_TT_IDENTIFIER)
		context.error<InvalidValue>("Variable name must be an identifier!");
	auto const id = varname.value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		context.error<InvalidValue>("Variable name cannot be a reserved keyword!");
	context.fetchNext();
	if (!isGlobalVar) {
		context.writeAdaptive("push null");
	}
	auto const sym = resolveNamespaceMember(context);
	doVarDecl(context, sym.value, isGlobalVar);
}

#define ASSIGN_FN [=] (Breve::Context& context, Solution& result) -> void

BREVE_SYMBOL_ASSEMBLE_FN(SubscriptAssignment) {
	auto const accessor = context.varAccessor(sym);
	context.fetchNext();
	auto nameOrID = doValueResolution(context);
	usize stackUsage = 0;
	if (nameOrID.value == ".") {
		context.writeLine("push .");
		nameOrID.value = "&[-0]";
		++stackUsage;
	}
	auto const type = Makai::Cast::as<Makai::Data::Value::Kind, int16>(sym.value["type"]);
	switch (type) {
		case Value::Kind::DVK_OBJECT: {
			if (nameOrID.type != Value::Kind::DVK_STRING)
				context.error<InvalidValue>("Object subscription location must be a string!");
		} break;
		case Value::Kind::DVK_ARRAY: {
			if (!Value::isInteger(nameOrID.type))
				context.error<InvalidValue>("Array subscription location must be an integer!");
		} break;
		default: context.error<InvalidValue>("Subscription is only allowed in objects and arrays!");
	}
	context.fetchNext();
	if (!context.hasToken(Type{'='}))
		context.error<InvalidValue>("Expected '=' here!");
	context.fetchNext();
	auto v = doValueResolution(context);
	if (v.value == ".") {
		context.writeLine("push .");
		v.value = "&[-0]";
		if (stackUsage++) nameOrID.value = "&[-1]";
	}
	context.writeLine("set", v.value, "->", accessor, "[", nameOrID.value, "]");
	context.writeLine("copy", v.value, "-> .");
	if (stackUsage) context.writeLine("clear", stackUsage);
	return {type, "."};
}

BREVE_SYMBOL_ASSEMBLE_FN(Assignment) {
	context.fetchNext();
	auto const current = context.stream.current();
	PreAssignFunction pre, post;
	switch (current.type) {
		case Type{':'}: {
			doVarDecl(context, sym, false);
			if (sym.value.contains("type")) {
				auto const varType	= Makai::Cast::as<Value::Kind, int16>(sym.value["type"]);
				auto const accessor	= Makai::toString("&[", context.stackIndex(sym), "]");
				return {varType, accessor};
			} else context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing variable type!");
		}
		case Type{'['}: {
			return doSubscriptAssignment(context, sym);
		} break;
		case Type{'='}: break;
		case LTS_TT_ADD_ASSIGN:
		case LTS_TT_SUB_ASSIGN:
		case LTS_TT_MUL_ASSIGN:
		case LTS_TT_DIV_ASSIGN:
		case LTS_TT_MOD_ASSIGN: {
			Makai::String const accessor = context.varAccessor(sym);
			Makai::String operation;
			switch (current.type) {
				case LTS_TT_ADD_ASSIGN: operation = "+"; break;
				case LTS_TT_SUB_ASSIGN: operation = "-"; break;
				case LTS_TT_MUL_ASSIGN: operation = "*"; break;
				case LTS_TT_DIV_ASSIGN: operation = "/"; break;
				case LTS_TT_MOD_ASSIGN: operation = "%"; break;
			}
			pre = ASSIGN_FN {
				context.writeLine("bop", accessor, operation, result.value, "-> .");
				result.value = ".";
			};
		} break;
		default: context.error<InvalidValue>("Invalid assignment operation!");
	}
	context.fetchNext();
	if (sym.value.contains("type")) {
		auto const varType	= Makai::Cast::as<Value::Kind, int16>(sym.value["type"]);
		auto const accessor	= Makai::toString("&[", context.stackIndex(sym), "]");
		doVarAssign(context, sym, varType, false, false, pre);
		return {varType, accessor};
	} else context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing variable type!");
}

static Solution doFunctionCall(Context& context, Context::Scope::Member& sym) {
	auto const id = sym.name;
	context.fetchNext();
	if (context.stream.current().type != Type{'('})
		context.error<InvalidValue>("Expected '(' here!");
	usize pushes = 0;
	Makai::List<Solution> args;
	auto const start = context.currentScope().stackc + context.currentScope().varc;
	auto legalName = id + "_";
	while (context.stream.next()) {
		if (context.stream.current().type == Type{')'}) break;
		args.pushBack(doValueResolution(context));
		legalName += "_" + argname(args.back().type);
		if (args.back().value == ".") {
			context.writeLine("push .");
			args.back().value = Makai::toString("&[", start + pushes, "]");
			++pushes;
		}
		context.fetchNext();
		if (context.stream.current().type == Type{')'}) break;
		else if (context.stream.current().type != Type{','})
			context.error<InvalidValue>("Expected ',' here!");
	}
	if (context.stream.current().type != Type{')'})
		context.error<InvalidValue>("Expected ')' here!");
	Makai::String call = "( ";
	for (auto const& [arg, index]: Makai::Range::expand(args))
		call += Makai::toString(index, "=", arg.value) + " ";
	call += ")";
	DEBUGLN("Overloads: [", sym.value["overloads"].get<Value::ObjectType>().keys().join("], ["), "]");
	DEBUGLN("Looking for: [", legalName, "]");
	if (!sym.value["overloads"].contains(legalName))
		context.error<InvalidValue>("Function overload does not exist!");
	auto const overload = sym.value["overloads"][legalName];
	context.writeLine("call", overload["full_name"].get<Makai::String>(), call);
	if (pushes)
		context.writeLine("clear", pushes);
	if (overload.contains("return"))
		return {
			Makai::Cast::as<Value::Kind, int16>(overload["return"]),
			"."
		};
	else context.error<FailedAction>(Makai::toString("[", __LINE__, "]") + " INTERNAL ERROR: Missing return type!");
}

BREVE_ASSEMBLE_FN(Assembly) {
	if (context.currentScope().secure)
		context.error<NonexistentValue>("Assembly is only allowed in a [fatal] context!");
	context.fetchNext();
	if (context.stream.current().type != Type{'{'})
		context.error<NonexistentValue>("Expected '{' here!");
	context.fetchNext();
	while (context.stream.current().type != Type{'}'}) {
		context.writeLine(context.stream.tokenText());
		context.stream.next();
	}
	context.fetchNext();
}

BREVE_ASSEMBLE_FN(LooseContext) {
	context.fetchNext();
	context.startScope();
	context.currentScope().secure = false;
	doExpression(context);
	context.currentScope().secure = true;
	context.endScope();
}

BREVE_ASSEMBLE_FN(Return) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have returns outside of functions!");
	context.fetchNext();
	Solution result = {Value::Kind::DVK_VOID};
	auto const expectedType = context.functionScope().result;
	if (context.stream.current().type == Type{';'}) {
		if (expectedType != Value::Kind::DVK_VOID)
			context.error<NonexistentValue>("Missing return value!");
	} else {
		if (expectedType == Value::Kind::DVK_VOID)
			context.error<InvalidValue>("Function does not return a value!");
		result = doValueResolution(context);
		if (
			result.type != expectedType
		&&	!Value::isNumber(stronger(result.type, expectedType))
		) context.error<InvalidValue>("Return type does not match!");
	}
	context.addFunctionExit();
	if (expectedType == Value::Kind::DVK_VOID)
		context.writeLine("end");
	else context.writeLine("ret", result.value);
}

BREVE_ASSEMBLE_FN(Main) {
	context.fetchNext();
	if (context.hasMain)
		context.error<NonexistentValue>("Only one entrypoint is allowed!");
	if (!context.inGlobalScope())
		context.error<NonexistentValue>("Main can only be declared on the global scope!");
	context.hasMain = true;
	if (context.stream.current().type != Type{'{'})
		context.error<InvalidValue>("Expected '{' here!");
	context.writeLine(context.main.entryPoint, ":");
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	doScope(context);
	context.endScope();
	context.writeLine("end");
	if (context.stream.current().type != Type{'}'})
		context.error<InvalidValue>("Expected '}' here!");
}

BREVE_ASSEMBLE_FN(Conditional) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have branches outside of functions!");
	context.fetchNext();
	auto const scopeName = context.scopePath() + context.uniqueName() + "_if";
	auto const ifTrue	= scopeName + "_true";
	auto const ifFalse	= scopeName + "_false";
	auto const endIf	= scopeName + "_end";
	auto const val = doValueResolution(context);
	context.fetchNext();
	context.writeLine("jump if true", val.value, ifTrue);
	context.writeLine("jump if false", val.value, ifFalse);
	context.writeLine(ifTrue, ":");
	context.startScope();
	doExpression(context);
	context.endScope();
	context.writeLine("jump", endIf);
	context.fetchNext();
	if (context.stream.current().type == LTS_TT_IDENTIFIER) {
		auto const id = context.stream.current().value.get<Makai::String>();
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

BREVE_ASSEMBLE_FN(ForLoop) {
	// TODO: This
}

BREVE_ASSEMBLE_FN(RepeatLoop) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have loops outside of functions!");
	context.fetchNext();
	auto const times = doValueResolution(context);
	auto const loopStart	= context.scopePath() + context.uniqueName() + "_repeat";
	auto const loopEnd		= loopStart + "_end";
	auto const tmpVar		= loopStart + "_tmpvar";
	context.writeLine(loopStart, ":");
	context.fetchNext();
	if (times.type == Value::Kind::DVK_VOID) {
		context.startScope(Context::Scope::Type::AV2_TA_ST_LOOP);
		doExpression(context);
		context.writeLine("jump", loopStart);
		context.endScope();
	} else if (times.type == Value::Kind::DVK_UNSIGNED) {
		context.writeLine("push", times.value);
		context.currentScope().addVariable(tmpVar);
		auto const stackID = context.stackIndex(*context.currentScope().ns->members[tmpVar]);
		context.startScope();
		doExpression(context);
		context.writeLine("uop dec &[", stackID, "] -> &[", stackID, "]");
		context.writeLine("jump if pos &[", stackID, "]", loopStart);
		context.endScope();
	}
	else context.error<InvalidValue>("Expected unsigned integer or void here!");
	context.writeLine(loopEnd, ":");
}

BREVE_ASSEMBLE_FN(DoLoop) {
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
		context.stream.current().type != LTS_TT_IDENTIFIER
	||	context.stream.current().value.get<Makai::String>() != "while"
	) context.error<InvalidValue>("Expected 'while' here!");
	auto const cond = doValueResolution(context);
	if (!Value::isVerifiable(cond.type))
		context.error<InvalidValue>("Condition result must be a verifiable type!");
	context.writeLine("jump if true", cond.value, uname);
}

BREVE_ASSEMBLE_FN(WhileLoop) {
	if (!context.inFunction())
		context.error<InvalidValue>("Cannot have loops outside of functions!");
	auto const uname = context.scopePath() + context.uniqueName() + "_do";
	auto const loopend = uname + "_end";
	context.fetchNext();
	auto const cond = doValueResolution(context);
	if (!Value::isVerifiable(cond.type))
		context.error<InvalidValue>("Condition result must be a verifiable type!");
	context.writeLine(uname, ":");
	context.writeLine("jump if false", cond.value, loopend);
	context.fetchNext();
	context.startScope(Context::Scope::Type::AV2_TA_ST_LOOP);
	doExpression(context);
	context.endScope();
	context.writeLine("jump if true", cond.value, uname);
	context.writeLine(loopend, ":");
}

BREVE_ASSEMBLE_FN(Terminate) {
	context.writeLine("halt");
}

BREVE_ASSEMBLE_FN(Error) {
	if (context.inGlobalScope())
		context.error<InvalidValue>("Errors cannot be thrown in the global scope!");
	context.fetchNext();
	auto const err = doValueResolution(context);
	context.writeLine("error", err.value);
}

BREVE_TYPED_ASSEMBLE_FN(UnaryOperation) {
	auto const current = context.stream.current();
	context.fetchNext();
	auto result = doValueResolution(context);
	switch (current.type) {
		case Type{'-'}: {
			if (!Value::isNumber(result.type))
				context.error<NonexistentValue>("Negation can only happen on numbers!");
			context.writeLine("uop -", result.value, "-> .");
			//result.source = result.value;
			result.value = ".";
		} break;
		case Type{'+'}: {
			if (!Value::isNumber(result.type))
				context.error<NonexistentValue>("Positration can only happen on numbers!");
			context.writeLine("copy", result.value, "-> .");
			//result.source = result.value;
			result.value = ".";
		} break;
		case LTS_TT_DECREMENT: {
			if (!Value::isNumber(result.type))
				context.error<NonexistentValue>("Incrementation can only happen on numbers!");
			context.writeLine("uop inc", result.value, "->", result.value);
		} break;
		case LTS_TT_INCREMENT: {
			if (!Value::isNumber(result.type))
				context.error<NonexistentValue>("Decrementation can only happen on numbers!");
			context.writeLine("uop dec", result.value, "->", result.value);
		} break;
	}
	return result;
}

struct ModuleResolution {
	Makai::String path;
	Makai::String fullName;
	Makai::String head;
};

ModuleResolution resolveModuleName(Context& context) {
	Makai::String path		= "";
	Makai::String fullName	= "";
	Makai::String head		= "";
	while (true) {
		context.fetchNext();
		if (context.stream.current().type != LTS_TT_IDENTIFIER)
			context.error<InvalidValue>("Expected module name here!");
		auto const node = context.stream.current().value.get<Makai::String>();
		path += "/" + node;
		fullName += "_" + node;
		if (head.empty())
			head = node;
		context.fetchNext();
		if (context.stream.current().type != Type{'.'}) break;
	}
	return {path, fullName, head};
}

BREVE_ASSEMBLE_FN(ModuleImport) {
	if (!context.inGlobalScope())
		context.error<InvalidValue>("Module imports/exports can only be declared in the global scope!");
	Context submodule;
	ModuleResolution mod = resolveModuleName(context);
	submodule.fileName		= mod.path;
	submodule.isModule		= true;
	submodule.sourcePaths	= context.sourcePaths;
	submodule.stream.open(context.getModuleFile(mod.path));
	submodule.main.preEntryPoint	+= "_" + mod.fullName;
	submodule.main.entryPoint		+= "_" + mod.fullName;
	submodule.main.postEntryPoint	+= "_" + mod.fullName;
	submodule.global.stackc = submodule.global.stackc + submodule.global.varc;
	Breve assembler(submodule);
	assembler.assemble();
	context.writeFinale(submodule.compose());
	context.writeMainPreamble("call", submodule.main.preEntryPoint, "()");
	context.writeMainPostscript("call", submodule.main.postEntryPoint, "()");
	submodule.global.ns->name = mod.head;
	if (submodule.global.ns->hasChild(mod.head))
		context.importModule(submodule.global.ns->children[mod.head]);
	else context.importModule(submodule.global.ns);
}

BREVE_ASSEMBLE_FN(UsingDeclaration) {
	context.fetchNext();
	auto ns = resolveNamespace(context);
	context.currentNamespace().append(ns);
}

BREVE_ASSEMBLE_FN(Namespace) {
	if (!context.inNamespace())
		context.error<InvalidValue>("You can only declare sub-namespaces inside other namespaces!");
	usize scopeCount = 0;
	auto ns = context.currentNamespaceRef();
	while (
		context.stream.current().type == Type{'.'}
	||	context.stream.current().type == LTS_TT_IDENTIFIER
	) {
		context.fetchNext();
		if (context.stream.current().type != LTS_TT_IDENTIFIER)
			context.error<NonexistentValue>("Expected identifier for namespace name!");
		auto const id = context.stream.current().value.get<Makai::String>();
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
		if (context.stream.current().type == Type {'{'})
			break;
		if (context.stream.current().type != Type{'.'})
			context.error<NonexistentValue>("Expected '.' here!!");
	}
	if (context.stream.current().type != Type {'{'})
		context.error<NonexistentValue>("Expected '{' here!");
	doExpression(context);
	if (context.stream.current().type != Type {'}'})
		context.fetchNext();
	if (context.stream.current().type != Type {'}'})
		context.error<NonexistentValue>("Expected '}' here!");
	while (scopeCount--)
		context.endScope();
}

BREVE_ASSEMBLE_FN(Signal) {
	context.fetchNext();
	if (!context.hasToken(LTS_TT_IDENTIFIER))
		context.error<NonexistentValue>("Signal name must be an identifier!");
	auto const name = context.stream.current().value.get<Makai::String>();
	if (context.isReservedKeyword(name))
		context.error<NonexistentValue>("Signal name cannot be a reserved keyword!");
	auto const fullName = context.namespacePath("_") + "_" + name;
	context.currentScope().addFunction(name);
	auto& overloads = context.getSymbolByName(name).value["overloads"];
	auto& overload = overloads[fullName];
	overload["args"]		= Value::array();
	overload["full_name"]	= "_signal" + fullName;
	overload["return"]		= Value::Kind::DVK_VOID;
	overload["extern"]		= false;
	context.writeLine("hook _signal" + fullName, ":");
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	doExpression(context);
	context.endScope();
	context.writeLine("end");
}

BREVE_ASSEMBLE_FN(Yield) {
	if (!context.inFunction())
		context.error<InvalidValue>("Can only yield inside functions!");
	else context.writeLine("yield");
}

BREVE_ASSEMBLE_FN(Expression) {
	auto const current = context.stream.current();
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
			else if (context.hasSymbol(id)) {
				auto& sym = context.getSymbolByName(id);
				switch (sym.type) {
					case Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION: doFunctionCall(context, sym);	break;
					case Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE: doAssignment(context, sym);		break;
					default: context.error<InvalidValue>("Invalid/Unsupported expression!");
				}
			} else if (context.hasNamespace(id)) {
				auto const sym = resolveNamespaceMember(context);
				switch (sym.value.type) {
					case Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION: doFunctionCall(context, sym.value);	break;
					case Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE: doAssignment(context, sym.value);	break;
					default: context.error<InvalidValue>("Invalid/Unsupported expression!");
				}
			} else context.error<InvalidValue>("Invalid/Unsupported expression ["+id+"]!");
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

void Breve::assemble() {
	if (!context.isModule) {
		context.writeGlobalPreamble("call", context.main.preEntryPoint, "()");
		context.writeGlobalPreamble("call", context.main.entryPoint, "()");
		context.writeGlobalPreamble("call", context.main.postEntryPoint, "()");
		context.writeGlobalPreamble("flush");
		context.writeGlobalPreamble("halt");
	}
	context.writeMainPreamble(context.main.preEntryPoint, ":");
	context.writeMainPostscript(context.main.postEntryPoint, ":");
	while (context.stream.next()) doExpression(context);
	context.writeMainPreamble("end");
	context.writeMainPostscript("end");
	if (!context.isModule && !context.hasMain)
		context.error<NonexistentValue>("Missing main entrypoint!");
}

CTL_DIAGBLOCK_END