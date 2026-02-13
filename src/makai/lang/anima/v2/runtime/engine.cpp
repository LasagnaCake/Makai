#include "engine.hpp"
#include "context.hpp"
#include "../../../../../makai/net/net.hpp"
#include "../../../../../makai/file/flow.hpp"

using Makai::Anima::V2::Runtime::Engine;

using namespace Makai::Anima::V2;

namespace Runtime	= Makai::Anima::V2::Runtime;

using Makai::Data::Value;

bool Engine::yieldCycle() {
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
		default: crash(invalidInstructionError());
	}
	if (revertContext) context.mode = context.prevMode;
	return !isFinished;
}

bool Engine::process() {
	paused = false;
	if (wait) return isFinished;
	else wait.clear();
	while (Engine::yieldCycle() && !paused) {}
	return isFinished;
}

void Engine::crash(Engine::Error const& e) {
	err = e;
	terminate();
}

void Engine::v2Get() {
	Instruction::GetRequest get = bitcast<Instruction::GetRequest>(current.type);
	auto const src		= consumeValue(get.from);
	auto const field	= consumeValue(get.field);
	auto& dst			= accessValue(get.to);
	if (!dst) dst = new Value();
	if (!src->isStructured()) return crash(invalidSourceError("Source value is not a structured type!"));
	if (src->isArray()) {
		if (!field->isNumber())
			return crash(invalidFieldError("Expected number for array index!"));
		auto const i = src->getSigned();
		*dst = (*src)[i];
	} else if (src->isObject()) {
		if (!field->isString())
			return crash(invalidFieldError("Expected string for object field!"));
		auto const k = src->getString();
		*dst = (*src)[k];
	} else if (src->isVector() && field->isUnsigned()) {
		if (!field->isNumber())
			return crash(invalidFieldError("Expected number for vector component index!"));
		*dst = src->getVector()[field->getUnsigned()];
	} else return crash(invalidSourceError("Source value is not an array, object or vector!"));
}

void Engine::v2Set() {
	Instruction::SetRequest set = bitcast<Instruction::SetRequest>(current.type);
	auto const src		= consumeValue(set.from);
	auto const field	= consumeValue(set.field);
	auto& dst			= accessValue(set.to);
	if (!dst) dst = new Value();
	if (!src->isStructured()) return crash(invalidSourceError("Source value is not a structured type!"));
	if (src->isObject()) {
		if (!field->isString())
			return crash(invalidFieldError("Expected string for object field!"));
		(*dst)[field->getString()] = (*src);
	} else if (src->isArray() && field->isInteger()) {
		if (!field->isNumber())
			return crash(invalidFieldError("Expected number for array index!"));
		(*dst)[field->getSigned()] = (*src);
	} else if (src->isVector() && field->isUnsigned()) {
		auto vec = dst->getVector();
		auto const c = field->getUnsigned();
		vec[c] = src;
		*dst = vec;
	} else return crash(invalidSourceError("Source value is not an array, object or vector!"));
}

void Engine::v2Yield() {
	paused = true;
}

void Engine::v2Await() {
	Instruction::WaitRequest req = bitcast<Instruction::WaitRequest>(current.type);
	wait.type = req.wait;
	wait.condition = consumeValue(req.val);
}

void Engine::v2Compare() {
	Instruction::Comparison comp = bitcast<Instruction::Comparison>(current.type);
	auto const lhs	= consumeValue(comp.lhs);
	auto const rhs	= consumeValue(comp.rhs);
	auto& out		= accessValue(comp.out);
	Value::OrderType order = Value::Order::EQUAL;
	if (lhs->type() == rhs->type())					order = *lhs <=> *rhs;
	else if (lhs->isNumber() && rhs->isNumber())	order = lhs->get<double>() <=> lhs->get<double>();
	else if (inStrictMode())
		return crash(invalidComparisonError("Types do not match!"));
	else {
		*out = Value::undefined();
		return;
	}
	if (order == Value::Order::EQUAL)			*out = 0l;
	else if (order == Value::Order::GREATER)	*out = 1l;
	else if (order == Value::Order::LESS)		*out = -1l;
	else if (inStrictMode())
		return crash(invalidComparisonError("Failed to compare types!"));
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

Engine::Error Engine::invalidInstructionError() {
	return makeErrorHere("Invalid/Unsupported instruction!");
}

Engine::Error Engine::invalidBinaryMathError(String const& description) {
	return makeErrorHere("INVALID BINARY MATH: " + description);
}

Engine::Error Engine::invalidUnaryMathError(String const& description) {
	return makeErrorHere("INVALID UNARYs MATH: " + description);
}

Engine::Error Engine::invalidInternalValueError(uint64 const id) {
	return makeErrorHere(CTL::toString("Internal value of ID [", id, "] does not exist!"));
}

Engine::Error Engine::invalidSourceError(String const& description) {
	return makeErrorHere("INVALID SOURCE: " + description);
}

Engine::Error Engine::invalidDestinationError(String const& description) {
	return makeErrorHere("INVALID DESTINATION: " + description);
}

Engine::Error Engine::invalidFunctionError(String const& description) {
	return makeErrorHere("INVALID FUNCTION: " + description);
}

Engine::Error Engine::invalidComparisonError(String const& description) {
	return makeErrorHere("INVALID COMPARISON: " + description);
}

Engine::Error Engine::invalidFieldError(String const& description) {
	return makeErrorHere("INVALID FIELD: " + description);
}

Engine::Error Engine::missingArgumentsError() {
	return makeErrorHere("Missing arguments for built-in function!");
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
	returnBack();
}

void Engine::v2Copy() {
	Instruction::Transfer tf = bitcast<Instruction::Transfer>(current.type);
	auto const from	= consumeValue(tf.from);
	auto& to		= accessValue(tf.to);
	to = from;
}

void Engine::v2Call() {
	// Get invocation
	Instruction::Invocation invocation = bitcast<Instruction::Invocation>(current.type);
	// If invocation is internal call, do so
	if (invocation.location == DataLocation::AV2_DL_INTERNAL)
		return callBuiltIn(Cast::as<Engine::BuiltInFunction>(invocation.argc), invocation.mod);
	// Get function name (if not external function)
	advance(true);
	uint64 funcName = bitcast<uint64>(current);
	auto const isExtern = (invocation.location & DataLocation::AV2_DL_EXTERNAL) == DataLocation::AV2_DL_EXTERNAL;
	if (!isExtern) {
		auto fn = getValueFromLocation(invocation.location, funcName);
		if (!fn->isUnsigned())
			return crash(invalidFunctionError("Invalid function name!"));
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
	if (isExtern) {
		advance(true);
		ssize returnType = Cast::as<ssize>(current.name);
		auto const fn = program.constants[funcName].toString();
		auto const args = context.valueStack.sliced(-Cast::as<int>(invocation.argc), -1);
		Context::Storage result = new Value();
		// Invoke from appropriate source
		if ((invocation.location & DataLocation::AV2_DLM_BY_REF) == DataLocation::AV2_DLM_BY_REF) {
			auto const ns = fn.splitAtFirst('/');
			if (
				inStrictMode()
			&&	!context.shared.libraries.contains(ns.front())
			&&	!context.shared.has(ns.front(), ns.back())
			)
				return crash(invalidFunctionError("Shared library function [" + fn + "] does not exist, or its library is not loaded!"));
			result = context.shared.fetch(ns.front(), ns.back())->invoke(args);
		} else {
			if (inStrictMode() && !functions.has(fn))
				return crash(invalidFunctionError("Function [" + fn + "] does not exist!"));
			result = functions.invoke(fn, args);
		}
		context.valueStack.eraseRange(-Cast::as<int>(invocation.argc), -1);
		if (!result)
			result = new Value();
		// Check if return type matches expected type
		if (
			returnType != -1
		&&	Cast::as<Value::Kind>(returnType) != result->type()
		) {
			if (
				inStrictMode()
			&&	(
					current.type == 0
				||	(current.type == 1 && !result->isNull())
				)
			) return crash(
				invalidFunctionError(
					"Invalid external function return type!"
					"\nType is ["+Value::asNameString(result->type())+"]"
					"\nExpected type is ["+Value::asNameString(Cast::as<Value::Kind>(returnType))+"]"
				)
			);
		}
		context.temporary = result;
		return;
	}
	// Else, jump to function location
	context.pointers.function = invocation.argc;
	jumpBy(funcName, true);
}

Runtime::Context::Storage Engine::consumeValue(DataLocation const from) {
	if (
		(from >= asRegister(0) && from < asRegister(REGISTER_COUNT))
	||	(from == DataLocation::AV2_DL_TEMPORARY)
	) {
		auto const store = getValueFromLocation(from, 0);
		if (!store) return new Value();
	}
	advance(true);
	auto const store = getValueFromLocation(from, bitcast<uint64>(current));
	if (!store) return new Value();
	return store;
}

static Runtime::Context::Storage accessor(Runtime::Context::Storage const& v, bool const noCopy) {
	if (!v) return new Value();
	return noCopy ? v : new Value(*v);
}

Runtime::Context::Storage Engine::getValueFromLocation(DataLocation const loc, usize const id) {
	bool byRef	= (loc & Cast::as<DataLocation>(0b11000000)) == DataLocation::AV2_DLM_BY_REF;
	bool byMove	= (loc & Cast::as<DataLocation>(0b11000000)) == DataLocation::AV2_DLM_MOVE;
	if (loc >= asRegister(0) && loc < asRegister(REGISTER_COUNT)) {
		return accessor(context.registers[(enumcast(loc) - enumcast(DataLocation::AV2_DL_REGISTER))], byRef | byMove);
	}
	switch (loc) {
		case DataLocation::AV2_DL_CONST:
			if (program.constants.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return new Value(Value::undefined());
			}
			return Runtime::Context::Storage::create(program.constants[id % program.constants.size()]);
		case DataLocation::AV2_DL_STACK: {
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return new Value(Value::undefined());
			}
			auto& loc = context.valueStack[id  % context.valueStack.size()];
			auto const v = loc;
			if (byMove) loc = nullptr;
			return accessor(v, byRef);
		}
		case DataLocation::AV2_DL_STACK_OFFSET: {
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return new Value(Value::undefined());
			}
			auto& loc = context.valueStack[-Cast::as<ssize>(id  % context.valueStack.size() +1)];
			auto const v = loc;
			if (byMove) loc = nullptr;
			return accessor(v, byRef);
		}
//		case DataLocation::AV2_DL_HEAP:			{} break;
		case DataLocation::AV2_DL_GLOBAL:		return global(id);
		case DataLocation::AV2_DL_INTERNAL:		return internal(id);
		case DataLocation::AV2_DL_EXTERNAL:		return external(program.constants[id].get<String>(), byRef);
		case DataLocation::AV2_DL_TEMPORARY: {
			auto& loc = temporary();
			auto const v = loc;
			if (byMove) loc = nullptr;
			return accessor(v, byRef);
		}
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

Runtime::Context::Storage& Engine::accessValue(DataLocation const from) {
	if (
		(from >= asRegister(0) && from < asRegister(REGISTER_COUNT))
	||	(from == DataLocation::AV2_DL_TEMPORARY)
	) {
		auto& loc = accessLocation(from, 0);
		if (!loc) loc = new Value();
		return loc;
	}
	advance(true);
	auto& loc = accessLocation(from, bitcast<uint64>(current));
	if (!loc) loc = new Value();
	return loc;
}

Runtime::Context::Storage& Engine::accessLocation(DataLocation const loc, usize const id) {
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

Runtime::Context::Storage& Engine::temporary() {
	return context.temporary;
}

Runtime::Context::Storage& Engine::global(uint64 const id) {
	return context.globals[id];
}

void Engine::jumpTo(usize const point, bool returnable) {
	if (returnable)
		context.pointerStack.pushBack(context.pointers);
	context.pointers.instruction = point;
}

void Engine::jumpBy(usize const tableID, bool returnable) {
	if (tableID < program.jumpTable.size())
		jumpTo(program.jumpTable[tableID], false);
	else crash(invalidJump());
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
	auto& out		= accessValue(op.out);
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
					return crash(invalidBinaryMathError("Invalid/Unsupported operator!"));
				*out = Value::undefined();
			}
		}
	} else if (lhs->isAlgebraic() && rhs->isAlgebraic()) {
		switch (op.op) {
			case decltype(op.op)::AV2_IBM_OP_ADD:	*out = lhs->getVector() + rhs->getVector();
			case decltype(op.op)::AV2_IBM_OP_SUB:	*out = lhs->getVector() - rhs->getVector();
			case decltype(op.op)::AV2_IBM_OP_MUL:	*out = lhs->getVector() * rhs->getVector();
			case decltype(op.op)::AV2_IBM_OP_DIV:	*out = lhs->getVector() / rhs->getVector();
			default: {
				if (inStrictMode())
					return crash(invalidBinaryMathError("Invalid/Unsupported operator!"));
				*out = Value::undefined();
			}
		}
	} else {
		if (inStrictMode())
			return crash(invalidBinaryMathError("Both values are not numbers!"));
		*out = Value::undefined();
	}
}

void Engine::v2UnaryMath() {
	Instruction::UnaryMath op = Cast::bit<Instruction::UnaryMath>(current.type);
	auto const v	= consumeValue(op.v);
	if (err) return;
	auto& out		= accessValue(op.out);
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
					return crash(invalidUnaryMathError("Invalid/Unsupported operator!"));
				*out = Value::undefined();
			}
		}
	} else {
		if (inStrictMode())
			return crash(invalidUnaryMathError("Value is not a number!"));
		*out = Value::undefined();
	}
}

void Engine::pushUndefinedIfInLooseMode(String const& fname) {
	if (inStrictMode())
		return crash(invalidFunctionError("Failed operation for function \""+fname+"\"!"));
	context.temporary = new Value(Value::undefined());
}

void Engine::callBuiltIn(BuiltInFunction const func, uint8 const op) {
	if (context.valueStack.empty()) {
		if (inStrictMode())
			return crash(missingArgumentsError());
		else context.valueStack.pushBack(new Value(Value::undefined()));
	}
	else switch (func) {
		case BuiltInFunction::AV2_EBIF_ADD: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isNumber() && b->isNumber()) context.temporary = new Value(a->get<double>() + b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) context.temporary = new Value(a->getVector() + b->getVector());
			else pushUndefinedIfInLooseMode("builtin add");
		} break;
		case BuiltInFunction::AV2_EBIF_SUB: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isNumber() && b->isNumber()) context.temporary = new Value(a->get<double>() - b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) context.temporary = new Value(a->getVector() - b->getVector());
			else pushUndefinedIfInLooseMode("builtin sub");
		} break;
		case BuiltInFunction::AV2_EBIF_MUL: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isNumber() && b->isNumber()) context.temporary = new Value(a->get<double>() * b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) context.temporary = new Value(a->getVector() * b->getVector());
			else pushUndefinedIfInLooseMode("builtin mul");
		} break;
		case BuiltInFunction::AV2_EBIF_DIV: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isNumber() && b->isNumber()) context.temporary = new Value(a->get<double>() / b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) context.temporary = new Value(a->getVector() / b->getVector());
			else pushUndefinedIfInLooseMode("builtin div");
		} break;
		case BuiltInFunction::AV2_EBIF_REM: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isNumber() && b->isNumber()) {
				if (a->isUnsigned() && b->isUnsigned())
					context.temporary = new Value(a->get<usize>() % b->get<usize>());
				else if (a->isSigned() && b->isSigned())
					context.temporary = new Value(a->get<ssize>() % b->get<ssize>());
				else context.temporary = new Value(Math::mod(a->get<double>(), b->get<double>()));
			}
			else pushUndefinedIfInLooseMode("builtin mod");
		} break;
		case BuiltInFunction::AV2_EBIF_LAND: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			context.temporary = new Value(a->get<bool>() && b->get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_LOR: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			context.temporary = new Value(a->get<bool>() || b->get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_LNOT: {
			if (err) break;
			auto a = context.registers[0];
			context.temporary = new Value(!a->get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_NEG: {
			if (err) break;
			auto a = context.registers[0];
			if (a->isNumber()) context.temporary = new Value(-a->get<double>());
			else pushUndefinedIfInLooseMode("builtin negate");
		} break;
		case BuiltInFunction::AV2_EBIF_AND: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isInteger() && b->isInteger()) context.temporary = new Value(a->get<usize>() & b->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise and");
		} break;
		case BuiltInFunction::AV2_EBIF_OR: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isInteger() && b->isInteger()) context.temporary = new Value(a->get<usize>() | b->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise or");
		} break;
		case BuiltInFunction::AV2_EBIF_XOR: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			if (a->isInteger() && b->isInteger()) context.temporary = new Value(a->get<usize>() ^ b->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise xor");
		} break;
		case BuiltInFunction::AV2_EBIF_NOT: {
			if (err) break;
			auto a = context.registers[0];
			if (a->isInteger()) context.temporary = new Value(~a->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise not");
		} break;
		case BuiltInFunction::AV2_EBIF_COMP: {
			if (err) break;
			auto a = context.registers[0], b = context.registers[1];
			Value::OrderType order = Value::Order::EQUAL;
			if (a->type() == b->type())					order = *a <=> *b;
			else if (a->isNumber() && b->isNumber())	order = a->get<double>() <=> b->get<double>();
			else {
				pushUndefinedIfInLooseMode("builtin threeway compare");
				break;
			}
			if (order == Value::Order::EQUAL)			context.temporary = new Value(0l);
			else if (order == Value::Order::GREATER)	context.temporary = new Value(1l);
			else if (order == Value::Order::LESS)		context.temporary = new Value(-1l);
			else pushUndefinedIfInLooseMode("builtin threeway compare");
		} break;
		case BuiltInFunction::AV2_EBIF_INTERRUPT: {
		} break;
		case BuiltInFunction::AV2_EBIF_READ: {
			if (err) break;
			auto type	= context.registers[0];
			auto id		= context.registers[1];
			if (!(type->isUnsigned() && id->isUnsigned()))
				pushUndefinedIfInLooseMode("builtin indirect read");
			if (err) break;
			auto const ti = type->get<usize>();
			if (ti > Makai::Limit::MAX<uint8>) {
				pushUndefinedIfInLooseMode("builtin indirect read");
				break;
			} else context.temporary = getValueFromLocation(Cast::as<DataLocation>(ti), id);
		} break;
		case BuiltInFunction::AV2_EBIF_PRINT: {
			if (err) break;
			auto what = context.registers[0];
			onPrint(*what);
		} break;
		case BuiltInFunction::AV2_EBIF_SIZEOF: {
			if (err) break;
			auto val = context.registers[0];
			context.temporary = new Value(val->size());
		} break;
		case BuiltInFunction::AV2_EBIF_HTTP_REQUEST: {
			if (err) break;
			auto url	= context.registers[0];
			auto type	= context.registers[1];
			auto data	= context.registers[2];
			if (url->isString() && type->isString() && data)
				context.temporary = new Value(onHTTPRequest(url->getString(), type->getString().upper(), *data));
			else pushUndefinedIfInLooseMode("builtin HTTP request");
		} break;
		case BuiltInFunction::AV2_EBIF_STRING_OP:	return callBuiltInStringOp(BuiltInStringOperation(op));
		case BuiltInFunction::AV2_EBIF_ARRAY_OP:	return callBuiltInArrayOp(BuiltInArrayOperation(op));
		case BuiltInFunction::AV2_EBIF_OBJECT_OP:	return callBuiltInObjectOp(BuiltInObjectOperation(op));
		case BuiltInFunction::AV2_EBIF_VEC2_OP:		return callBuiltInVector2Op(BuiltInVectorOperation(op));
		case BuiltInFunction::AV2_EBIF_VEC3_OP:		return callBuiltInVector3Op(BuiltInVectorOperation(op));
		case BuiltInFunction::AV2_EBIF_VEC4_OP:		return callBuiltInVector4Op(BuiltInVectorOperation(op));
		default: pushUndefinedIfInLooseMode("invalid or unsupported builtin"); break;
	}
}

void Engine::callBuiltInStringOp(BuiltInStringOperation const func) {
	// TODO: This
}

void Engine::callBuiltInArrayOp(BuiltInArrayOperation const func) {
	// TODO: This
}

void Engine::callBuiltInObjectOp(BuiltInObjectOperation const func) {
	// TODO: This
}

void Engine::callBuiltInVector2Op(BuiltInVectorOperation const func) {
	switch (func) {
		case BuiltInVectorOperation::AV2_EBI_VO_NEW: {
			if (!context.registers[0]->isNumber())
				pushUndefinedIfInLooseMode("builtin vec2 new");
			else context.temporary = new Value(Value::VectorType(context.registers[0]->getReal()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_VEC_NEW: {
			if (!(context.registers[0]->isNumber() && context.registers[1]->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec2 vnew");
			else context.temporary = new Value(
				Value::VectorType(
					context.registers[0]->getReal(),
					context.registers[1]->getReal(),
					0
				)
			);
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_CROSS:
		case BuiltInVectorOperation::AV2_EBI_VO_FCROSS: {
			if (!(context.registers[0]->isAlgebraic() && context.registers[1]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 (f)cross");
			else context.temporary = new Value(context.registers[0]->getVector().xy().fcross(context.registers[1]->getVector().xy()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_DOT: {
			if (!(context.registers[0]->isAlgebraic() && context.registers[1]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 dot");
			else context.temporary = new Value(context.registers[0]->getVector().xy().dot(context.registers[1]->getVector().xy()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_TAN: {
			if (!(context.registers[0]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 tan");
			else context.temporary = new Value(context.registers[0]->getVector().xy().tangent());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_ANGLE: {
			if (!(context.registers[0]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 angle2");
			else context.temporary = new Value(context.registers[0]->getVector().xy().angle());
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin vec2"); break;
	}
}

void Engine::callBuiltInVector3Op(BuiltInVectorOperation const func) {
	switch (func) {
		case BuiltInVectorOperation::AV2_EBI_VO_NEW: {
			if (!(context.registers[0]->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec2 new");
			else context.temporary = new Value(Value::VectorType(context.registers[0]->getReal()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_VEC_NEW: {
			if (!(context.registers[0]->isNumber() && context.registers[1]->isNumber() && context.registers[2]->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec2 vnew");
			else context.temporary = new Value(
				Value::VectorType(
					context.registers[0]->getReal(),
					context.registers[1]->getReal(),
					context.registers[2]->getReal()
				)
			);
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_CROSS:{
			if (!(context.registers[0]->isAlgebraic() && context.registers[1]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 cross");
			else context.temporary = new Value(context.registers[0]->getVector().xyz().cross(context.registers[1]->getVector().xyz()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_FCROSS: {
			if (!(context.registers[0]->isAlgebraic() && context.registers[1]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 fcross");
			else context.temporary = new Value(context.registers[0]->getVector().xyz().fcross(context.registers[1]->getVector().xyz()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_DOT: {
			if (!(context.registers[0]->isAlgebraic() && context.registers[1]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 dot");
			else context.temporary = new Value(context.registers[0]->getVector().xyz().dot(context.registers[1]->getVector().xyz()));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin vec3"); break;
	}
}

void Engine::callBuiltInVector4Op(BuiltInVectorOperation const func) {
	// TODO: This
	switch (func) {
		case BuiltInVectorOperation::AV2_EBI_VO_NEW: {
			if (!(context.registers[0]->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec4 new");
			else context.temporary = new Value(Value::VectorType(context.registers[0]->getReal()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_VEC_NEW: {
			if (!(context.registers[0]->isNumber() && context.registers[1]->isNumber() && context.registers[2]->isNumber() && context.registers[3]->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec4 vnew");
			else context.temporary = new Value(
				Value::VectorType(
					context.registers[0]->getReal(),
					context.registers[1]->getReal(),
					context.registers[2]->getReal(),
					context.registers[3]->getReal()
				)
			);
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_CROSS:
		case BuiltInVectorOperation::AV2_EBI_VO_FCROSS: {
			if (!(context.registers[0]->isAlgebraic() && context.registers[1]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 (f)cross");
			else context.temporary = new Value(context.registers[0]->getVector().fcross(context.registers[1]->getVector()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_DOT: {
			if (!(context.registers[0]->isAlgebraic() && context.registers[1]->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 dot");
			else context.temporary = new Value(context.registers[0]->getVector().dot(context.registers[1]->getVector()));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin vec3"); break;
	}
}

void Engine::terminate() {
	isFinished = true;

}

void Engine::reset() {
	terminate();
	context		= {};
	current		= {};
	err			= {};
}

void Engine::load(Program const& prog) {
	reset();
	program = prog;
}

void Engine::execute() {
	isFinished	= false;
}

void Engine::onPrint(Value const& what) {
	if (what.isString())
		DEBUGLN(what.getString());
	else DEBUGLN(what.toFLOWString());
}

Value Engine::onHTTPRequest(String const& url, String const& action, Value const& data) {
	using namespace Makai::Net::HTTP;
	Request req;
	if (action == "GET")			req.type = Request::Type::MN_HRT_GET;
	else if (action == "POST")		req.type = Request::Type::MN_HRT_POST;
	else if (action == "PUT")		req.type = Request::Type::MN_HRT_PUT;
	else if (action == "HEAD")		req.type = Request::Type::MN_HRT_HEAD;
	else if (action == "PATCH")		req.type = Request::Type::MN_HRT_PATCH;
	else if (action == "DELETE")	req.type = Request::Type::MN_HRT_DELETE;
	else if (action == "UPDATE")	req.type = Request::Type::MN_HRT_UPDATE;
	else crash(invalidFetchRequest(action));
	req.data = data.toString();
	auto resp = Makai::Net::HTTP::fetch(url, req);
	Value respObj = respObj.object();
	respObj["status"]	= enumcast(resp.status);
	respObj["content"]	= resp.content;
	respObj["time"]		= resp.time;
	respObj["header"]	= resp.header;
	respObj["source"]	= resp.source;
	return respObj;
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
	auto& value = accessValue(inter.location);
	if (err) return;
	if (
		(inter.location & DataLocation::AV2_DLM_BY_REF) == DataLocation::AV2_DLM_BY_REF
	||	(inter.location & DataLocation::AV2_DLM_MOVE) == DataLocation::AV2_DLM_MOVE
	) value = context.valueStack.popBack();
	else *value = *context.valueStack.popBack();
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

void Engine::v2Jump() {
	Instruction::Leap op = current.getTypeAs<Instruction::Leap>();
	usize to = 0;
	if (op.source == DataLocation::AV2_DL_CONST) {
		advance(true);
		to = current.as<usize>();
	} else to = consumeValue(op.source)->getUnsigned();
	if (op.type == Instruction::Leap::Type::AV2_ILT_UNCONDITIONAL)
		return jumpBy(to, false);
	if (
		op.type == Instruction::Leap::Type::AV2_ILT_IF_TRUTHY
	&&	op.condition == DataLocation::AV2_DL_INTERNAL
	) return;
	auto const cond = consumeValue(op.condition);
	switch (op.type) {
		case decltype(op.type)::AV2_ILT_IF_FALSY:					if (cond->isFalsy())							jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_TRUTHY:					if (cond->isTruthy())							jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_NAN:						if (cond->isNaN())								jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_NEGATIVE:				if (cond->isNumber() && cond->getReal() < 0)	jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_POSITIVE:				if (cond->isNumber() && cond->getReal() > 0)	jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_ZERO:					if (cond->isNumber() && cond->getReal() == 0)	jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_NOT_ZERO:				if (cond->isNumber() && cond->getReal() != 0)	jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_NULL:					if (cond->isNull())								jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_UNDEFINED:				if (cond->isUndefined())						jumpBy(to, false); break;
		case decltype(op.type)::AV2_ILT_IF_NULL_OR_UNDEFINED:		if (cond->isNull() || cond->isUndefined())		jumpBy(to, false); break;
		default: break;
	}
}

void Engine::v2Cast() {
	Instruction::Casting op = current.getTypeAs<Instruction::Casting>();
	auto const src = consumeValue(op.src);
	auto const dst = accessValue(op.dst);
	if (src->isNumber())
		switch (op.type) {
			case Data::Value::Kind::DVK_BOOLEAN:	*dst = src->getBoolean();	return;
			case Data::Value::Kind::DVK_UNSIGNED:	*dst = src->getUnsigned();	return;
			case Data::Value::Kind::DVK_SIGNED:		*dst = src->getSigned();	return;
			case Data::Value::Kind::DVK_REAL:		*dst = src->getReal();		return;
			case Data::Value::Kind::DVK_STRING:		*dst = src->toString();		return;
			default: break;
		}
	else if (src->isString()) {
		switch (op.type) {
			case Data::Value::Kind::DVK_BOOLEAN:	*dst = toBool(src->getString());	return;
			case Data::Value::Kind::DVK_UNSIGNED:	*dst = toUInt64(src->getString());	return;
			case Data::Value::Kind::DVK_SIGNED:		*dst = toInt64(src->getString());	return;
			case Data::Value::Kind::DVK_REAL:		*dst = toDouble(src->getString());	return;
			case Data::Value::Kind::DVK_STRING:		*dst = *src;						return;
			default: break;
		}
	}
	crash(
		invalidCast(
			"Cannot cast from ["
		+	Data::Value::asNameString(src->type())
		+	"] to ["
		+	Data::Value::asNameString(op.type)
		+	"]!"
		)
	);
}

Engine::Error Engine::invalidLocationError(DataLocation const& loc) {
	return makeErrorHere("Invalid data location for instruction!");
}

Engine::Error Engine::invalidFetchRequest(String const& description) {
	return makeErrorHere(description);
}

Engine::Error Engine::invalidCast(String const& description) {
	return makeErrorHere(description);
}

Engine::Error Engine::invalidJump() {
	return makeErrorHere("Jump target does not exist!");
}
