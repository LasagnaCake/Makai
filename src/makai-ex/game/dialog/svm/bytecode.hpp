#ifndef MAKAILIB_EX_GAME_DIALOG_SVM_BYTECODE_H
#define MAKAILIB_EX_GAME_DIALOG_SVM_BYTECODE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog::SVM {
	using Binary		= List<uint16>;
	using Operands64	= List<uint64>;
	using Operands32	= List<uint32>;
	using Operands16	= List<uint16>;
	
	enum class Operation: uint16 {
		DSO_NO_OP,
		DSO_HALT,
		DSO_SP,
		DSO_ACTOR,
		DSO_LINE,
		DSO_EMOTION,
		DSO_ACTION,
		DSO_COLOR,
		DSO_WAIT,
		DSO_SYNC,
		DSO_USER_INPUT,
		DSO_SET_GLOBAL,
		DSO_NAMED_OP,
		DSO_JUMP,
	};
	
	constexpr Operation asOperation(uint16 const op) {return static_cast<Operation>(op);}

	struct ByteCode {
		StringList	data;
		Binary		code;
	};
}

#endif