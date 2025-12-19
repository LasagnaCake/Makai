#include "maxima.hpp"
#include "context.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = Maxima::TokenStream::Token::Type;
using enum Type;
using Value = Makai::Data::Value;

using Solution = Makai::KeyValuePair<Value::Kind, Makai::String>;

#define MAXIMA_ASSEMBLE_FN(NAME) static void do##NAME (Maxima::Context& context)
#define MAXIMA_TYPED_ASSEMBLE_FN(NAME) static Solution do##NAME (Maxima::Context& context)

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

#define MAXIMA_ERROR(TYPE, WHAT) error<Makai::Error::TYPE>({WHAT}, context)

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

MAXIMA_ASSEMBLE_FN(Scope);
MAXIMA_ASSEMBLE_FN(Expression);
MAXIMA_ASSEMBLE_FN(Return);
MAXIMA_ASSEMBLE_FN(Conditional);
MAXIMA_ASSEMBLE_FN(ForLoop);
MAXIMA_ASSEMBLE_FN(WhileLoop);
MAXIMA_ASSEMBLE_FN(RepeatLoop);
MAXIMA_ASSEMBLE_FN(DoLoop);
MAXIMA_ASSEMBLE_FN(Main);
MAXIMA_TYPED_ASSEMBLE_FN(FunctionCall);
MAXIMA_TYPED_ASSEMBLE_FN(Assignment);
MAXIMA_TYPED_ASSEMBLE_FN(ReservedValueResolution);
MAXIMA_TYPED_ASSEMBLE_FN(BinaryOperation);
MAXIMA_TYPED_ASSEMBLE_FN(ValueResolution);

static bool isReserved(Makai::String const& keyword);

static void doDefaultValue(Maxima::Context& context, Makai::String const& var, Makai::String const& uname) {
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed value definition!");
	auto const dvloc = "__" + context.scopePath() + "_" + var + "_set_default" + uname;
	context.getSymbolByName(var).value["default_setter"] = dvloc;
	auto dv = dvloc + ":\n";
	doValueResolution(context);
	dv += "push &["+ Makai::toString(context.getSymbolByName(var).value["stack_id"].get<uint64>()) +"]\nend\n";
	context.ir = dv + context.ir;
}

constexpr auto const DVK_ANY = Makai::Cast::as<decltype(Makai::Data::Value::Kind::DVK_VOID)>(-1);

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
		default: MAXIMA_ERROR(InvalidValue, "Invalid/Unsupported type!");
	}
	MAXIMA_ERROR(InvalidValue, "Invalid/Unsupported type!");
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

MAXIMA_ASSEMBLE_FN(Function) {
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed function!");
	auto const fname = context.stream.current();
	if (fname.type != Type::LTS_TT_IDENTIFIER)
		MAXIMA_ERROR(InvalidValue, "Function name must be an identifier!");
	auto const fid = fname.value.get<Makai::String>();
	auto id = fid;
	auto args = Makai::Data::Value::array();
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed function!");
	if (context.stream.current().type != Type{'('})
		MAXIMA_ERROR(NonexistentValue, "Expected '(' here!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed function!");
	context.startScope();
	CTL::Ex::Data::Value::Kind retType = DVK_ANY;
	id += "_";
	auto const signature = context.uniqueName();
	Makai::List<Makai::KeyValuePair<Makai::String, Value>> optionals;
	bool inOptionalRegion = false;
	while (context.stream.next() && context.stream.current().type != Type{')'}) {
		bool isOptional = false;
		auto const argn = context.stream.current();
		if (argn.type != Type::LTS_TT_IDENTIFIER)
			MAXIMA_ERROR(InvalidValue, "Argument name must be an identifier!");
		auto const argID = argn.value.get<Makai::String>();
		if (context.isReservedKeyword(argID))
			MAXIMA_ERROR(InvalidValue, "Argument name cannot be a reserved keyword!");
		if (!context.currentScope().contains(argID))
			context.currentScope().addVariable(argID);
		else MAXIMA_ERROR(InvalidValue, "Argument with this name already exists!");
		if (!context.stream.next())
			MAXIMA_ERROR(NonexistentValue, "Malformed function argument list!");
		if (context.stream.current().type != Type{':'})
			MAXIMA_ERROR(InvalidValue, "Expected ':' here!");
		if (!context.stream.next())
			MAXIMA_ERROR(NonexistentValue, "Malformed function argument list!");
		auto const argt = getType(context);
		if (argt == CTL::Ex::Data::Value::Kind::DVK_UNDEFINED)
			MAXIMA_ERROR(InvalidValue, "Invalid argument type!");
		auto& var = context.currentScope().members[argID].value;
		if (!context.stream.next())
			MAXIMA_ERROR(NonexistentValue, "Malformed function argument list!");
		if (context.stream.current().type == Type{')'})
			break;
		if (context.stream.current().type == Type{'='}) {
			isOptional = true;
			inOptionalRegion = true;
			doDefaultValue(context, argID, signature);
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
			MAXIMA_ERROR(NonexistentValue, "Missing value for optional argument!");
		if (context.stream.current().type != Type{','})
			MAXIMA_ERROR(InvalidValue, "Expected ',' here!");
	}
	if (context.stream.current().type != Type{')'})
		MAXIMA_ERROR(InvalidValue, "Expected ')' here!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed function!");
	if (context.stream.current().type == Type{':'}) {
		if (!context.stream.next())
			MAXIMA_ERROR(NonexistentValue, "Malformed function!");
		retType = getType(context);
		if (!context.stream.next())
			MAXIMA_ERROR(NonexistentValue, "Malformed function!");
	}
	if (context.stream.current().type != Type{'{'})
		MAXIMA_ERROR(InvalidValue, "Expected '{' here!");
	context.currentScope().result	= retType;
	context.currentScope().label	= fid;
	auto const baseName =
		context.scopePath()
	+	signature
	+	"_" + id
	;
	auto resolutionName = id;
	auto fullName = baseName;
	for (auto& opt: optionals)
		fullName += "_" + opt.value["type"].get<Makai::String>();
	context.writeLine(fullName, ":");
	doScope(context);
	auto const scopath = context.scopePath();
	context.writeLine("clear ", context.currentScope().varc);
	context.endScope();
	// TODO: Proper overloading
	auto subName = baseName;
	Makai::String postscript = "";
	for (auto& opt: Makai::Range::reverse(optionals)) {
		auto const dvloc = "__" + context.scopePath() + "_" + opt.key + "_set_default" + signature;
		Makai::String preamble = "";
		context.startScope();
		fullName = fullName.sliced(0, -(opt.value["type"].get<Makai::String>().size() + 2));
		opt.value["declname"] = fullName;
		preamble += Makai::toString(fullName, ":");
		preamble += Makai::toString("call " + dvloc + "()");
		postscript = preamble + postscript;
		context.endScope();
	}
	if (optionals.size()) {
		context.writeLine("call " + fullName + "()");
		context.writeLine("end");
	}
	if (!context.currentScope().contains(fid))
		context.currentScope().addFunction(fid);
	else if (context.currentScope().members[fid].type != Context::Scope::Member::Type::AV2_TA_SMT_FUNCTION)
		MAXIMA_ERROR(InvalidValue, "Symbol with this name already exists!");
	auto& mem = context.currentScope().members[fid];
	auto& overloads	= mem.value["overloads"];
	if (overloads.contains(resolutionName))
		MAXIMA_ERROR(InvalidValue, "Function with similar signature already exists!");
	auto& overload	= overloads[resolutionName];
	for (auto& opt: optionals)
		fullName += "_" + opt.value["type"].get<Makai::String>();
	overload["args"]		= args;
	overload["full_name"]	= fullName;
	overload["return"]		= Makai::enumcast(retType);
	for (auto& opt: optionals) {
		resolutionName += "_" + opt.key;
		if (overloads.contains(resolutionName))
			MAXIMA_ERROR(InvalidValue, "Function with similar signature already exists!");
		auto& overload	= overloads[resolutionName];
		args[args.size()]		= opt.value;
		overload["args"]		= args;
		overload["full_name"]	= opt.value["declname"];
		overload["return"]		= Makai::enumcast(retType);
	}
}

MAXIMA_ASSEMBLE_FN(Scope) {
	while (context.stream.next()) {
		auto const current = context.stream.current();
		if (current.type == Type{'}'}) break; 
		else doExpression(context);
	}
}

MAXIMA_TYPED_ASSEMBLE_FN(ValueResolution) {
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
		case Type{'+'}: {
			auto const negative = current.type == Type{'-'};
			// TODO: this
		} break;
		case LTS_TT_DECREMENT:
		case LTS_TT_INCREMENT: {
			auto const negative = current.type == LTS_TT_DECREMENT;
			// TODO: this
		} break;
		case LTS_TT_SINGLE_QUOTE_STRING: 
		case LTS_TT_DOUBLE_QUOTE_STRING:	return {Value::Kind::DVK_STRING,	current.value.toString()								};
		case LTS_TT_CHARACTER: 				return {Value::Kind::DVK_STRING,	Makai::toString("'", current.value.get<char>(), "'")	};
		case LTS_TT_INTEGER:				return {Value::Kind::DVK_UNSIGNED,	current.value.toString()								};
		case LTS_TT_REAL:					return {Value::Kind::DVK_REAL,		current.value.toString()								};
		default: MAXIMA_ERROR(InvalidValue, "Invalid expression!");
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

static Value::Kind handleTernary(Maxima::Context& context, Solution const& cond, Solution const& ifTrue, Solution const& ifFalse) {
	auto const result = stronger(ifTrue.key, ifFalse.key);
	if (Value::isUndefined(cond.key))
		MAXIMA_ERROR(InvalidValue, "Invalid condition type!");
	if (!Value::isVerifiable(cond.key))
		MAXIMA_ERROR(InvalidValue, "Condition must be a verifiable type!");
	auto const trueJump		= context.scopePath() + "_ternary_true"		+ context.uniqueName();
	auto const falseJump	= context.scopePath() + "_ternary_false"	+ context.uniqueName();
	auto const endJump		= context.scopePath() + "_ternary_end"		+ context.uniqueName();
	context.writeLine("jump if is " + cond.value + " " + trueJump);
	context.writeLine("jump if not " + cond.value + " " + falseJump);
	context.writeLine(trueJump + ":");
	context.writeLine("copy " + ifTrue.value + "-> .");
	context.writeLine("jump " + endJump);
	context.writeLine(falseJump + ":");
	context.writeLine("copy " + ifFalse.value + "-> .");
	context.writeLine("jump " + endJump);
	return result;
}

MAXIMA_TYPED_ASSEMBLE_FN(BinaryOperation) {
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed operation!");
	auto lhs = doValueResolution(context);
	usize stackUsage = 0;
	if (lhs.value == ".") {
		context.writeLine("push .");
		lhs.value = "&[-0]";
		++stackUsage;
	}
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed operation!");
	auto const opname = context.stream.current();
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed operation!");
	auto rhs = doValueResolution(context);
	if (rhs.value == ".") {
		context.writeLine("push .");
		rhs.value = "&[-0]";
		if (stackUsage++) lhs.value = "&[-1]";
	}
	auto result = stronger(lhs.key, rhs.key);
	if (Value::isUndefined(lhs.key) || Value::isUndefined(rhs.key))
		MAXIMA_ERROR(InvalidValue, "Invalid operand types!");
	switch (opname.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = opname.value.get<Makai::String>();
			if (id == "as") {
				if (!context.isCastable(rhs.key))
					MAXIMA_ERROR(InvalidValue, "Casts can only happen to scalar types!");
				context.writeLine("cast ", lhs.value, " : ", toTypeName(rhs.key), " -> .");
				result = rhs.key;
			} else if (id == "if") {
				if (!context.stream.next())
					MAXIMA_ERROR(NonexistentValue, "Malformed operation!");
				if (
					context.stream.current().type != LTS_TT_IDENTIFIER
				&&	context.stream.current().value.get<Makai::String>() != "else"
				) MAXIMA_ERROR(InvalidValue, "Expected 'else' here!");
				if (!context.stream.next())
					MAXIMA_ERROR(NonexistentValue, "Malformed operation!");
				auto const elseVal = doValueResolution(context);
				result = handleTernary(context, lhs, rhs, elseVal);
			} else MAXIMA_ERROR(InvalidValue, "Invalid/Unsupported operation!");
		} break;
		case Type{'+'}: {
			if (Value::isNumber(result)) context.writeLine("calc ", lhs.value, " + ", rhs.value, " -> .");
			else if (Value::isString(lhs.key) && Value::isString(rhs.key))
				context.writeLine("str cat ", lhs.value, " (", rhs.value, ") -> .");
			else MAXIMA_ERROR(InvalidValue, "Invalid expression type(s) for operation!");
		} break;
		case Type{'/'}: {
			if (Value::isNumber(result)) context.writeLine("calc ", lhs.value, " / ", rhs.value, " -> .");
			else if (Value::isString(result)) 
				context.writeLine("str sep ", lhs.value, " (", rhs.value, ") -> .");
			else MAXIMA_ERROR(InvalidValue, "Invalid expression type(s) for operation!");
		};
		case Type{'-'}:
		case Type{'*'}:
		case Type{'%'}: {
			auto const opstr = Makai::toString(Makai::Cast::as<char>(opname.type));
			if (Value::isNumber(result)) context.writeLine("calc ", lhs.value, " ", opstr, " -> .");
			else MAXIMA_ERROR(InvalidValue, "Invalid expression type(s) for operation!");
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
				MAXIMA_ERROR(InvalidValue, "Left-hand side MUST be an object!");
			if (!Value::isString(rhs.key))
				MAXIMA_ERROR(InvalidValue, "Right-hand side MUST be a string!");
			context.writeLine("get &[", lhs.value, "][&[", rhs.value, "]] -> .");
			result = DVK_ANY;
			if (!context.stream.next())
				MAXIMA_ERROR(NonexistentValue, "Malformed operation!");
			if (context.stream.current().type != Type{']'})
				MAXIMA_ERROR(InvalidValue, "Expected ']' here!");
			if (!context.stream.next())
				MAXIMA_ERROR(NonexistentValue, "Malformed operation!");
		} break;
		case Type{'='}: {
			if (lhs.key != rhs.key) {
				if (context.isCastable(result)) {
					context.writeLine("cast ", rhs.value, " : ", toTypeName(lhs.key), " -> .");
					context.writeLine("copy . -> ", lhs.value);
				} else MAXIMA_ERROR(InvalidValue, "Types are not convertible to each other!");
			}
			context.writeLine("copy ", rhs.value, " -> ", lhs.value);
		}
		default: MAXIMA_ERROR(InvalidValue, "Invalid/Unsupported operation!");
	}
	if (context.stream.current().type != Type{')'})
		MAXIMA_ERROR(InvalidValue, "Expected ')' here!");
	for (usize i = 0; i < stackUsage; ++i)
		context.writeLine("pop void");
	return {result, "."};
}

MAXIMA_TYPED_ASSEMBLE_FN(ReservedValueResolution) {
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

using PreAssignFunction = Makai::Functor<void(Maxima::Context&, Solution&)>;

void doVarAssign(
	Maxima::Context& context,
	Makai::String const& id,
	Value::Kind const& type,
	bool const isGlobalVar = false,
	bool const isNewVar = false,
	PreAssignFunction const& preassign = {}
) {
	auto result = doValueResolution(context);
	if (result.key != type) {
		if (context.isCastable(result.key))
			MAXIMA_ERROR(InvalidValue, "Invalid expression type for assignment!");
		context.writeLine("cast ", result.value, " : ", toTypeName(type), " -> .");
		result.value = ".";
	}
	if (isNewVar) {
		if (context.currentScope().contains(id)) {
			auto const sym = context.currentScope().members[id];
			if (sym.type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
				MAXIMA_ERROR(InvalidValue, "Symbol has already been defined as a different type in a previous scope!");
			else if (isGlobalVar && sym.value["global"] && Makai::Cast::as<Value::Kind, int16>(sym.value["type"]) != type)
				MAXIMA_ERROR(InvalidValue, "Global variable expression does not match its prevoius type!");
		} else {
			if (context.currentScope().contains(id))
				MAXIMA_ERROR(InvalidValue, "Variable already exists in current scope!");
			else context.currentScope().addVariable(id, isGlobalVar);
		}
	} else {
		if (!context.hasSymbol(id))
			MAXIMA_ERROR(InvalidValue, "Variable does not exist in the current scope!");
		auto const sym = context.currentScope().members[id];
		if (sym.type != Context::Scope::Member::Type::AV2_TA_SMT_VARIABLE)
			MAXIMA_ERROR(InvalidValue, "Symbol has already been defined as a different type in a previous scope!");
	}
	auto const sym = context.symbol(id);
	preassign(context, result);
	if (isGlobalVar)
		context.writeLine("copy ", result.value, " -> :", id);
	else context.writeLine("copy ", result.value, " -> &[", sym().value["stack_id"].get<uint64>(), "]");
	sym().value["init"] = true;
}

void doVarDecl(Maxima::Context& context, Makai::String const& id, bool const isGlobalVar = false) {
	if (context.stream.current().type != Type{':'})
		MAXIMA_ERROR(InvalidValue, "Expected ':' here!");
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
			MAXIMA_ERROR(NonexistentValue, "Malformed variable!");
	}
	if (context.stream.current().type == Type{'='}) {
		if (!context.stream.next())
			MAXIMA_ERROR(NonexistentValue, "Malformed variable!");
		doVarAssign(context, id, type, isGlobalVar, true);
	}
	if (context.stream.current().type != Type{';'})
		MAXIMA_ERROR(InvalidValue, "Expected ';' here!");
}

MAXIMA_ASSEMBLE_FN(VarDecl) {
	bool const isGlobalVar = context.stream.current().value.get<Makai::String>() == "global";
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed variable!");
	auto const varname = context.stream.current();
	if (varname.type != LTS_TT_IDENTIFIER)
		MAXIMA_ERROR(InvalidValue, "Variable name must be an identifier!");
	auto const id = varname.value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		MAXIMA_ERROR(InvalidValue, "Variable name cannot be a reserved keyword!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed variable!");
	if (isGlobalVar)
		context.writePreamble("push null");
	doVarDecl(context, id, isGlobalVar);
}

#define PREASSIGN [=] (Maxima::Context& context, Solution& result) -> void

MAXIMA_TYPED_ASSEMBLE_FN(Assignment) {
	auto const id = context.stream.current().value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		MAXIMA_ERROR(InvalidValue, "Variable name cannot be a reserved keyword!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed variable!");
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
				context.writeLine("calc ", accessor, operation, result.value, " -> .");
				result.value = ".";
			};
		} break;
	}
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed assignment!");
	auto const sym = context.symbol(id);
	auto const varType	= Makai::Cast::as<Value::Kind, int16>(sym().value["type"]);
	auto const accessor	= Makai::toString("&[", sym().value["stack_id"].get<uint64>(), "]");
	return {varType, accessor};
	doVarAssign(context, id, varType, false, false, pre);
}

MAXIMA_TYPED_ASSEMBLE_FN(FunctionCall) {
	// TODO: This
	auto const id = context.stream.current().value.get<Makai::String>();
	if (context.isReservedKeyword(id))
		MAXIMA_ERROR(InvalidValue, "Function name cannot be a reserved keyword!");
	if (!context.hasSymbol(id))
		MAXIMA_ERROR(NonexistentValue, "Function does not exist!");
	if (context.getSymbolByName(id).type != decltype(context.getSymbolByName(id).type)::AV2_TA_SMT_FUNCTION)
		MAXIMA_ERROR(NonexistentValue, "Symbol was not declared a function!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed function call!");
	if (context.stream.current().type != Type{'('})
		MAXIMA_ERROR(InvalidValue, "Expected '(' here!");
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
			MAXIMA_ERROR(NonexistentValue, "Malformed function call!");
		if (context.stream.current().type == Type{','})
			MAXIMA_ERROR(InvalidValue, "Expected ',' here!");
	}
	if (context.stream.current().type != Type{')'})
		MAXIMA_ERROR(InvalidValue, "Expected ')' here!");
	Makai::String call = "( ";
	for (auto const& [arg, index]: Makai::Range::expand(args))
		call += Makai::toString(index, "=", arg.value) + " ";
	call += ")";
	auto const sym = context.symbol(id);
	if (!sym().value["overloads"].contains(legalName))
		MAXIMA_ERROR(InvalidValue, "Function overload does not exist!");
	context.writeLine("call " + sym().value["overloads"][legalName]["full_name"].get<Makai::String>() + call);
}

MAXIMA_ASSEMBLE_FN(Assembly) {
	if (context.currentScope().secure)
		MAXIMA_ERROR(NonexistentValue, "Assembly is only allowed in a [fatal] context!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed assembly!");
	if (context.stream.current().type != Type{'{'})
		MAXIMA_ERROR(NonexistentValue, "Expected '{' here!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed assembly!");
	while (context.stream.current().type != Type{'}'}) {
		context.writeLine(context.stream.tokenText());
		context.stream.next();
	}
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed variable!");
}

MAXIMA_ASSEMBLE_FN(LooseContext) {
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed context declaration!");
	context.startScope();
	context.currentScope().secure = false;
	doExpression(context);
	context.currentScope().secure = true;
	context.endScope();
}

MAXIMA_ASSEMBLE_FN(Return) {
	// TODO: This
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed return!");
	Solution result;
	auto const expectedType = context.currentScope().result;
	if (context.stream.current().type == Type{';'}) {
		if (expectedType != Value::Kind::DVK_VOID)
			MAXIMA_ERROR(NonexistentValue, "Missing return value!");
	} else {
		if (expectedType == Value::Kind::DVK_VOID)
			MAXIMA_ERROR(InvalidValue, "Function does not return a value!");
		result = doValueResolution(context);
		if (
			result.key != expectedType
		&&	!Value::isNumber(stronger(result.key, expectedType))
		) MAXIMA_ERROR(InvalidValue, "Return type does not match!");
	}
}

MAXIMA_ASSEMBLE_FN(Main) {
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed main!");
	if (context.hasMain)
		MAXIMA_ERROR(NonexistentValue, "Only one entrypoint is allowed!");
	if (context.scope.size() > 1)
		MAXIMA_ERROR(NonexistentValue, "Main can only be declared on the global scope!");
	context.hasMain = true;
	if (context.stream.current().type != Type{'{'})
		MAXIMA_ERROR(InvalidValue, "Expected '{' here!");
	if (!context.stream.next())
		MAXIMA_ERROR(NonexistentValue, "Malformed main!");
	context.writeLine("__main:");
	context.startScope();
	doScope(context);
	context.endScope();
	context.writeLine("halt");
	if (context.stream.current().type != Type{'}'})
		MAXIMA_ERROR(InvalidValue, "Expected '}' here!");
}

MAXIMA_ASSEMBLE_FN(Expression) {
	auto const current = context.stream.current();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			if (id == "function" || id == "func" || id == "fn")	doFunction(context);
			else if (id == "global" || id == "local")			doVarDecl(context);
			else if (id == "minima" || id == "asm")				doAssembly(context);
			else if (id == "fatal")								doLooseContext(context);
			else if (id == "return")							doReturn(context);
			else if (id == "if")								doConditional(context);
			else if (id == "do")								doDoLoop(context);
			else if (id == "for")								doForLoop(context);
			else if (id == "repeat")							doRepeatLoop(context);
			else if (id == "main")								doMain(context);
			else if (context.hasSymbol(id)) {
				auto const sym = context.symbol(id);
				switch (sym().type) {
					case decltype(sym().type)::AV2_TA_SMT_FUNCTION: doFunctionCall(context);
					case decltype(sym().type)::AV2_TA_SMT_VARIABLE: doAssignment(context);
					default: MAXIMA_ERROR(InvalidValue, "Invalid/Unsupported expression!");
				}
			}
			else MAXIMA_ERROR(InvalidValue, "Invalid/Unsupported expression!");
		}
		case Type{'{'}: {
			if (!context.stream.next())
				MAXIMA_ERROR(NonexistentValue, "Malformed expression!");
			context.startScope();
			doScope(context);
			context.endScope();
		}
		case Type{'}'}:
		case Type{';'}: break;
		default: MAXIMA_ERROR(InvalidValue, "Invalid expression!");
	}
	if (context.stream.current().type != Type{';'} || context.stream.current().type != Type{'}'})
		MAXIMA_ERROR(InvalidValue, "Expected closure here!");
}

void Maxima::assemble() {
	context.writeLine("jump __main");
	context.startScope();
	while (context.stream.next()) doExpression(context);
	context.endScope();
	if (!context.hasMain)
		MAXIMA_ERROR(NonexistentValue, "Missing main entrypoint!");
}

CTL_DIAGBLOCK_END