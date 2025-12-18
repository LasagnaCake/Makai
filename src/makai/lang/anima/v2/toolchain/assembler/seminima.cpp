#include "seminima.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = Seminima::TokenStream::Token::Type;
using enum Type;

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

#define SEMINIMA_ASSEMBLE_FN(NAME) static void do##NAME (Seminima::Context& context)

struct Location {
	DataLocation	at;
	uint64			id;
};

template <class T>
[[noreturn]] static void error(Makai::String what, Context& ctx) {
	auto const pos = ctx.stream.position();
	throw T(
		Makai::toString(
			"At:\nLINE: ", pos.line,
			"\nCOLUMN: ", pos.column,
			"\n", ctx.stream.tokenText()
		),
		what,
		Makai::CPP::SourceFile{"n/a", pos.line, ctx.fileName}
	);
}

#define SEMINIMA_ERROR(TYPE, WHAT) error<Makai::Error::TYPE>({WHAT}, context)

static Location getDataLocation(Context& ctx) {
	
}

SEMINIMA_ASSEMBLE_FN(Jump);
SEMINIMA_ASSEMBLE_FN(NoOp);
SEMINIMA_ASSEMBLE_FN(StackSwap);
SEMINIMA_ASSEMBLE_FN(StackFlush);
SEMINIMA_ASSEMBLE_FN(StackPush);
SEMINIMA_ASSEMBLE_FN(StackPop);
SEMINIMA_ASSEMBLE_FN(StackClear);
SEMINIMA_ASSEMBLE_FN(Return);
SEMINIMA_ASSEMBLE_FN(EmptyReturn);
SEMINIMA_ASSEMBLE_FN(Halt);
SEMINIMA_ASSEMBLE_FN(ErrorHalt);
SEMINIMA_ASSEMBLE_FN(Call);
SEMINIMA_ASSEMBLE_FN(Compare);
SEMINIMA_ASSEMBLE_FN(Copy);
SEMINIMA_ASSEMBLE_FN(Context);
SEMINIMA_ASSEMBLE_FN(ImmediateContext);
SEMINIMA_ASSEMBLE_FN(BinaryMath);
SEMINIMA_ASSEMBLE_FN(UnaryMath);
SEMINIMA_ASSEMBLE_FN(Yield);
SEMINIMA_ASSEMBLE_FN(Await);
SEMINIMA_ASSEMBLE_FN(Cast);
SEMINIMA_ASSEMBLE_FN(Get);
SEMINIMA_ASSEMBLE_FN(Set);
SEMINIMA_ASSEMBLE_FN(StringOperation);
SEMINIMA_ASSEMBLE_FN(Label);

SEMINIMA_ASSEMBLE_FN(Expression) {
	auto const current = context.stream.current();
	if (current.type == LTS_TT_IDENTIFIER) {
		auto const id = current.value.get<Makai::String>();
		if (id == "go")								doJump(context);
		else if (id == "nop")						doNoOp(context);
		else if (id == "swap")						doStackSwap(context);
		else if (id == "flush")						doStackFlush(context);
		else if (id == "push")						doStackPush(context);
		else if (id == "pop")						doStackPop(context);
		else if (id == "clear")						doStackClear(context);
		else if (id == "ret")						doReturn(context);
		else if (id == "end")						doEmptyReturn(context);
		else if (id == "halt")						doHalt(context);
		else if (id == "err")						doErrorHalt(context);
		else if (id == "do")						doCall(context);
		else if (id == "cmp")						doCompare(context);
		else if (id == "copy")						doCopy(context);
		else if (id == "mode")						doContext(context);
		else if (id == "loose" || id == "strict")	doImmediateContext(context);
		else if (id == "bmath")						doBinaryMath(context);
		else if (id == "umath")						doUnaryMath(context);
		else if (id == "yield")						doYield(context);
		else if (id == "wait")						doAwait(context);
		else if (id == "cast")						doCast(context);
		else if (id == "get")						doGet(context);
		else if (id == "set")						doSet(context);
		else if (id == "str")						doStringOperation(context);
		else if (id == "label")						doLabel(context);
	} else SEMINIMA_ERROR(InvalidValue, "Instruction must be an identifier!");
}

void Seminima::assemble() {
	while (context.stream.next()) doExpression(context);
	auto const unmapped = context.mapJumps();
	if (unmapped.size())
		SEMINIMA_ERROR(NonexistentValue, "Some jump targets do not exist!\nTargets:\n[" + unmapped.join("]\n[") + "]");
}