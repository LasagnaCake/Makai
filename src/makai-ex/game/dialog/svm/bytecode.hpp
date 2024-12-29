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

	constexpr uint64 SCRIPT_VERSION		= 0;
	constexpr uint64 SCRIPT_MIN_VERSION	= 0;

	constexpr Operation asOperation(uint16 const op) {return static_cast<Operation>(op);}

	using JumpTable = Map<uint64, uint64>;
	using JumpEntry = typename JumpTable::PairType;

	struct ByteCode {
		JumpTable	jumps;
		StringList	data;
		Binary		code;
	};

	struct [[gnu::packed]] FileHeader {
		uint64 const headerSize		= sizeof(FileHeader);
		uint64 version				= SCRIPT_VERSION;
		uint64 minVersion			= SCRIPT_MIN_VERSION;
		uint64 flags;
		uint64 dataStart, dataSize;
		uint64 jumpTableStart, jumpTableSize;
		uint64 byteCodeStart, byteCodeSize;
		// Put new things BELOW this line
	};

	constexpr BinaryData<> toBytes(ByteCode const& code) {
		BinaryData<>	out;
		FileHeader		fh;
		out.resize(fh.headerSize, '\0');
		// Data division
		fh.dataStart = fh.headerSize;
		for (String const& s: code.data) {
			out.expand(s.nullTerminated() ?  : s.size() + 1, '\0');
			MX::memcpy(out.end() - s.size(), s.data(), s.size());
		}
		fh.dataSize = out.size() - fh.dataStart;
		// Jump tables
		fh.jumpTableStart = fh.dataStart + fh.dataSize;
		out.expand(code.jumps.size() * sizeof(JumpEntry), '\0');
		MX::memcpy(((JumpEntry*)(out.data() + fh.jumpTableStart)), code.jumps.data(), code.jumps.size());
		fh.jumpTableSize = code.jumps.size() * sizeof(JumpEntry);
		// Bytecode
		fh.byteCodeStart = fh.jumpTableStart + fh.jumpTableSize;
		out.expand(code.code.size() * sizeof(Operation), '\0');
		MX::memcpy(((uint16*)(out.data() + fh.byteCodeStart)), code.code.data(), code.code.size());
		// Main header
		MX::memcpy(((void*)out.data()), &fh, fh.headerSize);
		return out;
	}

	constexpr ByteCode fromBytes(BinaryData<> const& data) {
		ByteCode out;
		if (data.size() < sizeof(uint64))
			throw Error::FailedAction(
				"Failed at loading script binary!",
				"File size is too small!",
				CTL_CPP_PRETTY_SOURCE
			);
		// Main header
		FileHeader fh;
		MX::memmove((void*)&fh.headerSize, (void*)data.data(), sizeof(uint64));
		if (data.size() < fh.headerSize) 
			throw Error::FailedAction(
				"Failed at loading script binary!",
				"File size is too small!",
				CTL_CPP_PRETTY_SOURCE
			);
		MX::memmove((void*)&fh, (void*)data.data(), fh.headerSize);
		// Check if sizes are OK
		if (
			data.size() < fh.headerSize + fh.dataSize + fh.jumpTableSize + fh.byteCodeSize
		||	data.size() < fh.dataStart
		||	data.size() < fh.jumpTableStart
		||	data.size() < fh.byteCodeStart
		||	data.size() < (fh.jumpTableStart + fh.jumpTableSize)
		||	data.size() < (fh.byteCodeStart + fh.byteCodeSize)
		||	data.size() < (fh.dataStart + fh.dataSize)
		) throw Error::FailedAction(
			"Failed at loading script binary!",
			"File size is too small!",
			CTL_CPP_PRETTY_SOURCE
		);
		// Data division
		usize i = fh.headerSize;
		String buf;
		auto c			= data.data() + fh.headerSize + fh.dataStart;
		auto const end	= c + fh.dataSize;
		auto const eof	= data.end();
		while (c != end && c != eof) {
			if (*c == '\0') {
				out.data.pushBack(buf);
				buf.clear();
			} else buf.pushBack(*c);
			++c;
		}
		if (!buf.empty()) {
			out.data.pushBack(buf);
			buf.clear();
		}
		// Jump tables
		if (fh.jumpTableSize % sizeof(JumpEntry) != 0) 
			throw Error::FailedAction(
				"Failed at loading script binary!",
				"Malformed jump table section!",
				CTL_CPP_PRETTY_SOURCE
			);
		auto jte = (JumpEntry*)c;
		for (usize i = 0; i < (fh.jumpTableSize / sizeof(JumpEntry)); ++i)
			out.jumps[jte[i].front()] = jte[i].back();
		// Bytecode
		if (fh.byteCodeSize % sizeof(Operation) != 0) 
			throw Error::FailedAction(
				"Failed at loading script binary!",
				"Malformed bytecode section!",
				CTL_CPP_PRETTY_SOURCE
			);
		out.code.resize(fh.byteCodeSize / sizeof(Operation), 0);
		MX::memmove((void*)out.code.data(), (void*)c + fh.byteCodeStart, fh.byteCodeSize);
		return out;
	}
}

#endif