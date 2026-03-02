#include "engine.hpp"
#include "context.hpp"
#include "../../../../../makai/net/net.hpp"
#include "../../../../../makai/file/flow.hpp"
#include "../../../../../makai/data/data.hpp"
#include "../../../../../makai/parser/parser.hpp"
#include "../../../../../makai/tool/tool.hpp"

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
		DEBUGLN("Next instruction, please!");
		advance();
	} while (current.name == Instruction::Name::AV2_IN_NO_OP && current.type);
	if (isFinished) return false;
	DEBUGLN("Instruction:", Instruction::asString(current.name));
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
	if (wait) {
		DEBUGLN("Waiting...");
		return isFinished;
	}
	else wait.clear();
	while (Engine::yieldCycle() && !paused) {}
	DEBUGLN("Done processing for now!");
	return !isFinished;
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
		case Instruction::Stop::Mode::AV2_ISM_WITH_VALUE: {
			auto const v = consumeValue(stop.source);
			context.result = *v;
		}
		case Instruction::Stop::Mode::AV2_ISM_EMPTY: return terminate();
		case Instruction::Stop::Mode::AV2_ISM_ERROR: {
			auto const v = consumeValue(stop.source);
			if (err) return;
			return crash(makeErrorHere("PROGRAM_ERROR: " + v->toString()));
		};
		default: terminate();
	};
}

Engine::Error Engine::makeErrorHere(String const& message) {
	if (CTL::CPP::Debug::hasDebugger())
		throw Makai::Error::FailedAction(
			"ANIMA_ERROR: " + message,
			CTL::CPP::SourceFile("BYTECODE", context.pointers.instruction, "ANP")
		);
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
	DEBUGLN("Advancing...");
	++context.pointers.instruction;
	if (context.pointers.instruction < program.code.size())
		current = program.code[context.pointers.instruction];
	else if (isRequired)
		crash(endOfProgramError());
	else {
		DEBUGLN("End of program reached!");
		terminate();
	}
}

void Engine::v2Return() {
	returnBack();
}

void Engine::v2Copy() {
	Instruction::Transfer tf = bitcast<Instruction::Transfer>(current.type);
	DEBUGLN("Reading source...");
	auto const from	= consumeValue(tf.from);
	if (err) return;
	DEBUGLN("Writing to destination...");
	auto& to		= accessValue(tf.to);
	if (err) return;
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
	auto const isExtern	= (invocation.location & DataLocation::AV2_DL_EXTERNAL) == DataLocation::AV2_DL_EXTERNAL;
	auto const isConst	= (invocation.location & DataLocation::AV2_DL_CONST) == DataLocation::AV2_DL_CONST;
	if (!(isExtern || isConst)) {
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
		temporary() = result;
		return;
	}
	// Else, jump to function location
	context.pointers.function = invocation.argc;
	jumpBy(funcName, true);
}

Runtime::Context::Storage Engine::consumeValue(DataLocation const from) {
	if (
		(isRegister(from))
	||	(asPlace(from) == DataLocation::AV2_DL_TEMPORARY)
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
	auto const place = asPlace(loc);
	auto const mod = asModifiers(loc);
	bool byRef	= mod == DataLocation::AV2_DLM_BY_REF;
	bool byMove	= mod == DataLocation::AV2_DLM_MOVE;
	if (isRegister(place))
		return accessor(iregister((enumcast(place) - enumcast(DataLocation::AV2_DL_REGISTER))), byRef || byMove);
	else switch (place) {
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
			auto& loc = context.valueStack[-Cast::as<ssize>(id % context.valueStack.size() + 1)];
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
		(isRegister(from))
	||	(asPlace(from) == DataLocation::AV2_DL_TEMPORARY)
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
	auto const place = asPlace(loc);
	if (isRegister(place)) {
		return iregister((enumcast(place) - enumcast(DataLocation::AV2_DL_REGISTER)));
	}
	else switch (place) {
		case DataLocation::AV2_DL_STACK:
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return temporary();
			}
			return context.valueStack[id % context.valueStack.size()];
		case DataLocation::AV2_DL_STACK_OFFSET:
			if (context.valueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return temporary();
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
	return temporary();
}

Runtime::Context::Storage& Engine::temporary() {
	return context.temporary;
}

Runtime::Context::Storage& Engine::global(uint64 const id) {
	return context.globals[id];
}

Runtime::Context::Storage& Engine::iregister(uint64 const id) {
	return context.registers[id % Makai::Anima::V2::REGISTER_COUNT];
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
	temporary() = new Value(Value::undefined());
}

void Engine::callBuiltIn(BuiltInFunction const func, uint8 const op) {
	switch (func) {
		case BuiltInFunction::AV2_EBIF_ADD: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isNumber() && b->isNumber()) temporary() = new Value(a->get<double>() + b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) temporary() = new Value(a->getVector() + b->getVector());
			else pushUndefinedIfInLooseMode("builtin add");
		} break;
		case BuiltInFunction::AV2_EBIF_SUB: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isNumber() && b->isNumber()) temporary() = new Value(a->get<double>() - b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) temporary() = new Value(a->getVector() - b->getVector());
			else pushUndefinedIfInLooseMode("builtin sub");
		} break;
		case BuiltInFunction::AV2_EBIF_MUL: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isNumber() && b->isNumber()) temporary() = new Value(a->get<double>() * b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) temporary() = new Value(a->getVector() * b->getVector());
			else pushUndefinedIfInLooseMode("builtin mul");
		} break;
		case BuiltInFunction::AV2_EBIF_DIV: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isNumber() && b->isNumber()) temporary() = new Value(a->get<double>() / b->get<double>());
			else if (a->isAlgebraic() && b->isAlgebraic()) temporary() = new Value(a->getVector() / b->getVector());
			else pushUndefinedIfInLooseMode("builtin div");
		} break;
		case BuiltInFunction::AV2_EBIF_REM: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isNumber() && b->isNumber()) {
				if (a->isUnsigned() && b->isUnsigned())
					temporary() = new Value(a->get<usize>() % b->get<usize>());
				else if (a->isSigned() && b->isSigned())
					temporary() = new Value(a->get<ssize>() % b->get<ssize>());
				else temporary() = new Value(Math::mod(a->get<double>(), b->get<double>()));
			}
			else pushUndefinedIfInLooseMode("builtin mod");
		} break;
		case BuiltInFunction::AV2_EBIF_LAND: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			temporary() = new Value(a->get<bool>() && b->get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_LOR: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			temporary() = new Value(a->get<bool>() || b->get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_LNOT: {
			if (err) break;
			auto a = iregister(0);
			temporary() = new Value(!a->get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_NEG: {
			if (err) break;
			auto a = iregister(0);
			if (a->isNumber()) temporary() = new Value(-a->get<double>());
			else pushUndefinedIfInLooseMode("builtin negate");
		} break;
		case BuiltInFunction::AV2_EBIF_AND: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isInteger() && b->isInteger()) temporary() = new Value(a->get<usize>() & b->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise and");
		} break;
		case BuiltInFunction::AV2_EBIF_OR: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isInteger() && b->isInteger()) temporary() = new Value(a->get<usize>() | b->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise or");
		} break;
		case BuiltInFunction::AV2_EBIF_XOR: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			if (a->isInteger() && b->isInteger()) temporary() = new Value(a->get<usize>() ^ b->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise xor");
		} break;
		case BuiltInFunction::AV2_EBIF_NOT: {
			if (err) break;
			auto a = iregister(0);
			if (a->isInteger()) temporary() = new Value(~a->get<usize>());
			else pushUndefinedIfInLooseMode("builtin bitwise not");
		} break;
		case BuiltInFunction::AV2_EBIF_COMP: {
			if (err) break;
			auto a = iregister(0), b = iregister(1);
			Value::OrderType order = Value::Order::EQUAL;
			if (a->type() == b->type())					order = *a <=> *b;
			else if (a->isNumber() && b->isNumber())	order = a->get<double>() <=> b->get<double>();
			else {
				pushUndefinedIfInLooseMode("builtin threeway compare");
				break;
			}
			if (order == Value::Order::EQUAL)			temporary() = new Value(0l);
			else if (order == Value::Order::GREATER)	temporary() = new Value(1l);
			else if (order == Value::Order::LESS)		temporary() = new Value(-1l);
			else pushUndefinedIfInLooseMode("builtin threeway compare");
		} break;
		case BuiltInFunction::AV2_EBIF_INTERRUPT: {
		} break;
		case BuiltInFunction::AV2_EBIF_READ: {
			if (err) break;
			auto type	= iregister(0);
			auto id		= iregister(1);
			if (!(type->isUnsigned() && id->isUnsigned()))
				pushUndefinedIfInLooseMode("builtin indirect read");
			if (err) break;
			auto const ti = type->get<usize>();
			if (ti > Makai::Limit::MAX<uint8>) {
				pushUndefinedIfInLooseMode("builtin indirect read");
				break;
			} else temporary() = getValueFromLocation(Cast::as<DataLocation>(ti), id);
		} break;
		case BuiltInFunction::AV2_EBIF_PRINT: {
			if (err) break;
			auto what = iregister(0);
			onPrint(*what);
		} break;
		case BuiltInFunction::AV2_EBIF_SIZEOF: {
			if (err) break;
			auto val = iregister(0);
			temporary() = new Value(val->size());
		} break;
		case BuiltInFunction::AV2_EBIF_HTTP_REQUEST: {
			if (err) break;
			auto url	= iregister(0);
			auto type	= iregister(1);
			auto data	= iregister(2);
			if (url->isString() && type->isString() && data)
				temporary() = new Value(onHTTPRequest(url->getString(), type->getString().upper(), *data));
			else pushUndefinedIfInLooseMode("builtin HTTP request");
		} break;
		case BuiltInFunction::AV2_EBIF_TO_STRING: {
			if (err) break;
			temporary() = new Value(iregister(0)->isString() ? iregister(0)->getString() : iregister(0)->toFLOWString());
		} break;
		case BuiltInFunction::AV2_EBIF_FROM_STRING: {
			if (err) break;
			if (iregister(0)->isString()) try{
				Makai::Parser::Data::FLOWParser parser;
				parser.tryParse(
					iregister(0)->getString()
				).then(
					[&] (auto const& e) {temporary() = new Value(e);}
				).onError(
					[&] (auto const& e) {pushUndefinedIfInLooseMode("builtin from-string");}
				);
			} catch (...) {
				pushUndefinedIfInLooseMode("builtin from-string");
			} else pushUndefinedIfInLooseMode("builtin from-string");
		} break;
		case BuiltInFunction::AV2_EBIF_STRING_OP:		return callBuiltInStringOp(BuiltInStringOperation(op));
		case BuiltInFunction::AV2_EBIF_ARRAY_OP:		return callBuiltInArrayOp(BuiltInArrayOperation(op));
		case BuiltInFunction::AV2_EBIF_OBJECT_OP:		return callBuiltInObjectOp(BuiltInObjectOperation(op));
		case BuiltInFunction::AV2_EBIF_VEC2_OP:			return callBuiltInVector2Op(BuiltInVectorOperation(op));
		case BuiltInFunction::AV2_EBIF_VEC3_OP:			return callBuiltInVector3Op(BuiltInVectorOperation(op));
		case BuiltInFunction::AV2_EBIF_VEC4_OP:			return callBuiltInVector4Op(BuiltInVectorOperation(op));
		case BuiltInFunction::AV2_EBIF_OS_OP:			return callBuiltInOSOp(BuiltInOSOperation(op));
		case BuiltInFunction::AV2_EBIF_FS_OP:			return callBuiltInFSOp(BuiltInFSOperation(op));
		case BuiltInFunction::AV2_EBIF_ARCHIVE_OP:		return callBuiltInArchiveOp(BuiltInArchiveOperation(op));
		case BuiltInFunction::AV2_EBIF_CRYPTOGRAPY_OP:	return callBuiltInCryptographyOp(BuiltInCryptographyOperation(op));
		default: pushUndefinedIfInLooseMode("invalid or unsupported builtin"); break;
	}
}

void Engine::callBuiltInStringOp(BuiltInStringOperation const func) {
	// TODO: This
	switch (func) {
		case BuiltInStringOperation::AV2_EBI_SO_CONTAINS: {
		} break;
		case BuiltInStringOperation::AV2_EBI_SO_FIND: {} break;
		case BuiltInStringOperation::AV2_EBI_SO_JOIN: {} break;
		case BuiltInStringOperation::AV2_EBI_SO_REMOVE: {} break;
		case BuiltInStringOperation::AV2_EBI_SO_REPLACE: {} break;
		case BuiltInStringOperation::AV2_EBI_SO_SLICE: {} break;
		case BuiltInStringOperation::AV2_EBI_SO_SPLIT: {} break;
		case BuiltInStringOperation::AV2_EBI_SO_MATCHES: {} break;
	}
}

void Engine::callBuiltInArrayOp(BuiltInArrayOperation const func) {
	// TODO: This
	switch (func) {}
}

void Engine::callBuiltInObjectOp(BuiltInObjectOperation const func) {
	// TODO: This
	switch (func) {}
}

void Engine::callBuiltInVector2Op(BuiltInVectorOperation const func) {
	switch (func) {
		case BuiltInVectorOperation::AV2_EBI_VO_NEW: {
			if (!iregister(0)->isNumber())
				pushUndefinedIfInLooseMode("builtin vec2 new");
			else temporary() = new Value(Value::VectorType(iregister(0)->getReal()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_VEC_NEW: {
			if (!(iregister(0)->isNumber() && iregister(1)->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec2 vnew");
			else temporary() = new Value(
				Value::VectorType(
					iregister(0)->getReal(),
					iregister(1)->getReal(),
					0
				)
			);
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_CROSS:
		case BuiltInVectorOperation::AV2_EBI_VO_FCROSS: {
			if (!(iregister(0)->isAlgebraic() && iregister(1)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 (f)cross");
			else temporary() = new Value(iregister(0)->getVector().xy().fcross(iregister(1)->getVector().xy()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_DOT: {
			if (!(iregister(0)->isAlgebraic() && iregister(1)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 dot");
			else temporary() = new Value(iregister(0)->getVector().xy().dot(iregister(1)->getVector().xy()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_TAN: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 tan");
			else temporary() = new Value(iregister(0)->getVector().xy().tangent());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_ANGLE: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 angle");
			else temporary() = new Value(iregister(0)->getVector().xy().angle());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_NORMAL: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 normal");
			else temporary() = new Value(iregister(0)->getVector().xy().normalize());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_LENGTH: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 len");
			else temporary() = new Value(iregister(0)->getVector().xy().length());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_LENGTH_SQUARED: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 len2");
			else temporary() = new Value(iregister(0)->getVector().xy().lengthSquared());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_TRI_CROSS: {
			if (!(
				iregister(0)->isAlgebraic()
			&&	iregister(1)->isAlgebraic()
			&&	iregister(2)->isAlgebraic()
			))
				pushUndefinedIfInLooseMode("builtin vec2 tri");
			else temporary() = new Value(iregister(0)->getVector().xy().tri(iregister(1)->getVector().xy(), iregister(2)->getVector().xy()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_INVERSE_TRI_CROSS: {
			if (!(
				iregister(0)->isAlgebraic()
			&&	iregister(1)->isAlgebraic()
			&&	iregister(2)->isAlgebraic()
			))
				pushUndefinedIfInLooseMode("builtin vec2 itri");
			else temporary() = new Value(iregister(0)->getVector().xy().itri(iregister(1)->getVector().xy(), iregister(2)->getVector().xy()));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin vec2"); break;
	}
}

void Engine::callBuiltInVector3Op(BuiltInVectorOperation const func) {
	switch (func) {
		case BuiltInVectorOperation::AV2_EBI_VO_NEW: {
			if (!(iregister(0)->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec2 new");
			else temporary() = new Value(Value::VectorType(iregister(0)->getReal()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_VEC_NEW: {
			if (!(iregister(0)->isNumber() && iregister(1)->isNumber() && iregister(2)->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec2 vnew");
			else temporary() = new Value(
				Value::VectorType(
					iregister(0)->getReal(),
					iregister(1)->getReal(),
					iregister(2)->getReal()
				)
			);
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_CROSS:{
			if (!(iregister(0)->isAlgebraic() && iregister(1)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec2 cross");
			else temporary() = new Value(iregister(0)->getVector().xyz().cross(iregister(1)->getVector().xyz()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_FCROSS: {
			if (!(iregister(0)->isAlgebraic() && iregister(1)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 fcross");
			else temporary() = new Value(iregister(0)->getVector().xyz().fcross(iregister(1)->getVector().xyz()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_DOT: {
			if (!(iregister(0)->isAlgebraic() && iregister(1)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 dot");
			else temporary() = new Value(iregister(0)->getVector().xyz().dot(iregister(1)->getVector().xyz()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_ANGLE: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 angle");
			else temporary() = new Value(iregister(0)->getVector().xyz().angle());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_NORMAL: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 normal");
			else temporary() = new Value(iregister(0)->getVector().xyz().normalize());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_LENGTH: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 len");
			else temporary() = new Value(iregister(0)->getVector().xyz().length());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_LENGTH_SQUARED: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec3 len2");
			else temporary() = new Value(iregister(0)->getVector().xyz().lengthSquared());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_TRI_CROSS: {
			if (!(
				iregister(0)->isAlgebraic()
			&&	iregister(1)->isAlgebraic()
			&&	iregister(2)->isAlgebraic()
			))
				pushUndefinedIfInLooseMode("builtin vec3 tri");
			else temporary() = new Value(iregister(0)->getVector().xyz().tri(iregister(1)->getVector().xyz(), iregister(2)->getVector().xyz()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_INVERSE_TRI_CROSS: {
			if (!(
				iregister(0)->isAlgebraic()
			&&	iregister(1)->isAlgebraic()
			&&	iregister(2)->isAlgebraic()
			))
				pushUndefinedIfInLooseMode("builtin vec3 itri");
			else temporary() = new Value(iregister(0)->getVector().xyz().itri(iregister(1)->getVector().xyz(), iregister(2)->getVector().xyz()));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin vec3"); break;
	}
}

void Engine::callBuiltInVector4Op(BuiltInVectorOperation const func) {
	switch (func) {
		case BuiltInVectorOperation::AV2_EBI_VO_NEW: {
			if (!(iregister(0)->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec4 new");
			else temporary() = new Value(Value::VectorType(iregister(0)->getReal()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_VEC_NEW: {
			if (!(iregister(0)->isNumber() && iregister(1)->isNumber() && iregister(2)->isNumber() && iregister(3)->isNumber()))
				pushUndefinedIfInLooseMode("builtin vec4 vnew");
			else temporary() = new Value(
				Value::VectorType(
					iregister(0)->getReal(),
					iregister(1)->getReal(),
					iregister(2)->getReal(),
					iregister(3)->getReal()
				)
			);
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_CROSS:
		case BuiltInVectorOperation::AV2_EBI_VO_FCROSS: {
			if (!(iregister(0)->isAlgebraic() && iregister(1)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 (f)cross");
			else temporary() = new Value(iregister(0)->getVector().fcross(iregister(1)->getVector()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_DOT: {
			if (!(iregister(0)->isAlgebraic() && iregister(1)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 dot");
			else temporary() = new Value(iregister(0)->getVector().dot(iregister(1)->getVector()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_NORMAL: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 normal");
			else temporary() = new Value(iregister(0)->getVector().normalize());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_LENGTH: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 len");
			else temporary() = new Value(iregister(0)->getVector().length());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_LENGTH_SQUARED: {
			if (!(iregister(0)->isAlgebraic()))
				pushUndefinedIfInLooseMode("builtin vec4 len2");
			else temporary() = new Value(iregister(0)->getVector().lengthSquared());
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_TRI_CROSS: {
			if (!(
				iregister(0)->isAlgebraic()
			&&	iregister(1)->isAlgebraic()
			&&	iregister(2)->isAlgebraic()
			))
				pushUndefinedIfInLooseMode("builtin vec4 tri");
			else temporary() = new Value(iregister(0)->getVector().tri(iregister(1)->getVector(), iregister(2)->getVector()));
		} break;
		case BuiltInVectorOperation::AV2_EBI_VO_INVERSE_TRI_CROSS: {
			if (!(
				iregister(0)->isAlgebraic()
			&&	iregister(1)->isAlgebraic()
			&&	iregister(2)->isAlgebraic()
			))
				pushUndefinedIfInLooseMode("builtin vec4 itri");
			else temporary() = new Value(iregister(0)->getVector().itri(iregister(1)->getVector(), iregister(2)->getVector()));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin vec4"); break;
	}
}

void Engine::callBuiltInOSOp(Engine::BuiltInOSOperation const func) {
	switch (func) {
		case BuiltInOSOperation::AV2_EBI_OSO_RUN_EXECUTABLE: {
			temporary() = new Value(onSystemRequest(func));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin os");
	}
}

int Engine::onSystemRequest(Engine::BuiltInOSOperation const func) {
	switch (func) {
		case BuiltInOSOperation::AV2_EBI_OSO_RUN_EXECUTABLE: {
			if (!(
				iregister(0)->isString()
			&&	iregister(1)->isArray()
			&&	iregister(2)->isString()
			))
			temporary() = new Value(
				Makai::OS::launch(
					iregister(0)->getString(),
					iregister(2)->getString(),
					iregister(2)->getArray().toList<Makai::String>(
						[&] (Value const& e) -> Makai::String {
							return e.isString() ? e.getString() : e.toString();
						}
					)
				)
			);
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin os");
	}
}

void Engine::callBuiltInFSOp(Engine::BuiltInFSOperation const func) {
	switch (func) {
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_GET_BINARY:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_GET_JSON:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_GET_TEXT:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_GET_FLOW: {
			if (!iregister(0)->isString())
				pushUndefinedIfInLooseMode("builtin get file");
			else try {
				temporary() = new Value(onFileGetRequest(func));
			} catch (...) {
				temporary() = new Value(null);
			}
		} break;
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_SAVE_BINARY:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_SAVE_TEXT:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_SAVE_JSON:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_SAVE_FLOW: {
			if (!(
				iregister(0)->isString()
			&&	iregister(1)->isBytes()
			))
				pushUndefinedIfInLooseMode("builtin save file");
			else onFileSaveRequest(func);
		} break;
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_HAS_PATH:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_IS_DIR:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_MAKE_DIR:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_DELETE:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_COPY:
		case Engine::BuiltInFSOperation::AV2_EBI_FSO_MOVE: {
			if (!iregister(0)->isString())
				pushUndefinedIfInLooseMode("builtin fs op");
			else temporary() = new Value(onFilesystemRequest(func));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin fs");
	}
}

Makai::Data::Value Engine::onFileGetRequest(Engine::BuiltInFSOperation const func) {
	switch (func) {
		case BuiltInFSOperation::AV2_EBI_FSO_GET_BINARY: {
			if (!(
				iregister(0)->isString()
			)) pushUndefinedIfInLooseMode("builtin file get text");
		} break;
	}
}

Makai::Data::Value Engine::onFileSaveRequest(Engine::BuiltInFSOperation const func) {
	switch (func) {

	}
}

Makai::Data::Value Engine::onFilesystemRequest(Engine::BuiltInFSOperation const func) {
	switch (func) {

	}
}

Makai::Data::Value Engine::onArchiveRequest(Engine::BuiltInArchiveOperation const op) {
	switch (func) {

	}
}

void Engine::callBuiltInArchiveOp(BuiltInArchiveOperation const func) {
	switch (func) {
		case Engine::BuiltInArchiveOperation::AV2_EBI_AFO_NEW:
		case Engine::BuiltInArchiveOperation::AV2_EBI_AFO_LOAD:
		case Engine::BuiltInArchiveOperation::AV2_EBI_AFO_UNLOAD: {
			if (!(
				iregister(0)->isString()
			&&	iregister(1)->isString()
			&&	iregister(2)->isString()
			)) pushUndefinedIfInLooseMode("builtin arch op");
			else onArchiveRequest(func);
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin arch"); break;
	}
}

void Engine::callBuiltInCryptographyOp(BuiltInCryptographyOperation const func) {
	// TODO: This
	switch (func) {
		case BuiltInCryptographyOperation::AV2_EBI_EO_ENCODE: {
			if (!(
				iregister(0)->isBytes()
			&&	iregister(1)->isString()
			)) pushUndefinedIfInLooseMode("builtin crypt encode");
			else temporary() = new Value(Makai::Data::encode(iregister(0)->getBytes(), Makai::Data::fromString(iregister(1)->getString())));
		} break;
		case BuiltInCryptographyOperation::AV2_EBI_EO_DECODE: {
			if (!(
				iregister(0)->isString()
			&&	iregister(1)->isString()
			)) pushUndefinedIfInLooseMode("builtin crypt decode");
			else temporary() = new Value(Makai::Data::decode(iregister(0)->getString(), Makai::Data::fromString(iregister(1)->getString())));
		} break;
		case BuiltInCryptographyOperation::AV2_EBI_EO_ENCRYPT: {
			if (!(
				iregister(0)->isString()
			&&	iregister(1)->isString()
			)) pushUndefinedIfInLooseMode("builtin crypt encrypt");
			else temporary() = new Value(Makai::Tool::Arch::encrypt(iregister(0)->getBytes(), iregister(1)->getString()));
		} break;
		case BuiltInCryptographyOperation::AV2_EBI_EO_DECRYPT: {
			if (!(
				iregister(0)->isString()
			&&	iregister(1)->isString()
			)) pushUndefinedIfInLooseMode("builtin crypt decrypt");
			else temporary() = new Value(Makai::Tool::Arch::decrypt(iregister(0)->getBytes(), iregister(1)->getString()));
		} break;
		case BuiltInCryptographyOperation::AV2_EBI_EO_HASH: {
			if (!(
				iregister(0)->isString()
			)) pushUndefinedIfInLooseMode("builtin crypt hash");
			else temporary() = new Value(Makai::Tool::Arch::hashPassword(iregister(0)->getString()));
		} break;
		default: pushUndefinedIfInLooseMode("invalid builtin crypt"); break;
	}
}

void Engine::terminate() {
	DEBUGLN("Terminating...");
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
	else if (inStrictMode()) {
        crash(invalidFetchRequest(action));
        return Value();
    } else return Value();
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
	return makeErrorHere("Invalid data location for instruction ["+ toString(enumcast(loc)) + "]!");
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
