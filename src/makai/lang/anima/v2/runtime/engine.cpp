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

using namespace Core;

using Makai::Data::Value;

bool Engine::yieldCycle() {
	bool revertContext = false;
	if (context.scopeStack.empty())
		context.scopeStack.pushBack({});
	if (context.scopeStack.back().prevMode != context.scopeStack.back().mode)
		revertContext = true;
	if (isFinished) return false;
	do {
		DEBUGLN("Next instruction, please!");
		advance();
	} while (current.name == Instruction::Name::AV2_IN_NO_OP && current.type);
	if (isFinished) return false;
	DEBUGLN("Instruction: ", Instruction::asString(current.name));
	switch (current.name) {
		using enum Instruction::Name;
		case AV2_IN_HALT:			v2Halt();		break;
		case AV2_IN_STACK_POP:		v2StackPop();	break;
		case AV2_IN_STACK_PUSH:		v2StackPush();	break;
		case AV2_IN_STACK_CLEAR:	v2StackClear();	break;
		case AV2_IN_COPY:			v2Copy();		break;
		case AV2_IN_RETURN: 		v2Return();		break;
		case AV2_IN_CALL:			v2Call();		break;
		case AV2_IN_CAST:			v2Cast();		break;
		case AV2_IN_OP:				v2Op();			break;
		case AV2_IN_COMPARE:		v2Compare();	break;
		case AV2_IN_MODE:			v2SetContext();	break;
		case AV2_IN_JUMP:			v2Jump();		break;
		case AV2_IN_YIELD:			v2Yield();		break;
		case AV2_IN_NO_OP: break;
		default: crash(invalidInstructionError());
	}
	if (revertContext) context.scopeStack.back().mode = context.scopeStack.back().prevMode;
	return !isFinished;
}

bool Engine::process() {
	paused = false;
	while (Engine::yieldCycle() && !paused) {}
	DEBUGLN("Done processing for now!");
	return !isFinished;
}

void Engine::crash(Engine::Error const& e) {
	err = e;
	terminate();
}

void Engine::v2Yield() {
	paused = true;
}

void Engine::v2Compare() {
	Instruction::Comparison comp = bitcast<Instruction::Comparison>(current.type);
	if (context.globalValueStack.size() < 2)
		return crash(invalidSourceError("Missing values to compare!"));
	auto rhs	= context.globalValueStack.popBack();
	auto lhs	= context.globalValueStack.back();
	Value::OrderType order = Value::Order::EQUAL;
	if (
		lhs->getCurrentType() == rhs->getCurrentType()
	||	lhs->getCurrentType()->canBecome(rhs->getCurrentType())
	) order = lhs->compareWith(rhs);
	else if (lhs->isBoolean() && rhs->isBoolean())		order = lhs->toValue<bool>() <=> lhs->toValue<bool>();
	else if (lhs->isUnsigned() && rhs->isUnsigned())	order = lhs->toValue<uint64>() <=> lhs->toValue<uint64>();
	else if (lhs->isInteger() && rhs->isInteger())		order = lhs->toValue<int64>() <=> lhs->toValue<int64>();
	else if (lhs->isNumber() && rhs->isNumber())		order = lhs->toValue<double>() <=> lhs->toValue<double>();
	else if (inStrictMode())
		return crash(invalidComparisonError("Types do not match!"));
	else {
		context.globalValueStack.pushBack(Object::create());
		return;
	}
	if (order == Makai::StandardOrder::UNORDERED) {
		if (inStrictMode())
			return crash(invalidComparisonError("Failed to compare types!"));
		else {
			context.globalValueStack.pushBack(Object::create());
			return;
		}
	}
	switch (comp.comp) {
		using enum As<decltype(comp.comp)>;
		case AV2_OP_THREEWAY:
			*context.globalValueStack.back() = *context.art.newValue(enumcast<Makai::StandardOrder>(order));
		break;
		using enum As<Makai::StandardOrder>;
		case AV2_OP_EQUALS:			*context.globalValueStack.back() = *context.art.newValue(order == EQUAL);	break;
		case AV2_OP_NOT_EQUALS:		*context.globalValueStack.back() = *context.art.newValue(order != EQUAL);	break;
		case AV2_OP_GREATER_THAN:	*context.globalValueStack.back() = *context.art.newValue(order == GREATER);	break;
		case AV2_OP_GREATER_EQUALS:	*context.globalValueStack.back() = *context.art.newValue(order != LESS);	break;
		case AV2_OP_LESS_THAN:		*context.globalValueStack.back() = *context.art.newValue(order == LESS);	break;
		case AV2_OP_LESS_EQUALS:	*context.globalValueStack.back() = *context.art.newValue(order != GREATER);	break;
	}
}

void Engine::v2Halt() {
	Instruction::Stop stop = bitcast<Instruction::Stop>(current.type);
	switch (stop.mode) {
		case Core::Instruction::Stop::Mode::AV2_ISM_ERROR: {
			auto const v = consumeValue(DataLocation::AV2_DL_STRING);
			if (err) return;
			return crash(makeErrorHere("PROGRAM_ERROR: " + v->toValue<String>()));
		};
		default: terminate();
	};
}

Engine::Error Engine::makeErrorHere(String const& message) {
	if (CTL::CPP::Debug::hasDebugger())
		throw Makai::Error::FailedAction(
			"ANIMA_ERROR: " + message,
			CTL::CPP::SourceFile("BYTECODE:", context.pointers.instruction, ":ANP")
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
	uint64 loc = 0;
	if (invocation.dynamic) {
		if (context.globalValueStack.empty())
			return crash(invalidSourceError("Global stack is empty!"));
		loc = context.globalValueStack.popBack()->toValue<uint64>();
	} else {
		advance(true);
		loc = Makai::Cast::bit<uint64>(current);
	}
	if (invocation.external) {
		context.art
			.invokeExternalMethod(program.ani->out[loc], context.globalValueStack.reversed())
			.then(
				[&] (auto const& v) {
					context.globalValueStack.pushBack(v);
				}
			).onError(
				[&] (auto const& e) {
					Makai::String err = "EXTERNAL FUNCTION: ";
					switch (e) {
						using enum Core::Context::Error;
						case AV2_CCE_MISSING_METHOD:	err += "Function does not exist";
						case AV2_CCE_MISSING_ARGS:		err += "Not enough args for function";
						case AV2_CCE_MISSING_ART_TYPE:	err += "Return type does not exist in the current ART context";
					}
					crash(invalidFunctionError(err));
				}
			)
		;
	} else jumpBy(loc, true);
}

Runtime::Context::Storage Engine::consumeValue(DataLocation const from) {
	advance(true);
	auto const store = getValueFromLocation(from, bitcast<uint64>(current));
	if (!store) return Object::create();
	return store;
}

static Runtime::Context::Storage accessor(Runtime::Context::Storage const& v, bool const noCopy) {
	if (!v) return Object::create();
	return noCopy ? v : Object::create(*v);
}

Runtime::Context::Storage Engine::getValueFromLocation(DataLocation const loc, uint64 const id) {
	auto const place = asPlace(loc);
	auto const mod = asModifiers(loc);
	bool byRef	= mod == DataLocation::AV2_DLM_BY_REF;
	bool byMove	= mod == DataLocation::AV2_DLM_MOVE;
	switch (place) {
		case DataLocation::AV2_DL_INT: {
			return context.art.newValue(Makai::Cast::bit<int64>(id));
		} break;
		case DataLocation::AV2_DL_UINT: {
			return context.art.newValue(Makai::Cast::bit<uint64>(id));
		} break;
		case DataLocation::AV2_DL_REAL: {
			return context.art.newValue(Makai::Cast::bit<double>(id));
		} break;
		case DataLocation::AV2_DL_STRING: {
			return context.art.newValue(program.strings[id]);
		} break;
		case DataLocation::AV2_DL_STACK: {
			if (context.globalValueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return Object::create();
			}
			auto& loc = context.globalValueStack[id  % context.globalValueStack.size()];
			auto const v = loc;
			if (byMove) loc = nullptr;
			return accessor(v, byRef);
		}
		case DataLocation::AV2_DL_STACK_OFFSET: {
			if (context.globalValueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return Object::create();
			}
			auto& loc = context.globalValueStack[-Cast::as<ssize>(id % context.globalValueStack.size() + 1)];
			auto const v = loc;
			if (byMove) loc = nullptr;
			return accessor(v, byRef);
		}
		case DataLocation::AV2_DL_GLOBAL:		return global(id);
		case DataLocation::AV2_DL_EXTERNAL:		return external(program.strings[id], byRef);
		default: {
			if (inStrictMode())
				crash(invalidLocationError(loc));
			return Object::create();
		}
	}
	if (inStrictMode())
		crash(invalidLocationError(loc));
	return Object::create();
}

Runtime::Context::Storage& Engine::accessValue(DataLocation const from) {
	auto& loc = accessLocation(from, bitcast<uint64>(current));
	if (!loc) loc = Object::create();
	return loc;
}

Runtime::Context::Storage& Engine::accessLocation(DataLocation const loc, usize const id) {
	auto const place = asPlace(loc);
	switch (place) {
		case DataLocation::AV2_DL_STACK: {
			if (context.globalValueStack.empty()) {
				crash(invalidLocationError(loc));
			}
			return context.globalValueStack[id  % context.globalValueStack.size()];
		}
		case DataLocation::AV2_DL_STACK_OFFSET: {
			if (context.globalValueStack.empty()) {
				crash(invalidLocationError(loc));
			}
			return context.globalValueStack[-Cast::as<ssize>(id % context.globalValueStack.size() + 1)];
		}
		default:
			crash(invalidLocationError(loc));
	}
	crash(invalidLocationError(loc));
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
	return program.ani && program.ani->in.contains(signal);
}

void Engine::fire(String const& signal) {
	if (hasSignal(signal))
		jumpTo(program.jumpTable[program.ani->in[signal]], true);
}

void Engine::returnBack() {
	context.pointers = context.pointerStack.popBack();
}

Runtime::Context::Storage Engine::external(String const& name, bool const byRef) {
	return Object::create();
}

template <class T>
static bool bopIt(Object::Storage const& out, Object::Storage const& lhs, Object::Storage const& rhs, Operator const op, Runtime::Context& context) {
	switch (op) {
		using enum Operator;
		case AV2_BOP_ADD:	*out = *context.art.newValue<T>(lhs->toValue<T>() + rhs->toValue<T>()); return true;
		case AV2_BOP_SUB:	*out = *context.art.newValue<T>(lhs->toValue<T>() - rhs->toValue<T>()); return true;
		case AV2_BOP_MUL:	*out = *context.art.newValue<T>(lhs->toValue<T>() * rhs->toValue<T>()); return true;
		case AV2_BOP_DIV:	*out = *context.art.newValue<T>(lhs->toValue<T>() / rhs->toValue<T>()); return true;
		default: break;
	}
	if constexpr (Makai::Type::Number<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_REM:	*out = *context.art.newValue<T>(Makai::Math::mod(lhs->toValue<T>(), rhs->toValue<T>()));	return true;
			case AV2_BOP_POW:	*out = *context.art.newValue<T>(Makai::Math::pow(lhs->toValue<T>(), rhs->toValue<T>()));	return true;
			case AV2_BOP_ATAN2:	*out = *context.art.newValue<T>(Makai::Math::atan2(lhs->toValue<T>(), rhs->toValue<T>()));	return true;
			case AV2_BOP_LOGX:	*out = *context.art.newValue<T>(Makai::Math::logn(lhs->toValue<T>(), rhs->toValue<T>()));	return true;;
			default: break;
		}
	}
	if constexpr (Makai::Type::Integer<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_BIT_AND:	*out = *context.art.newValue<T>(lhs->toValue<T>() & rhs->toValue<T>());	return true;
			case AV2_BOP_BIT_OR:	*out = *context.art.newValue<T>(lhs->toValue<T>() | rhs->toValue<T>());	return true;
			case AV2_BOP_BIT_XOR:	*out = *context.art.newValue<T>(lhs->toValue<T>() ^ rhs->toValue<T>());	return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_BIT_AND:	*out = *context.art.newValue<T>(lhs->toValue<T>() && rhs->toValue<T>());	return true;
			case AV2_BOP_BIT_OR:	*out = *context.art.newValue<T>(lhs->toValue<T>() || rhs->toValue<T>());	return true;
			case AV2_BOP_BIT_XOR:	*out = *context.art.newValue<T>(lhs->toValue<T>() != rhs->toValue<T>());	return true;
			default: break;
		}
	}
	return false;
}

void Engine::doBinaryOperation(Operator const op) {
	if (context.globalValueStack.size() < 2)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto rhs	= context.globalValueStack.popBack();
	auto lhs	= context.globalValueStack.back();
	auto out	= lhs;
	if (err) return;
	bool success = false;
	if (lhs->isBoolean() && rhs->isBoolean())			success = bopIt<bool>(out, lhs, rhs, op, context);
	if (lhs->isUnsigned() && rhs->isUnsigned())			success = bopIt<uint64>(out, lhs, rhs, op, context);
	else if (lhs->isSigned() && rhs->isSigned())		success = bopIt<int64>(out, lhs, rhs, op, context);
	else if (lhs->isNumber() && rhs->isNumber())		success = bopIt<double>(out, lhs, rhs, op, context);
	else if (lhs->isAlgebraic() && rhs->isAlgebraic())	success = bopIt<Vector4>(out, lhs, rhs, op, context);
	if (!success) {
		if (inStrictMode())
			return crash(invalidBinaryMathError("Invalid/Unsupported operator!"));
		*out = *Object::create();
	}
}

template <class T>
static bool uopIt(Object::Storage const& out, Object::Storage const& lhs, Operator const op, Runtime::Context& context) {
	switch (op) {
		using enum Operator;
		case AV2_UOP_NEGATE:	*out = *context.art.newValue<T>(-lhs->toValue<T>());						return true;
		case AV2_UOP_INVERSE:	*out = *context.art.newValue<T>(Makai::Cast::as<T>(1) / lhs->toValue<T>());	return true;
		default: break;
	}
	if constexpr (Makai::Type::Ex::Math::Vector::Vector<T>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_LENGTH:	*out = *context.art.newValue<T>(lhs->toValue<T>().length());			return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Number<T>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_INCREMENT:	*out = *context.art.newValue<T>(++lhs->toValue<T>());					return true;
			case AV2_UOP_DECREMENT:	*out = *context.art.newValue<T>(--lhs->toValue<T>());					return true;
			case AV2_UOP_SIN:		*out = *context.art.newValue<T>(Makai::Math::sin(lhs->toValue<T>()));	return true;
			case AV2_UOP_COS:		*out = *context.art.newValue<T>(Makai::Math::cos(lhs->toValue<T>()));	return true;
			case AV2_UOP_TAN:		*out = *context.art.newValue<T>(Makai::Math::tan(lhs->toValue<T>()));	return true;
			case AV2_UOP_ASIN:		*out = *context.art.newValue<T>(asin(lhs->toValue<T>()));				return true;
			case AV2_UOP_ACOS:		*out = *context.art.newValue<T>(acos(lhs->toValue<T>()));				return true;
			case AV2_UOP_ATAN:		*out = *context.art.newValue<T>(atan(lhs->toValue<T>()));				return true;
			case AV2_UOP_SINH:		*out = *context.art.newValue<T>(sinh(lhs->toValue<T>()));				return true;
			case AV2_UOP_COSH:		*out = *context.art.newValue<T>(cosh(lhs->toValue<T>()));				return true;
			case AV2_UOP_TANH:		*out = *context.art.newValue<T>(tanh(lhs->toValue<T>()));				return true;
			case AV2_UOP_LOG2:		*out = *context.art.newValue<T>(Makai::Math::log2(lhs->toValue<T>()));	return true;
			case AV2_UOP_LOG10:		*out = *context.art.newValue<T>(Makai::Math::log10(lhs->toValue<T>()));	return true;
			case AV2_UOP_LN:		*out = *context.art.newValue<T>(Makai::Math::log(lhs->toValue<T>()));	return true;
			case AV2_UOP_SQRT:		*out = *context.art.newValue<T>(Makai::Math::sqrt(lhs->toValue<T>()));	return true;
			case AV2_UOP_LENGTH:	*out = *context.art.newValue<T>(Makai::Math::abs(lhs->toValue<T>()));	return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Integer<T>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_BIT_NOT:	*out = *context.art.newValue<T>(~lhs->toValue<T>());	return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_LOGIC_NOT:	*out = *context.art.newValue<T>(!lhs->toValue<T>());	return true;
			default: break;
		}
	}
}

void Engine::doUnaryOperation(Operator const op) {
	if (context.globalValueStack.size() < 1)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto lhs	= context.globalValueStack.back();
	auto out	= lhs;
	if (err) return;
	bool success = false;
	if (lhs->isBoolean())			success = uopIt<bool>(out, lhs, op, context);
	if (lhs->isUnsigned())			success = uopIt<uint64>(out, lhs, op, context);
	else if (lhs->isSigned())		success = uopIt<int64>(out, lhs, op, context);
	else if (lhs->isNumber())		success = uopIt<double>(out, lhs, op, context);
	else if (lhs->isAlgebraic())	success = uopIt<Vector4>(out, lhs, op, context);
	if (!success) {
		if (inStrictMode())
			return crash(invalidBinaryMathError("Invalid/Unsupported operator!"));
		*out = *Object::create();
	}
}

void Engine::v2Op() {
	Instruction::Operation op = Cast::bit<Instruction::Operation>(current.type);
	if (op.op < Operator::AV2_BOP_START)
		doBinaryOperation(op.op);
	else doUnaryOperation(op.op);
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

void Engine::load(Module const& prog) {
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
