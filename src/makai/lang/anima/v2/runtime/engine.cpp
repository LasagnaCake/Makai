#include "engine.hpp"
#include "context.hpp"

using Makai::Anima::V2::Runtime::Engine;

using namespace Makai::Anima::V2;

namespace Runtime	= Makai::Anima::V2::Runtime;

using namespace Core;

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
		case AV2_IN_STACK_BLIT:		v2StackBlit();	break;
		case AV2_IN_STACK_POP:		v2StackPop();	break;
		case AV2_IN_STACK_PUSH:		v2StackPush();	break;
		case AV2_IN_STACK_CLEAR:	v2StackClear();	break;
		case AV2_IN_STACK_FLUSH:	v2StackFlush();	break;
		case AV2_IN_STACK_SWAP:		v2StackSwap();	break;
		case AV2_IN_SCOPE_ENTER:	v2ScopeEnter();	break;
		case AV2_IN_SCOPE_EXIT:		v2ScopeExit();	break;
		case AV2_IN_SCOPE_BRING:	v2ScopeBring();	break;
		case AV2_IN_SCOPE_BIND:		v2ScopeBind();	break;
		case AV2_IN_SIZEOF:			v2Sizeof();		break;
		case AV2_IN_TYPEOF:			v2Typeof();		break;
		case AV2_IN_FIELD_GET:		v2FieldGet();	break;
		case AV2_IN_RANDOM:			v2Random();		break;
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
//		default: crash(invalidInstructionError());
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
	auto rhs	= context.pop();
	auto lhs	= context.top();
	Makai::Ordered::OrderType order = Makai::Ordered::Order::EQUAL;
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
			*context.top() = *context.art.newValue(enumcast<Makai::StandardOrder>(order));
		break;
		using enum As<Makai::StandardOrder>;
		case AV2_OP_EQUALS:			*context.top() = *context.art.newValue(order == EQUAL);	break;
		case AV2_OP_NOT_EQUALS:		*context.top() = *context.art.newValue(order != EQUAL);	break;
		case AV2_OP_GREATER_THAN:	*context.top() = *context.art.newValue(order == GREATER);	break;
		case AV2_OP_GREATER_EQUALS:	*context.top() = *context.art.newValue(order != LESS);	break;
		case AV2_OP_LESS_THAN:		*context.top() = *context.art.newValue(order == LESS);	break;
		case AV2_OP_LESS_EQUALS:	*context.top() = *context.art.newValue(order != GREATER);	break;
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

Engine::Error Engine::invalidOperationError(String const& description) {
	return makeErrorHere("INVALID OPERATOR: " + description);
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
		loc = context.pop()->toValue<uint64>();
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
	auto const place	= asPlace(loc);
	auto const mod		= asModifiers(loc);
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
		case DataLocation::AV2_DL_GLOBAL:	return global(id);
		case DataLocation::AV2_DL_LOCAL: {
			if (context.scopeStack.back().localStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return Object::create();
			}
			auto& loc = context.scopeStack.back().localStack[id  % context.scopeStack.back().localStack.size()];
			auto const v = loc;
			if (byMove) loc = nullptr;
			return accessor(v, byRef);
		}
		case DataLocation::AV2_DL_EXTERNAL: {
			if (program.ani)
				return external(program.ani->out[id], byRef);
			else if (inStrictMode())
				crash(invalidLocationError(loc));
			return Object::create();
		}
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
	static Context::Storage failsafe;
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
		case DataLocation::AV2_DL_LOCAL: {
			if (context.locals().empty()) {
				crash(invalidLocationError(loc));
			}
			return context.locals()[id % context.locals().size()];
		}
		default:
			crash(invalidLocationError(loc));
	}
	crash(invalidLocationError(loc));
	return failsafe;
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

template <Makai::Type::Integer T>
using Int = Makai::Meta::If<Makai::Type::Unsigned<T>, uint64, int64>;

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
			case AV2_BOP_REM:	*out = *context.art.newValue<T>(Makai::Math::mod<double>(lhs->toValue<T>(), rhs->toValue<T>()));	return true;
			case AV2_BOP_POW:	*out = *context.art.newValue<T>(Makai::Math::pow<double>(lhs->toValue<T>(), rhs->toValue<T>()));	return true;
			case AV2_BOP_ATAN2:	*out = *context.art.newValue<T>(Makai::Math::atan2<double>(lhs->toValue<T>(), rhs->toValue<T>()));	return true;
			case AV2_BOP_LOGX:	*out = *context.art.newValue<T>(Makai::Math::logn<double>(lhs->toValue<T>(), rhs->toValue<T>()));	return true;;
			default: break;
		}
	}
	if constexpr (Makai::Type::Integer<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_BIT_AND:	*out = *context.art.newValue<T>(lhs->toValue<Int<T>>() & rhs->toValue<Int<T>>());	return true;
			case AV2_BOP_BIT_OR:	*out = *context.art.newValue<T>(lhs->toValue<Int<T>>() | rhs->toValue<Int<T>>());	return true;
			case AV2_BOP_BIT_XOR:	*out = *context.art.newValue<T>(lhs->toValue<Int<T>>() ^ rhs->toValue<Int<T>>());	return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_LOGIC_AND:	*out = *context.art.newValue<T>(lhs->toValue<T>() && rhs->toValue<T>());	return true;
			case AV2_BOP_LOGIC_OR:	*out = *context.art.newValue<T>(lhs->toValue<T>() || rhs->toValue<T>());	return true;
			case AV2_BOP_LOGIC_XOR:	*out = *context.art.newValue<T>(lhs->toValue<T>() != rhs->toValue<T>());	return true;
			default: break;
		}
	}
	return false;
}

void Engine::doBinaryOperation(Operator const op) {
	if (context.globalValueStack.size() < 2)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto rhs	= context.pop();
	auto lhs	= context.top();
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
			return crash(invalidOperationError("Invalid/Unsupported operator for the given values!"));
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
			case AV2_UOP_INCREMENT:	*out = *context.art.newValue<T>(lhs->toValue<T>()+1);							return true;
			case AV2_UOP_DECREMENT:	*out = *context.art.newValue<T>(lhs->toValue<T>()-1);							return true;
			case AV2_UOP_SIN:		*out = *context.art.newValue<T>(Makai::Math::sin(lhs->toValue<double>()));		return true;
			case AV2_UOP_COS:		*out = *context.art.newValue<T>(Makai::Math::cos(lhs->toValue<double>()));		return true;
			case AV2_UOP_TAN:		*out = *context.art.newValue<T>(Makai::Math::tan(lhs->toValue<double>()));		return true;
			case AV2_UOP_ASIN:		*out = *context.art.newValue<T>(asin(lhs->toValue<T>()));						return true;
			case AV2_UOP_ACOS:		*out = *context.art.newValue<T>(acos(lhs->toValue<T>()));						return true;
			case AV2_UOP_ATAN:		*out = *context.art.newValue<T>(atan(lhs->toValue<T>()));						return true;
			case AV2_UOP_SINH:		*out = *context.art.newValue<T>(sinh(lhs->toValue<T>()));						return true;
			case AV2_UOP_COSH:		*out = *context.art.newValue<T>(cosh(lhs->toValue<T>()));						return true;
			case AV2_UOP_TANH:		*out = *context.art.newValue<T>(tanh(lhs->toValue<T>()));						return true;
			case AV2_UOP_LOG2:		*out = *context.art.newValue<T>(Makai::Math::log2(lhs->toValue<double>()));		return true;
			case AV2_UOP_LOG10:		*out = *context.art.newValue<T>(Makai::Math::log10(lhs->toValue<double>()));	return true;
			case AV2_UOP_LN:		*out = *context.art.newValue<T>(Makai::Math::log(lhs->toValue<double>()));		return true;
			case AV2_UOP_SQRT:		*out = *context.art.newValue<T>(Makai::Math::sqrt(lhs->toValue<double>()));		return true;
			case AV2_UOP_LENGTH:	*out = *context.art.newValue<T>(Makai::Math::abs(lhs->toValue<T>()));			return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Integer<T>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_BIT_NOT:	*out = *context.art.newValue<T>(~lhs->toValue<Int<T>>());	return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_LOGIC_NOT:	*out = *context.art.newValue<T>(!lhs->toValue<T>());		return true;
			default: break;
		}
	}
	return false;
}

void Engine::doUnaryOperation(Operator const op) {
	if (context.globalValueStack.size() < 1)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto lhs	= context.top();
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
			return crash(invalidOperationError("Invalid/Unsupported operator for the given values!"));
		*out = *Object::create();
	}
}

void Engine::v2Op() {
	Instruction::Operation op = Cast::bit<Instruction::Operation>(current.type);
	if (op.op < Operator::AV2_BOP_START)
		doUnaryOperation(op.op);
	else doBinaryOperation(op.op);
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

void Engine::v2SetContext() {
	auto const ctx = Cast::bit<Instruction::Context>(current.type);
	if (!ctx.immediate)
		context.scope().prevMode	= ctx.mode;
	context.scope().mode			= ctx.mode;
}

void Engine::v2StackPush() {
	auto const inter = Cast::bit<Instruction::StackPush>(current.type);
	auto const value = consumeValue(inter.location);
	if (err) return;
	context.push(value);
}

void Engine::v2StackPop() {
	if (context.globalValueStack.size())
		context.pop();
}

void Engine::v2StackSwap() {
	if (context.globalValueStack.size() >= 2)
		Makai::swap(context.globalValueStack[-1], context.globalValueStack[-2]);
}

void Engine::v2StackClear() {
	if (current.type)
		context.globalValueStack.removeRange(-Math::max<int>(current.type, context.globalValueStack.size()));
}

void Engine::v2StackFlush() {
	context.globalValueStack.clear();
}

void Engine::v2Jump() {
	Instruction::Leap leap = current.getTypeAs<Instruction::Leap>();
	using enum As<decltype(leap.type)>;
	uint64 loc = 0;
	if (context.globalValueStack.size() < ((leap.type != AV2_ILT_UNCONDITIONAL) + leap.dyn))
		return crash(invalidSourceError("Not enough parameters for jump!"));
	if (leap.dyn) {
		if (context.globalValueStack.empty())
			return crash(invalidSourceError("Global stack is empty!"));
		loc = context.pop()->toValue<uint64>();
	} else {
		advance(true);
		loc = Makai::Cast::bit<uint64>(current);
	}
	bool shouldJump = false;
	if (leap.type == AV2_ILT_UNCONDITIONAL) {
		shouldJump = true;
	} else {
		if (context.globalValueStack.empty())
			return crash(invalidSourceError("Global stack is empty!"));
		auto const cond = context.pop();
		switch (leap.type) {
			case AV2_ILT_IF_TRUTHY:				shouldJump	= cond->toValue<bool>();		break;
			case AV2_ILT_IF_FALSY:			 	shouldJump	= !cond->toValue<bool>();		break;
			case AV2_ILT_IF_ZERO:				shouldJump	= cond->toValue<double>() == 0;	break;
			case AV2_ILT_IF_NOT_ZERO:			shouldJump	= cond->toValue<double>() != 0;	break;
			case AV2_ILT_IF_NEGATIVE:			shouldJump	= cond->toValue<double>() < 0;	break;
			case AV2_ILT_IF_POSITIVE:			shouldJump	= cond->toValue<double>() > 0;	break;
			case AV2_ILT_IF_NULL:				shouldJump	= context.art.types.byName("nil").find(cond->getCurrentType()) != -1;	break;
			case AV2_ILT_IF_UNDEFINED:			shouldJump	= context.art.types.byName("void").find(cond->getCurrentType()) != -1;	break;
			case AV2_ILT_IF_NULL_OR_UNDEFINED:	shouldJump	= (
				context.art.types.byName("nil").find(cond->getCurrentType()) != -1
			||	context.art.types.byName("void").find(cond->getCurrentType()) != -1
			);	break;
			default: break;
		}
	}
	if (shouldJump)
		jumpBy(loc, false);
}

Engine::Error Engine::invalidLocationError(DataLocation const& loc) {
	return makeErrorHere("Invalid data location for instruction ["+ toString(enumcast(loc)) + "]!");
}

Engine::Error Engine::invalidCast(String const& description) {
	return makeErrorHere(description);
}

Engine::Error Engine::invalidJump() {
	return makeErrorHere("Jump target does not exist!");
}

void Engine::v2ScopeBring() {
	Instruction::Binding bind = Makai::Cast::bit<Instruction::Binding>(current.type);
	advance(true);
	auto const scope = Makai::Cast::bit<uint64>(current);
	advance(true);
	auto const count = Makai::Cast::bit<uint64>(current);
	if (!(scope < context.scopeStack.size()))
		return crash(outOfRangeError("Requested scope is out-of-range!"));
	auto& src = context.scopeStack[-scope].localStack;
	auto& dst = context.locals();
	if (!((bind.src + count) < src.size()))
		return crash(outOfRangeError("Requested source start + count is bigger than its stack size!"));
	if (!((bind.dst + count) < dst.size()))
		return crash(outOfRangeError("Requested destination start + count is bigger than its stack size!"));
	for (usize i = 0; i < count; ++i)
		dst[i + bind.dst] = src[i + bind.src];
}

void Engine::v2ScopeBind() {
	Instruction::Binding bind = Makai::Cast::bit<Instruction::Binding>(current.type);
	advance(true);
	auto const count = Makai::Cast::bit<uint64>(current);
	auto& src = context.globalValueStack;
	auto& dst = context.locals();
	if (!((bind.src + count) < src.size()))
		return crash(outOfRangeError("Requested global stack range falls outside its size!"));
	if (!((bind.dst + count) < dst.size()))
		return crash(outOfRangeError("Requested destination range falls outside its size!"));
	for (usize i = 0; i < count; ++i)
		dst[i + bind.dst] = src[i + (src.size() - count - bind.src + 1)];
}

void Engine::v2ScopeEnter() {
	auto const count = current.type;
	if (context.globalValueStack.size() < count)
		return crash(missingArgumentsError());
	context.scopeStack.pushBack({
		.mode		= context.scope().mode,
		.prevMode	= context.scope().mode
	});
	if (count) context.locals().resize(count, nullptr);
}

void Engine::v2ScopeExit() {
	if (context.scopeStack.size())
		context.scopeStack.popBack();
}

void Engine::v2FieldGet() {
	Instruction::Field field = current.getTypeAs<Instruction::Field>();
	uint64 loc = 0;
	if (field.dynamic) {
		if (context.globalValueStack.empty())
			return crash(invalidSourceError("Global stack is empty!"));
		loc = context.pop()->toValue<uint64>();
	} else {
		advance(true);
		loc = Makai::Cast::bit<uint64>(current);
	}
	advance(true);
	context.push(context.pop()->at(loc));
}

void Engine::v2Sizeof() {
	if (current.type) {
		context.push(context.pop()->count());
	} else {
		auto const val = context.pop();
		context.push(val->count() * val->getOriginalType()->byteSize);
	}
}

void Engine::v2Typeof() {
	context.push(context.pop()->getCurrentType()->id);
}

void Engine::v2Random() {
	Instruction::Randomness rng = Makai::Cast::bit<Instruction::Randomness>(current.type);
	Nullable<uint64> val;
	if (rng.getSeed) val = prng.getSeed();
	if (rng.setSeed) prng.setSeed(context.pop()->toValue<uint64>());
	if (val) context.push(*val);
	if (!(rng.setSeed || rng.getSeed)) {
		Object::Storage lo, hi;
		if (rng.bounded) {
			hi = context.pop();
			lo = context.pop();
		}
		if (rng.secure) switch (rng.type) {
			using enum Instruction::Randomness::Type;
			case AV2_IRT_INT:	context.push(rng.bounded ? srng.number<int64>(lo->toValue<int64>(), hi->toValue<int64>()) : srng.number<int64>());		break;
			case AV2_IRT_UINT:	context.push(rng.bounded ? srng.number<uint64>(lo->toValue<uint64>(), hi->toValue<uint64>()) : srng.number<uint64>());	break;
			case AV2_IRT_REAL:	context.push(rng.bounded ? srng.number<double>(lo->toValue<double>(), hi->toValue<double>()) : srng.number<double>());	break;
		} else switch (rng.type) {
			using enum Instruction::Randomness::Type;
			case AV2_IRT_INT:	context.push(rng.bounded ? prng.number<int64>(lo->toValue<int64>(), hi->toValue<int64>()) : prng.number<int64>());		break;
			case AV2_IRT_UINT:	context.push(rng.bounded ? prng.number<uint64>(lo->toValue<uint64>(), hi->toValue<uint64>()) : prng.number<uint64>());	break;
			case AV2_IRT_REAL:	context.push(rng.bounded ? prng.number<double>(lo->toValue<double>(), hi->toValue<double>()) : prng.number<double>());	break;
		}
	}
}

void Engine::v2StackBlit() {
	Instruction::Blitting blit = Makai::Cast::bit<Instruction::Blitting>(current.type);
	auto& src = blit.fromGlobal ? context.globalValueStack : context.locals();
	auto& dst = blit.fromGlobal ? context.locals() : context.globalValueStack;
	advance(true);
	auto const count = Makai::Cast::bit<uint64>(current);
	if (!(blit.offset + count < src.size()))
		return crash(outOfRangeError("Requested blit range falls outside source's size!"));
	if (blit.fromGlobal)
		dst.appendBack(src.sliced(-(blit.offset+1 + count), -(blit.offset+1)));
	else dst.appendBack(src.sliced(blit.offset, blit.offset + count));
}
