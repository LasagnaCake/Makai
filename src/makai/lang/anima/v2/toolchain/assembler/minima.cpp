#include "minima.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = Minima::TokenStream::Token::Type;
using enum Type;

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

#define MINIMA_ASSEMBLE_FN(NAME) static void do##NAME (Minima::Context& context)

struct Location {
	DataLocation	at;
	uint64			id;
};

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

#define MINIMA_ERROR(TYPE, WHAT) error<Makai::Error::TYPE>({WHAT}, context)

static Location getStack(Minima::Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing stack index!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	auto v = context.stream.current();
	bool fromTheBack;
	if (
		v.type == Type{'+'}
	||	v.type == Type{'-'}
	) {
		fromTheBack = v.type == Type{'-'};
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	}
	if ((v = context.stream.current()).type != Type::LTS_TT_INTEGER)
		MINIMA_ERROR(InvalidValue, "Stack index must be an integer!");
	Location loc{(fromTheBack ? DataLocation::AV2_DL_STACK_OFFSET : DataLocation::AV2_DL_STACK), v.value.get<usize>()};
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	return loc;
}

static Location getRegister(Minima::Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing register index!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	auto v = context.stream.current();
	bool fromTheBack;
	if (
		v.type == Type{'+'}
	||	v.type == Type{'-'}
	) {
		fromTheBack = v.type == Type{'-'};
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	}
	if ((v = context.stream.current()).type != Type::LTS_TT_INTEGER)
		MINIMA_ERROR(InvalidValue, "Register index must be an integer!");
	if (v.value.get<usize>() > 31)
		MINIMA_ERROR(InvalidValue, "Register index must be between 0 and 31!");
	Location loc{Makai::Anima::V2::asRegister(v.value.get<ssize>() + (fromTheBack ? Makai::Anima::V2::REGISTER_COUNT : 0)), -1};
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	return loc;
}

static Location getExtern(Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing external location name!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing external location name!");
	auto const name = context.stream.current();
	if (
		name.type != LTS_TT_INTEGER
	) MINIMA_ERROR(InvalidValue, "Expected integer for external location ID!");
	else return {
		DataLocation::AV2_DL_EXTERNAL,
		name.value.get<usize>()
	};
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing external location name!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
}

static Location getGlobal(Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing global variable name!");
	auto const name = context.stream.current();
	if (name.type != LTS_TT_IDENTIFIER)
		MINIMA_ERROR(InvalidValue, "Expected identifier for global variable name!");
	auto const id = name.value.get<Makai::String>();
	usize globalID = 0;
	if (context.globals.contains(id))
		globalID = context.globals[id];
	else {
		globalID = context.globals.size();
		context.globals[id] = globalID;
	}
	return {
		DataLocation::AV2_DL_GLOBAL,
		globalID
	};
}

static Location getConstantLocation(Minima::Context& context) {
	auto const current = context.stream.current();
	if (
		current.type == Type{'+'}
	||	current.type == Type{'-'}
	) {
		bool isNegative = current.type == Type{'-'};
		if (!context.stream.next()) {
			MINIMA_ERROR(NonexistentValue, "Missing value for unary operator!");
		}
		auto const v = context.stream.current();
		if (!(v.type == LTS_TT_INTEGER || v.type == LTS_TT_REAL))
			MINIMA_ERROR(InvalidValue, "Unary operator can only accept numbers!");
		if (v.type == LTS_TT_INTEGER && v.value.get<ssize>() == 0)
			return {
				DataLocation::AV2_DL_INTERNAL,
				5
			};
		else if (v.type == LTS_TT_REAL && v.value.get<double>() == 0)
			return {
				DataLocation::AV2_DL_INTERNAL,
				6
			};
		else  return {
			DataLocation::AV2_DL_CONST,
			context.program.constants.pushBack(
				(
					v.type == LTS_TT_INTEGER
				?	v.value.get<ssize>()
				:	v.value.get<double>()
				)
			*	(isNegative ? -1 : +1)
			).size() - 1
		};
	}
	return {
		DataLocation::AV2_DL_CONST,
		context.program.constants.pushBack(current.value).size() - 1
	};
}

static Location getDataLocation(Minima::Context& context) {
	auto const current = context.stream.current();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			if (id == "register" || id == "reg") {
				return getRegister(context);
			} else if (id == "stack") {
				return getStack(context);
			} else if (id == "external" || id == "extern" || id == "out") {
				return getExtern(context);
			} else if (id == "global" || id == "g") {
				return getGlobal(context);
			} else if (id == "temporary" || id == "temp") {
				return {DataLocation::AV2_DL_TEMPORARY, -1};
			} else if (id == "true") {
				return {DataLocation::AV2_DL_INTERNAL, 1};
			} else if (id == "false") {
				return {DataLocation::AV2_DL_INTERNAL, 0};
			} else if (id == "null") {
				return {DataLocation::AV2_DL_INTERNAL, 3};
			} else if (id == "undefined" || id == "void") {
				return {DataLocation::AV2_DL_INTERNAL, 2};
			} else if (id == "nan") {
				return {DataLocation::AV2_DL_INTERNAL, 4};
			} else MINIMA_ERROR(InvalidValue, "Invalid token for data location!"); 
		} break;
		case Type{'@'}:
			return getExtern(context);
		case Type{'$'}:
			return getRegister(context);
		case Type{'&'}:
			return getStack(context);
		case Type{':'}:
			return getGlobal(context);
		case Type{'.'}:
			return {DataLocation::AV2_DL_TEMPORARY, -1};
		case Type{'?'}:
			return {DataLocation::AV2_DL_INTERNAL, 2};
		case Type{'+'}:
		case Type{'-'}:
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
		case LTS_TT_CHARACTER:
		case LTS_TT_INTEGER:
		case LTS_TT_REAL: {
			return getConstantLocation(context);
		} break;
		default: {
			MINIMA_ERROR(InvalidValue, "Invalid token for data location!");
		};
	}
	MINIMA_ERROR(InvalidValue, "Invalid token for data location!");
}

static void doConditionalLeapType(Context& context, Instruction::Leap& leap) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed jump!");
	auto modifier = context.stream.current();
	switch (modifier.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = modifier.value.get<Makai::String>();
			if (id == "null")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_NULL;
			else if (id == "undefined" || id == "void")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_UNDEFINED;
			else if (id == "nan")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_NAN;
			else if (id == "not" || id == "false" || id == "falsy")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_FALSY;
			else if (id == "empty" || id == "_")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_NULL_OR_UNDEFINED;
			else if (id == "is" || id == "true" || id == "truthy")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_TRUTHY;
			else if (id == "negative" || id == "neg")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_NEGATIVE;
			else if (id == "positive" || id == "pos")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_POSITIVE;
			else if (id == "zero" || id == "z")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_ZERO;
			else if (id == "nonzero" || id == "nz")
				leap.type = Instruction::Leap::Type::AV2_ILT_IF_NOT_ZERO;
			else MINIMA_ERROR(InvalidValue, "Invalid jump type!");
		} break;
		case Type{'+'}:
		case Type{'>'}: {
			leap.type = Instruction::Leap::Type::AV2_ILT_IF_POSITIVE;
		} break;
		case Type{'?'}:
		case Type{'_'}: {
			leap.type = Instruction::Leap::Type::AV2_ILT_IF_NULL_OR_UNDEFINED;
		} break;
		case Type{'-'}:
		case Type{'<'}: {
			leap.type = Instruction::Leap::Type::AV2_ILT_IF_NEGATIVE;
		} break;
		case Type{'.'}:
		case Type{'='}: {
			leap.type = Instruction::Leap::Type::AV2_ILT_IF_TRUTHY;
		} break;
		case Type{'!'}: {
			leap.type = Instruction::Leap::Type::AV2_ILT_IF_FALSY;
		} break;
		case Type::LTS_TT_INTEGER: {
			auto const num = modifier.value.get<usize>();
			leap.type = num ? Instruction::Leap::Type::AV2_ILT_IF_NOT_ZERO : Instruction::Leap::Type::AV2_ILT_IF_ZERO;
		} break;
		default: MINIMA_ERROR(InvalidValue, "Unexpected token!");
	}
	auto const loc = getDataLocation(context);
	leap.condition = loc.at;
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
}

void doLeapType(Context& context, Instruction::Leap& leap) {
	auto loc = context.stream.current();
	if (!leap.isDynamic && loc.type == Type{';'})
		MINIMA_ERROR(NonexistentValue, "Malformed jump!");
	switch (loc.type) {
		case Type{'?'}: {
			doConditionalLeapType(context, leap);
		} break;
		case LTS_TT_IDENTIFIER: {
			auto const id = loc.value.get<Makai::String>();
			if (id == "if") doConditionalLeapType(context, leap);
			else goto TheRestOfTheLeap; 
		}
		default: MINIMA_ERROR(InvalidValue, "Unexpected token!");
	}
	TheRestOfTheLeap:
	if (!leap.isDynamic) {
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed jump!");
		auto const name = context.stream.current();
		if (name.type != LTS_TT_IDENTIFIER)
			MINIMA_ERROR(InvalidValue, "Expected identifier for jump label!");
		context.addJumpTarget(name.value.get<Makai::String>());
	}
}

void doDynamicLeap(Context& context, Instruction::Leap& leap) {
	leap.isDynamic = true;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed jump!");
	auto const loc = getDataLocation(context);
	leap.condition = loc.at;
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
	doLeapType(context, leap);
}

MINIMA_ASSEMBLE_FN(Jump) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed jump!");
	Instruction::Leap leap = {
		.type = Instruction::Leap::Type::AV2_ILT_UNCONDITIONAL,
		.isDynamic = false
	};
	auto const index = context.program.code.size();
	context.program.code.pushBack({});
	auto loc = context.stream.current();
	switch (loc.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = loc.value.get<Makai::String>();
			if (id == "dynamic" || id == "dyn") {
				doDynamicLeap(context, leap);
			} else doLeapType(context, leap);
		}
		case Type{'&'}: doDynamicLeap(context, leap);
		default: {
			if (!context.stream.next())
				MINIMA_ERROR(NonexistentValue, "Malformed jump!");
			doLeapType(context, leap);
		}
	}
	context.program.code[index] = {
		Instruction::Name::AV2_IN_JUMP,
		Makai::Cast::bit<uint32>(leap)
	};
}

MINIMA_ASSEMBLE_FN(NoOp) {
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_NO_OP,
		0
	});
}

MINIMA_ASSEMBLE_FN(StackSwap) {
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_STACK_SWAP,
		0
	});
}

MINIMA_ASSEMBLE_FN(StackFlush) {
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_STACK_FLUSH,
		0
	});
}

MINIMA_ASSEMBLE_FN(StackClear) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack clear!");
	auto const count = context.stream.current();
	if (count.type != Type::LTS_TT_INTEGER && count.value.isSigned())
		MINIMA_ERROR(InvalidValue, "Stack count must be an unsigned integer!");
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_STACK_CLEAR,
		count.value
	});
}

MINIMA_ASSEMBLE_FN(StackPush) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack push!");
	auto const loc = getDataLocation(context);
	Instruction inst = {
		Instruction::Name::AV2_IN_STACK_PUSH, 
		Makai::Cast::bit<uint32>(Instruction::StackPush{loc.at})
	};
	context.program.code.pushBack(inst);
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
}

MINIMA_ASSEMBLE_FN(StackPop) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack pop!");
	auto loc = getDataLocation(context);
	Instruction inst = {Instruction::Name::AV2_IN_STACK_POP};
	if (loc.at == DataLocation::AV2_DL_INTERNAL) {
		inst.type = 0;
		loc.id = -1;
	}
	else inst.type = Makai::Cast::bit<uint32>(Instruction::StackPop{loc.at, true});
	context.program.code.pushBack(inst);
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
}

MINIMA_ASSEMBLE_FN(Return) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed return!");
	auto loc = getDataLocation(context);
	Instruction::Result res = {loc.at, true};
	Instruction inst = {Instruction::Name::AV2_IN_RETURN};
	if (loc.at == DataLocation::AV2_DL_INTERNAL && loc.id == 2) {
		res.ignore = true;
		loc.id = -1;
	}
	inst.type = Makai::Cast::bit<uint32>(res);
	context.program.code.pushBack(inst);
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
}

MINIMA_ASSEMBLE_FN(EmptyReturn) {
	context.program.code.pushBack({Instruction::Name::AV2_IN_RETURN, 0});
}

MINIMA_ASSEMBLE_FN(InternalCall) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed internal call!");
	auto const func = context.stream.current();
	Instruction::Invocation invoke{DataLocation::AV2_DL_INTERNAL, 0};
	switch (func.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = func.value.get<Makai::String>();
			if (id == "add")							invoke.argc = '+';
			else if (id == "subtract" || id == "sub")	invoke.argc = '-';
			else if (id == "multiply" || id == "mul")	invoke.argc = '*';
			else if (id == "divide" || id == "div")		invoke.argc = '/';
			else if (id == "power" || id == "pow")		invoke.argc = 'p';
			else if (id == "remainder" || id == "rem")	invoke.argc = '%';
			else if (id == "compare" || id == "cmp")	invoke.argc = '=';
			else if (id == "negate" || id == "neg")		invoke.argc = 'n';
			else if (id == "band")						invoke.argc = '&';
			else if (id == "bor")						invoke.argc = '|';
			else if (id == "bxor")						invoke.argc = '^';
			else if (id == "bnot")						invoke.argc = '~';
			else if (id == "land" || id == "and")		invoke.argc = 'a';
			else if (id == "lor" || id == "or")			invoke.argc = 'o';
			else if (id == "lnot" || id == "not")		invoke.argc = '!';
			else if (id == "sin")						invoke.argc = 's';
			else if (id == "cos")						invoke.argc = 'c';
			else if (id == "tan")						invoke.argc = 't';
			else if (id == "arcsin" || id == "asin")	invoke.argc = 'S';
			else if (id == "arccos" || id == "acos")	invoke.argc = 'C';
			else if (id == "arctan" || id == "atan")	invoke.argc = 'T';
			else if (id == "atan2" || id == "a2")		invoke.argc = '2';
			else if (id == "interrupt" || id == "stop")	invoke.argc = '.';
			else if (id == "access" || id == "read")	invoke.argc = ':';
			else MINIMA_ERROR(InvalidValue, "Invalid internal call!");
		}
		case Type{'+'}:
		case Type{'-'}:
		case Type{'*'}:
		case Type{'/'}:
		case Type{'%'}:
		case Type{'&'}:
		case Type{'|'}:
		case Type{'~'}:
		case Type{'!'}:
		case Type{'='}:
		case Type{'.'}: {
			invoke.argc = Makai::Cast::as<uint8>(func.type);
		} break;
		case Type::LTS_TT_LOGIC_AND: {
			invoke.argc = 'a';
		} break;
		case Type::LTS_TT_LOGIC_OR: {
			invoke.argc = 'o';
		} break;
		case Type::LTS_TT_COMPARE_EQUALS: {
			invoke.argc = '=';
		} break;
		default:
			MINIMA_ERROR(InvalidValue, "Invalid internal function call!");
	}
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_CALL,
		Makai::Cast::bit<uint32>(invoke)
	});
}

MINIMA_ASSEMBLE_FN(Call) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed function call!");
	auto const func = context.stream.current();
	if (func.type != LTS_TT_IDENTIFIER)
		MINIMA_ERROR(InvalidValue, "Function call must be an identifier!");
	auto fname = func.value.get<Makai::String>();
	Instruction::Invocation invoke;
	if (fname == "external" || fname == "extern" || fname == "out")
		return doInternalCall(context);
	else if (fname == "internal" || fname == "intern" || fname == "in") {
		invoke.location = DataLocation::AV2_DL_INTERNAL;
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
		auto const func = context.stream.current();
		if (func.type != LTS_TT_IDENTIFIER)
			MINIMA_ERROR(InvalidValue, "Function call must be an identifier!");
		fname = func.value.get<Makai::String>();
	} else invoke.location = DataLocation::AV2_DL_CONST;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed function call!");
	auto const funcID = context.program.code.size();
	context.program.code.pushBack({});
	context.addJumpTarget(fname);
	if (func.type != Type{'('})
		MINIMA_ERROR(InvalidValue, "Expected '(' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed function call!");
	usize argi = 0;
	while (context.stream.current().type != Type{')'} && argi < 256) {
		Instruction::Invocation::Parameter param;
		auto const loc = getDataLocation(context);
		param.location	= loc.at;
		param.id		= loc.id;
		param.argument	= argi++;
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
	}
	if (func.type != Type{')'})
		MINIMA_ERROR(InvalidValue, "Expected ')' here!");
	invoke.argc = argi;
	context.program.code[funcID] = {
		Instruction::Name::AV2_IN_CALL,
		Makai::Cast::bit<uint32>(invoke)
	};
}

MINIMA_ASSEMBLE_FN(Compare) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed comparison!");
	auto const lhs = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed comparison!");
	Makai::Anima::V2::Comparator comp;
	{
		auto const cmp = context.stream.current();
		switch (cmp.type) {
			using enum As<decltype(comp)>;
			case LTS_TT_IDENTIFIER: {
				auto const id = cmp.value.get<Makai::String>();
				if (id == "equals" || id == "eq" || id == "is")				comp = AV2_OP_EQUALS;
				else if (id == "notequals" || id == "neq" || id == "not")	comp = AV2_OP_NOT_EQUALS;
				else if (id == "less" || id == "lt")						comp = AV2_OP_LESS_THAN;
				else if (id == "greater" || id == "gt")						comp = AV2_OP_GREATER_THAN;
				else if (id == "lessequals" || id == "le")					comp = AV2_OP_LESS_EQUALS;
				else if (id == "greaterequals" || id == "ge")				comp = AV2_OP_GREATER_EQUALS;
				else if (id == "threeway" || id == "order")					comp = AV2_OP_THREEWAY;
				else MINIMA_ERROR(InvalidValue, "Invalid comparison type!");
			} break;
			case Type{':'}:								comp = AV2_OP_THREEWAY;			break;
			case Type{'<'}:								comp = AV2_OP_LESS_THAN;		break;
			case Type{'>'}:								comp = AV2_OP_GREATER_THAN;		break;
			case Type{'='}:
			case Type::LTS_TT_COMPARE_EQUALS:			comp = AV2_OP_EQUALS;			break;
			case Type{'!'}:
			case Type::LTS_TT_COMPARE_NOT_EQUALS:		comp = AV2_OP_NOT_EQUALS;		break;
			case Type::LTS_TT_COMPARE_GREATER_EQUALS:	comp = AV2_OP_GREATER_EQUALS;	break;
			case Type::LTS_TT_COMPARE_LESS_EQUALS:		comp = AV2_OP_LESS_EQUALS;		break;
			default: MINIMA_ERROR(InvalidValue, "Invalid comparator for comparison!");
		}
	}
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed comparison!");
	auto const rhs = getDataLocation(context);
	if (!(context.stream.next() && context.stream.current().type == Type::LTS_TT_LITTLE_ARROW))
		MINIMA_ERROR(InvalidValue, "Expected '->' here!");
	auto const out = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed comparison!");
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_COMPARE,
		Makai::Cast::bit<uint32, Instruction::Comparison>({
			lhs.at,
			rhs.at,
			out.at,
			comp
		})
	});
	if (lhs.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(lhs.id));
	if (rhs.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(rhs.id));
	if (out.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(out.id));
}

MINIMA_ASSEMBLE_FN(Copy) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed copy!");
	auto const from = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed copy!");
	if (!(context.stream.next() && context.stream.current().type == Type::LTS_TT_LITTLE_ARROW))
		MINIMA_ERROR(InvalidValue, "Expected '->' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed copy!");
	auto const to = getDataLocation(context);
	Instruction::Transfer tf = {
		from.at,
		to.at
	};
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_COMPARE,
		Makai::Cast::bit<uint32>(tf)
	});
	if (from.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(from.id));
	if (to.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(to.id));
}

MINIMA_ASSEMBLE_FN(Context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed context declaration!");
	auto const mode = context.stream.current();
	if (mode.type != LTS_TT_IDENTIFIER)
		MINIMA_ERROR(InvalidValue, "Context mode name must be an identifier!");
	auto id = mode.value.get<Makai::String>();
	Instruction::Context ctx;
	if (id == "strict" || id == "default" || id == "none")
		ctx.mode = Makai::Anima::V2::ContextMode::AV2_CM_STRICT;
	else if (id == "loose")
		ctx.mode = Makai::Anima::V2::ContextMode::AV2_CM_LOOSE;
	else MINIMA_ERROR(InvalidValue, "Invalid context mode!");
	ctx.immediate = false;
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_MODE,
		Makai::Cast::bit<uint32>(ctx)
	});
}

MINIMA_ASSEMBLE_FN(ImmediateContext) {
	if (
		context.program.code.back().name == Instruction::Name::AV2_IN_MODE
	&&	Makai::Cast::bit<Instruction::Context>(context.program.code.back().type).immediate
	) MINIMA_ERROR(InvalidValue, "Only one immediate context allowed per instruction!");
	auto id = context.stream.current().value.get<Makai::String>();
	Instruction::Context ctx;
	if (id == "strict")
		ctx.mode = Makai::Anima::V2::ContextMode::AV2_CM_STRICT;
	else if (id == "loose")
		ctx.mode = Makai::Anima::V2::ContextMode::AV2_CM_LOOSE;
	else MINIMA_ERROR(InvalidValue, "Invalid immediate context mode!");
	ctx.immediate = false;
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_MODE,
		Makai::Cast::bit<uint32>(ctx)
	});
}

MINIMA_ASSEMBLE_FN(Halt) {
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_HALT,
		0
	});
}

MINIMA_ASSEMBLE_FN(ErrorHalt) {
	auto const err = getDataLocation(context);
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_HALT,
		Makai::Cast::bit<uint32>(Instruction::Stop{
			Instruction::Stop::Mode::AV2_ISM_ERROR,
			err.at
		})
	});
	if (err.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(err.id));
}

// TODO: Global declarations
MINIMA_ASSEMBLE_FN(Global) {
	
}

// TODO: Anima-V1 support
MINIMA_ASSEMBLE_FN(V1Operation) {
	
}

// TODO: Anima-V1 support
MINIMA_ASSEMBLE_FN(V1ContextExec) {
	
}

// TODO: Anima-V1 support
MINIMA_ASSEMBLE_FN(V1Expression) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed Anima-V1 expression!");
	auto const current = context.stream.current();
	if (current.type == LTS_TT_IDENTIFIER) {
		auto const id = current.value.get<Makai::String>();
		if (id == "run" || id == "do") {}
		else if (id == "line") {}
		else if (id == "emote") {}
		else if (id == "perform" || id == "do") {}
		else if (id == "actor") {}
		else if (id == "add") {

		}
	} else MINIMA_ERROR(InvalidValue, "Anima-V1 expression must be an identifier!");
}

MINIMA_ASSEMBLE_FN(BinaryMath) {

}

MINIMA_ASSEMBLE_FN(UnaryMath) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed unary math expression!");
	auto const op = context.stream.current();
	Instruction::UnaryMath umath;
	switch (op.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = op.value.get<Makai::String>();
			if (id == "negate" || id == "neg")			umath.op = decltype(umath.op)::AV2_IUM_OP_NEGATE;
			else if (id == "inverse" || id == "inv")	umath.op = decltype(umath.op)::AV2_IUM_OP_INVERSE;
			else if (id == "sin")						umath.op = decltype(umath.op)::AV2_IUM_OP_SIN;
			else if (id == "cos")						umath.op = decltype(umath.op)::AV2_IUM_OP_COS;
			else if (id == "tan")						umath.op = decltype(umath.op)::AV2_IUM_OP_TAN;
			else if (id == "arcsin" || id == "asin")	umath.op = decltype(umath.op)::AV2_IUM_OP_ASIN;
			else if (id == "arccos" || id == "acos")	umath.op = decltype(umath.op)::AV2_IUM_OP_ACOS;
			else if (id == "arctan" || id == "atan")	umath.op = decltype(umath.op)::AV2_IUM_OP_ATAN;
			else if (id == "sinh")						umath.op = decltype(umath.op)::AV2_IUM_OP_SINH;
			else if (id == "cosh")						umath.op = decltype(umath.op)::AV2_IUM_OP_COSH;
			else if (id == "tanh")						umath.op = decltype(umath.op)::AV2_IUM_OP_TANH;
			else if (id == "log2" || id == "l2")		umath.op = decltype(umath.op)::AV2_IUM_OP_LOG2;
			else if (id == "log10" || id == "l10")		umath.op = decltype(umath.op)::AV2_IUM_OP_LOG10;
			else if (id == "logn" || id == "ln")		umath.op = decltype(umath.op)::AV2_IUM_OP_LN;
			else if (id == "sqrt")						umath.op = decltype(umath.op)::AV2_IUM_OP_SQRT;
		} break;
		default: MINIMA_ERROR(NonexistentValue, "Invalid unary math operator!");
	}
}

MINIMA_ASSEMBLE_FN(Yield) {
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_YIELD,
		0
	});
}

static void doTruthyAwait(Context& context, Instruction::WaitRequest& wait) {
	auto const loc = getDataLocation(context);
	if (loc.at == DataLocation::AV2_DL_CONST)
		MINIMA_ERROR(InvalidValue, "Cannot await based on a constant value!");
	wait.val = loc.at;
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
}

static void doFalsyAwait(Context& context, Instruction::WaitRequest& wait) {
	wait.falsy = true;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed await!");
	doTruthyAwait(context, wait);
}

MINIMA_ASSEMBLE_FN(Await) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed await!");
	auto const await = context.stream.current();
	Instruction::WaitRequest wait {.falsy = false};
	auto const awaitID = context.program.code.size();
		context.program.code.pushBack({});
	switch (await.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = await.value.get<Makai::String>();
			if (id == "not") doFalsyAwait(context, wait);
			else doTruthyAwait(context, wait);
		} break;
		case Type{'!'}: doFalsyAwait(context, wait); break;
		default: doTruthyAwait(context, wait); break;
	}
	context.program.code[awaitID] = {
		Instruction::Name::AV2_IN_AWAIT,
		Makai::Cast::bit<uint32>(wait)
	};
}

MINIMA_ASSEMBLE_FN(IndirectRead) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed indirect read!");
	if (context.stream.current().type != Type{'('})
		MINIMA_ERROR(InvalidValue, "Expected '(' here!");
	auto const at = context.stream.current();
	DataLocation loc;
	if (at.type == LTS_TT_IDENTIFIER) {
		auto const id = at.value.get<Makai::String>();
		if (id == "register" || id == "reg") {
			loc = DataLocation::AV2_DL_REGISTER;
		} else if (id == "stack") {
			loc = DataLocation::AV2_DL_STACK;
		} else if (id == "external" || id == "extern" || id == "out") {
			loc = DataLocation::AV2_DL_EXTERNAL;
		} else if (id == "global" || id == "g") {
			loc = DataLocation::AV2_DL_GLOBAL;
		} else if (id == "temporary" || id == "temp") {
			loc = DataLocation::AV2_DL_TEMPORARY;
		} else MINIMA_ERROR(InvalidValue, "Invalid location target!");
	} else MINIMA_ERROR(InvalidValue, "Location target must be an identifier!");
	auto const id = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed indirect read!");
	if (context.stream.current().type != Type{')'})
		MINIMA_ERROR(InvalidValue, "Expected ')' here!");
	context.program.constants.pushBack(Makai::enumcast(loc));
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_STACK_PUSH,
		Makai::Cast::bit<uint32, Instruction::StackPush>({
			DataLocation::AV2_DL_CONST
		})
	});
	context.program.code.pushBack(Makai::Cast::bit<Instruction, uint64>(context.program.constants.size() - 1));
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_STACK_PUSH,
		Makai::Cast::bit<uint32, Instruction::StackPush>({
			id.at
		})
	});
	if (id.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(id.id));
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_CALL,
		Makai::Cast::bit<uint32, Instruction::Invocation>({
			.location = DataLocation::AV2_DL_INTERNAL,
			.argc = ':'
		})
	});
}

MINIMA_ASSEMBLE_FN(Expression) {
	auto const current = context.stream.current();
	if (current.type == LTS_TT_IDENTIFIER) {
		auto const id = current.value.get<Makai::String>();
		if (id == "jump")												doJump(context);
		else if (id == "nop")											doNoOp(context);
		else if (id == "swap")											doStackSwap(context);
		else if (id == "flush")											doStackFlush(context);
		else if (id == "push")											doStackPush(context);
		else if (id == "pop")											doStackPop(context);
		else if (id == "clear")											doStackClear(context);
		else if (id == "return" || id == "ret")							doReturn(context);
		else if (id == "finish" || id == "end")							doEmptyReturn(context);
		else if (id == "terminate" || id == "halt")						doHalt(context);
		else if (id == "error")											doErrorHalt(context);
		else if (id == "call" || id == "go")							doCall(context);
		else if (id == "compare" || id == "cmp")						doCompare(context);
		else if (id == "copy")											doCopy(context);
		else if (id == "context" || id == "mode")						doContext(context);
		else if (id == "loose" || id == "strict")						doImmediateContext(context);
		else if (id == "global")										doGlobal(context);
		else if (id == "dialog" || id == "diag" || id == "v1")			doV1Expression(context);
		else if (id == "calculate" || id == "bmath" || id == "calc")	doBinaryMath(context);
		else if (id == "umath")											doUnaryMath(context);
		else if (id == "yield")											doYield(context);
		else if (id == "await" || id == "wait")							doAwait(context);
		else if (id == "read")											doIndirectRead(context);
	}
}

void Minima::assemble() {
	Minima::Program result;
	while (context.stream.next()) doExpression(context);
}
CTL_DIAGBLOCK_END