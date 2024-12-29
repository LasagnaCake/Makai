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

	struct ByteCode {
		StringList	data;
		Binary		code;
	};

	struct [[gnu::packed]] FileHeader {
		uint64 const headerSize		= sizeof(FileHeader);
		uint64 version				= SCRIPT_VERSION;
		uint64 minVersion			= SCRIPT_MIN_VERSION;
		uint64 dataSize;
		uint64 flags;
		// Put new things BELOW this line
	};

	constexpr BinaryData<> toBytes(ByteCode const& code) {
		BinaryData<>	out;
		FileHeader		fh;
		out.resize(fh.headerSize);
		for (String const& s: code.data) {
			out.expand(s.nullTerminated() ?  : s.size() + 1, '\0');
			MX::memcpy(out.end() - s.size(), s.data(), s.size());
		}
		out.expand(code.code.size() * 2, '\0');
		MX::memcpy(((uint16*)&*out.end()) - code.code.size(), code.code.data(), code.code.size());
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
		FileHeader fh;
		MX::memmove((void*)&fh.headerSize, (void*)data.data(), sizeof(uint64));
		if (data.size() < fh.headerSize + fh.dataSize) 
			throw Error::FailedAction(
				"Failed at loading script binary!",
				"File size is too small!",
				CTL_CPP_PRETTY_SOURCE
			);
		MX::memmove((void*)&fh, (void*)data.data(), fh.headerSize);
		usize i = fh.headerSize;
		String buf;
		auto c			= data.begin() + fh.headerSize;
		auto const end	= c + fh.dataSize;
		auto const eof	= data.end();
		while (c != end && c != eof) {
			if (*c == '\0') {
				out.data.pushBack(buf);
				buf.clear();
			} else buf.pushBack(*c);
			++c;
		}
		if (!buf.empty()) out.data.pushBack(buf);
		if ((eof - c) % 2 != 0)
			throw Error::FailedAction(
				"Failed at loading script binary!",
				"Invalid bytecode section!",
				CTL_CPP_PRETTY_SOURCE
			);
		out.code.resize(eof - c);
		MX::memmove((void*)out.code.data(), (void*)&*c, eof - c);
		return out;
	}
}

#endif