#include "minima.hpp"

using namespace Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Assembler;
namespace Runtime = Makai::Anima::V2::Runtime;
using Type = Minima::TokenStream::Token::Type;
using enum Type;

using Context = Minima::Context;

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

struct Location {
	DataLocation			source;
	Makai::Nullable<uint64>	id		= null;

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

	constexpr bool hasID() const {
		return id.exists();
	}

	constexpr void addID(Context& context) const {
		if (hasID()) context.code.add(id.value());
	}
};

static DataLocation getLoadType(Context& context, DataLocation const prev) {
	DataLocation locAt = asPlace(prev);
	if (context.has(LTS_TT_IDENTIFIER)) {
		auto const id = context.value().getString();
		if (id == "reference" || id == "ref")
			locAt = locAt | DataLocation::AV2_DLM_BY_REF;
		else if (id == "move")
			locAt = locAt | DataLocation::AV2_DLM_MOVE;
		else if (id == "value" || id == "copy")
			locAt = locAt | DataLocation{0};
		context.next();
	}
	return locAt;
}

static Location getStack(Context& context) {
	Location loc;
	context
		.next()
		.expect(Type{'['})
	;
	context.next();
	bool backshots = false;
	while (
		context.has(Type{'-'})
	||	context.has(Type{'+'})
	) {
		if (context.has(Type{'-'}))
			backshots = !backshots;
		context.next();
	}
	loc.id = context.get(LTS_TT_INTEGER, "stack index").getUnsigned();
	loc.source = loc.source | (
		backshots
	?	DataLocation::AV2_DL_STACK_OFFSET
	:	DataLocation::AV2_DL_STACK
	);
	context.expectNext(Type{']'});
	return loc;
}

static Location getLocal(Context& context) {
	auto const localID =
		context
			.expectNext(Type{'['})
			.get(LTS_TT_IDENTIFIER, "local stack ID")
			.getUnsigned()
	;
	context.expectNext(Type{']'});
	return {
		DataLocation::AV2_DL_LOCAL,
		localID
	};
}

static Location getExtern(Context& context) {
	context.next();
	uint64 externID = 0;
	switch (context.current().type) {
		case LTS_TT_IDENTIFIER:
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
			externID = context.minima.addConstant(context.value().getString());
		break;
		default: context.error("Expected external location name here!");
	}
	return {DataLocation::AV2_DL_EXTERNAL, externID};
}

static Location getGlobal(Context& context) {
	auto const name =
		context
			.getNext(LTS_TT_IDENTIFIER, "global name")
			.getString()
	;
	uint64 globalID = context.minima.addGlobal(name);
	return {
		DataLocation::AV2_DL_GLOBAL,
		globalID
	};
}

static Location getConstantLocation(Context& context) {
	auto type = context.current().type;
	bool isNumber = false;
	bool negated = false;
	while (
		type == Type{'+'}
	||	type == Type{'-'}
	) {
		isNumber = true;
		if (type == Type{'-'})
			negated = !negated;
		type = context.next().current().type;
	}
	Location loc {.source = DataLocation::AV2_DL_CONST};
	if (isNumber) {
		switch (context.current().type) {
			case LTS_TT_INTEGER:
				loc.id = context.minima.addConstant(context.value().getSigned() * (negated ? -1 : +1));
			break;
			case LTS_TT_REAL:
				loc.id = context.minima.addConstant(context.value().getReal() * (negated ? -1 : +1));
			break;
			default: context.error("Expected number here!");
		}
	} else {
		switch (context.current().type) {
			case LTS_TT_SINGLE_QUOTE_STRING:
			case LTS_TT_DOUBLE_QUOTE_STRING:
			case LTS_TT_INTEGER:
			case LTS_TT_REAL:
			case LTS_TT_CHARACTER:
				loc.id = context.minima.addConstant(context.value());
			break;
			default: context.error("Invalid constant!");
		}
	}
	context.error("Invalid constant!");
}

static Location getLabelLocation(Context& context) {
	auto const current =
		context
			.getNext(LTS_TT_IDENTIFIER, "label name")
			.getString()
	;
	if (!context.minima.jumps.labels.contains(current))
		context.error("Jump target has not been declared yet!");
	return {DataLocation::AV2_DL_CONST, context.minima.addConstant(context.minima.jumps.labels[current])};
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
			else if (id == "temporary" || id == "temp")	loc |= Location{DataLocation::AV2_DL_TEMPORARY, uint64{-1}};
			else if (id == "false")						loc |= Location{DataLocation::AV2_DL_INTERNAL, uint64{0}};
			else if (id == "true")						loc |= Location{DataLocation::AV2_DL_INTERNAL, uint64{1}};
			else if (id == "void")						loc |= Location{DataLocation::AV2_DL_INTERNAL, uint64{2}};
			else if (id == "null" || id == "nil")		loc |= Location{DataLocation::AV2_DL_INTERNAL, uint64{3}};
			else if (id == "bytes" || id == "bin")		loc |= Location{DataLocation::AV2_DL_INTERNAL, uint64{9}};
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
			loc |= Location{DataLocation::AV2_DL_TEMPORARY, uint64{-1}};
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
			if (id == "zero" || id == "z")				leap.type = Instruction::Leap::Type::AV2_ILT_IF_ZERO;
			else if (id == "nonzero" || id == "nz")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NOT_ZERO;
			else if (id == "negative" || id == "n")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NEGATIVE;
			else if (id == "positive" || id == "p")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_POSITIVE;
			else if (id == "true" || id == "t")			leap.type = Instruction::Leap::Type::AV2_ILT_IF_TRUTHY;
			else if (id == "false" || id == "f")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_FALSY;
			else if (id == "null" || id == "nil")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NULL;
			else if (id == "void" || id == "v")			leap.type = Instruction::Leap::Type::AV2_ILT_IF_UNDEFINED;
			else if (id == "empty" || id == "e")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NULL_OR_UNDEFINED;
		}
	}
	if (!dynamic)
		context.addJumpTarget(context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "jump expression").getString());
}

static void doJump(Context& context, bool dynamic = false) {
	context.fetchNext();
	if (!dynamic) context.expectToken(LTS_TT_IDENTIFIER, "jump expression");
	else if (!context.hasToken(LTS_TT_IDENTIFIER)) {
		context.append.pad(1);
		context.addInstructionType(
			context.addNamedInstruction(Instruction::Name::AV2_IN_JUMP),
			Instruction::Leap{
				Instruction::Leap::Type::AV2_ILT_UNCONDITIONAL,
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
	Instruction::Invocation invoke {.dynamic = dynamic};
	if (!dynamic) {
		auto id = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "call expression").getString();
		context.addInstructionType(
			context.addNamedInstruction(Instruction::Name::AV2_IN_CALL),
			invoke
		);
		if (context.minima.methods.contains(id))
			context.addJumpTarget(context.minima.methods[id]->entry);
		else context.error("Method with this name does not exist!");
	} else {
		context.addInstructionType(
			context.addNamedInstruction(Instruction::Name::AV2_IN_CALL),
			invoke
		);
	}
}

static void doNoOp(Context& context) {
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_NO_OP),
		Makai::Cast::as<uint32>(context.currentValue().getString() == "next")
	);
}

static void doStackSwap(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_SWAP);
}

static void doStackFlush(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_FLUSH);
}

static void doStackPush(Context& context) {
	context.fetchNext();
	auto const src = getDataLocation(context);
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_PUSH),
		Instruction::StackPush{src.source}
	);
	src.addID(context);
}

static void doStackPop(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_POP);
}

static void doStackBlit(Context& context) {
	Instruction::Blitting blit {.type = Instruction::Blitting::Type::AV2_IBT_COPY};
	auto const count = context.fetchNext().fetchToken(LTS_TT_INTEGER).getUnsigned();
	context.fetchNext().expectToken(LTS_TT_LITTLE_ARROW).fetchNext();
	auto const src = context.fetchToken(LTS_TT_IDENTIFIER, "blit source").getString();
	if (src == "local" || src == "l")
		blit.fromGlobal = true;
	else if (src == "global" || src == "g")
		blit.fromGlobal = false;
	else context.error("Invalid blit source!");
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_BLIT),
		blit
	);
	context.addInstruction(count);
}

static void doReturn(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_RETURN);
}

static void doStackClear(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_STACK_CLEAR);
	context.addInstruction(context.fetchNext().fetchToken(LTS_TT_INTEGER).getUnsigned());
}

static void doFieldGet(Context& context, bool const dyn = false) {
	// TODO: The rest of this
	if (!dyn) {
		auto const field =
		context
			.fetchNext()
			.expectToken(Type{'['})
			.fetchNext()
			.fetchToken(LTS_TT_IDENTIFIER, "field ID")
			.getUnsigned()
		;
		context.fetchNext().expectToken(Type{']'});
	}
}

static void doArrayAt(Context& context, bool const dyn = false) {
	// TODO: The rest of this
	if (!dyn) {
		auto const index =
			context
				.fetchNext()
				.expectToken(Type{'['})
				.fetchNext()
				.fetchToken(LTS_TT_IDENTIFIER, "array index")
				.getUnsigned()
		;
		context.fetchNext().expectToken(Type{']'});
	}
}

static void doSizeOf(Context& context, bool const inBytes = false) {
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_SIZEOF),
		Makai::Cast::as<uint32>(inBytes)
	);
}

static void doTypeGet(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_TYPEOF);
}

static void doHalt(Context& context, bool const error = false) {
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_HALT),
		Instruction::Stop{
			error
		?	Instruction::Stop::Mode::AV2_ISM_ERROR
		:	Instruction::Stop::Mode::AV2_ISM_NORMAL
		}
	);
}

static void doCompare(Context& context) {
	auto const id = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "comparison").getString();
	Instruction::Comparison cmp;
	if (id == "less" || id == "l")					cmp.comp = Comparator::AV2_OP_LESS_THAN;
	else if (id == "greater" || id == "g")			cmp.comp = Comparator::AV2_OP_GREATER_THAN;
	else if (id == "equals" || id == "e")			cmp.comp = Comparator::AV2_OP_EQUALS;
	else if (id == "notequals" || id == "ne")		cmp.comp = Comparator::AV2_OP_NOT_EQUALS;
	else if (id == "greaterequals" || id == "ge")	cmp.comp = Comparator::AV2_OP_GREATER_EQUALS;
	else if (id == "lessequals" || id == "le")		cmp.comp = Comparator::AV2_OP_LESS_EQUALS;
	else if (id == "order" || id == "o")			cmp.comp = Comparator::AV2_OP_THREEWAY;
}

static void doScopeEnter(Context& context) {
	auto const count = context.fetchNext().fetchToken(LTS_TT_INTEGER, "scope-local stack size");
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_SCOPE_ENTER),
		Makai::Cast::as<uint32>(count)
	);
}

static void doScopeExit(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_SCOPE_EXIT);
}

static void doScopeBind(Context& context) {
	auto const count = context.fetchNext().fetchToken(LTS_TT_INTEGER, "bind count").getUnsigned();
	context.fetchNext().expectToken(Type{':'}).fetchNext().expectToken(Type{'['}).fetchNext();
	auto const src = context.fetchToken(LTS_TT_INTEGER, "global stack top offset").getUnsigned();
	context.fetchNext().expectToken(LTS_TT_LITTLE_ARROW).fetchNext();
	auto const dst = context.fetchToken(LTS_TT_INTEGER, "local stack bottom offset").getUnsigned();
	context.fetchNext().expectToken(Type{']'}).fetchNext();
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_SCOPE_BIND),
		Instruction::Binding {
			src,
			dst
		}
	);
	context.addInstruction(count);
}

static void doScopeBring(Context& context) {
	auto const count = context.fetchNext().fetchToken(LTS_TT_INTEGER, "bind count").getUnsigned();
	context.fetchNext().expectToken(Type{'['}).fetchNext();
	auto const src = context.fetchToken(LTS_TT_INTEGER, "source scope").getUnsigned();
	auto const offset = context.fetchNext().expectToken(Type{'['}).fetchToken(LTS_TT_INTEGER, "source local stack bottom offset").getUnsigned();
	context.fetchNext().expectToken(Type{']'}).fetchNext().expectToken(Type{LTS_TT_LITTLE_ARROW}).fetchNext();
	auto const dst = context.fetchToken(LTS_TT_INTEGER, "source local stack bottom offset").getUnsigned();
	context.fetchNext().expectToken(Type{']'});
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_SCOPE_BRING),
		Instruction::Binding {
			offset,
			dst
		}
	);
	context.addInstruction(src);
	context.addInstruction(count);
}


static void doCopy(Context& context) {
	auto const src = getDataLocation(context.fetchNext());
	context.fetchNext().expectToken(LTS_TT_LITTLE_ARROW);
	auto const dst = getDataLocation(context.fetchNext());
	Instruction::Transfer tf {
		src.source,
		dst.source
	};
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_COPY),
		tf
	);
	src.addID(context);
	dst.addID(context);
}

static void doContext(Context& context, bool immediate = false) {
	auto const mode = context.fetchToken(LTS_TT_IDENTIFIER, "context").getString();
	Instruction::Context ctx{.immediate = immediate};
	if (mode == "loose")		ctx.mode = ContextMode::AV2_CM_LOOSE;
	else if (mode == "strict")	ctx.mode = ContextMode::AV2_CM_STRICT;
	else context.error("Invalid context!");
	context.addInstructionType(
		context.addNamedInstruction(Instruction::Name::AV2_IN_MODE),
		ctx
	);
}

static void doBinaryOperation(Context& context) {
	Instruction::BinaryOperation bop;
	auto const op = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "binary operation name").getString();
	if (op == "add")			bop.op = BinaryOperator::AV2_BOP_ADD;
	else if (op == "sub")		bop.op = BinaryOperator::AV2_BOP_SUB;
	else if (op == "mul")		bop.op = BinaryOperator::AV2_BOP_MUL;
	else if (op == "div")		bop.op = BinaryOperator::AV2_BOP_DIV;
	else if (op == "mod")		bop.op = BinaryOperator::AV2_BOP_REM;
	else if (op == "land")		bop.op = BinaryOperator::AV2_BOP_LOGIC_AND;
	else if (op == "lor")		bop.op = BinaryOperator::AV2_BOP_LOGIC_OR;
	else if (op == "lxor")		bop.op = BinaryOperator::AV2_BOP_LOGIC_XOR;
	else if (op == "band")		bop.op = BinaryOperator::AV2_BOP_BIT_AND;
	else if (op == "bor")		bop.op = BinaryOperator::AV2_BOP_BIT_OR;
	else if (op == "bxor")		bop.op = BinaryOperator::AV2_BOP_BIT_XOR;
}

static void doUnaryOperation(Context& context) {
	Instruction::UnaryOperation bop;
	auto const op = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "unary operation name").getString();
	if (op == "neg")		bop.op = UnaryOperator::AV2_UOP_NEGATE;
	else if (op == "inc")	bop.op = UnaryOperator::AV2_UOP_INCREMENT;
	else if (op == "dec")	bop.op = UnaryOperator::AV2_UOP_DECREMENT;
	else if (op == "inv")	bop.op = UnaryOperator::AV2_UOP_INVERSE;
	else if (op == "sin")	bop.op = UnaryOperator::AV2_UOP_SIN;
	else if (op == "cos")	bop.op = UnaryOperator::AV2_UOP_COS;
	else if (op == "tan")	bop.op = UnaryOperator::AV2_UOP_TAN;
	else if (op == "asin")	bop.op = UnaryOperator::AV2_UOP_ASIN;
	else if (op == "acos")	bop.op = UnaryOperator::AV2_UOP_ACOS;
	else if (op == "atan")	bop.op = UnaryOperator::AV2_UOP_ATAN;
	else if (op == "sinh")	bop.op = UnaryOperator::AV2_UOP_SINH;
	else if (op == "cosh")	bop.op = UnaryOperator::AV2_UOP_COSH;
	else if (op == "tanh")	bop.op = UnaryOperator::AV2_UOP_TANH;
	else if (op == "log2")	bop.op = UnaryOperator::AV2_UOP_LOG2;
	else if (op == "log10")	bop.op = UnaryOperator::AV2_UOP_LOG10;
	else if (op == "ln")	bop.op = UnaryOperator::AV2_UOP_LN;
	else if (op == "sqrt")	bop.op = UnaryOperator::AV2_UOP_SQRT;
}

static void doYield(Context& context) {
	auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_YIELD);
}

static void doCast(Context& context, bool const dyn = false) {
	if (!dyn) {
		auto const type = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "type name").getString();
		if (context.minima.types.contains(type)) {
			auto const _ = context.addNamedInstruction(Instruction::Name::AV2_IN_CAST);
			context.addInstruction(context.minima.types[type]->id());
		} else context.error("Type with this name does not exist!");
	} else {
		context.addInstructionType(
			context.addNamedInstruction(Instruction::Name::AV2_IN_CAST),
			Instruction::Casting{.dynamic = true}
		);
	}
}

static void doRandomNumber(Context& context) {
	Instruction::Randomness rng;
	auto id = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "RNG operation").getString();
	if (id == "setseed")		rng.setSeed					= true;
	else if (id == "getseed")	rng.getSeed					= true;
	else if (id == "reseed")	rng.setSeed = rng.getSeed	= true;
	else if (id == "safe" || id == "fast") {
		rng.secure = id == "safe";
		if (context.fetchNext().hasToken(LTS_TT_IDENTIFIER) && context.getValue<Makai::String>() == "bounded")
			rng.bounded = true;
		else context.append.pad();
	} else context.error("Invalid RNG operation!");
}

static void doLabel(Context& context) {
	auto const name = context.fetchToken(LTS_TT_IDENTIFIER, "label name").getString();
	context.program.labels.jumps[name] = context.program.code.size();
}

static void doDynamic(Context& context) {
	auto const id = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "dynamic operation").getString();
	if (id == "jump" || id == "go")
		doJump(context, true);
	else if (id == "call" || id == "do")
		doCall(context, true);
	else if (id == "cast")
		doCast(context, true);
	else if (id == "field" || id == "get")
		doFieldGet(context, true);
	else if (id == "index" || id == "at")
		doArrayAt(context, true);
	else context.error("Invalid dynamic operation!");
}

static void declareTypeFields(Context& context, Context::Minima::Type& type) {
	if (type.fields.size())
		context.error("Redeclaration of type fields are not allowed!");
	else if (
		type.flags & (
			Definition::Flags::AV2_DF_DYNAMIC
		|	Definition::Flags::AV2_DF_ARRAY
		|	Definition::Flags::AV2_DF_BASIC
		)
	) context.error("Cannot declare fields on basic, array and dynamic types!");
	context.fetchNext().expectToken(Type{'['}).fetchNext();
	while (true) {
		if (context.fetchNext().hasToken(Type{']'})) break;
		auto const field = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "field type");
		if (!context.minima.types.contains(field))
			context.error("Field type does not exist!");
		type.fields.pushBack(context.minima.types[field]->id());
	}
}

static void declareType(Context& context) {
	auto const name = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "type name").getString();
		auto const type = new Context::Minima::Type();
		context.fetchNext().expectToken(Type{'['});
		while (true) {
			if (context.fetchNext().hasToken(Type{']'})) break;
			auto const flag = context.fetchToken(LTS_TT_IDENTIFIER, "type flag").getString();
			if (flag == "basic") {
				type->flags |= Definition::Flags::AV2_DF_BASIC;
				auto const basic =
					context
						.fetchNext()
						.expectToken(Type{'<'})
						.fetchNext()
						.fetchToken(LTS_TT_IDENTIFIER, "basic type")
						.getString()
				;
				if (basic == "void")		type->basic = BasicType::AV2_BT_VOID;
				else if (basic == "nil")	type->basic = BasicType::AV2_BT_NULL;
				else if (basic == "bool")	type->basic = BasicType::AV2_BT_BOOL;
				else if (basic == "int")	type->basic = BasicType::AV2_BT_INT;
				else if (basic == "uint")	type->basic = BasicType::AV2_BT_UINT;
				else if (basic == "real")	type->basic = BasicType::AV2_BT_REAL;
				else if (basic == "str")	type->basic = BasicType::AV2_BT_STRING;
				else if (basic == "bin")	type->basic = BasicType::AV2_BT_BYTES;
				else if (basic == "vec")	type->basic = BasicType::AV2_BT_VECTOR;
				else context.error("Invalid basic type!");
				context.fetchNext().expectToken(Type{'>'});
			}
			else if (flag == "nil") type->flags |= Definition::Flags::AV2_DF_NULLABLE;
			else if (flag == "derived") {
				if (type->flags | Definition::Flags::AV2_DF_ARRAY)
					context.error("Type cannot be both a derived type and an array type!");
				if (type->base)
					context.error("Redeclaration of base type!");
				auto const base =
					context
						.fetchNext()
						.expectToken(Type{'<'})
						.fetchNext()
						.fetchToken(LTS_TT_IDENTIFIER, "base type")
						.getString()
				;
				if (context.minima.types.contains(base))
					type->base = context.minima.types[base]->id();
				else context.error("Base type does not exist!");
				context.fetchNext().expectToken(Type{'>'});
			} else if (flag == "array") {
				type->flags |= Definition::Flags::AV2_DF_ARRAY;
				if (type->base)
					context.error("Redeclaration of element type!");
				auto const base =
					context
						.fetchNext()
						.expectToken(Type{'<'})
						.fetchNext()
						.fetchToken(LTS_TT_IDENTIFIER, "element type")
						.getString()
				;
				if (context.minima.types.contains(base))
					type->base = context.minima.types[base]->id();
				else context.error("Element type does not exist!");
				context.fetchNext().expectToken(Type{'>'});
			} else if (flag == "value")	type->flags |= Definition::Flags::AV2_DF_VALUE;
			else if (flag == "empty")	type->flags |= Definition::Flags::AV2_DF_EMPTY;
			else if (flag == "dyn")		type->flags |= Definition::Flags::AV2_DF_DYNAMIC;
			else if (flag == "struct")	type->flags |= Definition::Flags::AV2_DF_STRUCTURE;
			else if (flag == "align") {
				type->alignment =
					context
						.fetchNext()
						.expectToken(Type{'('})
						.fetchNext()
						.fetchToken(LTS_TT_INTEGER, "byte alignment")
						.getUnsigned()
				;
				context.fetchNext().expectToken(Type{')'});
			} else if (flag == "fields") {
				declareTypeFields(context, *type);
			} else if (flag == "copy")	type->flags |= Definition::Flags::AV2_DF_CLONABLE;
			else context.error("Invalid flag!");
		}
		if (!context.minima.types.contains(name))
			context.minima.types[name] = type;
		else context.error("Redeclaration of previously-declared type!");
}

static void declareAlias(Context& context) {
	auto const name = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "alias name");
	if (context.minima.types.contains(name))
		context.error("Type name is already in use!");
	auto const type =
		context
			.fetchNext()
			.expectToken(Type{':'})
			.fetchNext()
			.fetchToken(LTS_TT_IDENTIFIER, "aliased type name")
			.getString()
	;
	if (!context.minima.types.contains(type))
		context.error("Type to be aliased does not exist!");
	context.minima.types[name] = context.minima.types[type];
}

static void getMethodVisibility(Context& context, Context::Minima::Method& method) {
	while (context.fetchNext().hasToken(LTS_TT_IDENTIFIER)) {
		auto const vis = context.fetchToken(LTS_TT_IDENTIFIER, "visibility").getString();
		if (vis == "out")
			method.out = true;
		else if (vis == "local")
			method.local = true;
		else if (vis == "global")
			method.local = false;
		else if (vis == "in")
			method.out = false;
		else break;
	}
}

static void declareMethod(Context& context, bool forward = false) {
	auto const method = new Context::Minima::Method();
	getMethodVisibility(context, *method);
	auto id = context.fetchToken(LTS_TT_IDENTIFIER, "return type").getString();
	if (context.minima.types.contains(id))
		method->retType = context.minima.types[id]->id();
	else context.error("Return type does not exist!");
	context.fetchNext().expectToken(Type{'('});
	while (true) {
		if (context.fetchNext().hasToken(Type{')'})) break;
		id = context.fetchToken(LTS_TT_IDENTIFIER, "agument type").getString();
		if (context.minima.types.contains(id))
			method->argTypes.pushBack(context.minima.types[id]->id());
		else context.error("Argument type does not exist!");
	}
	auto const name = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "function name").getString();
	if (!forward) doLabel(context);
	method->entry = name;
	if (!context.minima.types.contains(name))
		context.minima.methods[name] = method;
	else context.error("Redeclaration of previously-declared method!");
}

static void declareModule(Context& context) {
	if (context.minima.module)
		context.error("Redeclaration of module name!");
	auto const name =
		context
			.fetchNext()
			.fetchToken(LTS_TT_IDENTIFIER, "module name")
			.getString()
	;
	context.minima.module = name;
}

static void declareImport(Context& context) {
	if (!context.minima.canImport)
		context.error("Import statements are disabled!");
	auto const path = (
		context.sourceDir
	+	context.fetchNext().fetchToken(LTS_TT_DOUBLE_QUOTE_STRING, "file name").getString()
	).replace('\\', '/');
	auto const mod = Makai::File::getText(path);
	Context ctx;
	ctx.minima	= context.minima;
	ctx.program	= context.program;
	ctx.minima.methods.filter(
		[] (auto const& e) {
			return !e.value->local;
		}
	);
	ctx.minima.module = null;
	ctx.minima.parentModule = context.minima.module ? context.minima.module : context.minima.parentModule;
	ctx.stream.open(mod);
	ctx.sourceDir = path.splitAtLast('/').front();
	Minima subm(ctx);
	subm.assemble();
	context.minima.methods.append(ctx.minima.methods);
	context.minima.types.append(ctx.minima.types);
	context.program	= ctx.program;
}

static void doDeclaration(Context& context) {
	auto const decl = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "declaration type").getString();
	if (decl == "hook") {
		auto const hook = context.fetchNext().fetchToken(LTS_TT_IDENTIFIER, "hook name").getString();
		doLabel(context);
		context.program.ani.in[hook] = context.program.labels.jumps[hook];
	} else if (decl == "fn")
		declareMethod(context);
	else if (decl == "def")
		declareMethod(context, true);
	else if (decl == "type")
		declareType(context);
	else if (decl == "module")
		declareModule(context);
	else if (decl == "import")
		declareImport(context);
	else if (decl == "alias")
		declareAlias(context);
	else context.error("Invalid declaration!");
}

static void doExpression(Context& context) {
	if (context.hasToken(Type{'@'}))
		return doDeclaration(context);
	auto const id = context.fetchToken(LTS_TT_IDENTIFIER, "instruction name").getString();
	if (id == "jump" || id == "go")				doJump(context);
	if (id == "dynamic" || id == "dyn")			doDynamic(context);
	else if (id == "nop" || id == "next")		doNoOp(context);
	else if (id == "swap")						doStackSwap(context);
	else if (id == "flush")						doStackFlush(context);
	else if (id == "push")						doStackPush(context);
	else if (id == "pop")						doStackPop(context);
	else if (id == "clear")						doStackClear(context);
	else if (id == "blit")						doStackBlit(context);
	else if (id == "return" || id == "ret")		doReturn(context);
	else if (id == "terminate" || id == "stop")	doHalt(context);
	else if (id == "error")						doHalt(context, true);
	else if (id == "call" || id == "do")		doCall(context);
	else if (id == "compare" || id == "cmp")	doCompare(context);
	else if (id == "enter" || id == "begin")	doScopeEnter(context);
	else if (id == "exit" || id == "end")		doScopeExit(context);
	else if (id == "bind")						doScopeBind(context);
	else if (id == "bring")						doScopeBring(context);
	else if (id == "copy")						doCopy(context);
	else if (id == "context" || id == "mode")	doContext(context.fetchNext());
	else if (id == "loose" || id == "strict")	doContext(context, true);
	else if (id == "binop" || id == "bop")		doBinaryOperation(context);
	else if (id == "unop"|| id == "uop")		doUnaryOperation(context);
	else if (id == "index" || id == "at")		doArrayAt(context);
	else if (id == "field" || id == "get")		doFieldGet(context);
	else if (id == "count")						doSizeOf(context);
	else if (id == "size")						doSizeOf(context, true);
	else if (id == "type")						doTypeGet(context);
	else if (id == "yield")						doYield(context);
	else if (id == "cast")						doCast(context);
	else if (id == "random" || id == "rng")		doRandomNumber(context);
	else doLabel(context);
}

void Minima::assemble() {
	while (context.stream.next()) doExpression(context);
	auto const unmapped = context.mapJumps();
	if (unmapped.size())
		context.error("Some jump targets do not exist!\nTargets:\n[" + unmapped.join("]\n[") + "]");
	for (auto& [id, loc]: context.program.labels.jumps)
		if (context.program.ani.in.contains(id))
			context.program.ani.in[id] = context.program.labels.jumps[id];
	context.minima.methods.filter(
		[] (auto const& e) {
			return !e.value->local;
		}
	);
	if (!(context.minima.module || context.minima.parentModule)) {
		context.program.types.resize(context.minima.types.size(), {});
		for (auto& [name, type]: context.minima.types) {
			auto& decl = context.program.types[type->id()];
			if (decl.aliases.size()) {
				decl.aliases.pushBack(name);
				continue;
			}
			decl = {
				.aliases	= Makai::StringList::from(name),
				.flags		= type->flags
			};
			if (type->basic)
				decl.basic = *type->basic;
			if (type->base)
				decl.base = *type->base;
		}
		context.program.methods.resize(context.minima.methods.size(), {});
		for (auto& [name, method]: context.minima.methods) {
			auto& decl = context.program.methods.pushBack({}).back();
			decl = {
				.id			= method->id(),
				.name		= name,
				.retType	= method->retType,
				.argTypes	= method->argTypes,
				.out		= method->out,
				.entry		= context.program.labels.jumps[method->entry]
			};
		}
		decltype (context.program.methods) temp;
		temp.resize(context.minima.methods.size(), {});
		for (auto& method: context.program.methods)
			temp[method.id] = method;
		context.program.methods = temp;
	}
}
CTL_DIAGBLOCK_END
