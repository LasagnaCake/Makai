#include "engine.hpp"

using Makai::Anima::V2::Runtime::Engine;

using namespace Makai::Anima::V2;

namespace Runtime = Makai::Anima::V2::Runtime;

using namespace Core;
using namespace Runtime;

static void printValueState(Object::Storage const& value) {
	MAKAILIB_DEBUG_BLOCK_FULL {
		MAKAILIB_DEBUG_FULL("> Value? ", value ? "YES" : "NO");
		if (value) {
			MAKAILIB_DEBUG_FULL(" (Type? ", value->getOriginalType() ? value->getOriginalType()->cleanName() : "?");
			MAKAILIB_DEBUG_FULL(" as ", value->getType() ? value->getType()->cleanName() : "?", ")");
			MAKAILIB_DEBUGLN_FULL(" = ", value->serialize().toString());
		}
		else MAKAILIB_DEBUGLN_FULL("");
	}
}

struct StackStateScopePrinter {
	StackStateScopePrinter(Makai::AtomicCell<Runtime::Environment> env): env(env) {
		MAKAILIB_DEBUG_BLOCK_FULL {
			MAKAILIB_DEBUGLN_FULL("<before>");
			print();
			MAKAILIB_DEBUGLN_FULL("</before>");
		}
	}

	~StackStateScopePrinter() {
		MAKAILIB_DEBUG_BLOCK_FULL {
			MAKAILIB_DEBUGLN_FULL("<after>");
			print();
			MAKAILIB_DEBUGLN_FULL("</after>");
		}
	}

	void print() {
		MAKAILIB_DEBUG_BLOCK_FULL {
			MAKAILIB_DEBUGLN_FULL("Stack Size: ", env->globalValueStack.size());
			MAKAILIB_DEBUGLN_FULL("Stack State {");
			if (env->globalValueStack.size())
				for (auto& v: env->globalValueStack)
					printValueState(v);
			MAKAILIB_DEBUGLN_FULL("}");
		}
	}

	Makai::AtomicCell<Runtime::Environment> env;
};

bool Engine::DefaultLibraryLoader::loadLibrary(Context& context, String const& path) {
	MAKAILIB_DEBUGLN_FULL("Searching for library [", path, "]...");
	StringList paths = {
		path,
		OS::FS::concatenate(OS::FS::currentDirectory(), path),
		OS::FS::concatenate(OS::FS::sourceLocation(), "av2", path),
	};
	if (auto const artHome = getenv("ART_HOME"))
		paths.pushBack(OS::FS::concatenate(artHome, "av2", path));
	for (auto const& p: paths) {
		MAKAILIB_DEBUGLN_FULL("  > On  path '", p, "'");
		if (OS::FS::exists(p))
			return context.openLibrary(p);
	}
	MAKAILIB_DEBUGLN_FULL("Not found");
	return false;
}

bool Engine::yieldCycle() {
	bool revertContext = false;
	if (env->scopeStack.empty())
		env->scopeStack.pushBack(AtomicCell<Environment::Scope>::create());
	if (env->scopeStack.back()->prevMode != env->scopeStack.back()->mode)
		revertContext = true;
	if (!running()) return false;
	do {
		MAKAILIB_DEBUGLN_FULL("Next instruction, please!");
		advance();
	} while (running() && current.name == Instruction::Name::AV2_IN_NO_OP && current.type);
	if (!running()) return false;
	MAKAILIB_DEBUGLN_FULL("Instruction: ", Instruction::asString(current.name));
	switch (current.name) {
		using enum Instruction::Name;
		case AV2_IN_HALT:			v2Halt();			break;
		case AV2_IN_STACK_BLIT:		v2StackBlit();		break;
		case AV2_IN_STACK_POP:		v2StackPop();		break;
		case AV2_IN_STACK_PUSH:		v2StackPush();		break;
		case AV2_IN_STACK_CLEAR:	v2StackClear();		break;
		case AV2_IN_STACK_FLUSH:	v2StackFlush();		break;
		case AV2_IN_STACK_GROW:		v2StackGrow();		break;
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
		case AV2_IN_CHECK:			v2Check();			break;
		case AV2_IN_OP:				v2Op();				break;
		case AV2_IN_COMPARE:		v2Compare();		break;
		case AV2_IN_MODE:			v2SetContext();		break;
		case AV2_IN_JUMP:			v2Jump();			break;
		case AV2_IN_YIELD:			v2Yield();			break;
		case AV2_IN_CLEAR:			v2Clear();			break;
		case AV2_IN_SELECT:			v2Select();			break;
		case AV2_IN_CREATE:			v2Create();			break;
		case AV2_IN_INITIALIZE:		v2Initialize();		break;
		case AV2_IN_BREAKPOINT:		v2Breakpoint();		break;
		case AV2_IN_SPREAD:			v2Spread();			break;
		case AV2_IN_TEST:			v2Test();			break;
		case AV2_IN_NO_OP: break;
//		default: crash(invalidInstructionError());
	}
	if (revertContext) env->scopeStack.back()->mode = env->scopeStack.back()->prevMode;
	return running();
}

bool Engine::process() {
	if (delay) --delay;
	else while (Engine::yieldCycle() && !delay) {}
	MAKAILIB_DEBUGLN_FULL("Done processing for now!");
	return running();
}

void Engine::crash(Engine::Error const& e) {
	err = e;
	terminate();
}

void Engine::v2Yield() {
	Instruction::Waiting wait = bitcast<Instruction::Waiting>(current.type);
	if (wait.dynamic)
		env->pop().perform([&] (auto const& obj) {delay = obj.template to<uint64>();});
	else if (wait.once)
		delay = 1;
	else {
		advance(true);
		delay = Makai::Cast::bit<uint64>(current);
	}
}

template <class T>
static int8 doFastCompareEX(T& lhs, T& rhs, Comparator const comp) {
	switch (comp) {
		case Comparator::AV2_OP_THREEWAY: {
			if (lhs < rhs) return -1;
			else return lhs > rhs;
		}
		case Comparator::AV2_OP_EQUALS:			return lhs == rhs;
		case Comparator::AV2_OP_NOT_EQUALS:		return lhs != rhs;
		case Comparator::AV2_OP_GREATER_THAN:	return lhs > rhs;
		case Comparator::AV2_OP_GREATER_EQUALS:	return lhs >= rhs;
		case Comparator::AV2_OP_LESS_THAN:		return lhs < rhs;
		case Comparator::AV2_OP_LESS_EQUALS:	return lhs <= rhs;
	}
	return 0;
}

template <class T>
inline static int8 doFastCompareStringOrBytes(Object::Storage const& lhs, Object::Storage const& rhs, Comparator const comp) {
	T lx = lhs->to<T>();
	T rx = rhs->to<T>();
	return doFastCompareEX(lx, rx, comp);
}

template <class T>
inline static int8 doFastCompareCoherent(Object::Storage const& lhs, Object::Storage const& rhs, Comparator const comp) {
	T& lx = *(T*)(lhs->data());
	T& rx = *(T*)(rhs->data());
	return doFastCompareEX(lx, rx, comp);
}


template <class T>
static int8 doImmediateCompareCoherent(Object::Storage const& lhs, uint64& rhs, Comparator const comp) {
	T& lx = *(T*)(lhs->data());
	T& rx = *(T*)(&rhs);
	return doFastCompareEX(lx, rx, comp);
}

static int8 doFastCompare(Object::Storage const& lx, Object::Storage const& rx, BasicType const type, Comparator const comp) {
	if (type == BasicType::AV2_BT_STRING) [[unlikely]]	return doFastCompareStringOrBytes<Makai::UTF8String>(lx, rx, comp);
	if (type == BasicType::AV2_BT_BYTES) [[unlikely]]	return doFastCompareStringOrBytes<Makai::Bytes<>>(lx, rx, comp);
	switch (type) {
		default:
		case Runtime::BasicType::AV2_BT_VOID:
		case Runtime::BasicType::AV2_BT_NULL:		return 0;
		case Runtime::BasicType::AV2_BT_BOOL:		return doFastCompareCoherent<bool>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_INT8:		return doFastCompareCoherent<int8>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_UINT8:		return doFastCompareCoherent<uint8>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_INT16:		return doFastCompareCoherent<int16>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_UINT16:	return doFastCompareCoherent<uint16>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_INT32:		return doFastCompareCoherent<int32>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_CHAR:
		case Runtime::BasicType::AV2_BT_UINT32:	return doFastCompareCoherent<uint32>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_INT64:		return doFastCompareCoherent<int64>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_UINT64:	return doFastCompareCoherent<uint64>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_REAL32:	return doFastCompareCoherent<float32>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_REAL64:	return doFastCompareCoherent<float64>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_REAL128:	return doFastCompareCoherent<float128>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_VECTOR:	return doFastCompareCoherent<Makai::Vector4>(lx, rx, comp);	break;
		case Runtime::BasicType::AV2_BT_TYPEID:
		case Runtime::BasicType::AV2_BT_JUMPID:
		case Runtime::BasicType::AV2_BT_CALLID:	return doFastCompareCoherent<Identifier>(lx, rx, comp);		break;
	}
}

static int8 doImmediateCompare(Object::Storage const& lx, uint64& rx, BasicType const type, Comparator const comp) {
	switch (type) {
		default:
		case Runtime::BasicType::AV2_BT_VOID:
		case Runtime::BasicType::AV2_BT_NULL:		return 0;
		case Runtime::BasicType::AV2_BT_BOOL:		return doImmediateCompareCoherent<bool>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_INT8:		return doImmediateCompareCoherent<int8>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_UINT8:		return doImmediateCompareCoherent<uint8>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_INT16:		return doImmediateCompareCoherent<int16>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_UINT16:	return doImmediateCompareCoherent<uint16>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_INT32:		return doImmediateCompareCoherent<int32>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_CHAR:
		case Runtime::BasicType::AV2_BT_UINT32:	return doImmediateCompareCoherent<uint32>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_INT64:		return doImmediateCompareCoherent<int64>(lx, rx, comp);			break;
		case Runtime::BasicType::AV2_BT_UINT64:	return doImmediateCompareCoherent<uint64>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_REAL32:	return doImmediateCompareCoherent<float32>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_REAL64:	return doImmediateCompareCoherent<float64>(lx, rx, comp);		break;
		case Runtime::BasicType::AV2_BT_TYPEID:
		case Runtime::BasicType::AV2_BT_JUMPID:
		case Runtime::BasicType::AV2_BT_CALLID:	return doImmediateCompareCoherent<Identifier>(lx, rx, comp);	break;
	}
	return 0;
}

void Engine::v2Halt() {
	Instruction::Stop stop = bitcast<Instruction::Stop>(current.type);
	switch (stop.mode) {
		case Runtime::Instruction::Stop::Mode::AV2_ISM_ERROR: {
			auto const v = env->pop();
			if (err) return;
			auto const _ = v.sync();
			return crash(makeErrorHere("PROGRAM_ERROR: " + v->to<String>()));
		};
		default: terminate();
	};
}

Engine::Error Engine::makeErrorHere(String const& message) {
	if (CTL::CPP::Debug::hasDebugger())
		throw Makai::Error::FailedAction(
			"ANIMA_ERROR: " + message,
			CTL::CPP::SourceFile("BYTECODE:", env->pointers.instruction, ":ANP")
		);
	return {
		message,
		env->pointers.instruction,
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

Engine::Error Engine::invalidTypeError(String const& description) {
	return makeErrorHere("INVALID TYPE: " + description);
}

Engine::Error Engine::missingArgumentsError() {
	return makeErrorHere("Missing arguments for built-in function!");
}

void Engine::advance(bool isRequired) {
	++env->pointers.instruction;
	if (!isRequired) MAKAILIB_DEBUGLN_FULL("Fetching instruction [", env->pointers.instruction, "] ...");
	if (env->pointers.instruction < program.code.size())
		current = program.code[env->pointers.instruction];
	else if (isRequired)
		return crash(endOfProgramError());
	else return terminate();
}

void Engine::v2Return() {
	returnBack();
}

void Engine::v2Copy() {
	Instruction::Transfer tf = bitcast<Instruction::Transfer>(current.type);
	MAKAILIB_DEBUGLN_FULL("Reading source...");
	auto const from	= consumeValue(tf.from);
	if (err) return;
	MAKAILIB_DEBUGLN_FULL("Writing to destination...");
	auto& to		= accessValue(tf.to);
	if (err) return;
	to = from;
}

Engine::ForeignCallResult Engine::callForeign(
	Runtime::Instruction::Invocation const& invocation,
	usize const lib,
	usize const fn,
	usize const argc,
	Runtime::BasicType const ret
) {
	if (env->globalValueStack.size() < argc)
		return ForeignCallError::AV2_RE_FCE_MISSING_ARGS;
	if (program.ani->shared.ffi.size() < lib)
		return ForeignCallError::AV2_RE_FCE_MISSING_LIBRARY_NAME;
	auto const lname = program.ani->shared.ffi[lib];
	if (program.strings.size() < fn)
		return ForeignCallError::AV2_RE_FCE_MISSING_FUNCTION_NAME;
	auto const fname = program.strings[fn];
	if (!context().ffilibs.contains(lname))
		return ForeignCallError::AV2_RE_FCE_LIBRARY_NOT_LOADED;
	auto const fcall = reinterpret_cast<ref<void(...)>>(context().ffilibs[lname]->variadicFunctionRef<void>(fname));
	if (!fcall)
		return ForeignCallError::AV2_RE_FCE_FUNCTION_DOES_NOT_EXIST;
	usize argi = argc;
	List<Cell<String>> tempStore;
	Makai::ABI::Dyn::Arguments callArgs;
	while (argi--) {
		auto const arg = env->pop();
		if (auto const argt = arg->getOriginalType()) {
			if (argt->basic)
				switch (*argt->basic) {
					case Runtime::BasicType::AV2_BT_BOOL:		callArgs.add(arg->to<bool>());					break;
					case Runtime::BasicType::AV2_BT_INT8:		callArgs.add(arg->to<int8>());					break;
					case Runtime::BasicType::AV2_BT_UINT8:		callArgs.add(arg->to<uint8>());				break;
					case Runtime::BasicType::AV2_BT_INT16:		callArgs.add(arg->to<int16>());				break;
					case Runtime::BasicType::AV2_BT_UINT16:	callArgs.add(arg->to<uint16>());				break;
					case Runtime::BasicType::AV2_BT_INT32:		callArgs.add(arg->to<int32>());				break;
					case Runtime::BasicType::AV2_BT_CHAR:
					case Runtime::BasicType::AV2_BT_UINT32:	callArgs.add(arg->to<uint32>());				break;
					case Runtime::BasicType::AV2_BT_INT64:		callArgs.add(arg->to<int64>());				break;
					case Runtime::BasicType::AV2_BT_UINT64:	callArgs.add(arg->to<uint64>());				break;
					case Runtime::BasicType::AV2_BT_REAL32:	callArgs.add(arg->to<float32>());				break;
					case Runtime::BasicType::AV2_BT_REAL64:	callArgs.add(arg->to<float64>());				break;
					case Runtime::BasicType::AV2_BT_REAL128:	callArgs.add(arg->to<float128>());				break;
					case Runtime::BasicType::AV2_BT_VECTOR:	callArgs.add((ref<Makai::Vector4>)arg->data());		break;
					case Runtime::BasicType::AV2_BT_MATRIX:	callArgs.add((ref<Makai::Matrix4x4>)arg->data());	break;
					case Runtime::BasicType::AV2_BT_ANY:		callArgs.add(arg->data());							break;
					case Runtime::BasicType::AV2_BT_STRING: {
						tempStore.pushBack(Cell<String>::create(arg->to<String>()));
						callArgs.add(tempStore.back()->cstr());
					} break;
					case Runtime::BasicType::AV2_BT_BYTES: callArgs.add((ref<byte>)arg->data()); break;
					default: return ForeignCallError::AV2_RE_FCE_INVALID_ARG_TYPE;
				}
			else if (argt->flags.isValueType)
				callArgs.add(arg->data());
			else return ForeignCallError::AV2_RE_FCE_INVALID_ARG_TYPE;
		}
	}
	switch (ret) {
		case Runtime::BasicType::AV2_BT_BOOL:		return env->newValue(Makai::ABI::Dyn::invoke<bool>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_INT8:		return env->newValue(Makai::ABI::Dyn::invoke<int8>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_UINT8:		return env->newValue(Makai::ABI::Dyn::invoke<uint8>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_INT16:		return env->newValue(Makai::ABI::Dyn::invoke<int16>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_UINT16:	return env->newValue(Makai::ABI::Dyn::invoke<uint16>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_INT32:		return env->newValue(Makai::ABI::Dyn::invoke<int32>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_CHAR:
		case Runtime::BasicType::AV2_BT_UINT32:	return env->newValue(Makai::ABI::Dyn::invoke<uint32>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_INT64:		return env->newValue(Makai::ABI::Dyn::invoke<int64>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_UINT64:	return env->newValue(Makai::ABI::Dyn::invoke<uint64>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_REAL32:	return env->newValue(Makai::ABI::Dyn::invoke<float32>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_REAL64:	return env->newValue(Makai::ABI::Dyn::invoke<float64>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_REAL128:	return env->newValue(Makai::ABI::Dyn::invoke<float128>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_VECTOR:	return env->newValue(*Makai::ABI::Dyn::invoke<ref<Makai::Vector4>>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_MATRIX:	return env->newValue(*Makai::ABI::Dyn::invoke<ref<Makai::Matrix4x4>>(fcall, callArgs));
		case Runtime::BasicType::AV2_BT_STRING:	return env->newValue(String(Makai::ABI::Dyn::invoke<cstring>(fcall, callArgs)));
		case Runtime::BasicType::AV2_BT_VOID:		Makai::ABI::Dyn::invoke<void>(fcall, callArgs); return Object::Storage();
		default: return ForeignCallError::AV2_RE_FCE_INVALID_RETURN_TYPE;
	}
}

void Engine::v2Call() {
	// Get invocation
	Instruction::Invocation invocation = bitcast<Instruction::Invocation>(current.type);
	uint64 loc = 0;
	if (invocation.dynamic) {
		if (env->globalValueStack.empty())
			return crash(invalidSourceError("Global stack is empty!"));
		env->pop().perform([&] (auto const& obj) {loc = obj.template to<CallID>().id;});
	} else {
		advance(true);
		loc = Makai::Cast::bit<uint64>(current);
	}
	MAKAILIB_DEBUGLN_FULL("Handling call...");
	if (invocation.ffi) {
		advance(true);
		auto const lib = Makai::Cast::bit<uint64>(current);
		advance(true);
		auto const argc	= Makai::Cast::bit<uint32>(current.name);
		auto const ret	= Makai::Cast::as<Runtime::BasicType>(current.type >> 24);
		callForeign(invocation, lib, loc, argc, ret)
		.then(
			[&] (auto const& v) {
				if (invocation.noResult) return;
				if (!v || v->isEmptyType())
					crash(invalidFunctionError("Expected return type, but function is void"));
				else env->globalValueStack.pushBack(v);
			}
		).onError(
			[&] (auto const& e) {
				if (invocation.optional) {
					if (!invocation.noResult)
						env->globalValueStack.pushBack(nullptr);
					return;
				}
				Makai::String err = "FOREIGN FUNCTION: ";
				switch (e) {
					using enum ForeignCallError;
					case AV2_RE_FCE_MISSING_ARGS:				err += "Not enough args for function";			break;
					case AV2_RE_FCE_MISSING_FUNCTION_NAME:		err += "Missing function name";					break;
					case AV2_RE_FCE_MISSING_LIBRARY_NAME:		err += "Missing library name";					break;
					case AV2_RE_FCE_LIBRARY_NOT_LOADED:			err += "Library failed to load";				break;
					case AV2_RE_FCE_FUNCTION_DOES_NOT_EXIST:	err += "Function does not exist in library";	break;
					case AV2_RE_FCE_INVALID_ARG_TYPE:			err += "Invalid argument type for function";	break;
					case AV2_RE_FCE_INVALID_RETURN_TYPE:		err += "Invalid return type for function";		break;
				}
				crash(invalidFunctionError(err));
			}
		);
	} else if (invocation.external) {
		decltype(env->globalValueStack) args;
		if (auto argc = context().argumentCountOf(loc)) {
			StackStateScopePrinter s3p{env};
			auto& gvs = env->globalValueStack;
			args = gvs.sliced(-argc, -1);
			gvs.eraseRange(- argc - 1, - 1);
		}
		context()
			.callNative(loc, args)
			.then(
				[&] (auto const& v) {
					if (invocation.noResult) return;
					if (!v || v->isEmptyType())
						crash(invalidFunctionError("Expected return type, but function is void"));
					else env->globalValueStack.pushBack(v);
				}
			).onError(
				[&] (auto const& e) {
					if (invocation.optional) {
						if (!invocation.noResult)
							env->globalValueStack.pushBack(nullptr);
						return;
					}
					Makai::String err = "EXTERNAL FUNCTION: ";
					switch (e) {
						using enum Runtime::Context::Error;
						case AV2_CCE_MISSING_METHOD:		err += "Function does not exist";								break;
						case AV2_CCE_MISSING_INVOKER:		err += "Invoker for function is mysteriously gone";				break;
						case AV2_CCE_MISSING_ARGS:			err += "Not enough args for function";							break;
						case AV2_CCE_MISSING_ART_TYPE:		err += "Return type does not exist in the current ART env";	break;
						case AV2_CCE_HOW_DID_YOU_GET_HERE:	err += "Somehow, execution reached an unreachable point";		break;
					}
					crash(invalidFunctionError(err));
				}
			)
		;
	} else if (invocation.async)
		env->push(forkTo(loc));
	else jumpByTableIndex(loc, true /*returnable*/);
}

Runtime::Context::Storage Engine::consumeValue(ValueLocation const from) {
	if (from.desc.source != ValueLocation::Source::AV2_VLS_BOOL) advance(true);
	auto const store = getValueFromLocation(from, bitcast<uint64>(current));
	printValueState(store);
	if (store && !store->getOriginalType())
		crash(makeErrorHere("Missing type information!"));
	return store;
}

Runtime::Context::Storage Engine::validate(Runtime::Context::Storage const& value, bool const passByCopy) {
	if (!value) return Object::Storage();
	auto const _ = value->getPromiseLock();
	if (auto const type = value->getType())
		if (passByCopy && !type->flags.isCopyable)
			return value->shallowClone();
	return passByCopy ? value->clone() : value;
}

Runtime::Context::Storage Engine::getValueFromLocation(ValueLocation const loc, uint64 const id) {
	bool byCopy	= loc.forObject.transfer == ValueLocation::ForObject::Transfer::AV2_VL_OT_COPY;
	bool byMove	= loc.forObject.transfer == ValueLocation::ForObject::Transfer::AV2_VL_OT_MOVE;
	MAKAILIB_DEBUGLN_FULL("Full Request: ", Makai::Cast::as<uint64>(loc.value));
	MAKAILIB_DEBUGLN_FULL("Data Location: ", Makai::Cast::as<uint64>(loc.desc.source));
	if (loc.forObject.source > ValueLocation::Source::AV2_VLS_STRING) {
		MAKAILIB_DEBUGLN_FULL("By copy? ", byCopy);
		MAKAILIB_DEBUGLN_FULL("By ref? ", loc.forObject.transfer == ValueLocation::ForObject::Transfer::AV2_VL_OT_REF);
		MAKAILIB_DEBUGLN_FULL("By move? ", byMove);
	}
	switch (loc.desc.source) {
		case ValueLocation::Source::AV2_VLS_NULL: {
			return nullptr;
		} break;
		case ValueLocation::Source::AV2_VLS_BOOL: {
			return context().newValue(loc.forBool.flag);
		} break;
		case ValueLocation::Source::AV2_VLS_INT: {
			MAKAILIB_DEBUGLN_FULL("Creating integer...");
			if (loc.forInt.isUnsigned) {
				MAKAILIB_DEBUGLN_FULL(":: UNSIGNED");
				switch (loc.forInt.size) {
					case ValueLocation::ForInteger::Size::AV2_VL_IS_16_BIT: return context().newValue(Makai::Cast::as<uint16>(id)); break;
					case ValueLocation::ForInteger::Size::AV2_VL_IS_32_BIT: return context().newValue(Makai::Cast::as<uint32>(id)); break;
					case ValueLocation::ForInteger::Size::AV2_VL_IS_64_BIT: return context().newValue(Makai::Cast::as<uint64>(id)); break;
					default: return context().newValue(Makai::Cast::as<uint8>(id)); break;
				}
			} else {
				MAKAILIB_DEBUGLN_FULL(":: SIGNED");
				switch (loc.forInt.size) {
					case ValueLocation::ForInteger::Size::AV2_VL_IS_16_BIT: return context().newValue(Makai::Cast::bit<int16, uint16>(id)); break;
					case ValueLocation::ForInteger::Size::AV2_VL_IS_32_BIT: return context().newValue(Makai::Cast::bit<int32, uint32>(id)); break;
					case ValueLocation::ForInteger::Size::AV2_VL_IS_64_BIT: return context().newValue(Makai::Cast::bit<int64, uint64>(id)); break;
					default: return context().newValue(Makai::Cast::bit<int8, uint8>(id)); break;
				}
			}
		} break;
		case ValueLocation::Source::AV2_VLS_REAL: {
			switch (loc.forReal.size) {
				case ValueLocation::ForReal::Size::AV2_VL_RS_64_BIT: return context().newValue(Makai::Cast::bit<float64>(id)); break;
				case ValueLocation::ForReal::Size::AV2_VL_RS_128_BIT: return context().newValue(Makai::Cast::as<float128>(Makai::Cast::bit<float64>(id))); break;
				default: return context().newValue(Makai::Cast::bit<float32, uint32>(id)); break;
			}
		} break;
		case ValueLocation::Source::AV2_VLS_STRING: {
			MAKAILIB_DEBUGLN_FULL("Creating string '", program.strings[id], "'");
			auto const v = context().newValue(program.strings[id]);
			MAKAILIB_DEBUGLN_FULL("Created '", v->to<String>(), "'");
			return v;
		} break;
		case ValueLocation::Source::AV2_VLS_STACK: {
			if (env->globalValueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return nullptr;
			}
			auto& loc = env->globalValueStack[id  % env->globalValueStack.size()];
			auto const v = loc;
			printValueState(v);
			if (byMove) loc = nullptr;
			return validate(v, byCopy);
		}
		case ValueLocation::Source::AV2_VLS_STACK_OFFSET: {
			if (env->globalValueStack.empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return nullptr;
			}
			auto& loc = env->globalValueStack[-Cast::as<ssize>(id % env->globalValueStack.size() + 1)];
			auto const v = loc;
			printValueState(v);
			if (byMove) loc = nullptr;
			return validate(v, byCopy);
		}
		case ValueLocation::Source::AV2_VLS_GLOBAL:	return global(id);
		case ValueLocation::Source::AV2_VLS_LOCAL: {
			if (env->locals().empty()) {
				if (inStrictMode())
					crash(invalidLocationError(loc));
				return nullptr;
			}
			auto& loc = env->locals()[id  % env->locals().size()];
			auto const v = loc;
			MAKAILIB_DEBUGLN_FULL("Local: ", id % env->locals().size());
			printValueState(v);
			if (byMove) loc = nullptr;
			return validate(v, byCopy);
		}
		case ValueLocation::Source::AV2_VLS_EXTERNAL: {
			if (program.ani)
				return external(program.ani->out[id], !byCopy);
			else if (inStrictMode())
				crash(invalidLocationError(loc));
			return nullptr;
		}
		default: {
			if (inStrictMode())
				crash(invalidLocationError(loc));
			return nullptr;
		}
	}
	if (inStrictMode())
		crash(invalidLocationError(loc));
	return nullptr;
}

Runtime::Context::Storage& Engine::accessValue(ValueLocation const from) {
	if (from.desc.source != ValueLocation::Source::AV2_VLS_BOOL) advance(true);
	auto& loc = accessLocation(from, bitcast<uint64>(current));
	if (loc && !loc->getOriginalType())
		crash(makeErrorHere("Missing type information!"));
	return loc;
}

Runtime::Context::Storage& Engine::accessLocation(ValueLocation const loc, usize const id) {
	static Context::Storage failsafe;
	switch (loc.desc.source) {
		case ValueLocation::Source::AV2_VLS_STACK: {
			if (env->globalValueStack.empty()) {
				crash(invalidLocationError(loc));
			}
			return env->globalValueStack[id  % env->globalValueStack.size()];
		}
		case ValueLocation::Source::AV2_VLS_STACK_OFFSET: {
			if (env->globalValueStack.empty()) {
				crash(invalidLocationError(loc));
			}
			return env->globalValueStack[-Cast::as<ssize>(id % env->globalValueStack.size() + 1)];
		}
		case ValueLocation::Source::AV2_VLS_LOCAL: {
			if (env->locals().empty()) {
				crash(invalidLocationError(loc));
			}
			return env->locals()[id % env->locals().size()];
		}
		default:
			crash(invalidLocationError(loc));
	}
	crash(invalidLocationError(loc));
	return failsafe;
}

Runtime::Context::Storage& Engine::global(uint64 const id) {
	return env->globals[id];
}

Runtime::Context& Engine::context() const {
	return *art;
}

void Engine::jumpTo(usize const point, bool returnable) {
	if (returnable)
		env->pointerStack.pushBack(env->pointers);
	env->pointers.instruction = point;
}

void Engine::jumpByTableIndex(usize const tableID, bool returnable) {
	MAKAILIB_DEBUGLN_FULL("Doing jump...");
	if (tableID == Makai::Limit::MAX<uint64>)
		return;
	MAKAILIB_DEBUGLN_FULL("Jumping to target...");
	if (tableID < program.jumpTable.size())
		jumpTo(program.jumpTable[tableID], returnable);
	else return crash(invalidJump());
	MAKAILIB_DEBUGLN_FULL("Table index: ", tableID);
	MAKAILIB_DEBUGLN_FULL("We goin to: ", program.jumpTable[tableID]);
	MAKAILIB_DEBUGLN_FULL("The Rabbit has Landed!");
}

void Engine::jumpByMode(Instruction::Leap::Mode const mode, usize const location, bool returnable) {
	switch (mode) {
		case JumpMode::AV2_JM_TABLE_INDEX:
			return jumpByTableIndex(location, returnable);
		case JumpMode::AV2_JM_ABSOLUTE: {
			if (location < program.code.size())
				return jumpTo(location, returnable);
			else crash(invalidJump());
		}
		case JumpMode::AV2_JM_RELATIVE: {
			auto const to = env->pointers.instruction + bitcast<int64>(location);
			if (to < program.code.size())
				return jumpTo(to, returnable);
			else crash(invalidJump());
		}
	}
}

bool Engine::hasExposedCall(String const& signal) {
	return program.ani && program.ani->in.contains(signal);
}

Makai::Nullable<usize> Engine::invokeExposedCall(String const& signal, Runtime::Context::Arguments const& args) {
	if (hasExposedCall(signal)) {
		if (args.size())
			env->globalValueStack.appendBack(args.reversed());
		jumpTo(program.jumpTable[program.ani->in[signal]], true);
		return env->pointerStack.size()-1;
	} else return null;
}

bool Engine::inFunction(usize const id) {
	return id > env->pointerStack.size();
}

void Engine::returnBack() {
	env->pointers = env->pointerStack.popBack();
	while (
		env->scopeStack.size()
	&&	env->scopeStack.back()->pointerFrame > env->pointerStack.size()
	) env->scopeStack.popBack();
}

Runtime::Context::Storage Engine::external(String const& name, bool const byRef) {
	return nullptr;
}

template <Makai::Type::Integer T>
using Int = Makai::Meta::If<Makai::Type::Unsigned<T>, uint64, int64>;

template <class T>
static bool bopIt(Object::Storage const& out, Object::Storage const& lhs, Object::Storage const& rhs, Operator const op, Runtime::Environment& env) {
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_ADD:	*out = *env.newValue<T>(lhs->to<T>() || rhs->to<T>());	return true;
			case AV2_BOP_SUB:	*out = *env.newValue<T>(lhs->to<T>() != rhs->to<T>());	return true;
			case AV2_BOP_MUL:	*out = *env.newValue<T>(lhs->to<T>() && rhs->to<T>());	return true;
			default: break;
		}
	} else if constexpr (Makai::Type::Number<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_ADD:	*out = *env.newValue<T>(lhs->to<T>() + rhs->to<T>()); return true;
			case AV2_BOP_SUB:	*out = *env.newValue<T>(lhs->to<T>() - rhs->to<T>()); return true;
			case AV2_BOP_MUL:	*out = *env.newValue<T>(lhs->to<T>() * rhs->to<T>()); return true;
			case AV2_BOP_DIV:	*out = *env.newValue<T>(lhs->to<T>() / rhs->to<T>()); return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Different<T, Makai::Matrix4x4>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_DIV:	*out = *env.newValue<T>(lhs->to<T>() / rhs->to<T>()); return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Number<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_REM: {
				if constexpr (Makai::Type::Real<T>)
					*out = *env.newValue<T>((T)Makai::Math::mod<T>(lhs->to<T>(), rhs->to<T>()));
				else *out = *env.newValue<T>(lhs->to<T>() % rhs->to<T>());
			} return true;
			case AV2_BOP_POW:	*out = *env.newValue<T>(Makai::Math::pow<double>(lhs->to<T>(), rhs->to<T>()));		return true;
			case AV2_BOP_ATAN2:	*out = *env.newValue<T>((T)Makai::Math::atan2<double>(lhs->to<T>(), rhs->to<T>()));	return true;
			case AV2_BOP_LOGX:	*out = *env.newValue<T>(Makai::Math::logn<double>(lhs->to<T>(), rhs->to<T>()));		return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Integer<T>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_BIT_AND:	*out = *env.newValue<T>(lhs->to<Int<T>>() & rhs->to<Int<T>>());	return true;
			case AV2_BOP_BIT_OR:	*out = *env.newValue<T>(lhs->to<Int<T>>() | rhs->to<Int<T>>());	return true;
			case AV2_BOP_BIT_XOR:	*out = *env.newValue<T>(lhs->to<Int<T>>() ^ rhs->to<Int<T>>());	return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_BOP_LOGIC_AND:	*out = *env.newValue<T>(lhs->to<T>() && rhs->to<T>());	return true;
			case AV2_BOP_LOGIC_OR:	*out = *env.newValue<T>(lhs->to<T>() || rhs->to<T>());	return true;
			case AV2_BOP_LOGIC_XOR:	*out = *env.newValue<T>(lhs->to<T>() != rhs->to<T>());	return true;
			default: break;
		}
	}
	return false;
}

static bool stringBopIt(Object::Storage const& out, Object::Storage const& lhs, Object::Storage const& rhs, Operator const op, Runtime::Environment& env) {
	using S = Makai::UTF8String;
	switch (op) {
		using enum Operator;
		case AV2_BOP_MUL: {
			S result;
			S const str = lhs->to<S>();
			auto const times = lhs->to<uint64>();
			for (usize i = 0; i < times; ++i) result += str;
			*out = *env.newValue<S>(result);
		} return true;
		case AV2_BOP_ADD:		*out = *env.newValue<S>(lhs->to<S>() + rhs->to<S>()); 								return true;
		case AV2_BOP_REM: {
			if (auto const m = Makai::Regex::findFirst(lhs->to<S>(), rhs->to<S>())) {
				*out = *env.newValue<S>(m.value().match);
			} else *out = *env.newValue<S>("");
			return true;
		}
		case AV2_BOP_BIT_OR:	*out = *env.newValue(Makai::Regex::contains(lhs->to<S>(), rhs->to<S>()));				return true;
		case AV2_BOP_BIT_AND:	*out = *env.newValue(Makai::Regex::matches(lhs->to<S>(), rhs->to<S>()));				return true;
		case AV2_BOP_SUB:		*out = *env.newValue<S>(Makai::Regex::replace(lhs->to<S>(), rhs->to<S>(), ""));		return true;
		case AV2_BOP_BIT_XOR:	*out = *env.newValue<S>(Makai::Regex::replace(lhs->to<S>(), rhs->to<S>(), ""));		return true;
		default: return false;
	}
	return false;
}

static bool arrayBopIt(Object::Storage const& lhs, Object::Storage const& rhs, Operator const op, Runtime::Environment& env) {
	if(lhs->isArray() && rhs->getType()->canBecome(lhs->getType()->base)) {
		switch (op) {
			using enum Operator;
			// TODO: These operators
			case AV2_BOP_MUL:		return true;
			case AV2_BOP_ADD:		return true;
			case AV2_BOP_REM:		return true;
			case AV2_BOP_DIV:		return true;
			case AV2_BOP_SUB:		return true;
			case AV2_BOP_PUSH: {
				if (!lhs->push(rhs))
					return false;
			} return true;
			default: return false;
		}
	}
	return false;
}

static bool arrayUopIt(Object::Storage const& val, Operator const op, Runtime::Environment& env) {
	if(val->isArray()) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_LENGTH: {
				env.pop();
				env.push(val->count());
				return true;
			} return true;
			case AV2_UOP_POP: {
				if (auto const v = val->pop()) {
					env.pop();
					env.push(v.value());
					return true;
				} else return false;
			}
			case AV2_UOP_INVERSE: {
				return val->reverse();
			}
			default: return false;
		}
	}
	return false;
}

void Engine::doBinaryOperation(Operator const op) {
	if (env->globalValueStack.size() < 2)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto rhs	= env->pop();
	if (!rhs) {
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Right-Side Operand does not exist!"));
		return;
	}
	auto const _r = rhs.sync();
	auto lhs	= env->top();
	if (!lhs) {
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Left-Side Operand does not exist!"));
		return;
	}
	auto const _l = lhs.sync();
	auto out	= lhs;
	MAKAILIB_DEBUGLN_FULL("LHS: ", lhs->getType() ? lhs->getType()->cleanName() : "##ERR");
	MAKAILIB_DEBUGLN_FULL("RHS: ", rhs->getType() ? lhs->getType()->cleanName() : "##ERR");
	if (err) return;
	bool success = false;
	if (lhs->isBoolean() && rhs->isBoolean())				success = bopIt<bool>(out, lhs, rhs, op, *env);
	else if (lhs->isUnsigned() && rhs->isUnsigned())		success = bopIt<uint64>(out, lhs, rhs, op, *env);
	else if (lhs->isSigned() && rhs->isSigned())			success = bopIt<int64>(out, lhs, rhs, op, *env);
	else if (lhs->isNumber() && rhs->isNumber())			success = bopIt<double>(out, lhs, rhs, op, *env);
	else if (lhs->isVectorable() && rhs->isVectorable())	success = bopIt<Vector4>(out, lhs, rhs, op, *env);
	else if (lhs->isAlgebraic() && rhs->isAlgebraic())		success = bopIt<Matrix4x4>(out, lhs, rhs, op, *env);
	else if (lhs->isString() && rhs->isString())			success = stringBopIt(out, lhs, rhs, op, *env);
	else if (lhs->isArray())								success = arrayBopIt(lhs, rhs, op, *env);
	if (!success) {
		if (inStrictMode())
			return crash(invalidOperationError("Invalid/Unsupported operator for the given values!"));
		env->pop();
		env->pushEmpty();
	}
	MAKAILIB_DEBUGLN_FULL("Result: ", out->getType() ? out->getType()->cleanName() : "##ERR");
}

template <class T>
static bool uopIt(Object::Storage const& out, Object::Storage const& lhs, Operator const op, Runtime::Environment& env) {
	switch (op) {
		using enum Operator;
		case AV2_UOP_NEGATE:	*out = *env.newValue<T>(-lhs->to<T>());						return true;
		case AV2_UOP_INVERSE:	*out = *env.newValue<T>(Makai::Cast::as<T>(1) / lhs->to<T>());	return true;
		default: break;
	}
	if constexpr (Makai::Type::Ex::Math::Vector::Vector<T>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_LENGTH:	*out = *env.newValue<T>(lhs->to<T>().length());			return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Number<T>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_INCREMENT:	*out = *env.newValue<T>(lhs->to<T>()+1);							return true;
			case AV2_UOP_DECREMENT:	*out = *env.newValue<T>(lhs->to<T>()-1);							return true;
			case AV2_UOP_SIN:		*out = *env.newValue<T>(Makai::Math::sin(lhs->to<double>()));		return true;
			case AV2_UOP_COS:		*out = *env.newValue<T>(Makai::Math::cos(lhs->to<double>()));		return true;
			case AV2_UOP_TAN:		*out = *env.newValue<T>(Makai::Math::tan(lhs->to<double>()));		return true;
			case AV2_UOP_ASIN:		*out = *env.newValue<T>((T)asin(lhs->to<T>()));					return true;
			case AV2_UOP_ACOS:		*out = *env.newValue<T>((T)acos(lhs->to<T>()));					return true;
			case AV2_UOP_ATAN:		*out = *env.newValue<T>((T)atan(lhs->to<T>()));					return true;
			case AV2_UOP_SINH:		*out = *env.newValue<T>((T)sinh(lhs->to<T>()));					return true;
			case AV2_UOP_COSH:		*out = *env.newValue<T>((T)cosh(lhs->to<T>()));					return true;
			case AV2_UOP_TANH:		*out = *env.newValue<T>((T)tanh(lhs->to<T>()));					return true;
			case AV2_UOP_LOG2:		*out = *env.newValue<T>(Makai::Math::log2(lhs->to<double>()));		return true;
			case AV2_UOP_LOG10:		*out = *env.newValue<T>(Makai::Math::log10(lhs->to<double>()));	return true;
			case AV2_UOP_LN:		*out = *env.newValue<T>(Makai::Math::log(lhs->to<double>()));		return true;
			case AV2_UOP_SQRT:		*out = *env.newValue<T>(Makai::Math::sqrt(lhs->to<double>()));		return true;
			case AV2_UOP_LENGTH:	*out = *env.newValue<T>(Makai::Math::abs(lhs->to<T>()));			return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Integer<T>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_BIT_NOT:	*out = *env.newValue<T>(~lhs->to<Int<T>>());	return true;
			default: break;
		}
	}
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (op) {
			using enum Operator;
			case AV2_UOP_LOGIC_NOT:	*out = *env.newValue<T>(!lhs->to<T>());		return true;
			default: break;
		}
	}
	return false;
}

void Engine::doUnaryOperation(Operator const op) {
	if (env->globalValueStack.size() < 1)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto lhs	= env->top();
	if (!lhs) {
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Operand does not exist!"));
		return;
	}
	auto const _ = lhs.sync();
	auto out	= lhs;
	if (err) return;
	bool success = false;
	if (lhs->isBoolean())			success = uopIt<bool>(out, lhs, op, *env);
	else if (lhs->isUnsigned())		success = uopIt<uint64>(out, lhs, op, *env);
	else if (lhs->isSigned())		success = uopIt<int64>(out, lhs, op, *env);
	else if (lhs->isNumber())		success = uopIt<double>(out, lhs, op, *env);
	else if (lhs->isAlgebraic())	success = uopIt<Vector4>(out, lhs, op, *env);
	else if (lhs->isArray())		success = arrayUopIt(lhs, op, *env);
	if (!success) {
		if (inStrictMode())
			return crash(invalidOperationError("Invalid/Unsupported operator for the given values!"));
		env->pop();
		env->pushEmpty();
	}
}

template <class T>
static void fbop(T& lhs, T& rhs, Operator const op) {
	if constexpr (Makai::Type::Equal<T, bool>) switch (op) {
		case Operator::AV2_BOP_LOGIC_OR:
		case Operator::AV2_BOP_ADD: lhs = lhs or rhs; return;
		case Operator::AV2_BOP_LOGIC_XOR:
		case Operator::AV2_BOP_SUB: lhs = lhs != rhs; return;
		case Operator::AV2_BOP_LOGIC_AND:
		case Operator::AV2_BOP_MUL: lhs = lhs and rhs; return;
		default: return;
	} else {
		switch (op) {
			case Operator::AV2_BOP_ADD: lhs += rhs; return;
			case Operator::AV2_BOP_SUB: lhs -= rhs; return;
			case Operator::AV2_BOP_MUL: lhs *= rhs; return;
			case Operator::AV2_BOP_DIV: lhs /= rhs; return;
			default: return;
		}
		if constexpr (Makai::Type::Number<T>) switch (op) {
			using TCalc = Makai::Meta::If<Makai::Type::Real<T>, T, double>;
			case Operator::AV2_BOP_REM: {
				if constexpr (Makai::Type::Real<T>)
					lhs = Makai::Math::mod(lhs, rhs);
				else lhs %= rhs;
			} return;
			case Operator::AV2_BOP_POW: lhs = Makai::Math::pow<TCalc>(lhs, rhs); return;
			case Operator::AV2_BOP_ATAN2: lhs = Makai::Math::atan2<TCalc>(lhs, rhs); return;
			case Operator::AV2_BOP_LOGX: lhs = Makai::Math::logn<TCalc>(lhs, rhs); return;
			default: return;
		}
		if constexpr (Makai::Type::Integer<T>) switch (op) {
			case Operator::AV2_BOP_BIT_AND: lhs &= rhs; return;
			case Operator::AV2_BOP_BIT_OR: lhs |= rhs; return;
			case Operator::AV2_BOP_BIT_XOR: lhs ^= rhs; return;
			default: return;
		}
		if constexpr (Makai::Type::Different<T, Makai::Matrix4x4>) switch (op) {
			case Operator::AV2_BOP_DIV: lhs /= rhs;
			default: return;
		}
	}
}

template <class T>
static void fbopu(pointer const lhs, pointer const rhs, Operator const op) {
	return fbop(*(T*)lhs, *(T*)rhs, op);
}

template <class T>
static void fuop(T& val, Operator const op) {
	if constexpr (Makai::Type::Different<T, bool>) {
		if constexpr (Makai::Type::Different<T, Makai::Matrix4x4>) switch (op) {
			using enum Operator;
			case AV2_UOP_NEGATE:	val = -val; return;
			case AV2_UOP_INVERSE:	val = Makai::Cast::as<T>(1) / val; return;
			default: break;
		} else switch (op) {
			using enum Operator;
			case AV2_UOP_NEGATE:	val = -val; return;
			case AV2_UOP_INVERSE:	val.invert(); return;
			default: break;
		}
		if constexpr (Makai::Type::Number<T>) {
			switch (op) {
				using enum Operator;
				using TCalc = Makai::Meta::If<Makai::Type::Real<T>, T, floatmax>;
				case AV2_UOP_INCREMENT:	++val;									return;
				case AV2_UOP_DECREMENT:	--val;									return;
				case AV2_UOP_SIN:		val = Makai::Math::sin<TCalc>(val);		return;
				case AV2_UOP_COS:		val = Makai::Math::cos<TCalc>(val);		return;
				case AV2_UOP_TAN:		val = Makai::Math::tan<TCalc>(val);		return;
				case AV2_UOP_ASIN:		val = asin(val);						return;
				case AV2_UOP_ACOS:		val = acos(val);						return;
				case AV2_UOP_ATAN:		val = Makai::Math::atan<TCalc>(val);	return;
				case AV2_UOP_SINH:		val = sinh(val);						return;
				case AV2_UOP_COSH:		val = cosh(val);						return;
				case AV2_UOP_TANH:		val = tanh(val);						return;
				case AV2_UOP_LOG2:		val = Makai::Math::log2<TCalc>(val);	return;
				case AV2_UOP_LOG10:		val = Makai::Math::log10<TCalc>(val);	return;
				case AV2_UOP_LN:		val = Makai::Math::log<TCalc>(val);		return;
				case AV2_UOP_SQRT:		val = Makai::Math::sqrt<TCalc>(val);	return;
				case AV2_UOP_LENGTH:	val = Makai::Math::abs(val);			return;
				default: break;
			}
		}
		if constexpr (Makai::Type::Equal<T, Makai::Vector4>) {
			switch (op) {
				using enum Operator;
				case AV2_UOP_INCREMENT:	val += 1;				return;
				case AV2_UOP_DECREMENT:	val -= 1;				return;
				case AV2_UOP_LENGTH:	val = val.absolute();	return;
				default: break;
			}
		}
		if constexpr (Makai::Type::Integer<T>) {
			switch (op) {
				using enum Operator;
				case AV2_UOP_BIT_NOT:	val = ~val;
				default: break;
			}
		}
	} else {
		switch (op) {
			using enum Operator;
			case AV2_UOP_LOGIC_NOT:	val = !val;
			default: break;
		}
	}
}

template <class T>
static void fuopu(pointer const val, Operator const op) {
	return fuop(*(T*)val, op);
}

void Engine::fastBinaryOperation(Operator const op, BasicType const type) {
	if (env->globalValueStack.size() < 2)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto rhs	= env->pop();
	auto const _r = rhs.sync();
	if (!rhs) [[unlikely]] {
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Right-Side Operand does not exist!"));
		return;
	}
	auto lhs	= env->top();
	auto const _l = lhs.sync();
	if (!lhs) [[unlikely]] {
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Left-Side Operand does not exist!"));
		return;
	}
	if (lhs->getType() != rhs->getType()) [[unlikely]] {
		auto const lname = lhs->getType()->cleanName();
		auto const rname = rhs->getType()->cleanName();
		String const tview = " (" + lname + " != " + rname + ")";
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Value types do not match!" + tview));
		return;
	}
	MAKAILIB_DEBUGLN_FULL("Left-Side  := ", lhs->serialize().toFLOWString());
	MAKAILIB_DEBUGLN_FULL("Right-Side := ", rhs->serialize().toFLOWString());
	switch (type) {
		case Runtime::BasicType::AV2_BT_VOID:
		case Runtime::BasicType::AV2_BT_NULL:		break;
		case Runtime::BasicType::AV2_BT_BOOL:		fbopu<bool>(lhs->data(), rhs->data(), op);		break;
		case Runtime::BasicType::AV2_BT_INT8:		fbopu<int8>(lhs->data(), rhs->data(), op);		break;
		case Runtime::BasicType::AV2_BT_UINT8:		fbopu<uint8>(lhs->data(), rhs->data(), op);		break;
		case Runtime::BasicType::AV2_BT_INT16:		fbopu<int16>(lhs->data(), rhs->data(), op);		break;
		case Runtime::BasicType::AV2_BT_UINT16:	fbopu<uint16>(lhs->data(), rhs->data(), op);	break;
		case Runtime::BasicType::AV2_BT_INT32:		fbopu<int32>(lhs->data(), rhs->data(), op);		break;
		case Runtime::BasicType::AV2_BT_CHAR:
		case Runtime::BasicType::AV2_BT_UINT32:	fbopu<uint32>(lhs->data(), rhs->data(), op);	break;
		case Runtime::BasicType::AV2_BT_INT64:		fbopu<int64>(lhs->data(), rhs->data(), op);		break;
		case Runtime::BasicType::AV2_BT_UINT64:	fbopu<uint64>(lhs->data(), rhs->data(), op);	break;
		case Runtime::BasicType::AV2_BT_REAL32:	fbopu<float32>(lhs->data(), rhs->data(), op);	break;
		case Runtime::BasicType::AV2_BT_REAL64:	fbopu<float64>(lhs->data(), rhs->data(), op);	break;
		case Runtime::BasicType::AV2_BT_REAL128:	fbopu<float128>(lhs->data(), rhs->data(), op);	break;
		case Runtime::BasicType::AV2_BT_VECTOR:	fbopu<Vector4>(lhs->data(), rhs->data(), op);	break;
		default: {
			if (inStrictMode()) [[likely]]
				return crash(invalidOperationError("Invalid/Unsupported fast operator for the given values!"));
		}
	}
}

void Engine::immediateBinaryOperation(Operator const op, BasicType const type, pointer const rhs) {
	if (env->globalValueStack.size() < 1)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto lhs	= env->top();
	auto const _l = lhs.sync();
	if (!lhs) [[unlikely]] {
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Left-Side Operand does not exist!"));
		return;
	}
	switch (type) {
		case Runtime::BasicType::AV2_BT_VOID:
		case Runtime::BasicType::AV2_BT_NULL:		break;
		case Runtime::BasicType::AV2_BT_BOOL:		fbopu<bool>(lhs->data(), rhs, op);		break;
		case Runtime::BasicType::AV2_BT_INT8:		fbopu<int8>(lhs->data(), rhs, op);		break;
		case Runtime::BasicType::AV2_BT_UINT8:		fbopu<uint8>(lhs->data(), rhs, op);		break;
		case Runtime::BasicType::AV2_BT_INT16:		fbopu<int16>(lhs->data(), rhs, op);		break;
		case Runtime::BasicType::AV2_BT_UINT16:	fbopu<uint16>(lhs->data(), rhs, op);	break;
		case Runtime::BasicType::AV2_BT_INT32:		fbopu<int32>(lhs->data(), rhs, op);		break;
		case Runtime::BasicType::AV2_BT_CHAR:
		case Runtime::BasicType::AV2_BT_UINT32:	fbopu<uint32>(lhs->data(), rhs, op);	break;
		case Runtime::BasicType::AV2_BT_INT64:		fbopu<int64>(lhs->data(), rhs, op);		break;
		case Runtime::BasicType::AV2_BT_UINT64:	fbopu<uint64>(lhs->data(), rhs, op);	break;
		case Runtime::BasicType::AV2_BT_REAL32:	fbopu<float32>(lhs->data(), rhs, op);	break;
		case Runtime::BasicType::AV2_BT_REAL64:	fbopu<float64>(lhs->data(), rhs, op);	break;
		case Runtime::BasicType::AV2_BT_REAL128:	fbopu<float128>(lhs->data(), rhs, op);	break;
		case Runtime::BasicType::AV2_BT_VECTOR:	fbopu<Vector4>(lhs->data(), rhs, op);	break;
		default: {
			if (inStrictMode()) [[likely]]
				return crash(invalidOperationError("Invalid/Unsupported fast operator for the given values!"));
		}
	}
}

void Engine::fastUnaryOperation(Operator const op, BasicType const type) {
	if (env->globalValueStack.size() < 1)
		return crash(invalidSourceError("Missing values to operate on!"));
	auto val	= env->top();
	if (!val) {
		if (!inStrictMode()) [[unlikely]] {env->pop(); env->pushEmpty();}
		else [[likely]] crash(invalidOperationError("Operand does not exist!"));
		return;
	}
	auto const _ = val.sync();
	switch (type) {
		case Runtime::BasicType::AV2_BT_VOID:
		case Runtime::BasicType::AV2_BT_NULL:		break;
		case Runtime::BasicType::AV2_BT_BOOL:		fuopu<bool>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_INT8:		fuopu<int8>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_UINT8:		fuopu<uint8>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_INT16:		fuopu<int16>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_UINT16:	fuopu<uint16>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_INT32:		fuopu<int32>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_CHAR:
		case Runtime::BasicType::AV2_BT_UINT32:	fuopu<uint32>(val->data(), op);		break;
		case Runtime::BasicType::AV2_BT_INT64:		fuopu<int64>(val->data(), op);		break;
		case Runtime::BasicType::AV2_BT_UINT64:	fuopu<uint64>(val->data(), op);		break;
		case Runtime::BasicType::AV2_BT_REAL32:	fuopu<float32>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_REAL64:	fuopu<float64>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_REAL128:	fuopu<float128>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_VECTOR:	fuopu<Vector4>(val->data(), op);	break;
		case Runtime::BasicType::AV2_BT_MATRIX:	fuopu<Matrix4x4>(val->data(), op);	break;
		default: {
			if (inStrictMode())
				return crash(invalidOperationError("Invalid/Unsupported fast operator for the given values!"));
		}
	}
}

void Engine::shortCircuitOperation(Operator const op, usize count) {
	auto lhs = env->top();
	auto const _l = lhs.sync();
	switch (op) {
		case Operator::AV2_BOP_LOGIC_AND: if (!lhs->to<bool>()) {
			env->globalValueStack.eraseRange(-(count+1), -1);
			*env->top() = *env->newValue(false);
			return;
		}
		case Operator::AV2_BOP_LOGIC_OR: if (lhs->to<bool>()) {
			env->globalValueStack.eraseRange(-(count+1), -1);
			*env->top() = *env->newValue(true);
			return;
		}
		default: break;
	}
	doBinaryOperation(op);
	while (--count) [[unlikely]] {
		auto lhs = env->top();
		auto const _r = lhs.sync();
		switch (op) {
			case Operator::AV2_BOP_LOGIC_AND: if (!lhs->to<bool>()) {
				env->globalValueStack.eraseRange(-(count+1), -1);
				*env->top() = *env->newValue(false);
				return;
			}
			case Operator::AV2_BOP_LOGIC_OR: if (lhs->to<bool>()) {
				env->globalValueStack.eraseRange(-(count+1), -1);
				*env->top() = *env->newValue(true);
				return;
			}
			default: break;
		}
		doBinaryOperation(op);
	}
}

void Engine::fastShortCircuitOperation(Operator const op, BasicType const type, usize count) {
	auto lhs = env->top();
	auto const _l = lhs.sync();
	switch (op) {
		case Operator::AV2_BOP_LOGIC_AND: if (!*(bool*)lhs->data()) {
			env->globalValueStack.eraseRange(-(count+1), -1);
			*env->top() = *env->newValue(false);
			return;
		}
		case Operator::AV2_BOP_LOGIC_OR: if (*(bool*)lhs->data()) {
			env->globalValueStack.eraseRange(-(count+1), -1);
			*env->top() = *env->newValue(true);
			return;
		}
		default: break;
	}
	fastBinaryOperation(op, type);
	while (--count) [[unlikely]] {
		auto lhs = env->top();
		auto const _r = lhs.sync();
		switch (op) {
			case Operator::AV2_BOP_LOGIC_AND: if (!*(bool*)lhs->data()) {
				env->globalValueStack.eraseRange(-(count+1), -1);
				*env->top() = *env->newValue(false);
				return;
			}
			case Operator::AV2_BOP_LOGIC_OR: if (*(bool*)lhs->data()) {
				env->globalValueStack.eraseRange(-(count+1), -1);
				*env->top() = *env->newValue(true);
				return;
			}
			default: break;
		}
		fastBinaryOperation(op, type);
	}
}

void Engine::v2Op() {
	StackStateScopePrinter s3p{env};
	Instruction::Operation op = Cast::bit<Instruction::Operation>(current.type);
	if (op.immediate) advance(true);
	if (
		op.sameType && (
			op.assume == BasicType::AV2_BT_STRING
		||	op.assume == BasicType::AV2_BT_BYTES
		)
	) [[unlikely]] {
		if (op.immediate)
			env->push(program.strings[Makai::bitcast<uint64>(current) % program.strings.size()]);
		if (op.op < Operator::AV2_BOP_START)
			return doUnaryOperation(op.op);
		else return doBinaryOperation(op.op);
	}
	if (op.op == Operator::AV2_UOP_LOGIC_NOT && !(op.count % 2))
		return;
	if (
		op.op == Operator::AV2_BOP_LOGIC_AND
	or	op.op == Operator::AV2_BOP_LOGIC_OR
	) {
		if (op.sameType) [[likely]]
			return fastShortCircuitOperation(op.op, op.assume, op.count);
		else [[unlikely]] return shortCircuitOperation(op.op, op.count);
	}
	if (op.op < Operator::AV2_BOP_START) {
		if (op.sameType) [[likely]]
			return fastUnaryOperation(op.op, op.assume);
		else [[unlikely]] return doUnaryOperation(op.op);
	} else {
		if (op.immediate) [[likely]]
			return immediateBinaryOperation(op.op, op.assume, &current);
		else if (op.sameType) [[likely]]
			return fastBinaryOperation(op.op, op.assume);
		else [[unlikely]] return doBinaryOperation(op.op);
	}
	while (--op.count) [[unlikely]] {
		if (op.op < Operator::AV2_BOP_START) {
			if (op.sameType) [[likely]]
				return fastUnaryOperation(op.op, op.assume);
			else return doUnaryOperation(op.op);
		} else {
			if (op.immediate) [[likely]]
				return immediateBinaryOperation(op.op, op.assume, &current);
			else if (op.sameType) [[likely]]
				return fastBinaryOperation(op.op, op.assume);
			else [[unlikely]] return doBinaryOperation(op.op);
		}
	}
}

void Engine::v2Compare() {
	StackStateScopePrinter s3p{env};
	Instruction::Comparison comp = bitcast<Instruction::Comparison>(current.type);
	if (env->globalValueStack.size() < 2)
		return crash(invalidSourceError("Missing values to compare!"));
	if (comp.immediate)
		advance(true);
	if (
		comp.sameType && (
			comp.assume == BasicType::AV2_BT_STRING
		||	comp.assume == BasicType::AV2_BT_BYTES
		)
	) [[unlikely]] {
		if (comp.immediate)
			env->push(program.strings[Makai::bitcast<uint64>(current) % program.strings.size()]);
		comp.sameType = false;
	} else if (comp.immediate) {
		auto lhs = env->top();
		uint64 val = bitcast<uint64>(current);
		*lhs = *env->newValue(doImmediateCompare(lhs, val, comp.assume, comp.comp));
		return;
	}
	auto rhs	= env->pop();
	auto lhs	= env->top();
	auto const _l = lhs.sync(), _r = rhs.sync();
	if (comp.sameType) {
		*rhs = *env->newValue(doFastCompare(lhs, rhs, comp.assume, comp.comp));
		return;
	}
	Makai::Ordered::OrderType order = Makai::Ordered::Order::EQUAL;
	if (lhs->isBoolean() && rhs->isBoolean())				order = lhs->to<bool>() <=> rhs->to<bool>();
	else if (lhs->isUnsigned() && rhs->isUnsigned())		order = lhs->to<uint64>() <=> rhs->to<uint64>();
	else if (lhs->isInteger() && rhs->isInteger())			order = lhs->to<int64>() <=> rhs->to<int64>();
	else if (lhs->isNumber() && rhs->isNumber())			order = lhs->to<double>() <=> rhs->to<double>();
	else if (lhs->isVectorable() && rhs->isVectorable())	order = lhs->to<Vector4>() <=> rhs->to<Vector4>();
	else if (
		lhs->getCurrentType() == rhs->getCurrentType()
	||	lhs->getCurrentType()->canBecome(rhs->getCurrentType())
	) order = lhs->compareWith(rhs);
	else if (inStrictMode())
		return crash(invalidComparisonError("Types do not match!"));
	else {
		env->pushEmpty();
		return;
	}
	if (order == Makai::StandardOrder::UNORDERED) {
		if (inStrictMode())
			return crash(invalidComparisonError("Failed to compare types!"));
		else {
			env->pushEmpty();
			return;
		}
	}
	MAKAILIB_DEBUGLN_FULL("Left: ", lhs->to<int64>());
	MAKAILIB_DEBUGLN_FULL("Right: ", rhs->to<int64>());
	MAKAILIB_DEBUGLN_FULL("Order = ", Cast::as<int16>(order.order()));
	switch (comp.comp) {
		using enum Core::Comparator;
		case AV2_OP_THREEWAY:
			*env->top() = *env->newValue(Cast::as<int8>(order.order()));
		break;
		using enum Makai::StandardOrder;
		case AV2_OP_EQUALS:			*env->top() = *env->newValue(order == EQUAL);		break;
		case AV2_OP_NOT_EQUALS:		*env->top() = *env->newValue(order != EQUAL);		break;
		case AV2_OP_GREATER_THAN:	*env->top() = *env->newValue(order == GREATER);	break;
		case AV2_OP_GREATER_EQUALS:	*env->top() = *env->newValue(order != LESS);		break;
		case AV2_OP_LESS_THAN:		*env->top() = *env->newValue(order == LESS);		break;
		case AV2_OP_LESS_EQUALS:	*env->top() = *env->newValue(order != GREATER);	break;
	}
	MAKAILIB_DEBUGLN_FULL("Result: ", env->top()->to<int64>());
}

void Engine::terminate() {
	MAKAILIB_DEBUGLN_FULL("Terminating...");
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
	art			= art.create();
	env			= env.create();
	env->art	= art;
	current		= {};
	err			= {};
	delay		= 0;
	engineState = State::AV2_RES_READY;
}

void Engine::load(Module const& prog) {
	reset();
	if (!(
		prog.type == decltype(prog.type)::AV2_CMT_EXE
	or	prog.type == decltype(prog.type)::AV2_CMT_CLI_EXE
	)) return;
	program = prog;
}

Runtime::Storage Engine::forkTo(usize const call) {
	Storage output = output.create();
	subs.pushBack(Thread::from(handleFork, output, program, *env, call));
	return output;
}

bool Engine::handleFork(
	Storage const&		output,
	Core::Module const&	program,
	Environment const&	env,
	usize const			call
) try {
	output->beginPromise();
	Engine sub;
	sub.program = program;
	sub.env = sub.env.create(env);
	sub.art = env.art;
	sub.engineState = State::AV2_RES_RUNNING;
	sub.jumpByTableIndex(call, true /* returnable */);
	auto const fid = sub.env->pointerStack.size()-1;
	while (sub.inFunction(fid)) {
		output->doingPromise();
		if (sub.delay) --sub.delay;
		else sub.yieldCycle();
		output->nextPromise(*sub.env->top());
	}
	output->finalPromise(*sub.env->top());
	return true;
} catch (...) {
	output->failedPromise();
	return false;
}

void Engine::execute() {
	if (running()) return;
	engineState = State::AV2_RES_INITIALIZING;
	load();
}

void Engine::load() {
	if (engineState != State::AV2_RES_INITIALIZING) return;
	if (program.code.empty())
		return crash(makeErrorHere("Module has no code!"));
	Map<uint64, uint64> inheritances;
	Map<uint64, uint64> boundTypes;
	Map<uint64, List<uint64>> fields;
	for (auto const& [type, i]: Range::expand(program.detail.types)) {
		AtomicCell<Runtime::TypeModel> dt = dt.create();
		dt->hash = type.hash;
		if (type.base) inheritances[i] = *type.base;
		if (type.fields.size())
			fields[i] = type.fields;
		dt->basic = type.basic;
		dt->flags = type.flags;
		dt->alignment = type.alignment;
		dt->byteSize = type.byteSize;
		if (type.basic)
			TypeModel::makeBasic(*dt);
		context().types.addElement(dt);
	}
	if (config.allowDynamicLibraries) {
		MAKAILIB_DEBUGLN_FULL("<dynlib-open>");
		if (program.ani && loader)
			for (auto& lib: program.ani->shared.libraries)
				loader->loadLibrary(context(), lib + ".andl");
		MAKAILIB_DEBUGLN_FULL("</dynlib-open>");
		MAKAILIB_DEBUGLN_FULL("<dynlib-load>");
		context().loadLibraries();
		MAKAILIB_DEBUGLN_FULL("</dynlib-load>");
	}
	MAKAILIB_DEBUGLN_FULL("Finishing loading step...");
	onLoad();
	for (auto const& [self, base]: inheritances)
		context().types.values[self]->base = context().types.values[base];
	if (context().types.values.size() < program.detail.types.size())
		return crash(makeErrorHere(toString(
			"Program has missing types [",
			context().types.values.size(),
			" < ",
			program.detail.types.size(),
			"]!"
		)));
	for (auto const& [self, fields]: fields)
		for (auto const& field: fields)
			context().types.values[self]->fields.pushBack(context().types.values[field]);
	List<String> missingTypes;
	for (auto const& [type, index]: Range::expand(context().types.values)) {
		if (!type)
			missingTypes.pushBack(toString(index));
	}
	if (missingTypes.size())
		return crash(makeErrorHere(toString(
			"Program has missing types [ " + missingTypes.join(", ") + " ]!"
		)));
	MAKAILIB_DEBUG_BLOCK_FULL {
		MAKAILIB_DEBUGLN_FULL("Types:");
		MAKAILIB_DEBUG_BLOCK_FULL
			for (auto& type : context().types.values)
				MAKAILIB_DEBUGLN_FULL("> ", type ? toString(type->cleanName()) : toString("NULL"));
	}
	if (program.entry != Limit::MAX<uint64>) jumpByTableIndex(program.entry, false /*not returnable*/);
	else return crash(makeErrorHere("Missing entrypoint!"));
	engineState = State::AV2_RES_RUNNING;
}

void Engine::unload() {
	context().unloadLibraries();
}

void Engine::v2SetContext() {
	auto const ctx = Cast::bit<Instruction::Context>(current.type);
	if (!ctx.immediate)
		env->scope().prevMode	= ctx.mode;
	env->scope().mode			= ctx.mode;
}

void Engine::v2StackPush() {
	StackStateScopePrinter s3p{env};
	auto const inter = Cast::bit<Instruction::StackPush>(current.type);
	auto const value = consumeValue(inter.location);
	MAKAILIB_DEBUGLN_FULL("You sure? ", value.exists());
	if (err) return;
	env->push(value);
}

void Engine::v2StackPop() {
	StackStateScopePrinter s3p{env};
	if (env->globalValueStack.size())
		env->pop();
}

void Engine::v2StackSwap() {
	StackStateScopePrinter s3p{env};
	if (env->globalValueStack.size() >= 2)
		Makai::swap(env->globalValueStack[-1], env->globalValueStack[-2]);
}

void Engine::v2StackClear() {
	StackStateScopePrinter s3p{env};
	if (current.type) {
		auto& stack = env->globalValueStack;
		auto const clear = Cast::bit<Instruction::StackClear>(current.type);
		MAKAILIB_DEBUGLN_FULL("Clearing ", clear.count, " entries...");
		if (stack.size() < clear.count) {
			env->globalValueStack.clear();
			return;
		}
		auto const maxOffset = stack.size() - clear.count;
		auto const offset = clear.offset < maxOffset ? clear.offset : maxOffset;
		if (!offset) {
			if (stack.size() && clear.count == 1)
				env->globalValueStack.popBack();
			else if (clear.count < stack.size())
				env->globalValueStack.eraseRange(-offset, -1);
			else env->globalValueStack.clear();
		} else {
			if (stack.size() && clear.count == 1)
				env->globalValueStack.erase(-(clear.offset+1));
			else if ((clear.count + offset) < stack.size())
				env->globalValueStack.eraseRange(-offset-clear.count, -offset);
			else env->globalValueStack.clear();
		}
	}
}

void Engine::v2StackFlush() {
	StackStateScopePrinter s3p{env};
	env->globalValueStack.clear();
}

void Engine::v2Jump() {
	Instruction::Leap leap = current.getTypeAs<Instruction::Leap>();
	using enum Instruction::Leap::Type;
	uint64 loc = 0;
	if (leap.dyn) {
		if (env->globalValueStack.empty())
			return crash(invalidSourceError("Missing jump target for dynamic jump!"));
		env->pop().perform([&] (auto const& obj) {loc = obj.template to<JumpID>().id;});
	} else {
		advance(true);
		loc = Makai::Cast::bit<uint64>(current);
	}
	bool shouldJump = false;
	if (leap.type == AV2_ILT_UNCONDITIONAL) {
		shouldJump = true;
	} else {
		if (env->globalValueStack.empty())
			return crash(invalidSourceError("Missing condition for conditional jump!"));
		auto const cond = env->pop();
		auto const _c = cond.sync();
		switch (leap.type) {
			case AV2_ILT_IF_TRUTHY:			shouldJump	= cond->to<bool>();				break;
			case AV2_ILT_IF_FALSY:			shouldJump	= !cond->to<bool>();				break;
			case AV2_ILT_IF_ZERO:			shouldJump	= cond->to<double>() == 0;			break;
			case AV2_ILT_IF_NOT_ZERO:		shouldJump	= cond->to<double>() != 0;			break;
			case AV2_ILT_IF_NEGATIVE:		shouldJump	= cond->to<double>() < 0;			break;
			case AV2_ILT_IF_POSITIVE:		shouldJump	= cond->to<double>() > 0;			break;
			case AV2_ILT_IF_NULL_OR_VOID:	shouldJump	= !(cond.exists() && cond->exists());	break;
			case AV2_ILT_IF_EXISTS:			shouldJump	= cond.exists() && cond->exists();		break;
			default: break;
		}
	}
	if (shouldJump != leap.invert) {
		if (leap.async) forkTo(loc);
		else jumpByMode(leap.mode, loc, false /*not returnable*/);
	}
}

Engine::Error Engine::invalidLocationError(ValueLocation const& loc) {
	return makeErrorHere("Invalid data location for instruction ["+ toString(enumcast(loc.desc.source)) + "]!");
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
	if (!(scope < env->scopeStack.size()))
		return crash(outOfRangeError("Requested scope is out-of-range!"));
	auto& src = env->scopeStack[-scope]->localStack;
	auto& dst = env->locals();
	if (!((bind.src + count) < src.size()))
		return crash(outOfRangeError("Requested source start + count is bigger than its stack size!"));
	if (!((bind.dst + count) < dst.size()))
		return crash(outOfRangeError("Requested destination start + count is bigger than its stack size!"));
	for (usize i = 0; i < count; ++i)
		dst[i + bind.dst] = src[i + bind.src];
}

void Engine::v2ScopeBind() {
	StackStateScopePrinter s3p{env};
	Instruction::Binding bind = Makai::Cast::bit<Instruction::Binding>(current.type);
	advance(true);
	auto const count = Makai::Cast::bit<uint64>(current);
	auto& src = env->globalValueStack;
	auto& dst = env->locals();
	MAKAILIB_DEBUGLN_FULL("Count: ", count);
	MAKAILIB_DEBUGLN_FULL("Source Size: ", src.size());
	MAKAILIB_DEBUGLN_FULL("Destination Size: ", dst.size());
	if ((bind.src + count) > src.size())
		return crash(outOfRangeError("Requested global stack range falls outside its size!"));
	if ((bind.dst + count) > dst.size())
		return crash(outOfRangeError("Requested destination range falls outside its size!"));
	MAKAILIB_DEBUGLN_FULL("Binding values...");
	for (usize i = 0; i < count; ++i) {
		auto const si = (src.size() - count - bind.src) + i;
		auto const di = i + bind.dst;
		MAKAILIB_DEBUG_FULL("> [", si, " -> ", di, "]", ": ");
		auto const v = src[si];
		printValueState(v);
		dst[di] = v;
	}
}

void Engine::v2ScopeEnter() {
	auto const count = current.type;
	env->scopeStack.pushBack(
		AtomicCell<Environment::Scope>::create(
			Environment::Scope{
				.mode			= env->scope().mode,
				.prevMode		= env->scope().mode,
				.pointerFrame	= env->pointerStack.size()
			}
		)
	);
	if (count == Cast::as<decltype(count)>(-1))
		env->locals() = env->scopeStack[-2]->localStack;
	else if (count) env->locals().resize(count, nullptr);
}

void Engine::v2ScopeExit() {
	if (env->scopeStack.size())
		env->scopeStack.popBack();
}

void Engine::v2FieldGet() {
	Instruction::Field field = current.getTypeAs<Instruction::Field>();
	uint64 loc = 0;
	if (field.dynamic) {
		if (env->globalValueStack.empty())
			return crash(invalidSourceError("Global stack is empty!"));
		env->pop().perform([&] (auto const& obj) {loc = obj.template to<uint64>();});
	} else {
		advance(true);
		loc = Makai::Cast::bit<uint64>(current);
	}
	if (!env->top())
		return crash(invalidSourceError("Value does not exist!"));
	if (!env->top()->canHaveFields())
		return crash(invalidSourceError("Value is not an array or structure!"));
	auto const src = env->pop();
	MAKAILIB_DEBUGLN_FULL("Source: ", src->getType()->hash);
	Object::Storage v;
	auto const _ = v.sync();
	src->getField(loc)
		.then([&] (auto const& val) {v = val;})
		.onError([&] (auto const& err) {
			switch (err) {
				using enum Object::GetError;
				case AV2_COGE_NO_TYPE:						crash(invalidSourceError("Object does not have a type!")); break;
				case AV2_COGE_TYPE_DOES_NOT_CONTAIN_FIELDS:	crash(invalidSourceError("Type does not contain fields!")); break;
				case AV2_COGE_FIELD_IS_NOT_COPYABLE:		crash(invalidSourceError("Field ["+Makai::toString(loc)+"] is not copyable!")); break;
				case AV2_COGE_FIELD_DOES_NOT_EXIST:			crash(invalidSourceError("Field ["+Makai::toString(loc)+"] does not exist!")); break;
			}
		});
	if  (err) return;
	MAKAILIB_DEBUGLN_FULL("Field[", loc, "] = ", v ? Makai::toString(v->getType()->cleanName()) : "NULL");
	env->push(v);
	printValueState(env->top());
}

void Engine::v2FieldSet() {
	Instruction::Field field = current.getTypeAs<Instruction::Field>();
	auto const v = env->pop();
	uint64 loc = 0;
	if (field.dynamic) {
		if (env->globalValueStack.empty())
			return crash(invalidSourceError("Global stack is empty!"));
		env->pop().perform([&] (auto const& obj) {loc = obj.template to<uint64>();});
	} else {
		advance(true);
		loc = Makai::Cast::bit<uint64>(current);
	}
	auto const dst = env->pop();
	MAKAILIB_DEBUGLN_FULL("Value: ", v->getType()->hash);
	MAKAILIB_DEBUGLN_FULL("Destination: ", dst->getType()->hash);
	MAKAILIB_DEBUGLN_FULL("Field: ", loc);
	if (!dst)
		return crash(invalidSourceError("Value does not exist!"));
	if (!dst->canHaveFields())
		return crash(invalidSourceError("Value is not an array or structure!"));
	MAKAILIB_DEBUGLN_FULL("Field Count: ", dst->count());
	auto const _s = dst.sync();
	auto const result = dst->setField(loc, v);
	if (result != Object::SetError::AV2_COSE_OK) [[unlikely]] {
		switch (result) {
			using enum Object::SetError;
			[[unlikely]] case AV2_COSE_OK: break;
			case AV2_COSE_NO_TYPE:						return crash(invalidSourceError("Object does not have a type!"));
			case AV2_COSE_TYPE_DOES_NOT_CONTAIN_FIELDS:	return crash(invalidSourceError("Type does not contain fields!"));
			case AV2_COSE_FIELD_IS_NOT_COPYABLE:		return crash(invalidSourceError("Field is not copyable!"));
			case AV2_COSE_FIELD_DOES_NOT_EXIST:			return crash(invalidSourceError("Field does not exist!"));
		}
	}
	env->push(v);
}

void Engine::v2Sizeof() {
	auto const val = env->pop();
	if (!val) return crash(invalidSourceError("Value does not exist!"));
	auto const sz = val->count();
	if (sz == Makai::Limit::MAX<usize>)
		return crash(invalidSourceError("Value type does not exist!"));
	if (current.type)
		env->push(sz * val->getOriginalType()->byteSize);
	else env->push(sz);
}

void Engine::v2Typeof() {
	env->push(TypeID{Identifier{.id = env->pop()->getCurrentType()->id}});
}

void Engine::v2Random() {
	Instruction::Randomness rng = Makai::Cast::bit<Instruction::Randomness>(current.type);
	Nullable<uint64> val;
	if (rng.getSeed) val = prng.getSeed();
	if (rng.setSeed) {
		auto const val = env->pop();
		auto const _ = val.sync();
		prng.setSeed(val->to<uint64>());
	}
	if (val) env->push(*val);
	if (!(rng.setSeed || rng.getSeed)) {
		Object::Storage lo, hi;
		if (rng.bounded) {
			hi = env->pop();
			lo = env->pop();
		}
		auto const _1 = lo.sync();
		auto const _2 = hi.sync();
		if (rng.secure) switch (rng.type) {
			using enum Instruction::Randomness::Type;
			case AV2_IRT_INT:	env->push(rng.bounded ? srng.number<int64>(lo->to<int64>(), hi->to<int64>()) : srng.number<int64>());		break;
			case AV2_IRT_UINT:	env->push(rng.bounded ? srng.number<uint64>(lo->to<uint64>(), hi->to<uint64>()) : srng.number<uint64>());	break;
			case AV2_IRT_REAL:	env->push(rng.bounded ? srng.number<double>(lo->to<double>(), hi->to<double>()) : srng.number<double>());	break;
		} else switch (rng.type) {
			using enum Instruction::Randomness::Type;
			case AV2_IRT_INT:	env->push(rng.bounded ? prng.number<int64>(lo->to<int64>(), hi->to<int64>()) : prng.number<int64>());		break;
			case AV2_IRT_UINT:	env->push(rng.bounded ? prng.number<uint64>(lo->to<uint64>(), hi->to<uint64>()) : prng.number<uint64>());	break;
			case AV2_IRT_REAL:	env->push(rng.bounded ? prng.number<double>(lo->to<double>(), hi->to<double>()) : prng.number<double>());	break;
		}
	}
}

void Engine::v2StackBlit() {
	StackStateScopePrinter s3p{env};
	Instruction::Blitting blit = Makai::Cast::bit<Instruction::Blitting>(current.type);
	auto& src = blit.fromGlobal ? env->globalValueStack : env->locals();
	auto& dst = blit.fromGlobal ? env->locals() : env->globalValueStack;
	advance(true);
	auto const count = Makai::Cast::bit<uint64>(current);
	if (!(blit.offset + count < src.size()))
		return crash(outOfRangeError("Requested blit range falls outside source's size!"));
	if (blit.fromGlobal)
		dst.appendBack(src.sliced(-(blit.offset+1 + count), -(blit.offset+1)));
	else dst.appendBack(src.sliced(blit.offset, blit.offset + count - 1));
}

void Engine::initializeObject(Object::Storage const& object) {
	if (env->globalValueStack.size() < object->count())
		return crash(invalidSourceError("Stack is too small to initialize the given value!"));
	if (!object->count()) return;
	auto const content = env->globalValueStack.sliced(-object->count(), -1);
	env->globalValueStack.eraseRange(-object->count(), -1);
	MAKAILIB_DEBUGLN_FULL("Object Type: ", object->getType()->cleanName());
	MAKAILIB_DEBUGLN_FULL("Field Size: ", object->count());
	MAKAILIB_DEBUGLN_FULL("Fields {");
	for (auto const& [e, i]: Makai::Range::expand(content)) {
		if (e) {
			auto const result = object->setField(i, e);
			if (result != Object::SetError::AV2_COSE_OK) [[unlikely]] {
				switch (result) {
					using enum Object::SetError;
					[[unlikely]] case AV2_COSE_OK: break;
					case AV2_COSE_NO_TYPE:						return crash(invalidSourceError("Object does not have a type!"));
					case AV2_COSE_TYPE_DOES_NOT_CONTAIN_FIELDS:	return crash(invalidSourceError("Type does not contain fields!"));
					case AV2_COSE_FIELD_IS_NOT_COPYABLE:		return crash(invalidSourceError("Field is not copyable!"));
					case AV2_COSE_FIELD_DOES_NOT_EXIST:			return crash(invalidSourceError("Field does not exist!"));
				}
			}
		}
		MAKAILIB_DEBUG_FULL("  [", i, "] -"); printValueState(e);
	}
	MAKAILIB_DEBUGLN_FULL("}");
}

void Engine::v2Create() {
	StackStateScopePrinter s3p{env};
	Instruction::Create create = Makai::bitcast<Instruction::Create>(current.type);
	uint64 typeID;
	if (create.dyn) {
		if (env->globalValueStack.empty())
			return crash(invalidSourceError("Missing type for dynamic creation!"));
		env->pop().perform([&] (auto const& obj) {typeID = obj.template to<TypeID>().id;});
	} else {
		advance(true);
		typeID = Makai::Cast::bit<uint64>(current);
	}
	auto const type = context().types.byID(typeID);
	if (!type)
		return crash(invalidTypeError("Type does not exist!"));
	if (type->flags.isEmpty)
		return crash(invalidTypeError("Cannot create empty types!"));
	auto obj = Object::create(type);
	auto const _ = obj.sync();
	if (create.forArray) {
		if (!type->flags.isArray)
			return crash(invalidTypeError("Type is not an array, but create instruction expects it!"));
		uint64 size;
		if (create.dynSize) {
			if (env->globalValueStack.empty())
				return crash(invalidSourceError("Missing size for dynamic creation size!"));
			size = env->pop()->to<uint64>();
		} else {
			advance(true);
			size = Makai::Cast::bit<uint64>(current);
		}
		obj->reserveFields(size);
	} else obj->reserveFields(type->fields.size());
	if (create.andInit)
		initializeObject(obj);
	env->push(obj);
}

void Engine::v2Clear() {
	// TODO: This
}

void Engine::v2StackGrow() {
	// TODO: This
}

void Engine::v2Initialize() {
	StackStateScopePrinter s3p{env};
	if (env->globalValueStack.empty())
		return crash(invalidSourceError("Missing value to initialize!"));
	if (!env->top()) return crash(invalidSourceError("Value does not exist!"));
	if (!env->top()->count()) return;
	auto const val = env->pop();
	initializeObject(val);
	env->push(val);
}

void Engine::v2Breakpoint() {
	onBreakpoint();
}

void Engine::onBreakpoint() {
	Makai::CPP::Debug::breakpoint();
}

void Engine::v2ScopeKeep() {
	if (env->scopeStack.size() < 2) return;
	auto& locals = env->locals();
	auto& parentLocals = env->scopeStack[-2]->localStack;
	if (parentLocals.empty())
		return;
	else if (locals.size() > parentLocals.size())
		for (usize i = 0; i < parentLocals.size(); ++i)
			locals[i] = parentLocals[i];
	else [[likely]] locals = parentLocals;
}

void Engine::v2ScopeDeclare() {
	env->locals().appendBack(Makai::List<Context::Storage>(current.type, nullptr));
}

void Engine::v2Cast() {
	Instruction::Casting cast = Makai::Cast::bit<Instruction::Casting>(current.type);
	if (env->globalValueStack.empty())
		return crash(outOfRangeError("Global stack is empty!"));
	uint64 typeID;
	if (cast.dynamic) {
		if (env->globalValueStack.size() < 2)
			return crash(outOfRangeError("Not enough arguments for dynamic cast!"));
		auto const id = env->pop();
		if (!(id && id->isTypeID()))
			return crash(makeErrorHere("Dynamic cast argument is not a type ID!"));
		typeID = id->to<TypeID>().id;
	} else {
		advance(true);
		typeID = Makai::Cast::bit<uint64>(current);
	}
	auto const copyConvert = [&] (AtomicCell<Runtime::TypeModel> const& type) {
		return env->push(env->pop()->as(type)).top().exists();
	};
	auto const moveConvert = [&] (AtomicCell<Runtime::TypeModel> const& type) {
		return env->top()->changeType(type);
	};
	auto const convert = [&] (AtomicCell<Runtime::TypeModel> const& type) {
		return cast.noCopy ? moveConvert(type) : copyConvert(type);
	};
	if (auto const t = context().types.byID(typeID)) {
		if (!env->top()) return crash(makeErrorHere("Value does not exist!"));
		if (env->top()->isBasic() && t->basic) {
			switch (*t->basic) {
				case BasicType::AV2_BT_BOOL:	env->push(env->newValue(env->pop()->to<bool>()));				break;
				case BasicType::AV2_BT_INT8:	env->push(env->newValue(env->pop()->to<int8>()));				break;
				case BasicType::AV2_BT_UINT8:	env->push(env->newValue(env->pop()->to<uint8>()));			break;
				case BasicType::AV2_BT_INT16:	env->push(env->newValue(env->pop()->to<int16>()));			break;
				case BasicType::AV2_BT_UINT16:	env->push(env->newValue(env->pop()->to<uint16>()));			break;
				case BasicType::AV2_BT_INT32:	env->push(env->newValue(env->pop()->to<int32>()));			break;
				case BasicType::AV2_BT_UINT32:	env->push(env->newValue(env->pop()->to<uint32>()));			break;
				case BasicType::AV2_BT_INT64:	env->push(env->newValue(env->pop()->to<int64>()));			break;
				case BasicType::AV2_BT_UINT64:	env->push(env->newValue(env->pop()->to<uint64>()));			break;
				case BasicType::AV2_BT_REAL32:	env->push(env->newValue(env->pop()->to<float32>()));			break;
				case BasicType::AV2_BT_REAL64:	env->push(env->newValue(env->pop()->to<float64>()));			break;
				case BasicType::AV2_BT_REAL128:	env->push(env->newValue(env->pop()->to<float128>()));			break;
				case BasicType::AV2_BT_VECTOR:	env->push(env->newValue(env->pop()->to<Vector4>()));			break;
				case BasicType::AV2_BT_MATRIX:	env->push(env->newValue(env->pop()->to<Matrix4x4>()));		break;
				case BasicType::AV2_BT_TYPEID:	env->push(env->newValue(env->pop()->to<Runtime::TypeID>()));	break;
				case BasicType::AV2_BT_JUMPID:	env->push(env->newValue(env->pop()->to<Runtime::JumpID>()));	break;
				case BasicType::AV2_BT_CALLID:	env->push(env->newValue(env->pop()->to<Runtime::CallID>()));	break;
				case BasicType::AV2_BT_STRING:	env->push(env->newValue(env->pop()->to<UTF8String>()));		break;
				case BasicType::AV2_BT_BYTES:	env->push(env->newValue(env->pop()->to<Bytes<>>()));			break;
				case BasicType::AV2_BT_ANY: {
					if (!convert(t))
						return crash(outOfRangeError("[any] conversion failed!"));
				} break;
				default: return crash(outOfRangeError("Invalid conversion!"));
			}
		} else if (!convert(t))
			return crash(makeErrorHere("Cannot convert value to requested type!"));
	} else return crash(makeErrorHere("Type does not exist!"));
}

void Engine::v2Check() {
	Instruction::Casting cast = Makai::Cast::bit<Instruction::Casting>(current.type);
	if (env->globalValueStack.empty())
		return crash(outOfRangeError("Global stack is empty!"));
	uint64 typeID;
	if (cast.dynamic) {
		if (env->globalValueStack.size() < 2)
			return crash(outOfRangeError("Not enough arguments for dynamic type checking!"));
		auto const id = env->pop();
		if (!(id && id->isTypeID()))
			return crash(makeErrorHere("Dynamic type check argument is not a type ID!"));
		typeID = id->to<TypeID>().id;
	} else {
		advance(true);
		typeID = Makai::Cast::bit<uint64>(current);
	}
	if (auto const t = context().types.byID(typeID)) {
		env->push(env->pop()->getType() == t);
	} else return crash(makeErrorHere("Type does not exist!"));
}

void Engine::v2Select() {
	auto const select = Makai::Cast::bit<Instruction::Selection>(current.type);
	if (env->globalValueStack.empty())
		return crash(outOfRangeError("Global stack is empty!"));
	if (!env->top())
		return crash(makeErrorHere("Select value does not exist!"));
	if (!(env->top()->isUnsigned() or env->top()->isBoolean()))
		return crash(makeErrorHere("Expected unsigned integer or boolean value for select!"));
	uint64 const to = env->pop()->to<uint64>();
	if (!select.count) return;
	usize at = (to < select.count ? to : select.count);
	if ((env->pointers.instruction + at) >= program.code.size())
		return crash(endOfProgramError());
	uint64 const loc = Makai::Cast::bit<uint64>(program.code[env->pointers.instruction + at]);
	jumpByMode(select.mode, loc, false /*not returnable*/);
}

void Engine::v2Spread() {
	Instruction::Spreading const spread = Makai::Cast::bit<Instruction::Spreading>(current.type);
	advance(true);
	auto const offset = Makai::Cast::bit<uint64>(current);
	advance(true);
	auto const count = Makai::Cast::bit<uint64>(current);
	auto const arr = env->pop();
	if (!(arr && arr->getType() && (arr->getType()->flags.isArray or arr->getType()->flags.isStructure)))
		crash(invalidSourceError("Value is not of a spreadable type!"));
	if (!spread.local)
		env->globalValueStack.expand(count);
	else env->locals().expand(count);
	for (usize i = 0; i < count; ++i) {
		if (
			!arr->getField(i + offset)
			.then([&] (auto const& v) {if (!spread.local) env->push(v); else env->locals().pushBack(v);})
			.onError([&] (auto const& err) {
				switch (err) {
					using enum Object::GetError;
					case AV2_COGE_NO_TYPE:						crash(invalidSourceError("Object does not have a type!")); break;
					case AV2_COGE_TYPE_DOES_NOT_CONTAIN_FIELDS:	crash(invalidSourceError("Type does not contain fields!")); break;
					case AV2_COGE_FIELD_IS_NOT_COPYABLE:		crash(invalidSourceError("Field ["+Makai::toString(i + offset)+"] is not copyable!")); break;
					case AV2_COGE_FIELD_DOES_NOT_EXIST:			crash(invalidSourceError("Field ["+Makai::toString(i + offset)+"] does not exist!")); break;
				}
			})
		) break;
	}
}

void Engine::v2Test() {
}
