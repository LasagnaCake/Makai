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

#define MINIMA_ERROR(TYPE, WHAT) context.error<Makai::Error::TYPE>(WHAT)

static DataLocation getLoadType(Context& context) {
	DataLocation locAt = DataLocation{0};
	if (context.stream.current().type == LTS_TT_IDENTIFIER) {
		auto const id = context.getValue<Makai::String>();
		if (id == "reference" || id == "ref")
			locAt = DataLocation::AV2_DLM_BY_REF;
		else if (id == "move")
			locAt = DataLocation::AV2_DLM_MOVE;
		else if (id == "value" || id == "copy")
			locAt = DataLocation{0};
		context.fetchNext();
	}
	return locAt;
}

static Location getStack(Minima::Context& context) {
	Location loc;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing stack index!");
	loc.at = getLoadType(context);
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	auto v = context.stream.current();
	bool fromTheBack = false;
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
	loc = {
		loc.at
	|	(fromTheBack ? DataLocation::AV2_DL_STACK_OFFSET : DataLocation::AV2_DL_STACK),
		v.value.get<usize>()
	};
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	return loc;
}

static Location getRegister(Minima::Context& context) {
	Location loc;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing register index!");
	loc.at = getLoadType(context);
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	auto v = context.stream.current();
	bool fromTheBack = false;
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
	loc = {
		loc.at
	|	Makai::Anima::V2::asRegister(v.value.get<ssize>() + (fromTheBack ? Makai::Anima::V2::REGISTER_COUNT : 0)),
		uint64(-1)
	};
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	return loc;
}

static Location getExtern(Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing external location name!");
	auto const name = context.stream.current();
	DataLocation locAt = getLoadType(context);
	if (!(
		name.type == LTS_TT_IDENTIFIER
	||	name.type == LTS_TT_SINGLE_QUOTE_STRING
	||	name.type == LTS_TT_DOUBLE_QUOTE_STRING
	)) MINIMA_ERROR(InvalidValue, "Expected name for external location!");
	return {
		locAt | DataLocation::AV2_DL_EXTERNAL,
		context.addConstant(name.value.get<Makai::String>())
	};
}

static Location getGlobal(Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing global variable name!");
	auto const name = context.stream.current();
	DataLocation locAt = getLoadType(context);
	if (name.type != LTS_TT_IDENTIFIER)
		MINIMA_ERROR(InvalidValue, "Expected identifier for global variable name!");
	auto const id = name.value.get<Makai::String>();
	usize globalID = 0;
	if (context.program.labels.globals.contains(id))
		globalID = context.program.labels.globals[id];
	else {
		globalID = context.program.labels.globals.size();
		context.program.labels.globals[id] = globalID;
	}
	return {
		locAt| DataLocation::AV2_DL_GLOBAL,
		globalID
	};
}

static usize addConstant(Minima::Context& context, Makai::Data::Value const& value) {
	return context.addConstant(value);
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
		else {
			Makai::Data::Value c =
				(
					v.type == LTS_TT_INTEGER
				?	v.value.get<ssize>()
				:	v.value.get<double>()
				)
			*	(isNegative ? -1 : +1)
			;
			return {DataLocation::AV2_DL_CONST, addConstant(context, c)};
		}
	}
	return {DataLocation::AV2_DL_CONST, addConstant(context, current.value)};
}

static Location getLabelLocation(Minima::Context& context) {
	auto const current = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "label name").getString();
	if (!context.jumps.labels.contains(current))
		context.error("Jump target has not been declared yet!");
	return {DataLocation::AV2_DL_CONST, addConstant(context, context.jumps.labels[current])};
}

static Location getDataLocation(Minima::Context& context) {
	auto const current = context.stream.current();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			if (id == "register" || id == "reg") {
				return getRegister(context);
			} else if (id == "placeof") {
				return getLabelLocation(context);
			} else if (id == "stack") {
				return getStack(context);
			} else if (id == "external" || id == "extern" || id == "out") {
				return getExtern(context);
			} else if (id == "global" || id == "g") {
				return getGlobal(context);
			} else if (id == "temporary" || id == "temp") {
				return {getLoadType(context) | DataLocation::AV2_DL_TEMPORARY, uint64(-1)};
			} else if (id == "true") {
				return {DataLocation::AV2_DL_INTERNAL, 1};
			} else if (id == "false") {
				return {DataLocation::AV2_DL_INTERNAL, 0};
			} else if (id == "null") {
				return {DataLocation::AV2_DL_INTERNAL, 3};
			} else if (id == "undefined" || id == "void") {
				return {DataLocation::AV2_DL_INTERNAL, 2};
			} else if (id == "array" || id == "arr") {
				return {DataLocation::AV2_DL_INTERNAL, 8};
			} else if (id == "object" || id == "obj") {
				return {DataLocation::AV2_DL_INTERNAL, 10};
			} else if (id == "binary" || id == "bytes" || id == "bin") {
				return {DataLocation::AV2_DL_INTERNAL, 9};
			} else if (id == "nan") {
				return {DataLocation::AV2_DL_INTERNAL, 4};
			} else {
				auto const lt = getLoadType(context);
				auto dloc = getDataLocation(context);
				dloc.at = dloc.at & ~(DataLocation::AV2_DLM_BY_REF | DataLocation::AV2_DLM_MOVE);
				dloc.at = dloc.at | lt;
				return dloc;
			};
		} break;
		case Type{'*'}:
			return getLabelLocation(context);
		case Type{'@'}:
			return getExtern(context);
		case Type{'$'}:
			return getRegister(context);
		case Type{'&'}:
			return getStack(context);
		case Type{':'}:
			return getGlobal(context);
		case Type{'.'}:
			return {getLoadType(context) | DataLocation::AV2_DL_TEMPORARY, uint64(-1)};
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
	context.fetchNext();
	auto const loc = getDataLocation(context);
	leap.condition = loc.at;
	if (leap.condition == DataLocation::AV2_DL_CONST) {
		leap.type = Instruction::Leap::Type::AV2_ILT_IF_TRUTHY;
		if (loc.id == 0) leap.condition = DataLocation::AV2_DL_INTERNAL;
	} else if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
}

void doLeapType(Context& context, Instruction::Leap& leap) {
	auto loc = context.stream.current();
	if ((leap.source == DataLocation::AV2_DL_CONST) && loc.type == Type{';'})
		MINIMA_ERROR(NonexistentValue, "Malformed jump!");
	switch (loc.type) {
		case Type{'?'}: {
			doConditionalLeapType(context, leap);
		} break;
		case LTS_TT_IDENTIFIER: {
			auto const id = loc.value.get<Makai::String>();
			if (id == "if") doConditionalLeapType(context, leap);
			break;
		}
		default: MINIMA_ERROR(InvalidValue, "Unexpected token!");
	}
	if (leap.source == DataLocation::AV2_DL_CONST) {
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed jump!");
		auto const name = context.stream.current();
		if (name.type != LTS_TT_IDENTIFIER)
			MINIMA_ERROR(InvalidValue, "Expected identifier for jump label!");
		context.addJumpTarget(name.value.get<Makai::String>());
	}
}

void doDynamicLeap(Context& context, Instruction::Leap& leap) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed jump!");
	auto const loc = getDataLocation(context);
	leap.source = loc.at;
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
	doLeapType(context, leap);
}

MINIMA_ASSEMBLE_FN(Jump) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed jump!");
	Instruction::Leap leap = {
		.type		= Instruction::Leap::Type::AV2_ILT_UNCONDITIONAL,
		.source		= Makai::Anima::V2::DataLocation::AV2_DL_CONST,
		.condition	= Makai::Anima::V2::DataLocation::AV2_DL_CONST
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
		} break;
		case Type{'&'}:
			doDynamicLeap(context, leap);
		break;
		default: {
			if (!context.stream.next())
				MINIMA_ERROR(NonexistentValue, "Malformed jump!");
			doLeapType(context, leap);
		} break;
	}
	context.program.code[index] = {
		Instruction::Name::AV2_IN_JUMP,
		Makai::Cast::bit<uint32>(leap)
	};
}

MINIMA_ASSEMBLE_FN(NoOp) {
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_NO_OP,
		context.stream.current().value == Makai::Data::Value("next")
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
	if (
		loc.at == DataLocation::AV2_DL_CONST
	) MINIMA_ERROR(NonexistentValue, "Destination cannot be a constant value!");
	context.program.code.pushBack(inst);
	if (loc.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(loc.id));
}

MINIMA_ASSEMBLE_FN(Return) {
	Instruction inst = {Instruction::Name::AV2_IN_RETURN};
	context.program.code.pushBack(inst);
}

MINIMA_ASSEMBLE_FN(InternalCall) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed internal call!");
	auto const func = context.stream.current();
	Instruction::Invocation invoke{DataLocation::AV2_DL_INTERNAL, 0};
	switch (func.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = func.value.get<Makai::String>();
			if (id == "add")								invoke.argc = '+';
			else if (id == "subtract" || id == "sub")		invoke.argc = '-';
			else if (id == "multiply" || id == "mul")		invoke.argc = '*';
			else if (id == "divide" || id == "div")			invoke.argc = '/';
			else if (id == "power" || id == "pow")			invoke.argc = 'p';
			else if (id == "remainder" || id == "rem")		invoke.argc = '%';
			else if (id == "compare" || id == "cmp")		invoke.argc = '=';
			else if (id == "negate" || id == "neg")			invoke.argc = 'n';
			else if (id == "band")							invoke.argc = '&';
			else if (id == "bor")							invoke.argc = '|';
			else if (id == "bxor")							invoke.argc = '^';
			else if (id == "bnot")							invoke.argc = '~';
			else if (id == "land" || id == "and")			invoke.argc = 'a';
			else if (id == "lor" || id == "or")				invoke.argc = 'o';
			else if (id == "lnot" || id == "not")			invoke.argc = '!';
			else if (id == "sin")							invoke.argc = 's';
			else if (id == "cos")							invoke.argc = 'c';
			else if (id == "tan")							invoke.argc = 't';
			else if (id == "stringify" || id == "strify")	invoke.argc = '_';
			else if (id == "typename" || id == "tname")		invoke.argc = 'i';
			else if (id == "arcsin" || id == "asin")		invoke.argc = 'S';
			else if (id == "arccos" || id == "acos")		invoke.argc = 'C';
			else if (id == "arctan" || id == "atan")		invoke.argc = 'T';
			else if (id == "atan2" || id == "a2")			invoke.argc = '2';
			else if (id == "interrupt" || id == "stop")		invoke.argc = '.';
			else if (id == "access" || id == "read")		invoke.argc = ':';
			else if (id == "print" || id == "echo")			invoke.argc = '@';
			else if (id == "sizeof")						invoke.argc = '#';
			else if (id == "http")							invoke.argc = 'H';
			else if (id == "string" || id == "str" || id == "s") {
				invoke.argc	= '"';
				auto const op = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "array operation").getString();
				if (op == "new")							invoke.mod = '.';
				else if (id == "slice" || op == "sub")		invoke.mod = '_';
				else if (op == "replace" || op == "rep")	invoke.mod = ':';
				else if (op == "split" || op == "sep")		invoke.mod = '/';
				else if (op == "concat" || op == "join")	invoke.mod = '+';
				else if (op == "match" || op == "is")		invoke.mod = '=';
				else if (op == "contains" || op == "has")	invoke.mod = 'f';
				else if (op == "find" || op == "in")		invoke.mod = 'i';
				else if (op == "remove" || op == "del")		invoke.mod = '-';
				else MINIMA_ERROR(InvalidValue, "Invalid internal call!");
			}
			else if (id == "array" || id == "arr" || id == "a") {
				invoke.argc	= '[';
				auto const op = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "array operation").getString();
				if (op == "new")							invoke.mod = '.';
				else if (op == "remove" || op == "del")		invoke.mod = '-';
				else if (op == "concat" || op == "join")	invoke.mod = '+';
				else if (op == "like")						invoke.mod = '=';
				else if (op == "unlike")					invoke.mod = '!';
				else if (op == "slice" || op == "sub")		invoke.mod = '_';
				else if (op == "find")						invoke.mod = 'f';
				else if (op == "fuzz")						invoke.mod = 'F';
				else if (op == "push")						invoke.mod = '<';
				else if (op == "pop")						invoke.mod = '>';
				else MINIMA_ERROR(InvalidValue, "Invalid internal call!");
			}
			else if (id == "object" || id == "obj" || id == "o") {
				invoke.argc	= '{';
				auto const op = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "array operation").getString();
				if (op == "has")							invoke.mod = ':';
				else if (op == "remove" || op == "del")		invoke.mod = '-';
				else if (op == "concat" || op == "join")	invoke.mod = '+';
				else if (op == "like")						invoke.mod = '=';
				else if (op == "unlike")					invoke.mod = '!';
				else if (op == "findkey" || op == "fink")	invoke.mod = 'f';
				else if (op == "fuzzkey" || op == "fuzk")	invoke.mod = 'F';
				else if (op == "findval" || op == "finv")	invoke.mod = 'x';
				else if (op == "fuzzval" || op == "fuzv")	invoke.mod = 'X';
				else if (op == "keys" || op == "k")			invoke.mod = 'k';
				else if (op == "values" || op == "v")		invoke.mod = 'v';
				else if (op == "items" || op == "i")		invoke.mod = 'i';
				else MINIMA_ERROR(InvalidValue, "Invalid internal call!");
			}
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
		case Type{'.'}:
		case Type{'@'}:
		case Type{'>'}:
		case Type{','}: {
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

static Makai::Data::Value::Kind getType(Context& context) {
	using enum Makai::Data::Value::Kind;
	constexpr auto const DVK_ANY = Makai::Cast::as<decltype(DVK_VOID)>(-1);
	auto const ret = context.stream.current();
	switch (ret.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = ret.value.get<Makai::String>();
			if (id == "any")														return DVK_ANY;
			else if (id == "boolean" || id == "bool" || id == "b")					return DVK_BOOLEAN;
			else if (id == "void" || id == "undefined" || id == "v")				return DVK_VOID;
			else if (id == "int" || id == "i")										return DVK_SIGNED;
			else if (id == "uint" || id == "u")										return DVK_UNSIGNED;
			else if (id == "float" || id == "real" || id == "f" || id == "r")		return DVK_REAL;
			else if (id == "string" || id == "text" || id == "str" || id == "s")	return DVK_STRING;
			else if (id == "array" || id == "list" || id == "a")					return DVK_ARRAY;
			else if (id == "binary" || id == "bytes" || id == "bin")				return DVK_BYTES;
			else if (id == "object" || id == "struct" || id == "o")					return DVK_OBJECT;
			MINIMA_ERROR(InvalidValue, "Invalid/Unsupported type!");
		}
		case Type{'?'}: return DVK_ANY;
		case Type{'_'}: return DVK_VOID;
		case Type{'+'}: return DVK_SIGNED;
		case Type{'-'}: return DVK_UNSIGNED;
		default: MINIMA_ERROR(InvalidValue, "Invalid/Unsupported type!");
	}
	MINIMA_ERROR(InvalidValue, "Invalid/Unsupported type!");
}

MINIMA_ASSEMBLE_FN(Call) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed function call!");
	auto const func = context.stream.current();
	if (func.type != LTS_TT_IDENTIFIER)
		MINIMA_ERROR(InvalidValue, "Function call must be an identifier!");
	auto fname = func.value.get<Makai::String>();
	Instruction::Invocation invoke;
	auto retType = Makai::Cast::as<Makai::Data::Value::Kind>(-2);
	if (fname == "internal" || fname == "intern" || fname == "in") {
		return doInternalCall(context);
	}
	else if (fname == "external" || fname == "extern" || fname == "out") {
		invoke.location = DataLocation::AV2_DL_EXTERNAL;
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
		auto const func = context.stream.current();
		if (!func.value.isString())
			MINIMA_ERROR(InvalidValue, "External call name must be a string!");
		fname = func.value.get<Makai::String>();
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
		retType = getType(context);
	} else invoke.location = DataLocation::AV2_DL_CONST;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed function call!");
	auto const funcID = context.program.code.size();
	context.program.code.pushBack({});
	if (invoke.location == DataLocation::AV2_DL_CONST) {
		context.addJumpTarget(fname);
	} else {
		context.addInstruction<uint64>(addConstant(context, fname));
	}
	if (!context.hasToken(Type{'('}))
		MINIMA_ERROR(InvalidValue, "Expected '(' here!");
	Makai::List<uint64> argi;
	while (context.stream.current().type != Type{')'} && argi.size() < 256) {
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
		auto const argIndex = context.stream.current();
		if (argIndex.type == Type{')'}) break;
		if (!argIndex.value.isUnsigned())
			MINIMA_ERROR(InvalidValue, "Argument index must be an unsigned integer!");
		auto const i = argIndex.value.get<uint64>();
		if (i > 255)
			MINIMA_ERROR(InvalidValue, "Maximum argument index is 255!");
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
		if (argi.find(i) != -1)
			MINIMA_ERROR(InvalidValue, "Duplicate argument!");
		if (context.stream.current().type != Type{'='})
			MINIMA_ERROR(InvalidValue, "Expected '=' here!");
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
		Instruction::Invocation::Parameter param;
		auto const loc = getDataLocation(context);
		param.location	= loc.at;
		param.id		= loc.id;
		param.argument	= i;
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed function call!");
		context.addInstruction(param);
	}
	if (invoke.location == DataLocation::AV2_DL_EXTERNAL) {

	}
	if (!context.hasToken(Type{')'}))
		MINIMA_ERROR(InvalidValue, "Expected ')' here!");
	for (auto& arg: argi)
		if (arg > invoke.argc)
		invoke.argc = invoke.argc;
	context.program.code[funcID] = {
		Instruction::Name::AV2_IN_CALL,
		Makai::Cast::bit<uint32>(invoke)
	};
	if (retType != Makai::Cast::as<decltype(retType)>(-2))
		context.program.code.pushBack(Makai::Cast::bit<Instruction, uint64>(Makai::enumcast(retType)));
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
				if (id == "equals" || id == "eq")							comp = AV2_OP_EQUALS;
				else if (id == "notequals" || id == "not" || id == "ne")	comp = AV2_OP_NOT_EQUALS;
				else if (id == "less" || id == "lt")						comp = AV2_OP_LESS_THAN;
				else if (id == "greater" || id == "gt")						comp = AV2_OP_GREATER_THAN;
				else if (id == "lessequals" || id == "le")					comp = AV2_OP_LESS_EQUALS;
				else if (id == "greaterequals" || id == "ge")				comp = AV2_OP_GREATER_EQUALS;
				else if (id == "threeway" || id == "order" || id == "ord")	comp = AV2_OP_THREEWAY;
				else if (id == "typeof" || id == "is")						comp = AV2_OP_TYPE_COMPARE;
				else MINIMA_ERROR(InvalidValue, "Invalid comparison type!");
			} break;
			case Type{':'}:						comp = AV2_OP_THREEWAY;			break;
			case Type{'<'}:						comp = AV2_OP_LESS_THAN;		break;
			case Type{'>'}:						comp = AV2_OP_GREATER_THAN;		break;
			case Type{'='}:
			case LTS_TT_COMPARE_EQUALS:			comp = AV2_OP_EQUALS;			break;
			case Type{'!'}:
			case LTS_TT_COMPARE_NOT_EQUALS:		comp = AV2_OP_NOT_EQUALS;		break;
			case LTS_TT_COMPARE_GREATER_EQUALS:	comp = AV2_OP_GREATER_EQUALS;	break;
			case LTS_TT_COMPARE_LESS_EQUALS:	comp = AV2_OP_LESS_EQUALS;		break;
			default: MINIMA_ERROR(InvalidValue, "Invalid comparator for comparison!");
		}
	}
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed comparison!");
	auto const rhs = getDataLocation(context);
	if (!(context.stream.next() && context.stream.current().type == Type::LTS_TT_LITTLE_ARROW))
		MINIMA_ERROR(InvalidValue, "Expected '->' here!");
	auto const out = getDataLocation(context);
	if (
		out.at == DataLocation::AV2_DL_CONST
	||	out.at == DataLocation::AV2_DL_INTERNAL
	) MINIMA_ERROR(NonexistentValue, "Destination cannot be a constant or internal value!");
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
	context.fetchNext().expectToken(LTS_TT_LITTLE_ARROW);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed copy!");
	auto const to = getDataLocation(context);
	if (
		to.at == DataLocation::AV2_DL_CONST
	||	to.at == DataLocation::AV2_DL_INTERNAL
	) MINIMA_ERROR(NonexistentValue, "Destination cannot be a constant or internal value!");
	Instruction::Transfer tf = {
		from.at,
		to.at
	};
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_COPY,
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

MINIMA_ASSEMBLE_FN(BinaryMath) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed binary math expression!");
	auto const lhs = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed binary math expression!");
	auto const op = context.stream.current();
	Instruction::BinaryMath bmath;
	switch (op.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = op.value.get<Makai::String>();
			if (id == "add")							bmath.op = decltype(bmath.op)::AV2_IBM_OP_ADD;
			else if (id == "subtract" || id == "sub")	bmath.op = decltype(bmath.op)::AV2_IBM_OP_SUB;
			else if (id == "multiply" || id == "mul")	bmath.op = decltype(bmath.op)::AV2_IBM_OP_MUL;
			else if (id == "divide" || id == "div")		bmath.op = decltype(bmath.op)::AV2_IBM_OP_DIV;
			else if (id == "remainder" || id == "rem")	bmath.op = decltype(bmath.op)::AV2_IBM_OP_REM;
			else if (id == "power" || id == "pow")		bmath.op = decltype(bmath.op)::AV2_IBM_OP_POW;
			else if (id == "atan2" || id == "a2")		bmath.op = decltype(bmath.op)::AV2_IBM_OP_ATAN2;
			else MINIMA_ERROR(NonexistentValue, "Invalid binary math operator!");
		} break;
		case Type{'+'}: {
			bmath.op = decltype(bmath.op)::AV2_IBM_OP_ADD;
		} break;
		case Type{'-'}: {
			bmath.op = decltype(bmath.op)::AV2_IBM_OP_SUB;
		} break;
		case Type{'*'}: {
			bmath.op = decltype(bmath.op)::AV2_IBM_OP_MUL;
		} break;
		case Type{'/'}: {
			bmath.op = decltype(bmath.op)::AV2_IBM_OP_DIV;
		} break;
		case Type{'%'}: {
			bmath.op = decltype(bmath.op)::AV2_IBM_OP_REM;
		} break;
		case Type{'^'}: {
			bmath.op = decltype(bmath.op)::AV2_IBM_OP_POW;
		} break;
		case Type{'.'}: {
			bmath.op = decltype(bmath.op)::AV2_IBM_OP_ATAN2;
		} break;
		default: MINIMA_ERROR(NonexistentValue, "Invalid binary math operator!");
	}
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed binary math expression!");
	auto const rhs = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed unary math expression!");
	if (context.stream.current().type != Type::LTS_TT_LITTLE_ARROW)
		MINIMA_ERROR(NonexistentValue, "Expected '->' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed binary math expression!");
	auto const out = getDataLocation(context);
	if (
		out.at == DataLocation::AV2_DL_CONST
	||	out.at == DataLocation::AV2_DL_INTERNAL
	) MINIMA_ERROR(NonexistentValue, "Destination cannot be a constant or internal value!");
	bmath.lhs = lhs.at;
	bmath.rhs = rhs.at;
	bmath.out = out.at;
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_MATH_UOP,
		Makai::Cast::bit<uint32>(bmath)
	});
	if (lhs.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(lhs.id));
	if (rhs.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(rhs.id));
	if (out.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(out.id));
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
			else if (id == "increment" || id == "inc")	umath.op = decltype(umath.op)::AV2_IUM_OP_INCREMENT;
			else if (id == "decrement" || id == "dec")	umath.op = decltype(umath.op)::AV2_IUM_OP_DECREMENT;
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
			else MINIMA_ERROR(NonexistentValue, "Invalid unary math operator!");
		} break;
		case LTS_TT_DECREMENT: umath.op = decltype(umath.op)::AV2_IUM_OP_DECREMENT;
		case LTS_TT_INCREMENT: umath.op = decltype(umath.op)::AV2_IUM_OP_INCREMENT;
		case Type{'-'}: umath.op = decltype(umath.op)::AV2_IUM_OP_NEGATE;
		case Type{'/'}: umath.op = decltype(umath.op)::AV2_IUM_OP_INVERSE;
		default: MINIMA_ERROR(NonexistentValue, "Invalid unary math operator!");
	}
	context.fetchNext();
	auto const v = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed unary math expression!");
	if (context.stream.current().type != Type::LTS_TT_LITTLE_ARROW)
		MINIMA_ERROR(NonexistentValue, "Expected '->' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed unary math expression!");
	auto const out = getDataLocation(context);
	if (
		out.at == DataLocation::AV2_DL_CONST
	||	out.at == DataLocation::AV2_DL_INTERNAL
	) MINIMA_ERROR(NonexistentValue, "Destination cannot be a constant or internal value!");
	context.program.code.pushBack({
		Instruction::Name::AV2_IN_MATH_UOP,
		Makai::Cast::bit<uint32>(umath)
	});
	if (v.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(v.id));
	if (out.id < Makai::Limit::MAX<uint64>)
		context.program.code.pushBack(Makai::Cast::bit<Instruction>(out.id));
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
	wait.wait = decltype(wait.wait)::AV2_IWRW_FALSY;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed await!");
	doTruthyAwait(context, wait);
}

MINIMA_ASSEMBLE_FN(Await) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed await!");
	auto const await = context.stream.current();
	Instruction::WaitRequest wait {.wait = decltype(wait.wait)::AV2_IWRW_TRUTHY};
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

MINIMA_ASSEMBLE_FN(Get) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed getter!");
	auto const getID = context.addNamedInstruction(Instruction::Name::AV2_IN_GET);
	Instruction::GetRequest get;
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed getter!");
	auto const from = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed getter!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed getter!");
	auto const field = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed getter!");
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed getter!");
	if (context.stream.current().type != Type::LTS_TT_LITTLE_ARROW)
		MINIMA_ERROR(InvalidValue, "Expected '->' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed getter!");
	auto const to = getDataLocation(context);
	get.from	= from.at;
	get.to		= to.at;
	get.field	= field.at;
	if (field.id < Makai::Limit::MAX<uint64>)
		context.addInstruction(field.id);
	if (from.id < Makai::Limit::MAX<uint64>)
		context.addInstruction(from.id);
	if (to.id < Makai::Limit::MAX<uint64>)
		context.addInstruction(to.id);
	context.addInstructionType(getID, get);
}

MINIMA_ASSEMBLE_FN(Set) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed setter!");
	auto const setID = context.addNamedInstruction(Instruction::Name::AV2_IN_SET);
	Instruction::SetRequest set;
	auto const to = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed setter!");
	if (context.stream.current().type != Type::LTS_TT_LITTLE_ARROW)
		MINIMA_ERROR(InvalidValue, "Expected '->' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed setter!");
	auto const from = getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed setter!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed setter!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed setter!");
	auto const field = getDataLocation(context);
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	set.from	= from.at;
	set.to		= to.at;
	set.field	= field.at;
	if (field.id < Makai::Limit::MAX<uint64>)
		context.addInstruction(field.id);
	if (from.id < Makai::Limit::MAX<uint64>)
		context.addInstruction(from.id);
	if (to.id < Makai::Limit::MAX<uint64>)
		context.addInstruction(to.id);
	context.addInstructionType(setID, set);
}

MINIMA_ASSEMBLE_FN(Cast) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed cast!");
	auto const castID = context.addNamedInstruction(Instruction::Name::AV2_IN_CAST);
	auto const from	= getDataLocation(context);
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed cast!");
	if (context.stream.current().type != Type{':'})
		MINIMA_ERROR(InvalidValue, "Expected ':' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed cast!");
	auto const type = getType(context);
	if (!context.isCastable(type)) MINIMA_ERROR(InvalidValue, "Casts can only happen to scalar types, string and [any]!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed cast!");
	if (!context.hasToken(LTS_TT_LITTLE_ARROW))
		context.error("Expected '->' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed cast!");
	auto const to	= getDataLocation(context);
	if (type != context.DVK_ANY) {
		Instruction::Casting cast = {from.at, to.at, type};
		context.addInstructionType(castID, cast);
	} else {
		context.instruction(castID).name = Instruction::Name::AV2_IN_COPY;
		context.addInstructionType(castID, Instruction::Transfer{from.at, to.at});
	}
	if (from.id < Makai::Limit::MAX<usize>)
		context.addInstruction(from.id);
	if (to.id < Makai::Limit::MAX<usize>)
		context.addInstruction(to.id);
}

MINIMA_ASSEMBLE_FN(Label) {
	auto const name = context.stream.current();
	if (name.type != LTS_TT_IDENTIFIER)
		MINIMA_ERROR(InvalidValue, "Label name must be an identifier!");
	if (!context.stream.next() && context.stream.current().type != Type{':'})
		MINIMA_ERROR(NonexistentValue, "Malformed jump label!");
	auto const id = name.value.get<Makai::String>();
	context.jumps.labels[id] = context.program.code.size();
	context.program.labels.jumps[id] = context.program.jumpTable.size();
	context.program.jumpTable.pushBack(context.program.code.size());
	auto const nopID = context.addNamedInstruction(Instruction::Name::AV2_IN_NO_OP);
	context.instruction(nopID).type = 1;
}

MINIMA_ASSEMBLE_FN(Indirect) {
	// TODO: This (what was this supposed to be, again?)
	// "Indirect" reads & writes (i.e. uhhhhhhhhhhhh)
}

MINIMA_ASSEMBLE_FN(Hook) {
	context.fetchNext();
	if (!context.hasToken(LTS_TT_IDENTIFIER))
		MINIMA_ERROR(InvalidValue, "Hook must be an identifier!");
	auto const hookName = context.stream.current().value;
	context.program.ani.in[hookName] = 0;
	doLabel(context);
}

MINIMA_ASSEMBLE_FN(RandomNumber) {
	context.fetchNext().expectToken(LTS_TT_IDENTIFIER, "RNG operation");
	auto id = context.currentValue().getString();
	DEBUGLN("RNG Action: [", id, "]");
	Instruction::Randomness rng;
	if (id == "seed") {
		context.fetchNext().expectToken(LTS_TT_IDENTIFIER, "RNG seed operation");
		auto const op = context.currentValue().getString();
		context.fetchNext().expectToken(LTS_TT_LITTLE_ARROW);
		context.fetchNext();
		auto const seed = getDataLocation(context);
		auto const inst = context.addNamedInstruction(Instruction::Name::AV2_IN_RANDOM);
		if (op == "set")
			rng.flags = Instruction::Randomness::Flags::AV2_IRF_SET_SEED;
		else if (op == "get")
			rng.flags = Instruction::Randomness::Flags::AV2_IRF_GET_SEED;
		else context.error("Invalid RNG seed operation!");
		rng.num = seed.at;
		context.addInstructionType(inst, rng);
		context.addInstruction(seed.id);
		return;
	}
	auto const inst = context.addNamedInstruction(Instruction::Name::AV2_IN_RANDOM);
	rng.flags = Instruction::Randomness::Flags::AV2_IRF_SET_SEED;
	bool secure = false;
	if (id == "secure" || id == "safe" || id == "srng") {
		secure = false;
	} else if (id == "pseudo" || id == "fast" || id == "prng") {
		secure = true;
	} else context.error("Invalid RNG operation!");
	context.fetchNext().expectToken(LTS_TT_IDENTIFIER, "RNG operation");
	id = context.currentValue().getString();
	if (id == "float" || id == "real" || id == "r") {
		rng.type = decltype(rng.type)::AV2_IRT_REAL;
	} else if (id == "signed" || id == "int" || id == "i") {
		rng.type = decltype(rng.type)::AV2_IRT_INT;
	} else if (id == "unsigned" || id == "uint" || id == "u") {
		rng.type = decltype(rng.type)::AV2_IRT_UINT;
	} else context.error("Invalid RNG generation type!");
	context.fetchNext();
	Location num;
	switch (context.currentToken().type) {
		case Type{'('}: {
			Instruction::Randomness::Number numDecl = {};
			auto const lo = getDataLocation(context.fetchNext());
			context.fetchNext().expectToken(Type{':'});
			auto const hi = getDataLocation(context.fetchNext());
			context.fetchNext().expectToken(Type{')'});
			rng.flags = decltype(rng.flags)::AV2_IRF_BOUNDED;
			context.fetchNext().expectToken(LTS_TT_LITTLE_ARROW);
			num = getDataLocation(context.fetchNext());
			numDecl.lo = lo.at;
			numDecl.hi = hi.at;
			context.addInstruction(numDecl);
			if (lo.id < Makai::Limit::MAX<uint64>)
				context.addInstruction(lo.id);
			if (hi.id < Makai::Limit::MAX<uint64>)
				context.addInstruction(hi.id);
		} break;
		case LTS_TT_LITTLE_ARROW: {
			num = getDataLocation(context.fetchNext());
		} break;
		default: context.error("Invalid RNG operation!");
	}
	if (secure)
		rng.flags = Makai::Cast::as<decltype(rng.flags)>(
			Makai::enumcast(decltype(rng.flags)::AV2_IRF_SECURE)
		|	Makai::enumcast(rng.flags)
		);
	context.addInstructionType(inst, rng);
	if (num.id < Makai::Limit::MAX<uint64>)
		context.addInstruction(num.id);
}

MINIMA_ASSEMBLE_FN(StructuredOperation) {
	// TODO: Structured operations
}

MINIMA_ASSEMBLE_FN(Instantiation) {
	// TODO: Instantiation
}

MINIMA_ASSEMBLE_FN(Expression) {
	auto const current = context.stream.current();
	if (current.type == LTS_TT_IDENTIFIER) {
		auto const id = current.value.get<Makai::String>();
		if (id == "jump" || id == "go")							doJump(context);
		else if (id == "nop" || id == "next")					doNoOp(context);
		else if (id == "swap")									doStackSwap(context);
		else if (id == "flush")									doStackFlush(context);
		else if (id == "push")									doStackPush(context);
		else if (id == "pop")									doStackPop(context);
		else if (id == "clear")									doStackClear(context);
		else if (id == "return" || id == "ret" || id == "end")	doReturn(context);
		else if (id == "terminate" || id == "halt")				doHalt(context);
		else if (id == "error" || id == "err")					doErrorHalt(context);
		else if (id == "call" || id == "do")					doCall(context);
		else if (id == "compare" || id == "cmp")				doCompare(context);
		else if (id == "copy")									doCopy(context);
		else if (id == "context" || id == "mode")				doContext(context);
		else if (id == "loose" || id == "strict")				doImmediateContext(context);
		else if (id == "binop" || id == "bmath" || id == "bop")	doBinaryMath(context);
		else if (id == "unop"|| id == "uop")					doUnaryMath(context);
		else if (id == "yield")									doYield(context);
		else if (id == "await" || id == "wait")					doAwait(context);
		else if (id == "convert" || id == "cast")				doCast(context);
		else if (id == "read" || id == "get")					doGet(context);
		else if (id == "write" || id == "set")					doSet(context);
		else if (id == "indirect"  || id == "ref")				doIndirect(context);
		else if (id == "in")									doHook(context);
		else if (id == "random" || id == "rng")					doRandomNumber(context);
		else if (id == "struct" || id == "data")				doStructuredOperation(context);
		else if (id == "new")									doInstantiation(context);
		else doLabel(context);
	} else MINIMA_ERROR(InvalidValue, "Instruction must be an identifier!");
}

void Minima::assemble() {
	while (context.stream.next()) doExpression(context);
	auto const unmapped = context.mapJumps();
	if (unmapped.size())
		MINIMA_ERROR(NonexistentValue, "Some jump targets do not exist!\nTargets:\n[" + unmapped.join("]\n[") + "]");
	for (auto& [id, loc]: context.program.labels.jumps)
		if (context.program.ani.in.contains(id))
			context.program.ani.in[id] = context.program.labels.jumps[id];
}
CTL_DIAGBLOCK_END
