#ifndef MAKAILIB_EX_ANIMA_BYTECODE_H
#define MAKAILIB_EX_ANIMA_BYTECODE_H

#include <makai/makai.hpp>

/// @brief Anima Virtual Machine.
namespace Makai::Ex::AVM {
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
		/// @brief No-op. If SP is set, sets the global SP mode.
		AVM_O_NO_OP,
		/// @brief No-op, but skips directly to the next instruction, and does not waste a cycle.
		AVM_O_NEXT,
		/// @brief Ends execution of the program. Behaves differently, depending on SP mode.
		/// @note
		///
		///		- MODE 0: Halt execution entirely.
		///
		/// 	- MODE 1: Return to previous return point.
		AVM_O_HALT,
		/// @brief Active actor. Behaves differently, depending on SP mode.
		/// @note
		///
		///		- MODE 0: Clear and set actor.
		///
		///		- MODE 1: Add actor.
		///
		///		- MODE 2: Clear and change exclude mode.
		AVM_O_ACTOR,
		/// @brief Actor line. Behaves differently, depending on SP mode.
		/// @note
		///
		///		- MODE 0: Say line.
		///
		///		- MODE 1: Add line.
		AVM_O_LINE,
		/// @brief Actor emote.
		AVM_O_EMOTION,
		/// @brief Actor perform. Behaves differently, depending on SP mode.
		/// @note
		///
		///		- MODE 0: Perform with no arguments.
		///
		///		- MODE 1: Perform with arguments.
		AVM_O_ACTION,
		/// @brief Text color. Behaves differently, depending on SP mode.
		/// @note
		///
		///		- MODE 0: RGB Value.
		///
		///		- MODE 1: Name Hash.
		AVM_O_COLOR,
		/// @brief Wait.
		AVM_O_WAIT,
		/// @brief Synchronization. Behaves differently, depending on SP mode.
		AVM_O_SYNC,
		/// @brief User input wait.
		AVM_O_USER_INPUT,
		/// @brief Named operation. Behaves differently, depending on SP mode.
		/// @note
		///
		///		- MODE 0: Single Argument.
		///
		///		- MODE 1: Multiple Arguments.
		AVM_O_NAMED_CALL,
		/// @brief Jump. Behaves differently, depending on SP mode.
		/// @note
		///
		///		- MODE 0: Jump without return.
		///
		///		- MODE 1: Jump with return.
		///
		///		- MODE 2: Selection jump (with return).
		///
		///		- MODE 10: Selection jump (without return).
		AVM_O_JUMP,
		/// @brief Value request. Behaves differently, depending on SP mode.
		///
		///		- MODE 0: Get integer.
		///
		///		- MODE 2: Get string.
		///
		///		- MODE 3: Get choice.
		AVM_O_GET_VALUE,
		/// @brief Menu. Behaves differently, depending on SP mode.
		///
		///		- MODE 0: Open menu.
		///
		///		- MODE 1: Close menu.
		///
		///		- MODE 2: Return to previous menu, if any.
		///
		///		- MODE 3: Highlight option.
		///
		///		- MODE 4: Return control to menu.
		AVM_O_MENU,
	};

	/// @brief Binary version.	
	constexpr uint64 ANIMA_VERSION		= 0;
	/// @brief Minimum required version to run binary.
	constexpr uint64 ANIMA_MIN_VERSION	= 0;

	/// @brief SP mode mask.
	constexpr uint16 SP_FLAG_MASK	= 0xf000u;
	/// @brief Operation mask.
	constexpr uint16 OPERATION_MASK	= 0x0fffu;

	/// @brief Converts the given data to an operation.
	/// @param op Data to convert.
	/// @return Operation.
	constexpr Operation	asOperation(uint16 const op)	{return static_cast<Operation>(op & OPERATION_MASK);	}

	static_assert(asOperation(SP_FLAG_MASK) == Operation::AVM_O_NO_OP);
	
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
	
	/// @brief File header content section.
	struct Section {
		/// @brief Section start.
		uint64 start;
		/// @brief Section size.
		uint64 size;

		/// @brief Returns the section offset.
		/// @return Section offset.
		constexpr uint64 offset() const {
			return start + size;
		}
	};

	using Tool::Arch::FileToken;

	/// @brief Anima program file header.
	struct [[gnu::packed]] AnimaBinaryHeader {
		uint64 headerSize		= sizeof(AnimaBinaryHeader);
		uint64 version			= ANIMA_VERSION;
		uint64 minVersion		= ANIMA_MIN_VERSION;
		uint64 flags;
		Section data;
		Section jumps;
		Section code;
		FileToken const token	= "Makai::AnimaBinary\0";
		// Put new things BELOW this line
	};

	/// @brief Compiled anima program.
	struct Anima {
		/// @brief Jump table.
		JumpTable	jumps;
		/// @brief Anima data.
		StringList	data;
		/// @brief Anima bytecode.
		Binary		code;

		/// @brief Converts a series of bytes to a processable anima binary.
		/// @param data Bytes to convert.
		/// @return Anima binary.
		/// @throw Error::FailedAction on errors.
		static Anima fromBytes(BinaryData<> const& data) {
			Anima out;
			if (data.size() < sizeof(uint64) + 12)
				throw Error::FailedAction(
					"Failed at loading anima binary!",
					"File size is too small!",
					CTL_CPP_PRETTY_SOURCE
				);
			// Main header
			AnimaBinaryHeader fh;
			MX::memmove((void*)&fh.headerSize, (void*)data.data(), sizeof(uint64));
			if (data.size() < fh.headerSize) 
				throw Error::FailedAction(
					"Failed at loading anima binary!",
					"File size is too small!",
					CTL_CPP_PRETTY_SOURCE
				);
			usize const fhs = (fh.headerSize < sizeof(AnimaBinaryHeader)) ? fh.headerSize : sizeof(AnimaBinaryHeader);
			MX::memmove((void*)&fh, (void*)data.data(), fhs);
			// Check if sizes are OK
			if (
				data.size() < (fh.headerSize + fh.data.size + fh.jumps.size + fh.code.size)
			||	data.size() < (fh.jumps.offset())
			||	data.size() < (fh.code.offset())
			||	data.size() < (fh.data.offset())
			) throw Error::FailedAction(
				"Failed at loading anima binary!",
				"File size is too small!",
				CTL_CPP_PRETTY_SOURCE
			);
			if (String(fh.token) != "Makai::AnimaBinary\0")
				throw Error::FailedAction(
					"Failed at loading anima binary!",
					"File is not an anima binary!",
					CTL_CPP_PRETTY_SOURCE
				);
			// Data division
			if (fh.data.size) {
				String buf;
				auto c			= data.data() + fh.data.start;
				auto const end	= c + fh.data.size;
				auto const eof	= data.end().raw();
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
			if (fh.jumps.size) {	
				if (fh.jumps.size % sizeof(JumpEntry) != 0) 
					throw Error::FailedAction(
						"Failed at loading anima binary!",
						"Malformed jump table section!",
						CTL_CPP_PRETTY_SOURCE
					);
				auto jte = (JumpEntry*)(data.data() + fh.jumps.start);
				for (usize i = 0; i < (fh.jumps.size / sizeof(JumpEntry)); ++i)
					out.jumps[jte[i].front()] = jte[i].back();
			}
			// Bytecode
			if (!fh.code.size || fh.code.size % sizeof(Operation) != 0) 
				throw Error::FailedAction(
					"Failed at loading anima binary!",
					"Malformed bytecode section!",
					CTL_CPP_PRETTY_SOURCE
				);
			out.code.resize(fh.code.size / sizeof(Operation), 0);
			MX::memmove((byte*)out.code.data(), (byte*)data.data() + fh.code.start, fh.code.size);
			return out;
		}
	};
}

#endif