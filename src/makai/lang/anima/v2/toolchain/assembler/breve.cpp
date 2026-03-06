#include "breve.hpp"
#include "context.hpp"
#include "core.hpp"
#include "semibreve.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using namespace Makai::Error;

void Breve::assemble() {
	Semibreve assembler(context);
	assembler.assemble();
}

CTL_DIAGBLOCK_END
