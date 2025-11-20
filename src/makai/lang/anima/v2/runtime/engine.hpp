#ifndef MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H

#include "context.hpp"
#include "program.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Engine {

		enum class BuiltInFunction {
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

		void process();

	private:
		void advance();

		void v2Invoke();
		void v2Global();
		void v2Copy();
		void v2Call();
		void v2Negate();
		void v2StackPush();
		void v2StackPop();
		void v2StackClear();
		void v2Return();

		void callBuiltIn(BuiltInFunction const func);

		void jumpTo(usize const point, bool returnable);
		void returnBack();

		Program				program;
		Context				context;
		Core::Instruction	current;
	};
}

#endif
