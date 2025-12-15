#include "minima.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
namespace Runtime = Makai::Anima::V2::Runtime;
using Instruction = Makai::Anima::V2::Instruction;
using DataLocation = Makai::Anima::V2::DataLocation;
using Type = Minima::TokenStream::Token::Type;
using enum Type;

#define MINIMA_ASSEMBLE_FN(NAME) static usize do##NAME (Minima::Context& context)

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

#define MINIMA_ERROR(TYPE, WHAT) error<Makai::Error::TYPE>({WHAT}, context)

static Location getStack(Minima::Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing stack index!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	auto v = context.stream.current();
	bool fromTheBack;
	if (
		v.type == Type{'+'}
	||	v.type == Type{'-'}
	) {
		fromTheBack = v.type == Type{'-'};
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	}
	if ((v = context.stream.current()).type != Type::LTS_TT_INTEGER)
		MINIMA_ERROR(InvalidValue, "Stack index must be an integer!");
	Location loc{(fromTheBack ? DataLocation::AV2_DL_STACK_OFFSET : DataLocation::AV2_DL_STACK), v.value.get<usize>()};
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed stack index!");
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	return loc;
}

static Location getRegister(Minima::Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing register index!");
	if (context.stream.current().type != Type{'['})
		MINIMA_ERROR(InvalidValue, "Expected '[' here!");
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	auto v = context.stream.current();
	bool fromTheBack;
	if (
		v.type == Type{'+'}
	||	v.type == Type{'-'}
	) {
		fromTheBack = v.type == Type{'-'};
		if (!context.stream.next())
			MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	}
	if ((v = context.stream.current()).type != Type::LTS_TT_INTEGER)
		MINIMA_ERROR(InvalidValue, "Register index must be an integer!");
	if (v.value.get<usize>() > 31)
		MINIMA_ERROR(InvalidValue, "Register index must be between 0 and 31!");
	Location loc{Makai::Anima::V2::asRegister(v.value.get<ssize>() + (fromTheBack ? Makai::Anima::V2::REGISTER_COUNT : 0)), -1};
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Malformed register index!");
	if (context.stream.current().type != Type{']'})
		MINIMA_ERROR(InvalidValue, "Expected ']' here!");
	return loc;
}

static Location getExtern(Context& context) {
	if (!context.stream.next())
		MINIMA_ERROR(NonexistentValue, "Missing external location name!");
	auto const name = context.stream.current();
	if (
		name.type != LTS_TT_IDENTIFIER
	||	name.type != LTS_TT_SINGLE_QUOTE_STRING
	||	name.type != LTS_TT_DOUBLE_QUOTE_STRING
	) MINIMA_ERROR(InvalidValue, "Expected identifier or string for external location name!");
	else return {
		DataLocation::AV2_DL_EXTERNAL,
		program.constants.pushBack(name.value.get<String>()).size() - 1;
	};
}

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH
static Location getDataLocation(Minima::Context& context) {
	auto const current = context.stream.current();
	switch (current.type) {
		case LTS_TT_IDENTIFIER: {
			auto const id = current.value.get<Makai::String>();
			if (id == "reg") {
				return getRegister(context);
			} else if (id == "stack") {
				return getStack(context);
			} else if (id == "extern") {
				return getExtern(context);
			} else if (id == "temp") {
				return {DataLocation::AV2_DL_TEMPORARY, -1};
			}
		} break;
		case Type{'+'}:
		case Type{'-'}:
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
		case LTS_TT_CHARACTER:
		case LTS_TT_INTEGER:
		case LTS_TT_REAL: {
			if (
				current.type == Type{'+'}
			||	current.type == Type{'-'}
			) {
				bool isNegative = current.type == Type{'-'};
				if (!context.stream.next()) {
					MINIMA_ERROR(NonexistentValue, "Missing value for unary operator!");
				}
				auto const v = context.stream.current();
				if (!(v.type == LTS_TT_INTEGER || v.type == LTS_TT_REAL))
					MINIMA_ERROR(InvalidValue, "Unary operator can only accept numbers!");
				return {
					DataLocation::AV2_DL_CONST,
					context.program.constants.pushBack(
						(
							v.type == LTS_TT_INTEGER
						?	v.value.get<ssize>()
						:	v.value.get<double>()
						)
					*	(isNegative ? -1 : +1)
					).size() - 1
				};
			}
			return {
				DataLocation::AV2_DL_CONST,
				context.program.constants.pushBack(current.value).size() - 1
			};
		} break;
		default: {
			throw Makai::Error::InvalidValue(
				toString("Invalid token \"", current.value, "\" for data location!")
			);
		};
	} 
}
CTL_DIAGBLOCK_END


MINIMA_ASSEMBLE_FN(Jump) {
}

void Minima::assemble() {
	Minima::Program result;
	usize i = 0;
	while (context.stream.next()) {
		auto const current = context.stream.current();
		if (current.type == LTS_TT_IDENTIFIER) {
			if (current.value.get<String>() == "jump") {
				i += doJump(context);
			}
		}
	}
}