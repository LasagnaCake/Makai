#ifndef MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H

#include "context.hpp"
#include "program.hpp"
#include "function.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Engine {
		struct Error {
			String				message;
			usize				location;
			Core::Instruction	instruction;
		};

		enum class BuiltInFunction: char {
			AV2_EBIF_ADD	= '+',
			AV2_EBIF_SUB	= '-',
			AV2_EBIF_MUL	= '*',
			AV2_EBIF_DIV	= '/',
			AV2_EBIF_MOD	= '%',
			AV2_EBIF_COMP	= '=',
			AV2_EBIF_NEG	= 'n',
			AV2_EBIF_AND	= '&',
			AV2_EBIF_OR		= '|',
			AV2_EBIF_NOT	= '~',
			AV2_EBIF_XOR	= '^',
			AV2_EBIF_LAND	= 'a',
			AV2_EBIF_LOR	= 'o',
			AV2_EBIF_LNOT	= '!',
		};

		bool process();

	protected:
		virtual Data::Value fetchExternal(uint64 const valueID);

		static Data::Value fetchInternal(uint64 const valueID);

	private:
		Engine::Error invalidInstructionEror();
		Engine::Error invalidSourceEror(String const& description);

		Data::Value consumeValue(Core::DataLocation const from);
		Data::Value getValueFromLocation(Core::DataLocation const location, usize const id);

		void crash(Engine::Error const& error);

		void advance();
		void terminate();

		void v2Invoke();
		void v2Global();
		void v2Copy();
		void v2Call();
		void v2Negate();
		void v2StackPush();
		void v2StackPop();
		void v2StackClear();
		void v2Return();
		void v2Halt();

		void callBuiltIn(BuiltInFunction const func);

		void jumpTo(usize const point, bool returnable);
		void returnBack();

		bool				isFinished = false;
		Program				program;
		Context				context;
		Core::Instruction	current;
		Nullable<Error>		err;
	};
}

#endif
