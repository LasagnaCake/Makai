#include "breve.hpp"
#include "context.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = Breve::TokenStream::Token::Type;
using enum Type;
using Value = Makai::Data::Value;

using Solution = Makai::KeyValuePair<Value::Kind, Makai::String>;

#define BREVE_ASSEMBLE_FN(NAME) static void do##NAME (Breve::Context& context)
#define BREVE_TYPED_ASSEMBLE_FN(NAME) static Solution do##NAME (Breve::Context& context)

template <class T>
[[noreturn]] static void error(Makai::String what, Context& ctx) {
	auto const pos = ctx.stream.position();
	throw T(
		Makai::toString(
			"At:\nLINE: ", pos.line,
			"\nCOLUMN: ", pos.column,
			"\n", ctx.stream.tokenText()
		),
		what,
		Makai::CPP::SourceFile{"n/a", pos.line, ctx.fileName}
	);
}

#define BREVE_ERROR(TYPE, WHAT) error<Makai::Error::TYPE>({WHAT}, context)

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

BREVE_ASSEMBLE_FN(Module);
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
BREVE_TYPED_ASSEMBLE_FN(FunctionCall);
BREVE_TYPED_ASSEMBLE_FN(Assignment);
BREVE_TYPED_ASSEMBLE_FN(ReservedValueResolution);
BREVE_TYPED_ASSEMBLE_FN(BinaryOperation);
BREVE_TYPED_ASSEMBLE_FN(UnaryOperation);
BREVE_TYPED_ASSEMBLE_FN(ValueResolution);
BREVE_TYPED_ASSEMBLE_FN(InternalPrint);
BREVE_TYPED_ASSEMBLE_FN(Internal);

static bool isReserved(Makai::String const& keyword);

static Makai::String doDefaultValue(Breve::Context& context, Makai::String const& var, Makai::String const& uname) {
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed value definition!");
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
		default: BREVE_ERROR(InvalidValue, "Invalid/Unsupported type!");
	}
	BREVE_ERROR(InvalidValue, "Invalid/Unsupported type!");
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

struct Prototype {
	Value::Kind		returnType;
	Makai::String	name;
	Makai::String	fullName;
	Makai::String	entryPoint;
};

static Prototype doFunctionPrototype(Context& context, bool const isExtern = false) {
	auto const fname = context.stream.current();
	if (fname.type != Type::LTS_TT_IDENTIFIER)
		BREVE_ERROR(InvalidValue, "Function name must be an identifier!");
	auto const fid = fname.value.get<Makai::String>();
	if (context.isReservedKeyword(fid))
		BREVE_ERROR(InvalidValue, "Function name cannot be a reserved keyword!");
	auto id = fid;
	auto args = Makai::Data::Value::array();
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed function!");
	if (context.stream.current().type != Type{'('})
		BREVE_ERROR(NonexistentValue, "Expected '(' here!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed function!");
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
			BREVE_ERROR(InvalidValue, "Argument name must be an identifier!");
		auto const argID = argn.value.get<Makai::String>();
		if (context.isReservedKeyword(argID))
			BREVE_ERROR(InvalidValue, "Argument name cannot be a reserved keyword!");
		if (!context.currentScope().contains(argID))
			context.currentScope().addVariable(argID);
		else BREVE_ERROR(InvalidValue, "Argument with this name already exists!");
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed function argument list!");
		if (context.stream.current().type != Type{':'})
			BREVE_ERROR(InvalidValue, "Expected ':' here!");
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed function argument list!");
		auto const argt = getType(context);
		if (argt == CTL::Ex::Data::Value::Kind::DVK_UNDEFINED)
			BREVE_ERROR(InvalidValue, "Invalid argument type!");
		auto& var = context.currentScope().ns->members[argID].value;
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed function argument list!");
		if (context.stream.current().type == Type{')'})
			break;
		if (context.stream.current().type == Type{'='}) {
			isOptional = true;
			inOptionalRegion = true;
			gpre.appendBack(doDefaultValue(context, argID, signature));
			optionals.pushBack({argID});
			optionals.back().value["name"] = argID;
			optionals.back().value["type"] = argname(argt);
		} else {
			id += "_" + argname(argt);
			auto& arg = args[args.size()];
			arg["name"] = argID;
			var["type"] = arg["type"] = argname(argt);
		}
		if (inOptionalRegion && !isOptional)
			BREVE_ERROR(NonexistentValue, "Missing value for optional argument!");
		if (context.stream.current().type != Type{','})
			BREVE_ERROR(InvalidValue, "Expected ',' here!");
	}
	if (context.stream.current().type != Type{')'})
		BREVE_ERROR(InvalidValue, "Expected ')' here!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed function!");
	if (context.stream.current().type == Type{':'}) {
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed function!");
		retType = getType(context);
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed function!");
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
	context.writeGlobalPreamble(gpre, "call", fullName, "()");
	context.writeGlobalPreamble("end");
	for (auto& opt: optionals)
		fullName += "_" + opt.value["type"].get<Makai::String>();
	Prototype const proto = {retType, fid, fullName, resolutionName};
	auto subName = baseName;
	for (auto& opt: Makai::Range::reverse(optionals)) {
		fullName = fullName.sliced(0, -(opt.value["type"].get<Makai::String>().size() + 2));
		opt.value["declname"] = fullName;
	}
	if (!context.currentScope().contains(fid))
		context.currentScope().addFunction(fid);
	else if (context.currentScope().ns->members[fid].type != Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION)
		BREVE_ERROR(InvalidValue, "Symbol with this name already exists!");
	auto& mem = context.currentScope().ns->members[fid];
	auto& overloads	= mem.value["overloads"];
	if (overloads.contains(resolutionName))
		BREVE_ERROR(InvalidValue, "Function with similar signature already exists!");
	auto& overload	= overloads[resolutionName];
	for (auto& opt: optionals) {
		fullName += "_" + opt.value["type"].get<Makai::String>();
	}
	overload["args"]		= args;
	overload["full_name"]	= fullName;
	overload["return"]		= Makai::enumcast(retType);
	overload["extern"]		= optionals.empty() ? isExtern : false;
	usize i = 0;
	for (auto& opt: optionals) {
		resolutionName += "_" + opt.key;
		if (overloads.contains(resolutionName))
			BREVE_ERROR(InvalidValue, "Function with similar signature already exists!");
		auto& overload	= overloads[resolutionName];
		args[args.size()]		= opt.value;
		overload["args"]		= args;
		overload["full_name"]	= opt.value["declname"];
		overload["return"]		= Makai::enumcast(retType);
		overload["extern"]		= ++i < optionals.size() ? false : isExtern;
	}
	return proto;
}

BREVE_ASSEMBLE_FN(Function) {
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed function!");
	auto const proto = doFunctionPrototype(context, false);
	if (context.stream.current().type != Type{'{'})
		BREVE_ERROR(InvalidValue, "Expected '{' here!");
	context.writeLine(proto.entryPoint, ":");
	doScope(context);
	context.endScope();
}

BREVE_ASSEMBLE_FN(ExternalFunction) {
	context.startScope();
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed function!");
	auto const proto = doFunctionPrototype(context, true);
	context.writeLine(proto.entryPoint, ":");
	Makai::String args;
	usize argc = 0;
	for (auto const& [name, overload]: context.currentScope().ns->members[proto.name].value["overloads"].items()) {
		if (overload["extern"]) {
			argc = overload["args"].size();
			break;
		}
	}
	if (argc) for (auto const i: Makai::range(argc))
		args += Makai::toString(i, "= &[-", argc - (i + 1), "] ");
	context.writeLine("call out", proto.name, "(", args, ")");
	if (context.stream.current().type != Type{';'})
		BREVE_ERROR(InvalidValue, "Expected ';' here!");
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

BREVE_ASSEMBLE_FN(External) {
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed external expression!");
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		BREVE_ERROR(NonexistentValue, "Expected keyword here!");
	auto const id = context.stream.current().value.get<Makai::String>();
	if (id == "function" || id == "func" || id == "fn") doExternalFunction(context);
	else BREVE_ERROR(NonexistentValue, "Invalid keyword!");
}

BREVE_TYPED_ASSEMBLE_FN(InternalPrint) {
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed print!");
	auto const v = doValueResolution(context);
	context.writeLine("push", v.value);
	context.writeLine("call in print");
	context.writeLine("pop .");
	return {Value::Kind::DVK_VOID, "."};
}

BREVE_TYPED_ASSEMBLE_FN(Internal) {
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed internal expression!");
	if (context.stream.current().type != LTS_TT_IDENTIFIER)
		BREVE_ERROR(NonexistentValue, "Expected keyword here!");
	auto const id = context.stream.current().value.get<Makai::String>();
	if (id == "print") return doInternalPrint(context);
	else BREVE_ERROR(NonexistentValue, "Invalid keyword!");
}

BREVE_TYPED_ASSEMBLE_FN(ValueResolution) {
	auto const current = context.stream.current();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			auto result = doReservedValueResolution(context);
			if (result.key != Value::Kind::DVK_VOID) return result;
			else if (context.hasSymbol(id)) {
				auto const sym = context.symbol(id);
				if (sym().type == Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION) {
					return doFunctionCall(context);
				}
				else if (sym().type == Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE) {
					sym().value["use"] = true;
					if (sym().value["global"]) {
						return {
							Makai::Cast::as<Makai::Data::Value::Kind, int16>(sym().value["type"]),
							":" + id
						};
					} else {
						auto const stackID = sym().value["stack_id"].get<uint64>();
						return {
							Makai::Cast::as<Makai::Data::Value::Kind, int16>(sym().value["type"]),
							Makai::toString("&[", stackID, "]")
						};
					}
				}
			}
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
		default: BREVE_ERROR(InvalidValue, "Invalid expression!");
	}
}

constexpr Value::Kind stronger(Value::Kind const a, Value::Kind const b) {
	if (a == b) return a;
	return a > b ? a : b;
}

constexpr Makai::String toTypeName(Value::Kind t) {
	if (t < DVK_ANY) {
		t = Makai::Cast::as<Value::Kind>(Makai::Math::abs(Makai::enumcast(t)) - 2);
	}
	switch (t) {
		case DVK_ANY:						return "any";
		case Value::Kind::DVK_UNDEFINED:	return "v";
		case Value::Kind::DVK_BOOLEAN:		return "b";
		case Value::Kind::DVK_UNSIGNED:		return "u";
		case Value::Kind::DVK_SIGNED:		return "i";
		case Value::Kind::DVK_REAL:			return "r";
		case Value::Kind::DVK_ARRAY:		return "a";
		case Value::Kind::DVK_OBJECT:		return "o";
		case Value::Kind::DVK_BYTES:		return "bin";
		case Value::Kind::DVK_VECTOR:		return "vec";
	}
}

static Value::Kind handleTernary(Breve::Context& context, Solution const& cond, Solution const& ifTrue, Solution const& ifFalse) {
	auto const result = stronger(ifTrue.key, ifFalse.key);
	if (Value::isUndefined(cond.key))
		BREVE_ERROR(InvalidValue, "Invalid condition type!");
	if (!Value::isVerifiable(cond.key))
		BREVE_ERROR(InvalidValue, "Condition must be a verifiable type!");
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
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed operation!");
	auto lhs = doValueResolution(context);
	usize stackUsage = 0;
	if (lhs.value == ".") {
		context.writeLine("push .");
		lhs.value = "&[-0]";
		++stackUsage;
	}
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed operation!");
	auto const opname = context.stream.current();
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed operation!");
	auto rhs = doValueResolution(context);
	if (rhs.value == ".") {
		context.writeLine("push .");
		rhs.value = "&[-0]";
		if (stackUsage++) lhs.value = "&[-1]";
	}
	auto result = stronger(lhs.key, rhs.key);
	if (Value::isUndefined(lhs.key) || Value::isUndefined(rhs.key))
		BREVE_ERROR(InvalidValue, "Invalid operand types!");
	switch (opname.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = opname.value.get<Makai::String>();
			if (id == "as") {
				if (!context.isCastable(rhs.key))
					BREVE_ERROR(InvalidValue, "Casts can only happen between scalar types, strings, and [any]!");
				if (rhs.key != context.DVK_ANY) {
					context.writeLine("cast", lhs.value, ":", toTypeName(rhs.key), "-> .");
					result = rhs.key;
				}
			} else if (id == "if") {
				if (!context.stream.next())
					BREVE_ERROR(NonexistentValue, "Malformed operation!");
				if (
					context.stream.current().type != LTS_TT_IDENTIFIER
				&&	context.stream.current().value.get<Makai::String>() != "else"
				) BREVE_ERROR(InvalidValue, "Expected 'else' here!");
				if (!context.stream.next())
					BREVE_ERROR(NonexistentValue, "Malformed operation!");
				auto const elseVal = doValueResolution(context);
				result = handleTernary(context, lhs, rhs, elseVal);
			} else BREVE_ERROR(InvalidValue, "Invalid/Unsupported operation!");
		} break;
		case Type{'+'}: {
			if (Value::isNumber(result)) context.writeLine("calc", lhs.value, "+", rhs.value, "-> .");
			else if (Value::isString(lhs.key) && Value::isString(rhs.key))
				context.writeLine("str cat", lhs.value, "(", rhs.value, ") -> .");
			else BREVE_ERROR(InvalidValue, "Invalid expression type(s) for operation!");
		} break;
		case Type{'/'}: {
			if (Value::isNumber(result)) context.writeLine("calc", lhs.value, "/", rhs.value, "-> .");
			else if (Value::isString(result)) 
				context.writeLine("str sep", lhs.value, "(", rhs.value, ") -> .");
			else BREVE_ERROR(InvalidValue, "Invalid expression type(s) for operation!");
		};
		case Type{'-'}:
		case Type{'*'}:
		case Type{'%'}: {
			auto const opstr = Makai::toString(Makai::Cast::as<char>(opname.type));
			if (Value::isNumber(result)) context.writeLine("calc", lhs.value, opstr, "-> .");
			else BREVE_ERROR(InvalidValue, "Invalid expression type(s) for operation!");
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
			if (!Value::isObject(lhs.key))
				BREVE_ERROR(InvalidValue, "Left-hand side MUST be an object!");
			if (!Value::isString(rhs.key))
				BREVE_ERROR(InvalidValue, "Right-hand side MUST be a string!");
			context.writeLine("get &[", lhs.value, "][&[", rhs.value, "]] -> .");
			result = DVK_ANY;
			if (!context.stream.next())
				BREVE_ERROR(NonexistentValue, "Malformed operation!");
			if (context.stream.current().type != Type{']'})
				BREVE_ERROR(InvalidValue, "Expected ']' here!");
			if (!context.stream.next())
				BREVE_ERROR(NonexistentValue, "Malformed operation!");
		} break;
		case Type{'='}: {
			if (lhs.key != rhs.key) {
				if (context.isCastable(result)) {
					context.writeLine("cast", rhs.value, ":", toTypeName(lhs.key), "-> .");
					context.writeLine("copy . ->", lhs.value);
				} else BREVE_ERROR(InvalidValue, "Types are not convertible to each other!");
			}
			context.writeLine("copy", rhs.value, "->", lhs.value);
		}
		default: BREVE_ERROR(InvalidValue, "Invalid/Unsupported operation!");
	}
	if (context.stream.current().type != Type{')'})
		BREVE_ERROR(InvalidValue, "Expected ')' here!");
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
	Makai::String const& id,
	Value::Kind const& type,
	bool const isGlobalVar = false,
	bool const isNewVar = false,
	PreAssignFunction const& preassign = {}
) {
	auto result = doValueResolution(context);
	if (result.key != type) {
		if (context.isCastable(result.key))
			BREVE_ERROR(InvalidValue, "Invalid expression type for assignment!");
		context.writeLine("cast", result.value, ":", toTypeName(type), "-> .");
		result.value = ".";
	}
	if (isNewVar) {
		if (context.currentScope().contains(id)) {
			auto const sym = context.currentScope().ns->members[id];
			if (sym.type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
				BREVE_ERROR(InvalidValue, "Symbol has already been defined as a different type in a previous scope!");
			else if (isGlobalVar && sym.value["global"] && Makai::Cast::as<Value::Kind, int16>(sym.value["type"]) != type)
				BREVE_ERROR(InvalidValue, "Global variable expression does not match its prevoius type!");
		} else {
			if (context.currentScope().contains(id))
				BREVE_ERROR(InvalidValue, "Variable already exists in current scope!");
			else context.currentScope().addVariable(id, isGlobalVar);
		}
	} else {
		if (!context.hasSymbol(id))
			BREVE_ERROR(InvalidValue, "Variable does not exist in the current scope!");
		auto const sym = context.currentScope().ns->members[id];
		if (sym.type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
			BREVE_ERROR(InvalidValue, "Symbol has already been defined as a different type in a previous scope!");
	}
	auto const sym = context.symbol(id);
	preassign(context, result);
	if (isGlobalVar)
		context.writeAdaptive("copy", result.value, "-> :", id);
	else context.writeAdaptive("copy", result.value, "-> &[", sym().value["stack_id"].get<uint64>(), "]");
	sym().value["init"] = true;
}

void doVarDecl(Breve::Context& context, Makai::String const& id, bool const isGlobalVar = false) {
	if (context.stream.current().type != Type{':'})
		BREVE_ERROR(InvalidValue, "Expected ':' here!");
	auto type = DVK_ANY;
	switch (context.stream.current().type) {
		case Type{':'}: {
			type = getType(context); 
		} break;
	}
	if (
		!context.stream.next()
	) {
		if (type == Value::Kind::DVK_VOID)
			BREVE_ERROR(NonexistentValue, "Malformed variable!");
	}
	if (context.stream.current().type == Type{'='}) {
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed variable!");
		doVarAssign(context, id, type, isGlobalVar, true);
	}
	if (context.stream.current().type != Type{';'})
		BREVE_ERROR(InvalidValue, "Expected ';' here!");
}

BREVE_ASSEMBLE_FN(VarDecl) {
	bool const isGlobalVar = context.stream.current().value.get<Makai::String>() == "global";
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed variable!");
	auto const varname = context.stream.current();
	if (varname.type != LTS_TT_IDENTIFIER)
		BREVE_ERROR(InvalidValue, "Variable name must be an identifier!");
	auto const id = varname.value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		BREVE_ERROR(InvalidValue, "Variable name cannot be a reserved keyword!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed variable!");
	if (!isGlobalVar) {
		context.writeAdaptive("push null");
	}
	doVarDecl(context, id, isGlobalVar);
}

#define PREASSIGN [=] (Breve::Context& context, Solution& result) -> void

BREVE_TYPED_ASSEMBLE_FN(Assignment) {
	auto const id = context.stream.current().value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		BREVE_ERROR(InvalidValue, "Variable name cannot be a reserved keyword!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed variable!");
	auto const current = context.stream.current();
	PreAssignFunction pre;
	switch (current.type) {
		case Type{':'}: {
			doVarDecl(context, id, false);
			auto const sym = context.symbol(id);
			auto const varType	= Makai::Cast::as<Value::Kind, int16>(sym().value["type"]);
			auto const accessor	= Makai::toString("&[", sym().value["stack_id"].get<uint64>(), "]");
			return {varType, accessor};
		}
		case Type{'='}: break;
		case LTS_TT_ADD_ASSIGN:
		case LTS_TT_SUB_ASSIGN:
		case LTS_TT_MUL_ASSIGN:
		case LTS_TT_DIV_ASSIGN:
		case LTS_TT_MOD_ASSIGN: {
			auto const sym = context.symbol(id);
			Makai::String accessor;
			if (sym().value["global"])
				accessor = ":" + id;
			else accessor = Makai::toString("&[", sym().value["stack_id"].get<uint64>(), "]");
			Makai::String operation;
			switch (current.type) {
				case LTS_TT_ADD_ASSIGN: operation = " + "; break;
				case LTS_TT_SUB_ASSIGN: operation = " - "; break;
				case LTS_TT_MUL_ASSIGN: operation = " * "; break;
				case LTS_TT_DIV_ASSIGN: operation = " / "; break;
				case LTS_TT_MOD_ASSIGN: operation = " % "; break;
			}
			pre = PREASSIGN {
				context.writeLine("calc", accessor, operation, result.value, "-> .");
				result.value = ".";
			};
		} break;
	}
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed assignment!");
	auto const sym = context.symbol(id);
	auto const varType	= Makai::Cast::as<Value::Kind, int16>(sym().value["type"]);
	auto const accessor	= Makai::toString("&[", sym().value["stack_id"].get<uint64>(), "]");
	return {varType, accessor};
	doVarAssign(context, id, varType, false, false, pre);
}

BREVE_TYPED_ASSEMBLE_FN(FunctionCall) {
	auto const id = context.stream.current().value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		BREVE_ERROR(InvalidValue, "Function name cannot be a reserved keyword!");
	if (!context.hasSymbol(id))
		BREVE_ERROR(NonexistentValue, "Function does not exist!");
	if (context.getSymbolByName(id).type != decltype(context.getSymbolByName(id).type)::AV2_TA_SMT_FUNCTION)
		BREVE_ERROR(NonexistentValue, "Symbol was not declared a function!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed function call!");
	if (context.stream.current().type != Type{'('})
		BREVE_ERROR(InvalidValue, "Expected '(' here!");
	usize pushes = 0;
	Makai::List<Solution> args;
	auto const start = context.currentScope().stackc + context.currentScope().varc;
	auto legalName = id + "_";
	while (context.stream.next()) {
		if (context.stream.current().type == Type{')'}) break;
		args.pushBack(doValueResolution(context));
		legalName += "_" + argname(args.back().key);
		if (args.back().value == ".") {
			context.writeLine("push .");
			args.back().value = Makai::toString("&[", start + pushes, "]");
			++pushes;
		}
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed function call!");
		if (context.stream.current().type == Type{','})
			BREVE_ERROR(InvalidValue, "Expected ',' here!");
	}
	if (context.stream.current().type != Type{')'})
		BREVE_ERROR(InvalidValue, "Expected ')' here!");
	Makai::String call = "( ";
	for (auto const& [arg, index]: Makai::Range::expand(args))
		call += Makai::toString(index, "=", arg.value) + " ";
	call += ")";
	auto const sym = context.symbol(id);
	if (!sym().value["overloads"].contains(legalName))
		BREVE_ERROR(InvalidValue, "Function overload does not exist!");
	auto const overload = sym().value["overloads"][legalName];
	context.writeLine("call", overload["full_name"].get<Makai::String>(), call);
	context.writeLine("clear", pushes);
	return {
		Makai::Cast::as<Value::Kind, int16>(overload["return"]),
		"."
	};
}

BREVE_ASSEMBLE_FN(Assembly) {
	if (context.currentScope().secure)
		BREVE_ERROR(NonexistentValue, "Assembly is only allowed in a [fatal] context!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed assembly!");
	if (context.stream.current().type != Type{'{'})
		BREVE_ERROR(NonexistentValue, "Expected '{' here!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed assembly!");
	while (context.stream.current().type != Type{'}'}) {
		context.writeLine(context.stream.tokenText());
		context.stream.next();
	}
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed variable!");
}

BREVE_ASSEMBLE_FN(LooseContext) {
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed context declaration!");
	context.startScope();
	context.currentScope().secure = false;
	doExpression(context);
	context.currentScope().secure = true;
	context.endScope();
}

BREVE_ASSEMBLE_FN(Return) {
	if (!context.inFunction())
		BREVE_ERROR(InvalidValue, "Cannot have returns outside of functions!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed return!");
	Solution result = {Value::Kind::DVK_VOID};
	auto const expectedType = context.functionScope().result;
	if (context.stream.current().type == Type{';'}) {
		if (expectedType != Value::Kind::DVK_VOID)
			BREVE_ERROR(NonexistentValue, "Missing return value!");
	} else {
		if (expectedType == Value::Kind::DVK_VOID)
			BREVE_ERROR(InvalidValue, "Function does not return a value!");
		result = doValueResolution(context);
		if (
			result.key != expectedType
		&&	!Value::isNumber(stronger(result.key, expectedType))
		) BREVE_ERROR(InvalidValue, "Return type does not match!");
	}
	context.addFunctionExit();
	if (expectedType == Value::Kind::DVK_VOID)
		context.writeLine("end");
	else context.writeLine("ret", result.value);
}

BREVE_ASSEMBLE_FN(Main) {
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed main!");
	if (context.hasMain)
		BREVE_ERROR(NonexistentValue, "Only one entrypoint is allowed!");
	if (!context.inGlobalScope())
		BREVE_ERROR(NonexistentValue, "Main can only be declared on the global scope!");
	context.hasMain = true;
	if (context.stream.current().type != Type{'{'})
		BREVE_ERROR(InvalidValue, "Expected '{' here!");
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed main!");
	context.writeLine(context.main.entryPoint, ":");
	context.startScope(Context::Scope::Type::AV2_TA_ST_FUNCTION);
	doScope(context);
	context.endScope();
	if (context.stream.current().type != Type{'}'})
		BREVE_ERROR(InvalidValue, "Expected '}' here!");
}

BREVE_ASSEMBLE_FN(Conditional) {
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed conditional!");
	auto const scopeName = context.scopePath() + context.uniqueName() + "_if";
	auto const ifTrue	= scopeName + "_true";
	auto const ifFalse	= scopeName + "_false";
	auto const endIf	= scopeName + "_end";
	auto const val = doValueResolution(context);
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed conditional!");
	context.writeLine("jump if true", val.value, ifTrue);
	context.writeLine("jump if false", val.value, ifFalse);
	context.writeLine(ifTrue, ":");
	context.startScope();
	doExpression(context);
	context.endScope();
	context.writeLine("jump", endIf);
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed conditional!");
	if (context.stream.current().type == LTS_TT_IDENTIFIER) {
		auto const id = context.stream.current().value.get<Makai::String>();
		if (id == "else") {
			if (!context.stream.next())
				BREVE_ERROR(NonexistentValue, "Malformed conditional!");
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

BREVE_ASSEMBLE_FN(ForLoop) {}

BREVE_ASSEMBLE_FN(RepeatLoop) {}

BREVE_ASSEMBLE_FN(DoLoop) {}

BREVE_ASSEMBLE_FN(Terminate) {}

BREVE_ASSEMBLE_FN(Error) {}

BREVE_TYPED_ASSEMBLE_FN(UnaryOperation) {
	auto const current = context.stream.current();
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed unary value!");
	auto result = doValueResolution(context);
	switch (current.type) {
		case Type{'-'}: {
			if (!Value::isNumber(result.key))
				BREVE_ERROR(NonexistentValue, "Negation can only happen on numbers!");
			context.writeLine("umath -", result.value, "-> .");
			result.value = ".";
		} break;
		case Type{'+'}: {
			if (!Value::isNumber(result.key))
				BREVE_ERROR(NonexistentValue, "Positration can only happen on numbers!");
			context.writeLine("copy", result.value, "-> .");
			result.value = ".";
		} break;
		case LTS_TT_DECREMENT: {
			if (!Value::isNumber(result.key))
				BREVE_ERROR(NonexistentValue, "Incrementation can only happen on numbers!");
			context.writeLine("umath inc", result.value, "-> .");
			result.value = ".";
		} break;
		case LTS_TT_INCREMENT: {
			if (!Value::isNumber(result.key))
				BREVE_ERROR(NonexistentValue, "Decrementation can only happen on numbers!");
			context.writeLine("umath dec", result.value, "-> .");
			result.value = ".";
		} break;
	}
	return result;
}

BREVE_ASSEMBLE_FN(Module) {
	if (!context.inGlobalScope())
		BREVE_ERROR(InvalidValue, "Module imports/exports can only be declared in the global scope!");
	auto const type = context.stream.current().value.get<Makai::String>();
}

BREVE_ASSEMBLE_FN(Namespace) {
	if (!context.inNamespace())
		BREVE_ERROR(InvalidValue, "You can only declare sub-namespaces inside other namespaces!");
	usize scopeCount = 0;
	while (context.stream.current().type == Type{'.'}) {
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed namespace!");
		if (context.stream.current().type != LTS_TT_IDENTIFIER)
			BREVE_ERROR(NonexistentValue, "Expected identifier for namespace name!");
		auto const id = context.stream.current().value.get<Makai::String>();
		if (context.isReservedKeyword(id))
			BREVE_ERROR(InvalidValue, "Namespace name cannot be a reserved keyword!");
		if (context.currentNamespace().hasChild(id))
			BREVE_ERROR(InvalidValue, "Namespace with this name already exists!");
		auto& ns = context.currentNamespace();
		context.startScope(Context::Scope::Type::AV2_TA_ST_NAMESPACE);
		auto& scope = context.currentScope();
		scope.name		=
		scope.ns->name	= id;
		ns.addChild(context.currentScope().ns);
		++scopeCount;
		if (!context.stream.next())
			BREVE_ERROR(NonexistentValue, "Malformed namespace!");
		if (context.stream.current().type == Type {'{'})
			break;
		if (context.stream.current().type != Type{'.'})
			BREVE_ERROR(NonexistentValue, "Expected '.' here!!");
	}
	if (context.stream.current().type != Type {'{'})
		BREVE_ERROR(NonexistentValue, "Expected '{' here!");
	doExpression(context);
	if (!context.stream.next())
		BREVE_ERROR(NonexistentValue, "Malformed namespace!");
	if (context.stream.current().type != Type {'}'})
		BREVE_ERROR(NonexistentValue, "Expected '}' here!");
	while (scopeCount--)
		context.endScope();
}

BREVE_ASSEMBLE_FN(Expression) {
	auto const current = context.stream.current();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			if (id == "function" || id == "func" || id == "fn")	doFunction(context);
			else if (id == "external" || id == "out")			doExternal(context);
			else if (id == "internal" || id == "in")			doInternal(context);
			else if (id == "namespace" || id == "module")		doNamespace(context);
			else if (id == "export" || id == "import")			doModule(context);
			else if (id == "global" || id == "local")			doVarDecl(context);
			else if (id == "minima" || id == "asm")				doAssembly(context);
			else if (id == "fatal")								doLooseContext(context);
			else if (id == "return")							doReturn(context);
			else if (id == "if")								doConditional(context);
			else if (id == "do")								doDoLoop(context);
			else if (id == "for")								doForLoop(context);
			else if (id == "repeat")							doRepeatLoop(context);
			else if (id == "main")								doMain(context);
			else if (id == "terminate")							doTerminate(context);
			else if (id == "error")								doError(context);
			else if (context.hasSymbol(id)) {
				auto const sym = context.symbol(id);
				switch (sym().type) {
					case decltype(sym().type)::AV2_TA_SMT_FUNCTION: doFunctionCall(context);
					case decltype(sym().type)::AV2_TA_SMT_VARIABLE: doAssignment(context);
					default: BREVE_ERROR(InvalidValue, "Invalid/Unsupported expression!");
				}
			}
			else BREVE_ERROR(InvalidValue, "Invalid/Unsupported expression!");
		} break;
		case Type{'('}: {
			doBinaryOperation(context);
		} break;
		case Type{'-'}:
		case Type{'+'}:
		case LTS_TT_DECREMENT:
		case LTS_TT_INCREMENT: {
			doUnaryOperation(context);
		} break;
		case Type{'{'}: {
			if (!context.stream.next())
				BREVE_ERROR(NonexistentValue, "Malformed expression!");
			context.startScope();
			doScope(context);
			context.endScope();
		}
		case Type{'}'}:
		case Type{';'}: break;
		default: BREVE_ERROR(InvalidValue, "Invalid expression!");
	}
	if (context.stream.current().type != Type{';'} || context.stream.current().type != Type{'}'})
		BREVE_ERROR(InvalidValue, "Expected closure here!");
}

void Breve::assemble() {
	context.writeGlobalPreamble("call", context.main.pre, "()");
	context.writeGlobalPreamble("call", context.main.entryPoint, "()");
	context.writeGlobalPreamble("call", context.main.post, "()");
	context.writeGlobalPreamble("clear");
	context.writeGlobalPreamble("halt");
	context.writeMainPreamble(context.main.preEntryPoint, ":");
	context.writeMainPostscript(context.main.postEntryPoint, ":");
	while (context.stream.next()) doExpression(context);
	context.writeMainPreamble("end");
	context.writeMainPostscript("end");
	if (!context.hasMain)
		BREVE_ERROR(NonexistentValue, "Missing main entrypoint!");
}

CTL_DIAGBLOCK_END