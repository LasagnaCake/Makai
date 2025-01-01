#ifndef MAKAILIB_EX_GAME_DIALOG_SVM_BYTECODE_H
#define MAKAILIB_EX_GAME_DIALOG_SVM_BYTECODE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog::SVM {
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
		DSO_NO_OP,
		/// @brief Ends execution of the program.
		DSO_HALT,
		/// @brief Active actor. Behaves differently, depending on SP mode.
		DSO_ACTOR,
		/// @brief Dialog line. Behaves differently, depending on SP mode.
		DSO_LINE,
		/// @brief Actor emote.
		DSO_EMOTION,
		/// @brief Actor perform. Behaves differently, depending on SP mode.
		DSO_ACTION,
		/// @brief Text color. Behaves differently, depending on SP mode.
		DSO_COLOR,
		/// @brief Wait.
		DSO_WAIT,
		/// @brief Synchronization. Behaves differently, depending on SP mode.
		DSO_SYNC,
		/// @brief User input wait.
		DSO_USER_INPUT,
		/// @brief Set global. Behaves differently, depending on SP mode.
		DSO_SET_GLOBAL,
		/// @brief Named operation. Behaves differently, depending on SP mode.
		DSO_NAMED_OP,
		/// @brief Jump.
		DSO_JUMP,
	};

	/// @brief Script version.	
	constexpr uint64 SCRIPT_VERSION		= 0;
	/// @brief Minimum required version to run script.
	constexpr uint64 SCRIPT_MIN_VERSION	= 0;

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

	/// @brief Dialog script.
	struct Script {
		/// @brief Jump table.
		JumpTable	jumps;
		/// @brief Script data.
		StringList	data;
		/// @brief Script bytecode.
		Binary		code;
	};

	/// @brief Compiled script file header.
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

	/// @brief Converts a script to a storeable binary file.
	/// @param code Script to convert.
	/// @return Script as binary.
	constexpr BinaryData<> toBytes(Script const& code) {
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

	/// @brief Converts a series of bytes to an executable script.
	/// @param data Bytes to convert.
	/// @return Script.
	/// @throw Error::FailedAction on errors.
	constexpr Script fromBytes(BinaryData<> const& data) {
		Script out;
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