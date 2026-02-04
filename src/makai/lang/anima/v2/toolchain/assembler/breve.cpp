#include "breve.hpp"
#include "context.hpp"
#include "core.hpp"
#include "semibreve.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using namespace Makai::Error;

namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = AAssembler::TokenStream::Token::Type;
using enum Type;
using Value = Makai::Data::Value;

struct Solution {
	Makai::Instance<Context::Scope::Member>		type;
	Makai::String								value;
	Makai::String								source;
};

using NamespaceMember	= Makai::KeyValuePair<Makai::String, Makai::Instance<Context::Scope::Member>>;

#define BREVE_ASSEMBLE_FN(NAME) static void do##NAME (Context& context)
#define BREVE_TYPED_ASSEMBLE_FN(NAME) static Solution do##NAME (Context& context)
#define BREVE_SYMBOL_ASSEMBLE_FN(NAME) static Solution do##NAME (Context& context, Context::Scope::Member& sym)

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

void Breve::assemble() {
	Semibreve assembler(context);
	assembler.assemble();
}

CTL_DIAGBLOCK_END
