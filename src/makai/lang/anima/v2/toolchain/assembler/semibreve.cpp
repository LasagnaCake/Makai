#include "semibreve.hpp"
#include "context.hpp"
#include "core.hpp"
#include "../../../../../data/data.hpp"

using namespace Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Assembler;

using Context = Semibreve::Context;

using Type = Context::Tokenizer::Token::Type;
using enum Type;

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

static void doExpression(Context& context) {

}

void Semibreve::assemble() {
	while (!context.empty()) doExpression(context);
}

CTL_DIAGBLOCK_END
