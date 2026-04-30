#include "engine.hpp"
#include "context.hpp"

using Makai::Anima::V2::Runtime::Engine;

using namespace Makai::Anima::V2;

namespace Runtime	= Makai::Anima::V2::Runtime;

using namespace Core;

bool Engine::DefaultLibraryLoader::loadLibrary(Context& context, String const& path) {
	StringList paths = {
		path,
		OS::FS::concatenate(OS::FS::sourceLocation(), path),
		OS::FS::concatenate(OS::FS::currentDirectory(), path),
	};
	if (auto const artHome = getenv("ART_HOME"))
		paths.pushBack(OS::FS::concatenate(artHome, path));
	for (auto const& p: paths)
		if (OS::FS::exists(p))
			return context.art.openLibrary(path);
	return false;
}

bool Engine::yieldCycle() {
	bool revertContext = false;
	if (context.scopeStack.empty())
		context.scopeStack.pushBack({});
	if (context.scopeStack.back().prevMode != context.scopeStack.back().mode)
		revertContext = true;
	if (!running()) return false;
	do {
		DEBUGLN("Next instruction, please!");
		advance();
	} while (running() && current.name == Instruction::Name::AV2_IN_NO_OP && current.type);
	if (!running()) return false;
	DEBUGLN("Instruction: ", Instruction::asString(current.name));
	switch (current.name) {
		using enum Instruction::Name;
		case AV2_IN_HALT:			v2Halt();			break;
		case AV2_IN_STACK_BLIT:		v2StackBlit();		break;
		case AV2_IN_STACK_POP:		v2StackPop();		break;
		case AV2_IN_STACK_PUSH:		v2StackPush();		break;
		case AV2_IN_STACK_CLEAR:	v2StackClear();		break;
		case AV2_IN_STACK_FLUSH:	v2StackFlush();		break;
		case AV2_IN_STACK_SWAP:		v2StackSwap();		break;
		case AV2_IN_SCOPE_ENTER:	v2ScopeEnter();		break;
		case AV2_IN_SCOPE_EXIT:		v2ScopeExit();		break;
		case AV2_IN_SCOPE_BRING:	v2ScopeBring();		break;
		case AV2_IN_SCOPE_BIND:		v2ScopeBind();		break;
		case AV2_IN_SCOPE_DECLARE:	v2ScopeDeclare();	break;
		case AV2_IN_SCOPE_KEEP:		v2ScopeKeep();		break;
		case AV2_IN_SIZEOF:			v2Sizeof();			break;
		case AV2_IN_TYPEOF:			v2Typeof();			break;
		case AV2_IN_FIELD_GET:		v2FieldGet();		break;
		case AV2_IN_FIELD_SET:		v2FieldSet();		break;
		case AV2_IN_RANDOM:			v2Random();			break;
		case AV2_IN_COPY:			v2Copy();			break;
		case AV2_IN_RETURN: 		v2Return();			break;
		case AV2_IN_CALL:			v2Call();			break;
		case AV2_IN_CAST:			v2Cast();			break;
		case AV2_IN_OP:				v2Op();				break;
		case AV2_IN_COMPARE:		v2Compare();		break;
		case AV2_IN_MODE:			v2SetContext();		break;
		case AV2_IN_JUMP:			v2Jump();			break;
		case AV2_IN_YIELD:			v2Yield();			break;
		case AV2_IN_CLEAR:			v2Clear();			break;
		case AV2_IN_SELECT:			v2Select();			break;
		case AV2_IN_CREATE:			v2Create();			break;
		case AV2_IN_INITIALIZE:		v2Initialize();		break;
		case AV2_IN_NO_OP: break;
//		default: crash(invalidInstructionError());
	}
	if (revertContext) context.scopeStack.back().mode = context.scopeStack.back().prevMode;
	return running();
}

bool Engine::process() {
	paused = false;
	while (Engine::yieldCycle() && !paused) {}
	DEBUGLN("Done processing for now!");
	return running();
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
		using enum Core::Comparator;
		case AV2_OP_THREEWAY:
			*context.top() = *context.art.newValue(enumcast<Makai::StandardOrder>(order));
		break;
		using enum Makai::StandardOrder;
		case AV2_OP_EQUALS:			*context.top() = *context.art.newValue(order == EQUAL);		break;
		case AV2_OP_NOT_EQUALS:		*context.top() = *context.art.newValue(order != EQUAL);		break;
		case AV2_OP_GREATER_THAN:	*context.top() = *context.art.newValue(order == GREATER);	break;
		case AV2_OP_GREATER_EQUALS:	*context.top() = *context.art.newValue(order != LESS);		break;
		case AV2_OP_LESS_THAN:		*context.top() = *context.art.newValue(order == LESS);		break;
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
			.invokeExternalMethod(loc, context.globalValueStack.reversed())
			.then(
				[&] (auto const& v) {
					if (v && !v->isEmptyType())
						context.globalValueStack.pushBack(v);
				}
			).onError(
				[&] (auto const& e) {
					if (invocation.optional) return;
					Makai::String err = "EXTERNAL FUNCTION: ";
					switch (e) {
						using enum Core::Context::Error;
						case AV2_CCE_MISSING_METHOD:		err += "Function does not exist";
						case AV2_CCE_MISSING_ARGS:			err += "Not enough args for function";
						case AV2_CCE_MISSING_ART_TYPE:		err += "Return type does not exist in the current ART context";
						case AV2_CCE_HOW_DID_YOU_GET_HERE:	err += "Somehow, execution reached an unreachable point";
					}
					crash(invalidFunctionError(err));
				}
			)
		;
	} else jumpBy(loc, true);
}

Runtime::Context::Storage Engine::consumeValue(DataLocation const from) {
	if (asPlace(from) != DataLocation::AV2_DL_BOOL) advance(true);
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
		case DataLocation::AV2_DL_BOOL: {
			return context.art.newValue((mod & DataLocation::AV2_DLB_TRUE) == DataLocation::AV2_DLB_TRUE);
		} break;
		case DataLocation::AV2_DL_INT: {
			if ((mod & DataLocation::AV2_DLI_UNSIGNED) == DataLocation::AV2_DLI_UNSIGNED) {
				switch (mod & ~DataLocation::AV2_DLI_UNSIGNED) {
					case DataLocation::AV2_DLI_16: return context.art.newValue(Makai::Cast::bit<int16, uint16>(id)); break;
					case DataLocation::AV2_DLI_32: return context.art.newValue(Makai::Cast::bit<int32, uint32>(id)); break;
					case DataLocation::AV2_DLI_64: return context.art.newValue(Makai::Cast::bit<int64, uint64>(id)); break;
					default: return context.art.newValue(Makai::Cast::bit<int8, uint8>(id)); break;
				}
			} else {
				switch (mod & ~DataLocation::AV2_DLI_UNSIGNED) {
					case DataLocation::AV2_DLI_16: return context.art.newValue(Makai::Cast::as<uint16>(id)); break;
					case DataLocation::AV2_DLI_32: return context.art.newValue(Makai::Cast::as<uint32>(id)); break;
					case DataLocation::AV2_DLI_64: return context.art.newValue(Makai::Cast::as<uint64>(id)); break;
					default: return context.art.newValue(Makai::Cast::as<uint8>(id)); break;
				}
			}
		} break;
		case DataLocation::AV2_DL_REAL: {
			switch (mod) {
				case DataLocation::AV2_DLF_64: return context.art.newValue(Makai::Cast::bit<float64>(id)); break;
				case DataLocation::AV2_DLF_128: return context.art.newValue(Makai::Cast::as<float128>(Makai::Cast::bit<float64>(id))); break;
				default: return context.art.newValue(Makai::Cast::bit<float32, uint32>(id)); break;
			}
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
	if (tableID == Makai::Limit::MAX<uint64>)
		return;
	if (tableID < program.jumpTable.size())
		jumpTo(program.jumpTable[tableID], returnable);
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
	while (
		context.scopeStack.size()
	&&	context.scopeStack.back().pointerFrame >= context.pointerStack.size()
	) context.scopeStack.popBack();
}

Runtime::Context::Storage Engine::external(String const& name, bool const byRef) {
	return Object::create();
}

template <Makai::Type::Integer T>
using Int = Makai::Meta::If<Makai::Type::Unsigned<T>, uint64, int64>;

template <class T>
static bool bopIt(Object::Storage const& out, Object::Storage const& lhs, Object::Storage const& rhs, Operator const op, Runtime::Context& context) {
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_ADD:	*out = *context.art.newValue<T>(lhs->toValue<T>() || rhs->toValue<T>());	return true;
			case AV2_BOP_SUB:	*out = *context.art.newValue<T>(lhs->toValue<T>() != rhs->toValue<T>());	return true;
			case AV2_BOP_MUL:	*out = *context.art.newValue<T>(lhs->toValue<T>() && rhs->toValue<T>());	return true;
			default: break;
		}
	} else if constexpr (Makai::Type::Number<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_ADD:	*out = *context.art.newValue<T>(lhs->toValue<T>() + rhs->toValue<T>()); return true;
			case AV2_BOP_SUB:	*out = *context.art.newValue<T>(lhs->toValue<T>() - rhs->toValue<T>()); return true;
			case AV2_BOP_MUL:	*out = *context.art.newValue<T>(lhs->toValue<T>() * rhs->toValue<T>()); return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Different<T, Makai::Matrix4x4>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_DIV:	*out = *context.art.newValue<T>(lhs->toValue<T>() / rhs->toValue<T>()); return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Number<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_REM:	*out = *context.art.newValue<T>((T)Makai::Math::mod<double>(lhs->toValue<T>(), rhs->toValue<T>()));		return true;
			case AV2_BOP_POW:	*out = *context.art.newValue<T>(Makai::Math::pow<double>(lhs->toValue<T>(), rhs->toValue<T>()));		return true;
			case AV2_BOP_ATAN2:	*out = *context.art.newValue<T>((T)Makai::Math::atan2<double>(lhs->toValue<T>(), rhs->toValue<T>()));	return true;
			case AV2_BOP_LOGX:	*out = *context.art.newValue<T>(Makai::Math::logn<double>(lhs->toValue<T>(), rhs->toValue<T>()));		return true;;
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
	if (lhs->isBoolean() && rhs->isBoolean())				success = bopIt<bool>(out, lhs, rhs, op, context);
	if (lhs->isUnsigned() && rhs->isUnsigned())				success = bopIt<uint64>(out, lhs, rhs, op, context);
	else if (lhs->isSigned() && rhs->isSigned())			success = bopIt<int64>(out, lhs, rhs, op, context);
	else if (lhs->isNumber() && rhs->isNumber())			success = bopIt<double>(out, lhs, rhs, op, context);
	else if (lhs->isVectorable() && rhs->isVectorable())	success = bopIt<Vector4>(out, lhs, rhs, op, context);
	else if (lhs->isAlgebraic() && rhs->isAlgebraic())		success = bopIt<Matrix4x4>(out, lhs, rhs, op, context);
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
			case AV2_UOP_ASIN:		*out = *context.art.newValue<T>((T)asin(lhs->toValue<T>()));					return true;
			case AV2_UOP_ACOS:		*out = *context.art.newValue<T>((T)acos(lhs->toValue<T>()));					return true;
			case AV2_UOP_ATAN:		*out = *context.art.newValue<T>((T)atan(lhs->toValue<T>()));					return true;
			case AV2_UOP_SINH:		*out = *context.art.newValue<T>((T)sinh(lhs->toValue<T>()));					return true;
			case AV2_UOP_COSH:		*out = *context.art.newValue<T>((T)cosh(lhs->toValue<T>()));					return true;
			case AV2_UOP_TANH:		*out = *context.art.newValue<T>((T)tanh(lhs->toValue<T>()));					return true;
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
	if (engineState == State::AV2_RES_RUNNING && program.exit) {
		engineState = State::AV2_RES_FINISHED;
		jumpBy(*program.exit, false);
		while (running()) process();
	}
	engineState = State::AV2_RES_FINISHED;
}

bool Engine::running() const {
	return (
		engineState == State::AV2_RES_INITIALIZING
	or	engineState == State::AV2_RES_RUNNING
	or	engineState == State::AV2_RES_EXITING
	);
}

bool Engine::finished() const {
	return (engineState == State::AV2_RES_FINISHED);
}

void Engine::reset() {
	terminate();
	unload();
	context		= {};
	current		= {};
	err			= {};
	engineState = State::AV2_RES_READY;
}

void Engine::load(Module const& prog) {
	reset();
	if (!(
		prog.type == decltype(prog.type)::AV2_CMT_EXE
	or	prog.type != decltype(prog.type)::AV2_CMT_CLI_EXE
	)) return;
	program = prog;
}

void Engine::execute() {
	if (running()) return;
	engineState = State::AV2_RES_INITIALIZING;
}

void Engine::load() {
	if (engineState != State::AV2_RES_INITIALIZING) return;
	Map<uint64, uint64> inheritances;
	Map<uint64, uint64> boundTypes;
	Map<uint64, List<uint64>> fields;
	for (auto const& [type, i]: Range::expand(program.detail.types)) {
		if (type.flags & Definition::Flags::AV2_DF_ART_EQUIVALENT) {
			boundTypes[i] = type.hash;
			context.art.types.values.pushBack(nullptr);
			continue;
		}
		Instance<Core::Definition> dt = dt.create();
		dt->hash = type.hash;
		if (type.base) inheritances[i] = *type.base;
		if (type.fields.size())
			fields[i] = type.fields;
		dt->basic = type.basic;
		dt->flags = type.flags;
		if (type.basic) {
			auto const basic = *type.basic;
			switch (basic) {
				case Core::BasicType::AV2_BT_BOOL:		dt->byteSize = sizeof(bool); break;
				case Core::BasicType::AV2_BT_INT8:		dt->byteSize = sizeof(int8); break;
				case Core::BasicType::AV2_BT_UINT8:		dt->byteSize = sizeof(uint8); break;
				case Core::BasicType::AV2_BT_INT16:		dt->byteSize = sizeof(int16); break;
				case Core::BasicType::AV2_BT_UINT16:	dt->byteSize = sizeof(uint16); break;
				case Core::BasicType::AV2_BT_INT32:		dt->byteSize = sizeof(int32); break;
				case Core::BasicType::AV2_BT_UINT32:	dt->byteSize = sizeof(uint32); break;
				case Core::BasicType::AV2_BT_INT64:		dt->byteSize = sizeof(int64); break;
				case Core::BasicType::AV2_BT_UINT64:	dt->byteSize = sizeof(uint64); break;
				case Core::BasicType::AV2_BT_REAL32:	dt->byteSize = sizeof(float32); break;
				case Core::BasicType::AV2_BT_REAL64:	dt->byteSize = sizeof(float64); break;
				case Core::BasicType::AV2_BT_REAL128:	dt->byteSize = sizeof(float128); break;
				case Core::BasicType::AV2_BT_CHAR:		dt->byteSize = sizeof(UTF8Char); break;
				case Core::BasicType::AV2_BT_STRING:	dt->byteSize = sizeof(UTF8String); break;
				case Core::BasicType::AV2_BT_VECTOR:	dt->byteSize = sizeof(Vector4); break;
				case Core::BasicType::AV2_BT_MATRIX:	dt->byteSize = sizeof(Matrix4x4); break;
				case Core::BasicType::AV2_BT_BYTES:		dt->byteSize = sizeof(Bytes<>); break;
				case Core::BasicType::AV2_BT_TYPEID:	dt->byteSize = sizeof(Core::TypeID); break;
				case Core::BasicType::AV2_BT_ANY:
				case Core::BasicType::AV2_BT_NOT_A_BASIC_TYPE:
				case Core::BasicType::AV2_BT_VOID:
				case Core::BasicType::AV2_BT_NULL: dt->byteSize = 0; break;
			}
			dt->alignment = 1;
			Definition::makeBasic(*dt);
		} else {
			dt->alignment = type.alignment;
			dt->byteSize = type.byteSize;
		}
		context.art.types.addElement(dt);
	}
	for (auto const& [self, base]: inheritances)
		context.art.types.values[self]->base = context.art.types.values[base].asWeak();
	for (auto const& [self, fields]: fields)
		for (auto const& field: fields)
			context.art.types.values[self]->fields.pushBack(context.art.types.values[field].asWeak());
	for (auto const& [self, artEquiv]: boundTypes) {
		auto const types = context.art.types.byNameHash(artEquiv);
		if (types.size())
			context.art.types.values[self] = types.front();
		else crash(makeErrorHere("ART context does not contain the requested type!"));
	}
	if (program.entry) jumpBy(*program.entry, false);
	else crash(makeErrorHere("Missing entrypoint!"));
	if (program.ani && loader)
		for (auto& lib: program.ani->shared.libraries)
			loader->loadLibrary(context, lib);
	context.art.loadLibraries();
	engineState = State::AV2_RES_RUNNING;
}

void Engine::unload() {
	context.art.unloadLibraries();
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
	using enum Instruction::Leap::Type;
	uint64 loc = 0;
	if (context.globalValueStack.size() < Makai::Cast::as<uint>((leap.type != AV2_ILT_UNCONDITIONAL) + leap.dyn))
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
			case AV2_ILT_IF_NULL:
			case AV2_ILT_IF_UNDEFINED:
			case AV2_ILT_IF_NULL_OR_UNDEFINED:	shouldJump	= cond.exists();				break;
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

Engine::Error Engine::outOfRangeError(String const& desc) {
	return makeErrorHere(desc);
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
		.mode			= context.scope().mode,
		.prevMode		= context.scope().mode,
		.pointerFrame	= context.pointerStack.size()
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

void Engine::v2FieldSet() {
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
	auto const v = context.pop();
	context.top()->at(loc).set(v);
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

void Engine::v2Create() {
	// TODO: This
}

void Engine::v2Clear() {
	// TODO: This
}

void Engine::v2Initialize() {
	// TODO: This
}

void Engine::v2ScopeKeep() {
	if (context.scopeStack.size() < 2) return;
	auto& locals = context.scope().localStack;
	auto& parentLocals = context.scopeStack[-2].localStack;
	if (locals.size() >= parentLocals.size())
		for (auto const& i: range(parentLocals.size()))
			locals[i] = parentLocals[i];
	if (locals.size() < parentLocals.size())
		locals = parentLocals;
	else locals.appendBack(parentLocals);
}

void Engine::v2ScopeDeclare() {
	for (usize i = 0; i < current.type; ++i)
		context.push(Object::create());
}

void Engine::v2Cast() {
	Instruction::Casting cast = Makai::Cast::bit<Instruction::Casting>(current.type);
	if (context.globalValueStack.empty())
		return crash(outOfRangeError("Global stack is empty!"));
	uint64 typeID;
	if (cast.dynamic) {
		if (context.globalValueStack.size() < 2)
			return crash(outOfRangeError("Not enough arguments for dynamic cast!"));
		auto const id = context.pop();
		if (!(id && id->isTypeID()))
			return crash(makeErrorHere("Dynamic cast argument is not a type ID!"));
		typeID = id->toValue<TypeID>().id;
	} else {
		advance(true);
		typeID = Makai::Cast::bit<uint64>(current);
	}
	if (auto const t = context.art.types.byID(typeID)) {
		auto const v = context.top();
		if (!v->changeType(t))
			return crash(makeErrorHere("Cannot convert value to requested type!"));
	} else return crash(makeErrorHere("Type does not exist!"));
}

void Engine::v2Select() {
	auto const selCount = current.type;
	if (context.globalValueStack.empty())
		return crash(outOfRangeError("Global stack is empty!"));
	if (!context.top())
		return crash(makeErrorHere("Select value does not exist!"));
	if (!(context.top()->isUnsigned() or context.top()->isBoolean()))
		return crash(makeErrorHere("Expected unsigned integer or boolean value for select!"));
	uint64 const to = context.pop()->toValue<uint64>();
	if (!selCount) return;
	List<uint64> targets;
	targets.resize(selCount);
	for (usize i = 0; i < selCount; ++i) {
		advance(true);
		targets.pushBack(Makai::Cast::bit<uint64>(current));
	}
	if (to >= targets.size())
		jumpBy(targets.back(), false);
	else jumpBy(targets[to], false);
}
