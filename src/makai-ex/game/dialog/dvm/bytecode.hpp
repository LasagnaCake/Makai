#ifndef MAKAILIB_EX_GAME_DIALOG_DVM_BYTECODE_H
#define MAKAILIB_EX_GAME_DIALOG_DVM_BYTECODE_H

#include <makai/makai.hpp>

/// @brief Dialog Virtual Machine.
namespace Makai::Ex::Game::Dialog::DVM {
	/// @brief Underlying code binary representation.
	using Binary		= List<uint16>;
	/// @brief 64-bit operand list.
	using Operands64	= List<uint64>;
	/// @brief 32-bit operand list.
	using Operands32	= List<uint32>;
	/// @brief 16-bit operand list.
	using Operands16	= List<uint16>;
	
	/// @brief Bytecode operation.
	enum class Operation: uint16 {
		/// @brief No-op. If SP is set, sets the internal SP mode.
		DVM_O_NO_OP,
		/// @brief Ends execution of the program.
		DVM_O_HALT,
		/// @brief Active actor. Behaves differently, depending on SP mode.
		DVM_O_ACTOR,
		/// @brief Dialog line. Behaves differently, depending on SP mode.
		DVM_O_LINE,
		/// @brief Actor emote.
		DVM_O_EMOTION,
		/// @brief Actor perform. Behaves differently, depending on SP mode.
		DVM_O_ACTION,
		/// @brief Text color. Behaves differently, depending on SP mode.
		DVM_O_COLOR,
		/// @brief Wait.
		DVM_O_WAIT,
		/// @brief Synchronization. Behaves differently, depending on SP mode.
		DVM_O_SYNC,
		/// @brief User input wait.
		DVM_O_USER_INPUT,
		/// @brief Named operation. Behaves differently, depending on SP mode.
		DVM_O_NAMED_CALL,
		/// @brief Jump.
		DVM_O_JUMP,
	};

	/// @brief Script version.	
	constexpr uint64 DIALOG_VERSION		= 0;
	/// @brief Minimum required version to run script.
	constexpr uint64 DIALOG_MIN_VERSION	= 0;

	/// @brief SP mode mask.
	constexpr uint16 SP_FLAG_MASK	= 0xf << 12;
	/// @brief Operation mask.
	constexpr uint16 OPERATION_MASK	= ~SP_FLAG_MASK;

	/// @brief Converts the given data to an operation.
	/// @param op Data to convert.
	/// @return Operation.
	constexpr Operation	asOperation(uint16 const op)	{return static_cast<Operation>(op & OPERATION_MASK);	}
	
	/// @brief Returns the given mode as the appropriate SP mode.
	/// @param mode SP mode to get. 
	/// @return SP mode.
	constexpr uint16	spFlag(uint16 const mode)		{return (mode & 0xF) << 12;			}
	/// @brief Returns the SP mode of a given operation.
	/// @param op Operation to get SP mode for.
	/// @return SP mode.
	constexpr uint16	getSPFlag(uint16 const op)		{return (op & SP_FLAG_MASK) >> 12;	}
	/// @brief Returns the SP mode of a given operation.
	/// @param op Operation to get SP mode for.
	/// @return SP mode.
	constexpr uint16	getSPFlag(Operation const op)	{return getSPFlag(enumcast(op));	}

	/// @brief Jump positions.
	using JumpTable = Map<uint64, uint64>;
	/// @brief Jump position entry.
	using JumpEntry = typename JumpTable::PairType;

	/// @brief Compiled dialog program.
	struct Dialog {
		/// @brief Jump table.
		JumpTable	jumps;
		/// @brief Dialog data.
		StringList	data;
		/// @brief Dialog bytecode.
		Binary		code;
	};

	/// @brief Dialog program file header.
	struct [[gnu::packed]] FileHeader {
		uint64 const headerSize		= sizeof(FileHeader);
		uint64 version				= DIALOG_VERSION;
		uint64 minVersion			= DIALOG_MIN_VERSION;
		uint64 flags;
		uint64 dataStart, dataSize;
		uint64 jumpTableStart, jumpTableSize;
		uint64 byteCodeStart, byteCodeSize;
		// Put new things BELOW this line
	};

	/// @brief Converts a dialog program to a storeable binary file.
	/// @param code Script to convert.
	/// @return Script as binary.
	constexpr BinaryData<> toBytes(Dialog const& code) {
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
		if (!code.jumps.empty()) {
			out.expand(code.jumps.size() * sizeof(JumpEntry), '\0');
			MX::memcpy(((JumpEntry*)(out.data() + fh.jumpTableStart)), code.jumps.data(), code.jumps.size());
		}
		fh.jumpTableSize = code.jumps.size() * sizeof(JumpEntry);
		// Bytecode
		fh.byteCodeStart = fh.jumpTableStart + fh.jumpTableSize;
		if (!code.code.empty()) {
			out.expand(code.code.size() * sizeof(Operation), '\0');
			MX::memcpy(((uint16*)(out.data() + fh.byteCodeStart)), code.code.data(), code.code.size());
		}
		fh.byteCodeSize = code.code.size() * sizeof(Operation);
		// Main header
		MX::memcpy(((void*)out.data()), &fh, fh.headerSize);
		return out;
	}

	/// @brief Converts a series of bytes to a processable dialog progrma.
	/// @param data Bytes to convert.
	/// @return Script.
	/// @throw Error::FailedAction on errors.
	constexpr Dialog fromBytes(BinaryData<> const& data) {
		Dialog out;
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
		if (fh.dataSize) {
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
		}
		// Jump tables
		if (fh.jumpTableSize) {	
			if (fh.jumpTableSize % sizeof(JumpEntry) != 0) 
				throw Error::FailedAction(
					"Failed at loading script binary!",
					"Malformed jump table section!",
					CTL_CPP_PRETTY_SOURCE
				);
			auto jte = (JumpEntry*)(data.data() + fh.jumpTableStart);
			for (usize i = 0; i < (fh.jumpTableSize / sizeof(JumpEntry)); ++i)
				out.jumps[jte[i].front()] = jte[i].back();
		}
		// Bytecode
		if (!fh.byteCodeSize || fh.byteCodeSize % sizeof(Operation) != 0) 
			throw Error::FailedAction(
				"Failed at loading script binary!",
				"Malformed bytecode section!",
				CTL_CPP_PRETTY_SOURCE
			);
		out.code.resize(fh.byteCodeSize / sizeof(Operation), 0);
		MX::memmove((void*)out.code.data(), (void*)data.data() + fh.byteCodeStart, fh.byteCodeSize);
		return out;
	}
}

#endif