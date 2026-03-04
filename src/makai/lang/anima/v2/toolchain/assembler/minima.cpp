#include "minima.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = Minima::TokenStream::Token::Type;
using enum Type;

using Context = Minima::Context;

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

struct Location {
	DataLocation	source;
	uint64			id;

	constexpr Location operator|(Location const& other) const {
		return {
			source | other.source,
			other.id
		};
	}

	constexpr Location& operator|=(Location const& other) {
		source = source | other.source;
		id = other.id;
		return *this;
	}
};

static DataLocation getLoadType(Context& context, DataLocation const prev) {
	DataLocation locAt = asPlace(prev);
	if (context.hasToken(LTS_TT_IDENTIFIER)) {
		auto const id = context.getValue<Makai::String>();
		if (id == "reference" || id == "ref")
			locAt = locAt | DataLocation::AV2_DLM_BY_REF;
		else if (id == "move")
			locAt = locAt | DataLocation::AV2_DLM_MOVE;
		else if (id == "value" || id == "copy")
			locAt = locAt | DataLocation{0};
		context.fetchNext();
	}
	return locAt;
}

static Location getStack(Context& context) {
	Location loc;
	context
		.fetchNext()
		.expectToken(Type{'['})
	;
	context.fetchNext();
	bool backshots = false;
	while (
		context.hasToken(Type{'-'})
	||	context.hasToken(Type{'+'})
	) {
		if (context.hasToken(Type{'-'}))
			backshots = !backshots;
		context.fetchNext();
	}
	loc.id = context.fetchToken(LTS_TT_INTEGER, "stack index").getUnsigned();
	loc.source = loc.source | (
		backshots
	?	DataLocation::AV2_DL_STACK_OFFSET
	:	DataLocation::AV2_DL_STACK
	);
	context.fetchNext().expectToken(Type{']'});
	return loc;
}

static Location getLocal(Context& context) {
	auto const localID =
		context
			.fetchNext()
			.expectToken(Type{'['})
			.fetchNext()
			.fetchToken(LTS_TT_IDENTIFIER, "local stack ID")
			.getUnsigned()
	;
	context.fetchNext().expectToken(Type{']'});
	return {
		DataLocation::AV2_DL_LOCAL,
		localID
	};
}

static Location getExtern(Context& context) {
	context.fetchNext();
	uint64 externID = 0;
	switch (context.currentToken().type) {
		case LTS_TT_IDENTIFIER:
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
			externID = context.addConstant(context.currentValue().getString());
		break;
		default: context.error("Expected external location name here!");
	}
	return {DataLocation::AV2_DL_EXTERNAL, externID};
}

static Location getGlobal(Context& context) {
	auto const name =
		context
			.fetchNext()
			.fetchToken(LTS_TT_IDENTIFIER, "global name")
			.getString()
	;
	uint64 globalID = 0;
	if (context.program.labels.globals.contains(name))
		globalID = context.program.labels.globals[name];
	else {
		globalID = context.program.labels.globals.size();
		context.program.labels.globals[name] = globalID;
	}
	return {
		DataLocation::AV2_DL_GLOBAL,
		globalID
	};
}

static Location getConstantLocation(Context& context) {
	auto type = context.currentToken().type;
	bool isNumber = false;
	bool negated = false;
	while (
		type == Type{'+'}
	||	type == Type{'-'}
	) {
		isNumber = true;
		if (type == Type{'-'})
			negated = !negated;
		type = context.fetchNext().currentToken().type;
	}
	Location loc {.source = DataLocation::AV2_DL_CONST};
	if (isNumber) {
		switch (context.currentToken().type) {
			case LTS_TT_INTEGER:
				loc.id = context.addConstant(context.currentValue().getSigned() * (negated ? -1 : +1));
			break;
			case LTS_TT_REAL:
				loc.id = context.addConstant(context.currentValue().getReal() * (negated ? -1 : +1));
			break;
			default: context.error("Expected number here!");
		}
	} else {
		switch (context.currentToken().type) {
			case LTS_TT_SINGLE_QUOTE_STRING:
			case LTS_TT_DOUBLE_QUOTE_STRING:
			case LTS_TT_INTEGER:
			case LTS_TT_REAL:
			case LTS_TT_CHARACTER:
				loc.id = context.addConstant(context.currentValue());
			break;
			default: context.error("Invalid constant!");
		}
	}
	context.error("Invalid constant!");
}

static Location getLabelLocation(Context& context) {
	auto const current =
		context
			.fetchNext()
			.fetchToken(LTS_TT_IDENTIFIER, "label name")
			.getString()
	;
	if (!context.jumps.labels.contains(current))
		context.error("Jump target has not been declared yet!");
	return {DataLocation::AV2_DL_CONST, context.addConstant(context.jumps.labels[current])};
}

static Location getDataLocation(Context& context) {
	Location loc;
	bool hasLoadType = false;
	while (context.hasToken(LTS_TT_IDENTIFIER)) {
		auto const lt = context.getValue<Makai::String>();
		if (
			lt == "move"
		||	lt == "ref"
		||	lt == "copy"
		) {
			if (!hasLoadType) {
				hasLoadType = true;
				loc.source = getLoadType(context, DataLocation{0});
			}
			context.fetchNext();
			continue;
		} else break;
	}
	switch (context.currentToken().type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = context.getValue<Makai::String>();
			if (id == "local" || id == "arg")			loc |= getLocal(context);
			else if (id == "placeof")					loc |= getLabelLocation(context);
			else if (id == "stack")						loc |= getStack(context);
			else if (id == "global" || id == "g")		loc |= getGlobal(context);
			else if (id == "external" || id == "out")	loc |= getExtern(context);
			else if (id == "temporary" || id == "temp")	loc |= {DataLocation::AV2_DL_TEMPORARY, uint64(-1)};
			else if (id == "false")						loc |= {DataLocation::AV2_DL_INTERNAL, 0};
			else if (id == "true")						loc |= {DataLocation::AV2_DL_INTERNAL, 1};
			else if (id == "undefined" || id == "void")	loc |= {DataLocation::AV2_DL_INTERNAL, 2};
			else if (id == "null" || id == "nil")		loc |= {DataLocation::AV2_DL_INTERNAL, 3};
			else if (id == "nan")						loc |= {DataLocation::AV2_DL_INTERNAL, 4};
			else if (id == "array" || id == "arr")		loc |= {DataLocation::AV2_DL_INTERNAL, 8};
			else if (id == "bytes" || id == "bin")		loc |= {DataLocation::AV2_DL_INTERNAL, 9};
			else if (id == "object" || id == "obj")		loc |= {DataLocation::AV2_DL_INTERNAL, 10};
			else context.error("Invalid data source!");
		}
		case Type{'&'}:
			loc |= getLabelLocation(context);
		break;
		case Type{':'}:
			loc |= getExtern(context);
		break;
		case Type{'$'}:
			loc |= getGlobal(context);
		break;
		case Type{'@'}:
			loc |= getLocal(context);
		case Type{'.'}:
			loc |= loc |= {DataLocation::AV2_DL_TEMPORARY, uint64(-1)};
		break;
		case Type{'+'}:
		case Type{'-'}:
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
		case LTS_TT_CHARACTER:
		case LTS_TT_INTEGER:
		case LTS_TT_REAL:
			loc |= getConstantLocation(context);
		break;
		default: context.error("Invalid data source!");
	}
	return loc;
}

static void doConditionalJump(Context& context, bool dynamic = false) {
	context.fetchNext();
	Instruction::Leap leap{.dyn = dynamic};
	switch (context.currentToken().type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = context.getValue<Makai::String>();
			if (id == "zero" || id == "z")				leap.type = Instruction::Leap::AV2_ILT_IF_ZERO;
			else if (id == "nonZero" || id == "nz")		leap.type = Instruction::Leap::AV2_ILT_IF_NOT_ZERO;
			else if (id == "negative" || id == "n")		leap.type = Instruction::Leap::AV2_ILT_IF_NEGATIVE;
			else if (id == "positive" || id == "p")		leap.type = Instruction::Leap::AV2_ILT_IF_POSITIVE;
			else if (id == "true" || id == "t")			leap.type = Instruction::Leap::AV2_ILT_IF_TRUTHY;
			else if (id == "false" || id == "f")		leap.type = Instruction::Leap::AV2_ILT_IF_FALSY;
			else if (id == "null" || id == "nil")		leap.type = Instruction::Leap::AV2_ILT_IF_NULL;
			else if (id == "void" || id == "v")			leap.type = Instruction::Leap::AV2_ILT_IF_UNDEFINED;
			else if (id == "nilOrVoid" || id == "_")	leap.type = Instruction::Leap::AV2_ILT_IF_NULL_OR_UNDEFINED;
		}
	}
	if (!dynamic)
		context.addJumpTarget(context.fetchNext.fetchToken(LTS_TT_IDENTIFIER, "jump expression").getString());
}

static void doJump(Context& context, bool dynamic = false) {
	context.fetchNext();
	if (!dynamic) context.expectToken(LTS_TT_IDENTIFIER, "jump expression");
	else if (!context.hasToken(LTS_TT_IDENTIFIER)) {
		context.append.pad(1);
		context.addInstructionType(
			context.addNamedInstruction(Instruction::Name::AV2_IN_JUMP),
			Instruction::Leap{
				Instruction::Leap::AV2_ILT_UNCONDITIONAL,
				true
			}
		);
	}
	auto const jt = context.getValue<Makai::String>();
	if (jt == "if")
		doConditionalJump(context, dynamic);
	else if (!dynamic)
		context.addJumpTarget(jt);
	else context.error("Invalid jump instruction!");
}

static void doCall(Context& context, bool dynamic = false) {
	context.fetchNext();
}

static void doDynamic(Context& context) {
	auto const id = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "dynamic operation").getString();
	if (id == "jump" || id == "go")
		doJump(context, true);
	else if (id == "call" || id == "do")
		doCall(context, true);
}

static void doNoOp(Context& context) {
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_NO_OP),
		Makai::Cast::as<uint32>(context.currentValue().getString() == "next")
	);
}

static void doStackSwap(Context& context) {
	context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_SWAP);
}

static void doStackFlush(Context& context) {
	context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_FLUSH);
}

static void doStackPush(Context& context) {

}

static void doStackPop(Context& context) {

}

static void doStackClear(Context& context) {
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_CLEAR),
		Makai::Cast::as<uint32>(context.fetchNext().fetchToken(LTS_TT_INTEGER).getUnsigned())
	);
}

static void doExpression(Context& context) {
	auto const id = context.fetchToken(LTS_TT_IDENTIFIER, "instruction name").getString();
	if (id == "jump" || id == "go")							doJump(context);
	if (id == "dyn")										doDynamic(context);
	else if (id == "nop" || id == "next")					doNoOp(context);
	else if (id == "swap")									doStackSwap(context);
	else if (id == "flush")									doStackFlush(context);
	else if (id == "push")									doStackPush(context);
	else if (id == "pop")									doStackPop(context);
	else if (id == "clear")									doStackClear(context);
	else if (id == "return" || id == "ret" || id == "end")	doReturn(context);
	else if (id == "terminate" || id == "halt")				doHalt(context);
	else if (id == "error" || id == "err")					doErrorHalt(context);
	else if (id == "exit")									doExitHalt(context);
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
