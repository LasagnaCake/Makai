#include "engine.hpp"
#include "context.hpp"

using Makai::Anima::V2::Runtime::Engine;

using namespace Makai::Anima::V2;

namespace Runtime	= Makai::Anima::V2::Runtime;

using Makai::Data::Value;

bool Engine::yield() {
	bool revertContext = false;
	if (context.prevMode != context.mode)
		revertContext = true;
	if (isFinished) return false;
	do {
		advance();
	} while (current.name == Instruction::Name::AV2_IN_NO_OP && current.type);
	if (isFinished) return false;
	switch (current.name) {
		using enum Instruction::Name;
		case AV2_IN_HALT:			v2Halt();		break;
		case AV2_IN_STACK_POP:		v2StackPop();	break;
		case AV2_IN_STACK_PUSH:		v2StackPush();	break;
		case AV2_IN_STACK_CLEAR:	v2StackClear();	break;
		case AV2_IN_COPY:			v2Copy();		break;
		case AV2_IN_RETURN: 		v2Return();		break;
		case AV2_IN_CALL:			v2Call();		break;
		case AV2_IN_GET:			v2Get();		break;
		case AV2_IN_SET:			v2Set();		break;
		case AV2_IN_CAST:			v2Cast();		break;
		case AV2_IN_MATH_BOP:		v2BinaryMath();	break;
		case AV2_IN_MATH_UOP:		v2UnaryMath();	break;
		case AV2_IN_COMPARE:		v2Compare();	break;
		case AV2_IN_MODE:			v2SetContext();	break;
		case AV2_IN_JUMP:			v2Jump();		break;
		case AV2_IN_AWAIT:			v2Await();		break;
		case AV2_IN_YIELD:			v2Yield();		break;
		case AV2_IN_NO_OP: break;
		default: crash(invalidInstructionEror());
	}
	if (revertContext) context.mode = context.prevMode;
	return !isFinished;
}

bool Engine::process() {
	paused = false;
	while (Engine::process() && !paused) {}
	return isFinished;
}

void Engine::crash(Engine::Error const& e) {
	err = e;
	terminate();
}

void Engine::terminate() {
	isFinished = true;
}

void Engine::v2Get() {
	Instruction::GetRequest get;
}

void Engine::v2Set() {
	Instruction::SetRequest set;
}

void Engine::v2Yield() {
	paused = true;
}

void Engine::v2Compare() {
	Instruction::Comparison comp = bitcast<Instruction::Comparison>(current.type);
	auto const lhs = consumeValue(comp.lhs);
	auto const rhs = consumeValue(comp.rhs);
	auto const out = accessValue(comp.out);
	Value::OrderType order = Value::Order::EQUAL;
	if (lhs->type() == rhs->type())					order = *lhs <=> *rhs;
	else if (lhs->isNumber() && rhs->isNumber())	order = lhs->get<double>() <=> lhs->get<double>();
	else if (inStrictMode())
		invalidComparisonEror("Types do not match!");
	else {
		*out = Value::undefined();
		return;
	}
	if (order == Value::Order::EQUAL)			*out = 0l;
	else if (order == Value::Order::GREATER)	*out = 1l;
	else if (order == Value::Order::LESS)		*out = -1l;
	else if (inStrictMode())
		invalidComparisonEror("Failed to compare types!");
	else {
		*out = Value::undefined();
		return;
	}
}

void Engine::v2Halt() {
	Instruction::Stop stop = bitcast<Instruction::Stop>(current.type);
	switch (stop.mode) {
		case Instruction::Stop::Mode::AV2_ISM_NORMAL:	return terminate();
		case Instruction::Stop::Mode::AV2_ISM_ERROR: {
			auto const v = consumeValue(stop.source);
			if (err) return;
			return crash(makeErrorHere("PROGRAM_ERROR: " + v->toString()));
		};
		default: terminate();
	};
}

Engine::Error Engine::makeErrorHere(String const& message) {
	return {
		message,
		context.pointers.instruction,
		current
	};
}

Engine::Error Engine::endOfProgramError() {
	return makeErrorHere("Program has reached an unexpected end!");
}

Engine::Error Engine::invalidInstructionEror() {
	return makeErrorHere("Invalid/Unsupported instruction!");
}

void Engine::advance(bool isRequired) {
	++context.pointers.instruction;
	if (context.pointers.instruction < program.code.size())
		current = program.code[context.pointers.instruction];
	else if (isRequired)
		crash(endOfProgramError());
	else terminate();
}

void Engine::v2Return() {
	Instruction::Result res = bitcast<Instruction::Result>(current.type);
	if (!res.ignore)
		*temporary() = *accessValue(res.location);
}

void Engine::v2Copy() {
	Instruction::Transfer tf = bitcast<Instruction::Transfer>(current.type);
	auto const from	= consumeValue(tf.from);
	auto const to	= accessValue(tf.to);
	*to = *from;
}

void Engine::v2Invoke() {
	// Get invocation
	Instruction::Invocation invocation = bitcast<Instruction::Invocation>(current.type);
	// If invocation is internal call, do so
	if (invocation.location == DataLocation::AV2_DL_INTERNAL)
		return callBuiltIn(Cast::as<Engine::BuiltInFunction>(invocation.argc));
	// Get function name (if not external function)
	advance(true);
	uint64 funcName = bitcast<uint64>(current);
	if (invocation.location != DataLocation::AV2_DL_EXTERNAL) {
		auto fn = getValueFromLocation(invocation.location, funcName);
		if (!fn->isUnsigned())
			return crash(invalidFunctionEror("Invalid function name!"));
		else funcName = fn->get<uint64>();
	}
	// Add arguments to stack
	context.valueStack.expand(invocation.argc, {});
	for (usize i = 0; i < invocation.argc; ++i) {
		advance(true);
		Instruction::Invocation::Parameter arg;
		arg = bitcast<decltype(arg)>(current);
		context.valueStack[-invocation.argc+arg.argument] = consumeValue(arg.location);
	}
	// If external function, invoke it
	if (invocation.location == DataLocation::AV2_DL_EXTERNAL) {
		advance(true);
		ssize returnType = Cast::as<ssize>(current.name);
		auto const fn = program.constants[funcName].toString();
		if (inStrictMode() && !functions.has(fn))
			return crash(invalidFunctionEror("Function [" + fn + "] does not exist!"));
		auto const result = functions.invoke(
			fn,
			context.valueStack.sliced(-Cast::as<int>(invocation.argc), -1)
		);
		context.valueStack.eraseRange(-Cast::as<int>(invocation.argc), -1);
		// Check if return type matches expected type
		if (
			returnType != -1
		&&	Cast::as<Value::Kind>(returnType) != result.type()
		) {
			if (
				inStrictMode()
			&&	(
					current.type == 0
				||	(current.type == 1 && !result.isNull())
				)
			) return crash(
				invalidFunctionEror(
					"Invalid external function return type!"
					"\nType is ["+Value::asNameString(result.type())+"]"
					"\nExpected type is ["+Value::asNameString(Cast::as<Value::Kind>(returnType))+"]"
				)
			);
		}
		context.temporary = context.temporary.create(result);
		return;
	}
	// Else, jump to function location
	context.pointers.function = invocation.argc;
	jumpTo(funcName, true);
}

Runtime::Context::Storage Engine::consumeValue(DataLocation const from) {
	if (
		(from >= asRegister(0) && from < asRegister(REGISTER_COUNT))
	||	(from == DataLocation::AV2_DL_TEMPORARY)
	) return getValueFromLocation(from, 0);
	advance(true);
	return getValueFromLocation(from, bitcast<uint64>(current));
}

static Runtime::Context::Storage accessor(Runtime::Context::Storage const& v, bool const byRef) {
	return byRef ? v : new Value(*v);
}

Runtime::Context::Storage Engine::getValueFromLocation(DataLocation const loc, usize const id) {
	bool byRef = (loc & DataLocation::AV2_DL_BY_REF) == DataLocation::AV2_DL_BY_REF;
	if (loc >= asRegister(0) && loc < asRegister(REGISTER_COUNT)) {
		return accessor(context.registers[(enumcast(loc) - enumcast(DataLocation::AV2_DL_REGISTER))], byRef);
	}
	switch (loc) {
		case DataLocation::AV2_DL_CONST:
			if (program.constants.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return new Value(Value::undefined());
			}
			return Runtime::Context::Storage::create(program.constants[id % program.constants.size()]);
		case DataLocation::AV2_DL_STACK:
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return new Value(Value::undefined());
			}
			return accessor(context.valueStack[id  % context.valueStack.size()], byRef);
		case DataLocation::AV2_DL_STACK_OFFSET:
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return new Value(Value::undefined());
			}
			return accessor(context.valueStack[-Cast::as<ssize>(id  % context.valueStack.size() +1)], byRef);
//		case DataLocation::AV2_DL_HEAP:			{} break;
		case DataLocation::AV2_DL_GLOBAL:		return global(id);
		case DataLocation::AV2_DL_INTERNAL:		return internal(id);
		case DataLocation::AV2_DL_EXTERNAL:		return external(program.constants[id].get<String>(), byRef);
		case DataLocation::AV2_DL_TEMPORARY:	return accessor(temporary(), byRef);
		default: {
			if (inStrictMode())
				crash(invalidLocationError(loc));
			return new Value(Value::undefined());
		}
	}
	if (inStrictMode())
		crash(invalidLocationError(loc));
	return new Value(Value::undefined());
}

Runtime::Context::Storage Engine::accessValue(DataLocation const from) {
	if (
		(from >= asRegister(0) && from < asRegister(REGISTER_COUNT))
	||	(from == DataLocation::AV2_DL_TEMPORARY)
	) return accessLocation(from, 0);
	advance(true);
	return accessLocation(from, bitcast<uint64>(current));
}

Runtime::Context::Storage Engine::accessLocation(DataLocation const loc, usize const id) {
	if (loc >= asRegister(0) && loc < asRegister(REGISTER_COUNT)) {
		return context.registers[(enumcast(loc) - enumcast(DataLocation::AV2_DL_REGISTER))];
	}
	switch (loc) {
		case DataLocation::AV2_DL_STACK:
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return context.temporary;
			}
			return context.valueStack[id % context.valueStack.size()];
		case DataLocation::AV2_DL_STACK_OFFSET:
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return context.temporary;
			}
			return context.valueStack[-Cast::as<ssize>(id % context.valueStack.size() + 1)];
//		case DataLocation::AV2_DL_HEAP:			{} break;
		case DataLocation::AV2_DL_GLOBAL:		return global(id);
		case DataLocation::AV2_DL_TEMPORARY:	return temporary();	
		default: {
			if (inStrictMode())
				crash(invalidLocationError(loc));
			return temporary();
		}
	}
	if (inStrictMode())
		crash(invalidLocationError(loc));
	return context.temporary;
}

Runtime::Context::Storage Engine::temporary() {
	return context.temporary;
}

Runtime::Context::Storage Engine::global(uint64 const id) {
	return context.globals[id];
}

void Engine::jumpTo(usize const point, bool returnable) {
	if (returnable)
		context.pointerStack.pushBack(context.pointers);
	context.pointers.instruction = point;
}

bool Engine::hasSignal(String const& signal) {
	return program.ani.in.contains(signal);
}

void Engine::fire(String const& signal) {
	if (hasSignal(signal))
		jumpTo(program.jumpTable[program.ani.in[signal]], true);
}

void Engine::returnBack() {
	if (context.pointers.function) {
		usize const end = context.valueStack.size() - 1;
		usize const start = end - context.pointers.function;
		context.valueStack.eraseRange(start, end);
	}
	context.pointers = context.pointerStack.popBack();
}

Runtime::Context::Storage Engine::external(String const& name, bool const byRef) {
	return Runtime::Context::Storage::create(Value::undefined());
}

Runtime::Context::Storage Engine::internal(uint64 const valueID) {
	static const Data::Value::ArrayType internals = {
		Data::Value(false),
		Data::Value(true),
		Data::Value::undefined(),
		Data::Value::null(),
		Data::Value(Data::Value::NotANumber()),
		Data::Value(0u),
		Data::Value(0d),
		Data::Value(""),
		Data::Value::array(),
		Data::Value::bytes(),
		Data::Value::object()
	};
	if (valueID >= internals.size()) {
		if (inStrictMode())
			crash(invalidInternalValueError(valueID));
		return new Value(Value::undefined());
	}
	return new Value(internals[valueID]);
}

void Engine::v2BinaryMath() {
	Instruction::BinaryMath op = Cast::bit<Instruction::BinaryMath>(current.type);
	auto const lhs	= consumeValue(op.lhs);
	if (err) return;
	auto const rhs	= consumeValue(op.rhs);
	if (err) return;
	auto out		= accessValue(op.out);
	if (err) return;
	if (lhs->isNumber() && rhs->isNumber()) {
		switch (op.op) {
			case decltype(op.op)::AV2_IBM_OP_ADD:	*out = lhs->get<double>() + rhs->get<double>();
			case decltype(op.op)::AV2_IBM_OP_SUB:	*out = lhs->get<double>() - rhs->get<double>();
			case decltype(op.op)::AV2_IBM_OP_MUL:	*out = lhs->get<double>() * rhs->get<double>();
			case decltype(op.op)::AV2_IBM_OP_DIV:	*out = lhs->get<double>() / rhs->get<double>();
			case decltype(op.op)::AV2_IBM_OP_REM:	*out = Math::mod(lhs->get<double>(), rhs->get<double>());
			case decltype(op.op)::AV2_IBM_OP_POW:	*out = Math::pow(lhs->get<double>(), rhs->get<double>());
			case decltype(op.op)::AV2_IBM_OP_ATAN2:	*out = Math::atan2(lhs->get<double>(), rhs->get<double>());
			case decltype(op.op)::AV2_IBM_OP_LOG:	*out = Math::logn(lhs->get<double>(), rhs->get<double>());
			default: {
				if (inStrictMode())
					return crash(invalidBinaryMathError(op));
				*out = Value::undefined();
			}
		}
	} else {
		if (inStrictMode())
			return crash(invalidBinaryMathError(op));
		*out = Value::undefined();
	}
}

void Engine::v2UnaryMath() {
	Instruction::UnaryMath op = Cast::bit<Instruction::UnaryMath>(current.type);
	auto const v	= consumeValue(op.v);
	if (err) return;
	auto out		= accessValue(op.out);
	if (err) return;
	if (v->isNumber()) {
		switch (op.op) {
			case decltype(op.op)::AV2_IUM_OP_NEGATE:	*out = -v->get<double>();
			case decltype(op.op)::AV2_IUM_OP_INVERSE:	*out = 1.0 / v->get<double>();
			case decltype(op.op)::AV2_IUM_OP_SIN:		*out = Math::sin(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_COS:		*out = Math::cos(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_TAN:		*out = Math::tan(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_ASIN:		*out = asin(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_ACOS:		*out = acos(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_ATAN:		*out = Math::atan(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_SINH:		*out = sinh(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_COSH:		*out = cosh(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_TANH:		*out = tanh(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_LOG2:		*out = Math::log2(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_LOG10:		*out = Math::log10(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_LN:		*out = Math::log(v->get<double>());
			case decltype(op.op)::AV2_IUM_OP_SQRT:		*out = Math::sqrt(v->get<double>());
			default: {
				if (inStrictMode())
					return crash(invalidUnaryMathError(op));
				*out = Value::undefined();
			}
		}
	} else {
		if (inStrictMode())
			return crash(invalidUnaryMathError(op));
		*out = Value::undefined();
	}
}

void Engine::pushUndefinedIfInLooseMode(String const& fname) {
	if (inStrictMode())
		return crash(invalidFunctionEror("Failed operation for function \""+fname+"\"!"));
	context.valueStack.pushBack(new Value(Value::undefined()));
}

void Engine::callBuiltIn(BuiltInFunction const func) {
	if (context.valueStack.empty()) {
		if (inStrictMode())
			return crash(missingArgumentsError());
		else context.valueStack.pushBack(new Value(Value::undefined()));
	}
	else switch (func) {
		case BuiltInFunction::AV2_EBIF_ADD: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin add");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isNumber() && b->isNumber()) context.valueStack.pushBack(new Value(a->get<double>() + b->get<double>()));
			else pushUndefinedIfInLooseMode("builtin add");
		} break;
		case BuiltInFunction::AV2_EBIF_SUB: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin sub");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isNumber() && b->isNumber()) context.valueStack.pushBack(new Value(a->get<double>() - b->get<double>()));
			else pushUndefinedIfInLooseMode("builtin sub");
		} break;
		case BuiltInFunction::AV2_EBIF_MUL: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin mul");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isNumber() && b->isNumber()) context.valueStack.pushBack(new Value(a->get<double>() * b->get<double>()));
			else pushUndefinedIfInLooseMode("builtin mul");
		} break;
		case BuiltInFunction::AV2_EBIF_DIV: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin div");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isNumber() && b->isNumber()) context.valueStack.pushBack(new Value(a->get<double>() / b->get<double>()));
			else pushUndefinedIfInLooseMode("builtin div");
		} break;
		case BuiltInFunction::AV2_EBIF_REM: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin mod");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isNumber() && b->isNumber()) {
				if (a->isUnsigned() && b->isUnsigned())
					context.valueStack.pushBack(new Value(a->get<usize>() % b->get<usize>()));
				else if (a->isSigned() && b->isSigned())
					context.valueStack.pushBack(new Value(a->get<ssize>() % b->get<ssize>()));
				else context.valueStack.pushBack(new Value(Math::mod(a->get<double>(), b->get<double>())));
			}
			else pushUndefinedIfInLooseMode("builtin mod");
		} break;
		case BuiltInFunction::AV2_EBIF_LAND: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin logic and");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			context.valueStack.pushBack(new Value(a->get<bool>() && b->get<bool>()));
		} break;
		case BuiltInFunction::AV2_EBIF_LOR: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin logic or");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			context.valueStack.pushBack(new Value(a->get<bool>() || b->get<bool>()));
		} break;
		case BuiltInFunction::AV2_EBIF_LNOT: {
			if (context.valueStack.size() < 1) pushUndefinedIfInLooseMode("builtin logic not");
			if (err) break;
			auto a = context.valueStack.popBack();
			context.valueStack.pushBack(new Value(!a->get<bool>()));
		} break;
		case BuiltInFunction::AV2_EBIF_NEG: {
			if (context.valueStack.size() < 1) pushUndefinedIfInLooseMode("builtin negate");
			if (err) break;
			auto a = context.valueStack.popBack();
			if (a->isNumber()) context.valueStack.pushBack(new Value(-a->get<float>()));
			else pushUndefinedIfInLooseMode("builtin negate");
		} break;
		case BuiltInFunction::AV2_EBIF_AND: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin bitwise and");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isInteger() && b->isInteger()) context.valueStack.pushBack(new Value(a->get<usize>() & b->get<usize>()));
			else pushUndefinedIfInLooseMode("builtin bitwise and");
		} break;
		case BuiltInFunction::AV2_EBIF_OR: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin bitwise or");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isInteger() && b->isInteger()) context.valueStack.pushBack(new Value(a->get<usize>() | b->get<usize>()));
			else pushUndefinedIfInLooseMode("builtin bitwise or");
		} break;
		case BuiltInFunction::AV2_EBIF_XOR: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin bitwise xor");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a->isInteger() && b->isInteger()) context.valueStack.pushBack(new Value(a->get<usize>() ^ b->get<usize>()));
			else pushUndefinedIfInLooseMode("builtin bitwise xor");
		} break;
		case BuiltInFunction::AV2_EBIF_NOT: {
			if (context.valueStack.size() < 1) pushUndefinedIfInLooseMode("builtin bitwise not");
			if (err) break;
			auto a = context.valueStack.popBack();
			if (a->isInteger()) context.valueStack.pushBack(new Value(~a->get<usize>()));
			else pushUndefinedIfInLooseMode("builtin bitwise not");
		} break;
		case BuiltInFunction::AV2_EBIF_COMP: {
			if (context.valueStack.size() < 1) pushUndefinedIfInLooseMode("builtin threeway compare");
			if (err) break;
			auto a = context.valueStack.popBack(), b = context.valueStack.popBack();
			Value::OrderType order = Value::Order::EQUAL;
			if (a->type() == b->type())					order = *a <=> *b;
			else if (a->isNumber() && b->isNumber())	order = a->get<double>() <=> b->get<double>();
			else {
				pushUndefinedIfInLooseMode("builtin threeway compare");
				break;
			}
			if (order == Value::Order::EQUAL)			context.valueStack.pushBack(new Value(0l));
			else if (order == Value::Order::GREATER)	context.valueStack.pushBack(new Value(1l));
			else if (order == Value::Order::LESS)		context.valueStack.pushBack(new Value(-1l));
			else pushUndefinedIfInLooseMode("builtin threeway compare");
		} break;
		case BuiltInFunction::AV2_EBIF_INTERRUPT: {
		} break;
		case BuiltInFunction::AV2_EBIF_READ: {
			if (context.valueStack.size() < 2) pushUndefinedIfInLooseMode("builtin indirect read");
			if (err) break;
			auto type	= context.valueStack.popBack();
			auto id		= context.valueStack.popBack();
			if (!(type->isUnsigned() && id->isUnsigned()))
				pushUndefinedIfInLooseMode("builtin indirect read");
			if (err) break;
			auto const ti = type->get<usize>();
			if (ti > Makai::Limit::MAX<uint8>) {
				pushUndefinedIfInLooseMode("builtin indirect read");
				break;
			} else context.valueStack.pushBack(
				getValueFromLocation(Cast::as<DataLocation>(ti), id)
			);
		} break;
		case BuiltInFunction::AV2_EBIF_PRINT: {
			if (context.valueStack.size() < 1) pushUndefinedIfInLooseMode("builtin print");
			if (err) break;
			auto what = context.valueStack.popBack();
			print(*what);
		} break;
		case BuiltInFunction::AV2_EBIF_SIZEOF: {
			if (context.valueStack.size() < 1) pushUndefinedIfInLooseMode("builtin sizeof");
			if (err) break;
			auto val = context.valueStack.popBack();
			context.valueStack.pushBack(new Value(val->size()));
		} break;
		default: pushUndefinedIfInLooseMode("invalid or unsupported builtin"); break;
	}
}

void Engine::print(Value const& what) {
	if (what.isString())
		DEBUGLN(what.get<String>());
	else DEBUGLN(what.toString());
}

void Engine::v2SetContext() {
	auto const ctx = Cast::bit<Instruction::Context>(current.type);
	if (!ctx.immediate)
		context.prevMode	= ctx.mode;
	context.mode			= ctx.mode;
}

void Engine::v2StackPush() {
	auto const inter = Cast::bit<Instruction::StackPush>(current.type);
	auto const value = consumeValue(inter.location);
	if (err) return;
	context.valueStack.pushBack(value);
}

void Engine::v2StackPop() {
	auto const inter = Cast::bit<Instruction::StackPop>(current.type);
	if (inter.discard) {
		context.valueStack.popBack();
		return;
	}
	auto value = accessValue(inter.location);
	if (err) return;
	*value = *context.valueStack.popBack();
}

void Engine::v2StackSwap() {
	if (context.valueStack.size() >= 2)
		Makai::swap(context.valueStack[-1], context.valueStack[-2]);
}

void Engine::v2StackClear() {
	if (current.type)
		context.valueStack.removeRange(-Math::max<int>(current.type, context.valueStack.size()));
}

void Engine::v2StackFlush() {
	context.valueStack.clear();
}