#include "engine.hpp"

using Makai::Anima::V2::Runtime::Engine;

namespace Core = Makai::Anima::V2::Core;

using Makai::Data::Value;

bool Engine::process() {
	if (isFinished) return false;
	do {
		advance();
	} while (current.name == Core::Instruction::Name::AV2_IN_NO_OP && current.type);
	switch (current.name) {
		using enum Core::Instruction::Name;
		case AV2_IN_HALT:			v2Halt();		break;
		case AV2_IN_STACK_POP:		v2StackPop();	break;
		case AV2_IN_STACK_PUSH:		v2StackPush();	break;
		case AV2_IN_STACK_CLEAR:	v2StackClear();	break;
		case AV2_IN_COPY:			v2Copy();		break;
		case AV2_IN_RETURN: 		v2Return();		break;
		case AV2_IN_GLOBAL:			v2Global();		break;
		case AV2_IN_CALL:			v2Call();		break;
		case AV2_IN_NO_OP: break;
		default: crash(invalidInstructionEror());
	}
	return true;
}

void Engine::crash(Engine::Error const& e) {
	err = e;
	v2Halt();
}

void Engine::v2Halt() {
	isFinished = true;
}

void Engine::advance() {
	++context.pointers.instruction;
	current = program.code[context.pointers.instruction];
	crash(invalidSourceEror(""));
}

void Engine::v2Invoke() {
	Core::Instruction::Invocation invocation = bitcast<Core::Instruction::Invocation>(current.type);
	switch (invocation.location) {
		default: 
	}
}

void Engine::jumpTo(usize const point, bool returnable) {
	if (returnable)
		context.pointerStack.pushBack(context.pointers);
	context.pointers.instruction = point;
}

void Engine::returnBack() {
	context.pointers = context.pointerStack.popBack();
}

void Engine::callBuiltIn(BuiltInFunction const func) {
	if (context.valueStack.empty())
		context.valueStack.pushBack(Value::undefined());
	else switch (func) {
		case BuiltInFunction::AV2_EBIF_ADD: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isNumber() && b.isNumber()) context.valueStack.pushBack(a.get<double>() + b.get<double>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_SUB: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isNumber() && b.isNumber()) context.valueStack.pushBack(a.get<double>() - b.get<double>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_MUL: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isNumber() && b.isNumber()) context.valueStack.pushBack(a.get<double>() * b.get<double>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_DIV: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isNumber() && b.isNumber()) context.valueStack.pushBack(a.get<double>() / b.get<double>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_MOD: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isNumber() && b.isNumber()) {
				if (a.isUnsigned() && b.isUnsigned())
					context.valueStack.pushBack(a.get<usize>() % b.get<usize>());
				else if (a.isSigned() && b.isSigned())
					context.valueStack.pushBack(a.get<ssize>() % b.get<ssize>());
				else context.valueStack.pushBack(Math::mod(a.get<double>(), b.get<double>()));
			}
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_LAND: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			context.valueStack.pushBack(a.get<bool>() && b.get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_LOR: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			context.valueStack.pushBack(a.get<bool>() || b.get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_LNOT: {
			if (context.valueStack.size() < 1) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack();
			context.valueStack.pushBack(!a.get<bool>());
		} break;
		case BuiltInFunction::AV2_EBIF_NEG: {
			if (context.valueStack.size() < 1) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack();
			if (a.isNumber()) context.valueStack.pushBack(-a.get<float>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_AND: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isInteger() && b.isInteger()) context.valueStack.pushBack(a.get<usize>() & b.get<usize>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_OR: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isInteger() && b.isInteger()) context.valueStack.pushBack(a.get<usize>() | b.get<usize>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_XOR: {
			if (context.valueStack.size() < 2) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			if (a.isInteger() && b.isInteger()) context.valueStack.pushBack(a.get<usize>() ^ b.get<usize>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_NOT: {
			if (context.valueStack.size() < 1) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack();
			if (a.isInteger()) context.valueStack.pushBack(~a.get<usize>());
			else context.valueStack.pushBack(Value::undefined());
		} break;
		case BuiltInFunction::AV2_EBIF_COMP: {
			if (context.valueStack.size() < 1) context.valueStack.pushBack(Value::undefined());
			Value a = context.valueStack.popBack(), b = context.valueStack.popBack();
			Value::OrderType order = Value::Order::EQUAL;
			if (a.type() == b.type())				order = a <=> b;
			else if (a.isNumber() &&b.isNumber())	order = a.get<float>() <=> b.get<float>();
			else {
				context.valueStack.pushBack(Value::undefined());
				break;
			}
			if (order == Value::Order::EQUAL)			context.valueStack.pushBack(0);
			else if (order == Value::Order::GREATER)	context.valueStack.pushBack(1);
			else if (order == Value::Order::LESS)		context.valueStack.pushBack(-1);
			else context.valueStack.pushBack(Value::undefined());
		} break;
	}
}