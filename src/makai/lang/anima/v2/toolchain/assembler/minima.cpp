#include "minima.hpp"

using namespace Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Assembler;

using Context = Minima::Context;

using Type = Context::Tokenizer::Token::Type;
using enum Type;

static Makai::String cleanPath(Makai::String const& path) {
	return Makai::Regex::replace(path, "\\/\\/+", "\\/");
}

uint64 Context::addStringLiteral(String const& str) {
	auto const strID = program.strings.find(str);
	if (strID != -1) return strID;
	program.strings.pushBack(str);
	return program.strings.size() - 1;
}

Makai::String Context::fullModulePath() const {
	String path = "";
	for (auto& module: moduleStack)
		path += "/" + module;
	return cleanPath(path);
}

void Context::addMethod(Makai::String const& name, Instance<Method> const& method) {
	if (methods.contains(name))
		error("Method with this name already exists!");
	auto const fullID = name + "@" + method->name;
	moduleMethods[fullID] = method;
	methods[name] = new Reference{.name = fullID};
	if (method->local) return;
	method->jump = name + "/entry";
	method->id = program.detail.methods.size();
	program.detail.methods.pushBack(*method);
	program.sym.methods.pushBack({nullptr, method->id, name});
}

void Context::addType(Makai::String const& name, Instance<Declaration> const& type) {
	if (types.contains(name))
		error("Type with this name already exists!");
	auto const fullID = name + "@" + type->name;
	moduleTypes[fullID] = type;
	types[name] = new Reference{.name = fullID};
	type->id = program.detail.types.size();
	program.detail.types.pushBack(*type);
	program.sym.types.pushBack({nullptr, type->id, name});
}

void Context::addExternalMethod(Makai::String const& module, Makai::String const& name, Instance<Method> const& method) {
	if (method->local) return;
	if (methods.contains(name))
		error("Method with this name already exists!");
	auto const fullID = module + ":" + name + "@" + method->name;
	auto const refID = cleanPath(module + "/" + name);
	externalMethods[fullID] = method;
	methods[refID] = new Reference{module, fullID};
	method->id = program.detail.methods.size();
	program.detail.methods.pushBack(*method);
	uint64 moduleID = program.ani->shared.modules.find(module);
	if (moduleID == Makai::Limit::MAX<uint64>)
		moduleID = program.ani->shared.modules.size();
	program.sym.methods.pushBack({moduleID, method->id, refID});
}

void Context::addExternalType(Makai::String const& module, Makai::String const& name, Instance<Declaration> const& type) {
	if (methods.contains(name))
		error("Type with this name already exists!");
	auto const fullID = module + ":" + name + "@" + type->name;
	auto const refID = cleanPath(module + "/" + name);
	externalTypes[fullID] = type;
	methods[refID] = new Reference{module, fullID};
	type->id = program.detail.methods.size();
	program.detail.types.pushBack(*type);
	uint64 moduleID = program.ani->shared.modules.find(module);
	if (moduleID == Makai::Limit::MAX<uint64>)
		moduleID = program.ani->shared.modules.size();
	program.sym.methods.pushBack({moduleID, type->id, refID});
}

Makai::Instance<Context::Method> Context::getMethod(Makai::String const& name) {
	if (!methods.contains(name))
		error("No methods with this name exist!");
	auto& method = methods[name];
	if (method->module.empty())
		return moduleMethods[method->name];
	return externalMethods[method->name];
}

Makai::Instance<Context::Declaration> Context::getType(Makai::String const& name) {
	if (!types.contains(name))
		error("No types with this name exist!");
	auto& type = types[name];
	if (type->module.empty())
		return moduleTypes[type->name];
	return externalTypes[type->name];
}

Makai::Instance<Context::Method> Context::getMethodByID(uint64 const& id) {
	if (id < program.sym.methods.size())
		return getMethod(program.sym.methods[id].name);
	error("Method with the given ID does not exist!");
}

Makai::Instance<Context::Declaration> Context::getTypeByID(uint64 const& id) {
	if (id < program.sym.types.size())
		return getType(program.sym.types[id].name);
	error("Type with the given ID does not exist!");
}

usize Context::add(uint64 const value) {
	program.code.pushBack(Makai::Cast::bit<Core::Instruction>(value));
	return program.code.size()-1;
}

usize Context::add(OpCode const opcode, uint32 const type) {
	program.code.pushBack(Core::Instruction{opcode, type});
	return program.code.size()-1;
}

usize Context::update(usize const inst, uint32 const type) {
	program.code[inst].type = type;
	return inst;
}

uint64 Context::addGlobal(String const& name) {
	program.strings.pushBack(name);
	return program.strings.size()-1;
}

void Context::addJumpTarget(String const& name) {
	if (name.empty())
		error("Missing jump target name!");
	if (hasJumpTarget(name)) {
		add(jumps[name]);
		return;
	}
	jumpsToMap[name].pushBack(program.code.size());
	add(0);
}

uint64 Context::getJumpTarget(String const& name) {
	return hasJumpTarget(name) ? jumps[name] : 0;
}

bool Context::hasJumpTarget(String const& name) {
	return jumps.contains(name);
}

void Context::finalize() {
	auto unmapped = jumpsToMap.keys();
	for (auto& label: unmapped) {
		if (label.empty())
			error("Somehow, jump target is empty!");
		if (jumpsToMap.contains(label) && jumps.contains(label)) {
			for (auto& location: jumpsToMap[label])
				program.jumpTable[location] = jumps[label];
			jumpsToMap.erase(label);
		}
	}
	unmapped = jumpsToMap.keys();
	if (unmapped.size())
		error("Some jump targets do not exist!\nTargets:\n[" + unmapped.join("]\n[") + "]");
	if (entry.size() && jumps.contains(entry))
		program.entry = jumps[entry];
	else if (entry.size()) error("Missing entry location!");
	if (exit.size() && jumps.contains(exit))
		program.exit = jumps[exit];
	else if (exit.size()) error("Missing exit location!");
}

static Makai::String resolvePath(Context& context, bool absolute = false, Type const pathSeparator = Type{'.'}) {
	if (context.has(pathSeparator)) {
		absolute = true;
		context.expectNext(LTS_TT_IDENTIFIER, "namespace/symbol name");
	}
	Makai::String result = context.get(LTS_TT_IDENTIFIER, "namespace/symbol name").getString();
	while(context.peek().type == Type{'.'}) {
		result +=
			"/" + context
				.expectNext(Type{'.'})
				.getNext(LTS_TT_IDENTIFIER, "namespace/symbol name")
				.getString()
		;
	}
	if (absolute)
		result = context.fullModulePath() + result;
	return cleanPath(result);
}

struct Location {
	DataLocation			source;
	Makai::Nullable<uint64>	id		= null;

	constexpr Location operator|(Location const& other) const {
		return {
			source | other.source,
			other.id
		};
	}

	constexpr Location& operator|=(Location const& other) {
		source = source | other.source;
		id = other.id;
		return *this;
	}

	constexpr bool hasID() const {
		return id.exists();
	}

	constexpr void addID(Context& context) const {
		if (hasID()) context.add(id.value());
	}
};

static DataLocation getLoadType(Context& context, DataLocation const prev) {
	DataLocation locAt = asPlace(prev);
	if (context.has(LTS_TT_IDENTIFIER)) {
		auto const id = context.value().getString();
		if (id == "reference" || id == "ref")
			locAt = locAt | DataLocation::AV2_DLM_BY_REF;
		else if (id == "move")
			locAt = locAt | DataLocation::AV2_DLM_MOVE;
		else if (id == "value" || id == "copy")
			locAt = locAt | DataLocation{0};
	}
	return locAt;
}

static Location getStack(Context& context) {
	Location loc;
	context
		.next()
		.expect(Type{'['})
	;
	context.next();
	bool backshots = false;
	while (
		context.has(Type{'-'})
	||	context.has(Type{'+'})
	) {
		if (context.has(Type{'-'}))
			backshots = !backshots;
		context.next();
	}
	loc.id = context.get(LTS_TT_INTEGER, "stack index").getUnsigned();
	loc.source = loc.source | (
		backshots
	?	DataLocation::AV2_DL_STACK_OFFSET
	:	DataLocation::AV2_DL_STACK
	);
	context.expectNext(Type{']'});
	return loc;
}

static Location getLocal(Context& context) {
	auto const localID =
		context
			.expectNext(Type{'['})
			.getNext(LTS_TT_INTEGER, "local stack ID")
			.getUnsigned()
	;
	context.expectNext(Type{']'});
	return {
		DataLocation::AV2_DL_LOCAL,
		localID
	};
}

static Location getExtern(Context& context) {
	context.next();
	uint64 externID = 0;
	switch (context.type()) {
		case LTS_TT_IDENTIFIER:
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
			externID = context.addStringLiteral(context.value().getString());
		break;
		default: context.error("Expected external location name here!");
	}
	return {DataLocation::AV2_DL_EXTERNAL, externID};
}

static Location getGlobal(Context& context) {
	auto const name = resolvePath(context);
	uint64 globalID = context.addGlobal(name);
	return {
		DataLocation::AV2_DL_GLOBAL,
		globalID
	};
}

static DataLocation getIntWidth(Context& context) {
	auto const id = context.getNext(LTS_TT_IDENTIFIER, "int type").getString();
	if (id == "i8")
		return DataLocation{0};
	else if (id == "i16")
		return DataLocation::AV2_DLI_16;
	else if (id == "i32")
		return DataLocation::AV2_DLI_32;
	else if (id == "i64")
		return DataLocation::AV2_DLI_64;
	else context.error("Invalid integer type!");
}

static DataLocation getRealWidth(Context& context) {
	auto const id = context.getNext(LTS_TT_IDENTIFIER, "real type").getString();
	if (id == "f32")
		return DataLocation{0};
	else if (id == "f64")
		return DataLocation::AV2_DLF_64;
	else if (id == "f128")
		return DataLocation::AV2_DLF_128;
	else context.error("Invalid float type!");
}

static Location getConstantLocation(Context& context) {
	auto type = context.type();
	bool isNumber = false;
	bool negated = false;
	while (
		type == Type{'+'}
	||	type == Type{'-'}
	) {
		isNumber = true;
		if (type == Type{'-'})
			negated = !negated;
		type = context.next().type();
	}
	Location loc {.source = DataLocation::AV2_DL_STRING};
	if (isNumber) {
		switch (context.type()) {
			case LTS_TT_INTEGER:
				loc.id = context.value().getSigned() * (negated ? -1 : +1);
				loc.source = DataLocation::AV2_DL_INT;
				if (context.peek().type == LTS_TT_IDENTIFIER && context.peek().text[0] == Makai::UTF::U8Char{'i'})
					loc.source = loc.source | getIntWidth(context);
				else loc.source = loc.source | DataLocation::AV2_DLI_64;
			break;
			case LTS_TT_REAL:
				loc.id = Makai::Cast::bit<uint64>(context.value().getReal() * (negated ? -1 : +1));
				loc.source = DataLocation::AV2_DL_REAL;
				if (context.peek().type == LTS_TT_IDENTIFIER && context.peek().text[0] == Makai::UTF::U8Char{'f'})
					loc.source = loc.source | getRealWidth(context);
				else loc.source = loc.source | DataLocation::AV2_DLF_64;
			break;
			default: context.error("Expected number here!");
		}
		return loc;
	} else {
		switch (context.type()) {
			case LTS_TT_SINGLE_QUOTE_STRING:
			case LTS_TT_DOUBLE_QUOTE_STRING:
			case LTS_TT_BACKTICK_STRING:
			case LTS_TT_FR_SINGLE_QUOTE_STRING:
			case LTS_TT_FR_DOUBLE_QUOTE_STRING:
			case LTS_TT_JP_SINGLE_QUOTE_STRING:
			case LTS_TT_JP_DOUBLE_QUOTE_STRING:
				loc.id = context.addStringLiteral(context.value().getString());
			break;
			case LTS_TT_INTEGER:
				loc.id = context.value().getUnsigned();
				loc.source = DataLocation::AV2_DL_INT | DataLocation::AV2_DLI_UNSIGNED;
				if (context.peek().type == LTS_TT_IDENTIFIER && context.peek().text[0] == Makai::UTF::U8Char{'u'})
					loc.source = loc.source | getIntWidth(context);
				else loc.source = loc.source | DataLocation::AV2_DLI_64;
			break;
			case LTS_TT_REAL:
				loc.id = Makai::Cast::bit<uint64>(context.value().getReal());
				loc.source = DataLocation::AV2_DL_REAL;
				if (context.peek().type == LTS_TT_IDENTIFIER && context.peek().text[0] == Makai::UTF::U8Char{'f'})
					loc.source = loc.source | getRealWidth(context);
				else loc.source = loc.source | DataLocation::AV2_DLF_64;
			break;
			default: context.error("Invalid constant!");
		}
		return loc;
	}
	context.error("Invalid constant!");
}

static Location getLabelLocation(Context& context) {
	context.next();
	auto const current = resolvePath(context);
	if (!context.hasJumpTarget(current))
		context.error("Label has not been declared yet!");
	return {DataLocation::AV2_DL_INT | DataLocation::AV2_DLI_UNSIGNED, context.getJumpTarget(current)};
}

static Location getDataLocation(Context& context) {
	Location loc;
	bool hasLoadType = false;
	DEBUGLN("Getting move type...");
	while (context.has(LTS_TT_IDENTIFIER)) {
		auto const lt = context.value().getString();
		if (
			lt == "move"
		||	lt == "ref"
		||	lt == "copy"
		) {
			DEBUGLN("Has move!");
			if (!hasLoadType) {
				hasLoadType = true;
				loc.source = getLoadType(context, DataLocation{0});
			}
			context.next();
			continue;
		} else break;
	}
	DEBUGLN("Getting location...");
	switch (context.type()) {
		case LTS_TT_IDENTIFIER: {
			auto const id = context.value().getString();
			if (id == "local" || id == "arg")			loc |= getLocal(context);
			else if (id == "placeof")					loc |= getLabelLocation(context);
			else if (id == "stack")						loc |= getStack(context);
			else if (id == "global" || id == "g")		loc |= getGlobal(context);
			else if (id == "external" || id == "out")	loc |= getExtern(context);
			else if (id == "false")						loc |= Location{DataLocation::AV2_DL_BOOL, null};
			else if (id == "true")						loc |= Location{DataLocation::AV2_DL_BOOL | DataLocation::AV2_DLB_TRUE, null};
			else if (id == "void")						loc |= Location{DataLocation::AV2_DL_VOID, null};
			else if (id == "null" || id == "nil")		loc |= Location{DataLocation::AV2_DL_NULL, null};
			else context.error("Invalid data source!");
		} break;
		case Type{'&'}:
			loc |= getLabelLocation(context);
		break;
		case Type{':'}:
			loc |= getExtern(context);
		break;
		case Type{'$'}:
			loc |= getGlobal(context);
		break;
		case Type{'.'}:
			loc |= getLocal(context);
		break;
		case Type{'+'}:
		case Type{'-'}:
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
		case LTS_TT_BACKTICK_STRING:
		case LTS_TT_FR_SINGLE_QUOTE_STRING:
		case LTS_TT_FR_DOUBLE_QUOTE_STRING:
		case LTS_TT_JP_SINGLE_QUOTE_STRING:
		case LTS_TT_JP_DOUBLE_QUOTE_STRING:
		case LTS_TT_INTEGER:
		case LTS_TT_REAL:
			loc |= getConstantLocation(context);
		break;
		default: context.error("Invalid data source!");
	}
	return loc;
}

static void doConditionalJump(Context& context, bool dynamic = false) {
	context.next();
	Instruction::Leap leap{.dyn = dynamic};
	switch (context.type()) {
		case LTS_TT_IDENTIFIER: {
			auto const id = context.value().getString();
			if (id == "zero" || id == "z")				leap.type = Instruction::Leap::Type::AV2_ILT_IF_ZERO;
			else if (id == "nonzero" || id == "nz")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NOT_ZERO;
			else if (id == "negative" || id == "n")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NEGATIVE;
			else if (id == "positive" || id == "p")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_POSITIVE;
			else if (id == "true" || id == "t")			leap.type = Instruction::Leap::Type::AV2_ILT_IF_TRUTHY;
			else if (id == "false" || id == "f")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_FALSY;
			else if (id == "null" || id == "nil")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NULL;
			else if (id == "void" || id == "v")			leap.type = Instruction::Leap::Type::AV2_ILT_IF_UNDEFINED;
			else if (id == "empty" || id == "e")		leap.type = Instruction::Leap::Type::AV2_ILT_IF_NULL_OR_UNDEFINED;
		}
		default: context.error("Invalid condition!");
	}
	context.add(Instruction::Name::AV2_IN_JUMP, leap);
	if (!dynamic)
		context.addJumpTarget(context.getNext(LTS_TT_IDENTIFIER, "jump expression").getString());
}

static void doJump(Context& context, bool dynamic = false) {
	context.next();
	if (!dynamic) context.expect(LTS_TT_IDENTIFIER, "jump expression");
	else if (!context.has(LTS_TT_IDENTIFIER)) {
		context.pad(1);
		context.add(
			Instruction::Name::AV2_IN_JUMP,
			Instruction::Leap{
				Instruction::Leap::Type::AV2_ILT_UNCONDITIONAL,
				true
			}
		);
	}
	auto const jt = context.value().getString();
	if (jt == "if")
		doConditionalJump(context, dynamic);
	else if (!dynamic)
		context.addJumpTarget(resolvePath(context));
	else context.error("Invalid jump instruction!");
}

static void doCall(Context& context, bool dynamic = false) {
	Instruction::Invocation invoke {.dynamic = dynamic};
	if (!dynamic) {
		context.next();
		auto id = resolvePath(context);
		context.add(
			Instruction::Name::AV2_IN_CALL,
			invoke
		);
		if (context.methods.contains(id))
			context.addJumpTarget(context.getMethod(id)->jump);
		else context.error("Method with this name does not exist!");
	} else {
		context.add(
			Instruction::Name::AV2_IN_CALL,
			invoke
		);
	}
}

static void doNoOp(Context& context) {
	context.add(
		Instruction::Name::AV2_IN_NO_OP,
		Makai::Cast::as<uint32>(context.value().getString() == "next")
	);
}

static void doStackSwap(Context& context) {
	context.add(Instruction::Name::AV2_IN_STACK_SWAP);
}

static void doStackFlush(Context& context) {
	context.add(Instruction::Name::AV2_IN_STACK_FLUSH);
}

static void doStackPush(Context& context) {
	context.next();
	auto const src = getDataLocation(context);
	context.add(
		Instruction::Name::AV2_IN_STACK_PUSH,
		Instruction::StackPush{src.source}
	);
	src.addID(context);
}

static void doStackPop(Context& context) {
	context.add(Instruction::Name::AV2_IN_STACK_POP);
}

static void doStackBlit(Context& context) {
	Instruction::Blitting blit {.type = Instruction::Blitting::Type::AV2_IBT_COPY};
	auto const count = context.getNext(LTS_TT_INTEGER).getUnsigned();
	context.expectNext(LTS_TT_LITTLE_ARROW);
	auto const src = context.getNext(LTS_TT_IDENTIFIER, "blit source").getString();
	if (src == "local" || src == "l")
		blit.fromGlobal = true;
	else if (src == "global" || src == "g")
		blit.fromGlobal = false;
	else context.error("Invalid blit source!");
	context.add(
		Instruction::Name::AV2_IN_STACK_BLIT,
		blit
	);
	context.add(count);
}

static void doReturn(Context& context) {
	context.add(Instruction::Name::AV2_IN_RETURN);
}

static void doStackClear(Context& context) {
	context.add(Instruction::Name::AV2_IN_STACK_CLEAR);
	context.add(context.getNext(LTS_TT_INTEGER).getUnsigned());
}

static void doField(Context& context, bool const setter, bool const dyn = false) {
	Makai::Nullable<uint64> field;
	Instruction::Field f;
	if (!dyn) {
		field =
			context
				.expectNext(Type{'['})
				.getNext(LTS_TT_INTEGER, "field ID")
				.getUnsigned()
		;
		context.expectNext(Type{']'});
	} else f.dynamic = dyn;
	context.add(
		setter
	?	Instruction::Name::AV2_IN_FIELD_SET
	:	Instruction::Name::AV2_IN_FIELD_GET,
		f
	);
}

static void doSizeOf(Context& context, bool const inBytes = false) {
	context.add(
		Instruction::Name::AV2_IN_SIZEOF,
		inBytes
	);
}

static void doTypeGet(Context& context) {
	context.add(Instruction::Name::AV2_IN_TYPEOF);
}


static void doTypeCheck(Context& context, bool const dynamic = false) {
	// TODO: Dynamic type checking
}

static void doHalt(Context& context, bool const error = false) {
	context.add(
		Instruction::Name::AV2_IN_HALT,
		Instruction::Stop{
			error
		?	Instruction::Stop::Mode::AV2_ISM_ERROR
		:	Instruction::Stop::Mode::AV2_ISM_NORMAL
		}
	);
}

static void doCompare(Context& context) {
	auto const id = context.getNext(LTS_TT_IDENTIFIER, "comparison").getString();
	Instruction::Comparison cmp;
	if (id == "less" || id == "l")					cmp.comp = Comparator::AV2_OP_LESS_THAN;
	else if (id == "greater" || id == "g")			cmp.comp = Comparator::AV2_OP_GREATER_THAN;
	else if (id == "equals" || id == "e")			cmp.comp = Comparator::AV2_OP_EQUALS;
	else if (id == "notequals" || id == "ne")		cmp.comp = Comparator::AV2_OP_NOT_EQUALS;
	else if (id == "greaterequals" || id == "ge")	cmp.comp = Comparator::AV2_OP_GREATER_EQUALS;
	else if (id == "lessequals" || id == "le")		cmp.comp = Comparator::AV2_OP_LESS_EQUALS;
	else if (id == "order" || id == "o")			cmp.comp = Comparator::AV2_OP_THREEWAY;
	context.add(Instruction::Name::AV2_IN_COMPARE, cmp);
}

static void doScopeEnter(Context& context) {
	auto const count = context.getNext(LTS_TT_INTEGER, "scope-local stack size");
	context.add(
		Instruction::Name::AV2_IN_SCOPE_ENTER,
		count
	);
}

static void doScopeExit(Context& context) {
	context.add(Instruction::Name::AV2_IN_SCOPE_EXIT);
}

static void doScopeBind(Context& context) {
	auto const transfer = context.getNext(LTS_TT_IDENTIFIER, "transfer mode").getString();
	auto const count = context.getNext(LTS_TT_INTEGER, "bind count").getUnsigned();
	context.expectNext(Type{'['});
	auto const src = context.getNext(LTS_TT_INTEGER, "global stack top offset").get<uint16>();
	context.expectNext(LTS_TT_LITTLE_ARROW);
	auto const dst = context.getNext(LTS_TT_INTEGER, "local stack bottom offset").get<uint16>();
	context.expectNext(Type{']'});
	if (count < 1) return;
	context.add(
		Instruction::Name::AV2_IN_SCOPE_BIND,
		Instruction::Binding {
			src,
			dst,
			transfer == "copy"
		}
	);
	context.add(count);
}

static void doScopeBring(Context& context) {
	auto const transfer = context.getNext(LTS_TT_IDENTIFIER, "transfer mode").getString();
	auto const count = context.getNext(LTS_TT_INTEGER, "bind count").getUnsigned();
	context.expectNext(Type{'['});
	auto const src = context.getNext(LTS_TT_INTEGER, "source scope").getUnsigned();
	auto const offset = context.expectNext(Type{'['}).getNext(LTS_TT_INTEGER, "source local stack bottom offset").get<uint16>();
	context.expectNext(Type{']'}).expectNext(LTS_TT_LITTLE_ARROW);
	auto const dst = context.getNext(LTS_TT_INTEGER, "source local stack bottom offset").get<uint16>();
	context.expectNext(Type{']'});
	if (count < 1) return;
	context.add(
		Instruction::Name::AV2_IN_SCOPE_BRING,
		Instruction::Binding {
			offset,
			dst,
			transfer == "copy"
		}
	);
	context.add(src);
	context.add(count);
}

static void doScopeKeep(Context& context) {
	context.add(Instruction::Name::AV2_IN_SCOPE_KEEP);
}

static void doScopeDeclare(Context& context) {
	auto const count = context.getNext(LTS_TT_INTEGER, "declaration count").getUnsigned();
	if (count < 1) return;
	context.add(Instruction::Name::AV2_IN_SCOPE_DECLARE, count);
}

static void doCopy(Context& context) {
	context.next();
	auto const src = getDataLocation(context);
	context.expectNext(LTS_TT_LITTLE_ARROW).next();
	auto const dst = getDataLocation(context);
	Instruction::Transfer tf {
		src.source,
		dst.source
	};
	context.add(
		Instruction::Name::AV2_IN_COPY,
		tf
	);
	src.addID(context);
	dst.addID(context);
}

static void doContext(Context& context, bool immediate = false) {
	auto const mode = context.get(LTS_TT_IDENTIFIER, "context").getString();
	Instruction::Context ctx{.immediate = immediate};
	if (mode == "loose")		ctx.mode = ContextMode::AV2_CM_LOOSE;
	else if (mode == "strict")	ctx.mode = ContextMode::AV2_CM_STRICT;
	else context.error("Invalid context!");
	context.add(
		Instruction::Name::AV2_IN_MODE,
		ctx
	);
}

static void doOperation(Context& context) {
	Instruction::Operation bop;
	auto const op = context.getNext(LTS_TT_IDENTIFIER, "unary operation name").getString();
	if (op == "add")		bop.op = Operator::AV2_BOP_ADD;
	else if (op == "sub")	bop.op = Operator::AV2_BOP_SUB;
	else if (op == "mul")	bop.op = Operator::AV2_BOP_MUL;
	else if (op == "div")	bop.op = Operator::AV2_BOP_DIV;
	else if (op == "mod")	bop.op = Operator::AV2_BOP_REM;
	else if (op == "land")	bop.op = Operator::AV2_BOP_LOGIC_AND;
	else if (op == "lor")	bop.op = Operator::AV2_BOP_LOGIC_OR;
	else if (op == "lxor")	bop.op = Operator::AV2_BOP_LOGIC_XOR;
	else if (op == "band")	bop.op = Operator::AV2_BOP_BIT_AND;
	else if (op == "bor")	bop.op = Operator::AV2_BOP_BIT_OR;
	else if (op == "bxor")	bop.op = Operator::AV2_BOP_BIT_XOR;
	else if (op == "pow")	bop.op = Operator::AV2_BOP_POW;
	else if (op == "atan2")	bop.op = Operator::AV2_BOP_ATAN2;
	else if (op == "neg")	bop.op = Operator::AV2_UOP_NEGATE;
	else if (op == "inc")	bop.op = Operator::AV2_UOP_INCREMENT;
	else if (op == "dec")	bop.op = Operator::AV2_UOP_DECREMENT;
	else if (op == "inv")	bop.op = Operator::AV2_UOP_INVERSE;
	else if (op == "sin")	bop.op = Operator::AV2_UOP_SIN;
	else if (op == "cos")	bop.op = Operator::AV2_UOP_COS;
	else if (op == "tan")	bop.op = Operator::AV2_UOP_TAN;
	else if (op == "asin")	bop.op = Operator::AV2_UOP_ASIN;
	else if (op == "acos")	bop.op = Operator::AV2_UOP_ACOS;
	else if (op == "atan")	bop.op = Operator::AV2_UOP_ATAN;
	else if (op == "sinh")	bop.op = Operator::AV2_UOP_SINH;
	else if (op == "cosh")	bop.op = Operator::AV2_UOP_COSH;
	else if (op == "tanh")	bop.op = Operator::AV2_UOP_TANH;
	else if (op == "log2")	bop.op = Operator::AV2_UOP_LOG2;
	else if (op == "log10")	bop.op = Operator::AV2_UOP_LOG10;
	else if (op == "ln")	bop.op = Operator::AV2_UOP_LN;
	else if (op == "sqrt")	bop.op = Operator::AV2_UOP_SQRT;
	else if (op == "lnot")	bop.op = Operator::AV2_UOP_LOGIC_NOT;
	else if (op == "bnot")	bop.op = Operator::AV2_UOP_BIT_NOT;
	else context.error("Invalid/Unsupported operation!");
	context.add(Instruction::Name::AV2_IN_OP, bop);
}

static void doYield(Context& context) {
	context.add(Instruction::Name::AV2_IN_YIELD);
}

static void doCast(Context& context, bool const dyn = false) {
	if (!dyn) {
		auto const type = resolvePath(context);
		if (context.types.contains(type)) {
			context.add(Instruction::Name::AV2_IN_CAST);
			context.add(context.getType(type)->id);
		} else context.error("Type with this name does not exist!");
	} else {
		context.add(
			Instruction::Name::AV2_IN_CAST,
			Instruction::Casting{.dynamic = true}
		);
	}
}

static void doUnsafeCast(Context& context, bool const dyn = false) {
	if (!dyn) {
		auto const type = resolvePath(context);
		if (context.types.contains(type)) {
			context.add(
				Instruction::Name::AV2_IN_CAST,
				Instruction::Casting{.unsafe = true}
			);
			context.add(context.getType(type)->id);
		} else context.error("Type with this name does not exist!");
	} else {
		context.add(
			Instruction::Name::AV2_IN_CAST,
			Instruction::Casting{.dynamic = true, .unsafe = true}
		);
	}
}

static void doRandomNumber(Context& context) {
	Instruction::Randomness rng;
	auto id = context.getNext(LTS_TT_IDENTIFIER, "RNG operation").getString();
	if (id == "setseed")		rng.setSeed					= true;
	else if (id == "getseed")	rng.getSeed					= true;
	else if (id == "reseed")	rng.setSeed = rng.getSeed	= true;
	else if (id == "safe" || id == "fast") {
		rng.secure = id == "safe";
		if (context.getNext(LTS_TT_IDENTIFIER) && context.value().getString() == "bounded")
			rng.bounded = true;
		else context.pad(1);
	} else context.error("Invalid RNG operation!");
}

static Makai::String declareJumpTarget(Context& context) {
	auto const name = resolvePath(context);
	context.jumps[name] = context.program.code.size();
	context.expectNext(LTS_TT_COLON);
	return name;
}

static void doDynamic(Context& context) {
	auto const id = context.getNext(LTS_TT_IDENTIFIER, "dynamic operation").getString();
	if (id == "jump" || id == "go")
		doJump(context, true);
	else if (id == "call" || id == "do")
		doCall(context, true);
	else if (id == "cast" || id == "as")
		doCast(context, true);
	else if (id == "field" || id == "at")
		doField(context, false, true);
	else if (id == "set")
		doField(context, true, true);
	else if (id == "rewrite")
		doUnsafeCast(context, true);
	else if (id == "is")
		doTypeCheck(context, true);
	else context.error("Invalid dynamic operation!");
}

static void doSelect(Context& context) {
	uint32 count = 0;
	auto const inst = context.add(Instruction::Name::AV2_IN_SELECT);
	context.expectNext(LTS_TT_OPEN_BRACKET);
	while (true) {
		if (context.next().has(LTS_TT_CLOSE_BRACKET)) break;
		context.addJumpTarget(resolvePath(context));
		++count;
	}
	if (count < 2)
		context.error("Select must have at least two targets!");
	context.update(inst, count);
}

static void doClear(Context& context) {
	// TODO: this
}

static void doCreate(Context& context, bool const dynamic = false) {
	// TODO: this
}

static void declareTypeFields(Context& context, Context::Declaration& type) {
	if (type.fields.size())
		context.error("Redeclaration of type fields are not allowed!");
	else if (
		type.flags & (
			Definition::Flags::AV2_DF_DYNAMIC
		|	Definition::Flags::AV2_DF_ARRAY
		|	Definition::Flags::AV2_DF_BASIC
		)
	) context.error("Cannot declare fields on basic, array and dynamic types!");
	type.flags |= Definition::Flags::AV2_DF_STRUCTURE;
	context.expectNext(Type{'['});
	while (true) {
		if (context.next().has(Type{']'})) break;
		auto const field = resolvePath(context);
		if (!context.types.contains(field))
			context.error("Field type does not exist!");
		type.fields.pushBack(context.getType(field)->id);
	}
}

static void validateType(Context& context, Context::Declaration& type) {
		DEBUGLN("Name: ", type.name);
		DEBUGLN("Flags: ", type.flags);
	if (type.base && !(type.flags & Definition::Flags::AV2_DF_ARRAY)) {
		auto& base = *context.getTypeByID(type.base);
		type.flags |= base.flags;
		if (type.fields.size())
			type.fields.insert(base.fields, 0);
		else type.fields.appendBack(base.fields);
	}
	if (type.base && context.getTypeByID(*type.base)->flags & Definition::Flags::AV2_DF_FINAL)
		context.error("Final types cannot be inherited from!");
	if (type.alignment && !(type.flags & Definition::Flags::AV2_DF_VALUE))
		context.error("Only value types can have alignment size!");
	if (!type.alignment && type.flags & Definition::Flags::AV2_DF_VALUE)
		context.error("Value types must have an alignment!");
	if (type.flags & Definition::Flags::AV2_DF_VALUE) {
		for (auto& field : type.fields) {
			if (!(context.getTypeByID(field)->flags & Definition::Flags::AV2_DF_VALUE))
				context.error("Value types can only contain other value types!");
			if (context.getTypeByID(field)->flags & Definition::Flags::AV2_DF_ARRAY)
				context.error("Value types cannot contain arrays!");
		}
		if (
			type.flags & Definition::Flags::AV2_DF_ARRAY
		&&	!(context.getTypeByID(*type.base)->flags & Definition::Flags::AV2_DF_VALUE)
		) context.error("Value arrays can only contain value types!");
	}
	if (type.flags & Definition::Flags::AV2_DF_ARRAY && type.fields.size())
		context.error("Arrays cannot contain fields!");
	if (
		type.flags & Definition::Flags::AV2_DF_ARRAY
	&&	type.flags & Definition::Flags::AV2_DF_STRUCTURE
	) context.error("Cannot have a type be both an array or a structure!");
	if (
		type.flags & Definition::Flags::AV2_DF_DYNAMIC
	&&	type.flags & Definition::Flags::AV2_DF_VALUE
	) context.error("Cannot have a dynamic value type!");
	if (
		type.flags & Definition::Flags::AV2_DF_BASIC
	&&	type.flags & (
			Definition::Flags::AV2_DF_STRUCTURE
		|	Definition::Flags::AV2_DF_ARRAY
		|	Definition::Flags::AV2_DF_ART_EQUIVALENT
		|	Definition::Flags::AV2_DF_DYNAMIC
		)
	) context.error("Basic types cannot be structures, arrays, ART-Equivalent, or dynamic types!");
	if (
		type.flags & Definition::Flags::AV2_DF_STRUCTURE
	&&	type.flags & (
			Definition::Flags::AV2_DF_ARRAY
		|	Definition::Flags::AV2_DF_BASIC
		)
	) context.error("Structure types cannot be basic or array types!");
	if (type.flags & Definition::Flags::AV2_DF_EMPTY) {
		if (type.alignment) context.error("Empty types cannot have a size!");
		if (
			type.flags & ~(
				Definition::Flags::AV2_DF_EMPTY
			|	Definition::Flags::AV2_DF_NULLABLE
			|	Definition::Flags::AV2_DF_NO_RESULT
			|	Definition::Flags::AV2_DF_BASIC
			)
		) context.error("Malformed empty type!");
		type.byteSize	= 0;
		type.alignment	= 0;
	}
	if (type.flags & Definition::Flags::AV2_DF_CLONABLE) {
		if (type.flags & Definition::Flags::AV2_DF_ARRAY) {
			if (!(context.getTypeByID(*type.base)->flags & Definition::Flags::AV2_DF_CLONABLE))
				context.error("Clonable arrays must contain clonable elements!");
		} else for (auto const f: type.fields) {
			auto& field = *context.getTypeByID(f);
			if (!(field.flags & Definition::Flags::AV2_DF_CLONABLE))
				context.error("Clonable structures must contain clonable fields!");
		}
	}
	if (type.fields.size() && !(type.flags & Definition::Flags::AV2_DF_STRUCTURE))
		context.error("Fields can only be declared on structures!");
	if (!type.basic) {
		for (auto& field: type.fields)
			type.byteSize += context.getTypeByID(field)->byteSize;
		type.byteSize = (type.byteSize / type.alignment + 1) * type.alignment;
	} else if (type.basic != BasicType::AV2_BT_ANY) {
		type.flags |=
			Definition::Flags::AV2_DF_VALUE
		|	Definition::Flags::AV2_DF_CLONABLE
		|	Definition::Flags::AV2_DF_FINAL
		;
		type.byteSize	= 0;
		type.alignment	= 0;
	}
}

static void declareTypeOperators(Context& context, Context::Declaration& type) {}

static void declareTypeCasts(Context& context, Context::Declaration& type) {}

static void declareTypeMeta(Context& context, Context::Declaration& type) {
	context.expectNext(Type{'['});
	while (true) {
		if (context.next().has(Type{']'})) break;
		auto const key = resolvePath(context);
		if (type.meta.contains(key))
			context.error("Meta attribute has already been declared!");
		type.meta[key] = Makai::FLOW::parse(context.getNext(LTS_TT_DOUBLE_QUOTE_STRING, "Meta attribute value").getString());
	}
}

static void declareType(Context& context) {
	context.next();
	auto const name = resolvePath(context);
	auto const type = new Context::Declaration();
	type->name = name;
	context.expectNext(Type{'['});
	while (true) {
		if (context.next().has(Type{']'})) break;
		auto const flag = context.get(LTS_TT_IDENTIFIER, "type flag").getString();
		if (flag == "basic") {
			type->flags |= Definition::Flags::AV2_DF_BASIC;
			auto const basic =
				context
					.expectNext(Type{'<'})
					.getNext(LTS_TT_IDENTIFIER, "basic type")
					.getString()
			;
			if (basic == "void")		type->basic = BasicType::AV2_BT_VOID;
			else if (basic == "any")	type->basic = BasicType::AV2_BT_ANY;
			else if (basic == "nil")	type->basic = BasicType::AV2_BT_NULL;
			else if (basic == "bool")	type->basic = BasicType::AV2_BT_BOOL;
			else if (basic == "char")	type->basic = BasicType::AV2_BT_CHAR;
			else if (basic == "i8")		type->basic = BasicType::AV2_BT_INT8;
			else if (basic == "i16")	type->basic = BasicType::AV2_BT_INT16;
			else if (basic == "i32")	type->basic = BasicType::AV2_BT_INT32;
			else if (basic == "i64")	type->basic = BasicType::AV2_BT_INT64;
			else if (basic == "u8")		type->basic = BasicType::AV2_BT_UINT8;
			else if (basic == "u16")	type->basic = BasicType::AV2_BT_UINT16;
			else if (basic == "u32")	type->basic = BasicType::AV2_BT_UINT32;
			else if (basic == "u64")	type->basic = BasicType::AV2_BT_UINT64;
			else if (basic == "f32")	type->basic = BasicType::AV2_BT_REAL32;
			else if (basic == "f64")	type->basic = BasicType::AV2_BT_REAL64;
			else if (basic == "f128")	type->basic = BasicType::AV2_BT_REAL128;
			else if (basic == "str")	type->basic = BasicType::AV2_BT_STRING;
			else if (basic == "bin")	type->basic = BasicType::AV2_BT_BYTES;
			else if (basic == "vec")	type->basic = BasicType::AV2_BT_VECTOR;
			else if (basic == "mat")	type->basic = BasicType::AV2_BT_MATRIX;
			else if (basic == "type")	type->basic = BasicType::AV2_BT_TYPEID;
			else context.error("Invalid basic type!");
			context.expectNext(Type{'>'});
		}
		else if (flag == "nil") type->flags |= Definition::Flags::AV2_DF_NULLABLE;
		else if (flag == "derived") {
			if (type->flags & Definition::Flags::AV2_DF_ARRAY)
				context.error("Type cannot be both a derived type and an array type!");
			if (type->base)
				context.error("Redeclaration of base type!");
			context.expectNext(Type{'<'}).next();
			auto const base = resolvePath(context);
			if (context.types.contains(base))
				type->base = context.getType(base)->id;
			else context.error("Base type does not exist!");
			context.expectNext(Type{'>'});
		} else if (flag == "array") {
			if (type->base)
				context.error("Redeclaration of element type!");
			type->flags |= Definition::Flags::AV2_DF_ARRAY;
			context.expectNext(Type{'<'}).next();
			auto const base = resolvePath(context);
			if (context.types.contains(base))
				type->base = context.getType(base)->id;
			else context.error("Element type does not exist!");
			context.expectNext(Type{'>'});
		} else if (flag == "value")	type->flags |= Definition::Flags::AV2_DF_VALUE;
		else if (flag == "empty")	type->flags |= Definition::Flags::AV2_DF_EMPTY;
		else if (flag == "discard")	type->flags |= Definition::Flags::AV2_DF_NO_RESULT;
		else if (flag == "dyn")		type->flags |= Definition::Flags::AV2_DF_DYNAMIC;
		else if (flag == "struct")	type->flags |= Definition::Flags::AV2_DF_STRUCTURE;
		else if (flag == "pack") {
			if (type->alignment)
				context.error("Redefinition of alignment!");
			type->alignment = 1;
		} else if (flag == "align") {
			if (type->alignment)
				context.error("Redefinition of alignment!");
			type->alignment =
				context
					.expectNext(Type{'('})
					.getNext(LTS_TT_INTEGER, "byte alignment")
					.getUnsigned()
			;
			if (!type->alignment)
				context.error("Cannot have empty alignment!");
			context.expectNext(Type{')'});
		} else if (flag == "fields") {
			declareTypeFields(context, *type);
		} else if (flag == "operators") {
			declareTypeOperators(context, *type);
		} else if (flag == "casts") {
			declareTypeCasts(context, *type);
		} else if (flag == "copy")	type->flags |= Definition::Flags::AV2_DF_CLONABLE;
		else if (flag == "bound")	type->flags |= Definition::Flags::AV2_DF_ART_EQUIVALENT;
		else if (flag == "final")	type->flags |= Definition::Flags::AV2_DF_FINAL;
		else if (flag == "meta")
			declareTypeMeta(context, *type);
		else context.error("Invalid flag!");
	}
	validateType(context, *type);
	if (!context.types.contains(name))
		context.addType(name, type);
	else context.error("Redeclaration of previously-declared type!");
}

static void declareSymbolAlias(Context& context, Makai::Dictionary<Makai::Instance<Context::Reference>>& syms) {
	if (context.has(LTS_TT_IDENTIFIER) && context.value().getString() == "shared") {
		Makai::Instance<Context::Reference> share = share.create();
		context.next();
		auto const name = resolvePath(context);
		if (syms.contains(name))
			context.error("Symbol name is already in use!");
		context.expectNext(Type{':'}).expectNext(Type{'['}).next();
		share->module = resolvePath(context, true);
		context.expectNext(Type{':'}).next();
		share->name = resolvePath(context, true);
		context.expectNext(Type{']'});
		syms[name]	= share;
	} else {
		auto const name = resolvePath(context);
		if (syms.contains(name))
			context.error("Symbol name is already in use!");
		context.expectNext(Type{':'}).next();
		auto const symName = resolvePath(context);
		if (!syms.contains(symName))
			context.error("Symbol to be aliased does not exist!");
		syms[name]	= syms[symName];
	}
}

static void declareAlias(Context& context) {
	auto const what = context.getNext(LTS_TT_IDENTIFIER, "alias type").getString();
	if (what == "type")		declareSymbolAlias(context, context.types);
	else if (what == "fn")	declareSymbolAlias(context, context.methods);
	else context.error("Invalid aliasing!");
}

static void getMethodAttriutes(Context& context, Context::Method& method) {
	while (context.next().has(LTS_TT_IDENTIFIER)) {
		auto const vis = context.get(LTS_TT_IDENTIFIER, "visibility").getString();
		if (vis == "shared")
			method.shared = true;
		else if (vis == "local")
			method.local = true;
		else if (vis == "global")
			method.local = false;
		else if (vis == "art")
			method.shared = false;
		else break;
	}
}

static void declareMethodPrototype(Context& context) {
	auto const method = new Context::Method();
	getMethodAttriutes(context, *method);
	auto id = resolvePath(context);
	if (context.types.contains(id))
		method->retType = context.getType(id)->id;
	else context.error("Return type does not exist!");
	context.expectNext(Type{'('});
	while (true) {
		if (context.next().has(Type{')'})) break;
		id = resolvePath(context);
		if (context.types.contains(id))
			method->argTypes.pushBack(context.getType(id)->id);
		else context.error("Argument type does not exist!");
	}
	context.next();
	auto const name = resolvePath(context);
	if (context.methods.contains(name))
		context.error("Redeclaration of previously-declared method!");
	context.addMethod(name, method);
}

static void declareMethodBody(Context& context) {
	context.next();
	DEBUGLN("Body type: ", Context::Tokenizer::Token::asName(context.token().type));
	DEBUGLN("Body text: ", context.token().text);
	if (context.has(Type{'.'})) {
		if (context.methodStack.empty())
			context.error("Missing method body!");
		auto const method = context.methodStack.popBack();
		method->size = context.program.code.size() - method->size;
	} else {
		auto const name = resolvePath(context);
		if (!context.methods.contains(name))
			context.error("Method prototype does not exist!");
		auto const method = context.getMethod(name);
		if (method->shared || method->out) context.error("Cannot declare a body for a shared/external method!");
		context.methodStack.pushBack(method);
		context.jumps[method->jump] = context.program.code.size();
		method->entrypoint = context.jumps[method->jump];
		method->size = context.program.code.size();
		context.expectNext(LTS_TT_COLON);
	}
}

static void declareNamespaceStart(Context& context) {
	auto const name = resolvePath(context, true);
	context.moduleStack.pushBack(name);
}

static void declareNamespaceEnd(Context& context) {
	if (context.moduleStack.empty())
		context.error("Missing matching namespace opening statement!");
	context.moduleStack.popBack();
}

static void declareNamespace(Context& context) {
	if (context.next().has(Type{'.'}))
		declareNamespaceEnd(context);
	else declareNamespaceStart(context);
}

static void declareModule(Context& context) {
	context.expectNext(LTS_TT_OPEN_BRACKET);
	while (true) {
		if (context.next().has(LTS_TT_CLOSE_BRACKET)) break;
		auto const id = context.get(LTS_TT_IDENTIFIER, "module declaration").getString();
		if (id == "name") {
			if (context.program.name.size())
				context.error("Module name was already defined!");
			context.expectNext(Type{':'}).next();
			auto const name = resolvePath(context, true);
		};
	}
}

Makai::Instance<Module> resolveModule(Context& context, Makai::String const& name) {
	if (!context.linkedModules.contains(name))
		context.error("Required module [" + name + "] was not linked!");
	return context.linkedModules[name];
}

static void resolveImport(Context& context, Makai::String const& name) {
	auto const mod = resolveModule(context, name);
	for (auto& type: mod->sym.types)
		if (!type.source) context.addExternalType(
			mod->name,
			type.name,
			new Context::Declaration(mod->detail.types[type.id])
		);
	for (auto& method: mod->sym.methods)
		if (!method.source) context.addExternalMethod(
			mod->name,
			method.name,
			new Context::Method(mod->detail.methods[method.id])
		);
}

static void declareImport(Context& context) {
	context.next();
	auto const name = resolvePath(context, true);
	resolveImport(context, name);
}

static void declareEntry(Context& context) {
	if (context.entry.size())
		context.error("Redeclaration of previously-declared entry!");
	context.next();
	context.entry = resolvePath(context);
}

static void declareExit(Context& context) {
	if (context.entry.size())
		context.error("Redeclaration of previously-declared exit!");
	context.next();
	context.entry = resolvePath(context);
}

static void declareHook(Context& context) {
	context.next();
	auto const hook = resolvePath(context);
	declareJumpTarget(context);
	if (context.program.ani->in.contains(hook))
		context.error("Redeclaration of previously-declared hook!");
	context.program.ani->in[hook] = context.jumps[hook];
}

static void doDeclaration(Context& context) {
	auto const decl = context.getNext(LTS_TT_IDENTIFIER, "declaration type").getString();
	if (decl == "hook")
		declareHook(context);
	else if (decl == "def")
		declareMethodBody(context);
	else if (decl == "fn")
		declareMethodPrototype(context);
	else if (decl == "type")
		declareType(context);
	else if (decl == "ns")
		declareNamespace(context);
	else if (decl == "alias")
		declareAlias(context);
	else if (decl == "module")
		declareModule(context);
	else if (decl == "import")
		declareImport(context);
	else if (decl == "entry")
		declareEntry(context);
	else if (decl == "exit")
		declareExit(context);
	else if (decl == "target") {
		context.next();
		declareJumpTarget(context);
	} else context.error("Invalid declaration!");
}

static void doExpression(Context& context) {
	if (context.has(Type{'@'}))
		return doDeclaration(context);
	auto const id = context.get(LTS_TT_IDENTIFIER, "instruction name").getString();
	if (id == "jump" || id == "go")				doJump(context);
	else if (id == "dynamic" || id == "dyn")	doDynamic(context);
	else if (id == "nop" || id == "next")		doNoOp(context);
	else if (id == "swap")						doStackSwap(context);
	else if (id == "flush")						doStackFlush(context);
	else if (id == "push")						doStackPush(context);
	else if (id == "pop")						doStackPop(context);
	else if (id == "clear")						doStackClear(context);
	else if (id == "blit")						doStackBlit(context);
	else if (id == "return" || id == "ret")		doReturn(context);
	else if (id == "terminate" || id == "stop")	doHalt(context);
	else if (id == "error")						doHalt(context, true);
	else if (id == "call" || id == "do")		doCall(context);
	else if (id == "compare" || id == "cmp")	doCompare(context);
	else if (id == "enter" || id == "begin")	doScopeEnter(context);
	else if (id == "exit" || id == "end")		doScopeExit(context);
	else if (id == "bind")						doScopeBind(context);
	else if (id == "bring")						doScopeBring(context);
	else if (id == "keep")						doScopeKeep(context);
	else if (id == "decl")						doScopeDeclare(context);
	else if (id == "copy")						doCopy(context);
	else if (id == "context" || id == "mode")	{context.next(); doContext(context);}
	else if (id == "loose" || id == "strict")	doContext(context, true);
	else if (id == "operator" || id == "op")	doOperation(context);
	else if (id == "field" || id == "at")		doField(context, false);
	else if (id == "set")						doField(context, true);
	else if (id == "count")						doSizeOf(context);
	else if (id == "size")						doSizeOf(context, true);
	else if (id == "type")						doTypeGet(context);
	else if (id == "is")						doTypeCheck(context);
	else if (id == "yield")						doYield(context);
	else if (id == "cast" || id == "as")		doCast(context);
	else if (id == "rewrite")					doUnsafeCast(context);
	else if (id == "random" || id == "rng")		doRandomNumber(context);
	else if (id == "select" || id == "pick")	doSelect(context);
	else if (id == "unbind" || id == "drop")	doClear(context);
	else if (id == "create" || id == "new")		doCreate(context);
	else context.error("Invalid instruction!");
}

void Minima::invoke() {
	while (!context.empty()) {
		doExpression(context);
		context.next();
	}
	context.finalize();
	context.methods.filter(
		[] (auto const& lhs, auto const& rhs) {
			return lhs.value != rhs.value;
		}
	).filter(
		[this] (auto const& e) -> bool {
			return e.value->module.empty() && !context.getMethod(e.value->name)->local;
		}
	);
	context.types.filter(
		[] (auto const& lhs, auto const& rhs) {
			return lhs.value != rhs.value;
		}
	).filter(
		[] (auto const& e) -> bool {
			return e.value->module.empty();
		}
	);
	context.program.detail.types.resize(context.types.size());
}

Makai::Anima::V2::Core::Module Minima::assemble(UTF8String const& fname, UTF8String const& file) {
	Makai::Lexer::CStyle::TokenStream stream;
	stream.open(file);
	Makai::List<Assembler::BaseContext::Axiom> ax;
	Assembler::Minima::Context ctx;
	while (stream.next())
		ax.pushBack({stream.current(), true, fname});
	if (!stream.ok())
		throw Makai::Error::InvalidValue(
			"Parsing failure!",
			stream.error().value().what
		);
	ctx.put(ax);
	Assembler::Minima minAsm(ctx);
	minAsm.invoke();
	return ctx.program;
}
