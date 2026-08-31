#include "binary.hpp"
#include "../../../../file/flow.hpp"

using namespace Makai;
using namespace Makai::Anima::V2;
using namespace Makai::Anima::V2::Core::BinaryFormat;

namespace BF = Makai::Anima::V2::Core::BinaryFormat;

Result<Core::Module, BF::Error> BF::fromBytes(Bytes<> const& source) {
	ByteReader reader(source);
	Core::Module out;
	auto const base = FileHeader::build(reader);
	if (!base)
		return Error{"Failed to get file header!"};
	if (auto const header = FileHeader(base.value()).fromBytes(reader)) {
		auto const file = header.value();
		MAKAILIB_DEBUGLN_FULL("Magic bytes [", String(file.magic), "]");
		if (String(file.magic) != "AV2::ANPB")
			return Error{"File is not an ART-compatible binary!"};
		out.entry = file.entry;
		out.type = file.type;
		out.version	= {file.moduleVersion.major,	file.moduleVersion.minor,	file.moduleVersion.patch,	file.moduleVersion.hotfix	};
		out.art		= {file.artVersion.major,		file.artVersion.minor,		file.artVersion.patch,		file.artVersion.hotfix		};
		out.flags = file.moduleFlags;
		MAKAILIB_DEBUGLN_FULL("Getting bytecode...");
		if (auto const code = file.code.fromBytes(reader))
			out.code = code.value();
		else return Error{"Failed to get code!"};
		MAKAILIB_DEBUGLN_FULL("Getting jump table...");
		if (auto const jumps = unpack(file.jumps, reader))
			out.jumpTable = jumps.value();
		else return Error{"Failed to get jump table!"};
		MAKAILIB_DEBUGLN_FULL("Getting relocations...");
		if (auto const relocations = unpack(file.relocations, reader))
			out.relocations = relocations.value();
		else return Error{"Failed to get relocation table!"};
		MAKAILIB_DEBUGLN_FULL("Getting strings...");
		if (auto const strings = unpack<String>(file.strings, reader))
			out.strings = strings.value();
		else return Error{"Failed to get string table!"};
		MAKAILIB_DEBUGLN_FULL("Getting method & type information...");
		usize actualTotalTypes = 0;
		usize actualTotalMethods = 0;
		out.sym.methods.resize(file.totalMethods, {});
		out.sym.types.resize(file.totalTypes, {});
		MAKAILIB_DEBUGLN_FULL("Total methods = [", file.totalMethods, "]");
		MAKAILIB_DEBUGLN_FULL("Total types = [", file.totalTypes, "]");
		if (auto const header = file.module.fromBytes(reader)) {
			auto const symbols = header.value();
			if (auto const header = unpack(symbols.types, reader)) {
				auto const types = header.value();
				actualTotalTypes += types.size();
				for (auto& type: types) {
					auto& sym = out.sym.types[type.id];
					sym.id = out.detail.types.size();
					sym.source = null;
					auto& target = out.detail.types.pushBack({}).back();
					target.id = sym.id;
					if (auto const v = type.name.fromBytes<UTF8String>(reader))
						target.name = v.value();
					else return Error{"Failed to get type name!"};
					target.flags = type.flags;
					target.hash = type.hash;
					if (type.base < Limit::MAX<uint64>)
						target.base = type.base;
					if (type.basic != BasicType::AV2_BT_NOT_A_BASIC_TYPE)
						target.basic = type.basic;
					if (auto const fields = type.fields.fromBytes(reader))
						target.fields = fields.value();
					else return Error{"Failed to get type fields!"};
					target.byteSize = type.byteSize;
					target.alignment = type.alignment;
					target.meta = Makai::FLOW::parse(type.meta.fromBytes<String>(reader));
				}
			} else return Error{"Failed to get type data!"};
			if (auto const header = unpack(symbols.methods, reader)) {
				auto const methods = header.value();
				actualTotalMethods += methods.size();
				for (auto& method: methods) {
					auto& sym = out.sym.methods[method.id];
					sym.id = out.detail.methods.size();
					sym.source = null;
					auto& target = out.detail.methods.pushBack({}).back();
					target.id = sym.id;
					if (auto const v = method.name.fromBytes<UTF8String>(reader))
						target.name = v.value();
					else return Error{"Failed to get type name!"};
					target.flags = method.flags;
					target.retType = method.returnType;
					if (auto const args = unpack(method.argTypes, reader))
						target.argTypes = args.value();
					else return Error{"Failed to get method arguments!"};
					target.entrypoint = method.entry;
					target.size = method.size;
					target.meta = Makai::FLOW::parse(method.meta.fromBytes<String>(reader));
				}
			} else return Error{"Failed to get method data!"};
		} else return Error{"Failed to get module symbol data!"};
		if (auto const header = file.external.fromBytes(reader)) {
			auto const includes = header.value();
			if (auto const header = unpack(includes.modules, reader)) {
				auto const symbols = header.value();
				for (auto& include: symbols) {
					if (auto const header = unpack(include.types, reader)) {
						auto const types = header.value();
						actualTotalTypes += types.size();
						for (auto& type: types) {
							auto& sym = out.sym.types[type.id];
							sym.id = out.detail.types.size();
							sym.source = include.module;
						}
					} else return Error{"Failed to get external type data!"};
					if (auto const header = unpack(include.methods, reader)) {
						auto const methods = header.value();
						actualTotalMethods += methods.size();
						for (auto& method: methods) {
							auto& sym = out.sym.methods[method.id];
							sym.id = out.detail.methods.size();
							sym.source = include.module;
						}
					} else return Error{"Failed to get external method data!"};
				}
			} else return Error{"Failed to get module include data!"};
		} else return Error{"Failed to get external symbol data!"};
		MAKAILIB_DEBUGLN_FULL("Types: [", actualTotalTypes, " = ", out.sym.types.size(), "]");
		if (actualTotalTypes != out.sym.types.size())
			return Error{"Invalid/Missing type data!"};
		MAKAILIB_DEBUGLN_FULL("Types: [", actualTotalMethods, " = ", out.sym.methods.size(), "]");
		if (actualTotalMethods != out.sym.methods.size())
			return Error{"Invalid/Missing method data!"};
		if (auto const header = file.ani.fromBytes(reader)) {
			auto const ani = header.value();
			if (auto const header = unpack(ani.in, reader)) {
				auto const in = header.value();
				for (auto& sig: in)
					if (auto const s = sig.fromBytes<String>(reader))
						out.ani->in[s.value()] = sig.id;
					else return Error{"Failed to get signals"};
			} else return Error{"Failed to get signals!"};
			if (auto const header = unpack<String>(ani.out, reader)) {
				auto const outf = header.value();
				for (auto& fn: outf)
					out.ani->out.pushBack(fn);
			} else return Error{"Failed to get external calls!"};
			if (auto const header = ani.shared.fromBytes(reader)) {
				auto const shared = header.value();
				if (auto const header = unpack<String>(shared.interops, reader)) {
					auto const interops = header.value();
					for (auto& i: interops) out.ani->shared.interops.pushBack(i);
				} else return Error{"Failed to get interoperability features!"};
				if (auto const header = unpack<String>(shared.interops, reader)) {
					auto const modules = header.value();
					for (auto& m: modules) out.ani->shared.modules.pushBack(m);
				} else return Error{"Failed to get shared modules!"};
				if (auto const header = unpack<String>(shared.interops, reader)) {
					auto const dynlibs = header.value();
					for (auto& dy: dynlibs) out.ani->shared.libraries.pushBack(dy);
				} else return Error{"Failed to get shared libraries!"};
			} else return Error{"Failed to get shared declarations!"};
		} else return Error{"Failed to get ANI information!"};
	} else return Error{"Failed to get file header!"};
	return out;
}

struct ExternalMapping {
	List<Record> types;
	List<Record> methods;
};

Result<Bytes<>, BF::Error> BF::toBytes(Core::Module const& module, bool const strip) {
	Bytes<> output;
	ByteWriter writer(output);
	Builder builder(writer);
	builder
		.begin()
		.run(
			[&module, strip] (Builder& builder) {
				MAKAILIB_DEBUGLN_FULL("Adding basic information...");
				builder.file.type			= module.type;
				builder.file.artVersion		= {module.art.major,		module.art.minor,		module.art.patch,		module.art.hotfix		};
				builder.file.moduleVersion	= {module.version.major,	module.version.minor,	module.version.patch,	module.version.hotfix	};
				builder.file.moduleFlags	= module.flags;
				builder.file.entry			= module.entry;
				builder.file.totalTypes		= module.sym.types.size();
				builder.file.totalMethods	= strip ? 0 : module.sym.methods.size();
				builder.file.strings		= {builder.embed(module.strings.toList<String>())};
				builder.file.jumps			= {builder.append(module.jumpTable)};
				builder.file.code			= {builder.append(module.code)};
				builder.file.relocations	= {builder.append(module.relocations)};
				builder.file.ani			= {builder.processIf(
					module.ani.exists(),
					[&module] (Builder& builder) {
						ANI result;
						auto& ani = *module.ani;
						List<Label> labels;
						for (auto const& [label, id]: ani.in) {
							labels.pushBack({{builder.append(label)}});
							labels.back().id = id;
						}
						result.in = {builder.include(labels)};
						result.out = {builder.embed(ani.out.toList<String>())};
						result.shared = {builder.process(
							[&ani] (Builder& builder) {
								Shared shared;
								shared.libraries	= {builder.embed(ani.shared.libraries)};
								shared.modules		= {builder.embed(ani.shared.modules)};
								shared.interops		= {builder.embed(ani.shared.interops)};
								return builder.put(shared);
							}
						)};
						return builder.put(result);
					}
				)};
			}
		)
		.run(
			[&module, strip] (Builder& builder) {
				MAKAILIB_DEBUGLN_FULL("Adding types & methods...");
				List<Decl> moduleTypes;
				List<Method> moduleMethods;
				Map<usize, ExternalMapping> external;
				MAKAILIB_DEBUGLN_FULL("Total types: ", module.sym.types.size());
				for (auto& symbol: module.sym.types) {
					if (symbol.source) {
						external[symbol.source.value()].types.pushBack(
							Record {
								.id		= symbol.id
							}
						);
					} else {
						auto& type = module.detail.types[symbol.id];
						moduleTypes.pushBack(
							Decl {
								.basic		= type.basic ? type.basic.value() : BasicType::AV2_BT_NOT_A_BASIC_TYPE,
								.base		= type.base ? type.base.value() : -1,
								.byteSize	= type.byteSize,
								.alignment	= type.alignment,
								.fields		= {builder.append(type.fields)}
							}
						);
						auto& mt = moduleTypes.back();
						mt.id 		= type.id;
						mt.hash		= type.hash;
						mt.flags	= type.flags;
						if (!strip)
							mt.name	= {builder.append(type.name)};
						if (type.meta.isObject())
							mt.meta	= {builder.append(type.meta.toFLOWString())};
					}
				}
				MAKAILIB_DEBUGLN_FULL("Total methods: ", module.sym.methods.size());
				if (!strip)
					for (auto& symbol: module.sym.methods) {
						if (symbol.source) {
							external[symbol.source.value()].methods.pushBack(
								Record {
									.id		= symbol.id
								}
							);
						} else {
							auto& method = module.detail.methods[symbol.id];
							moduleMethods.pushBack(
								Method {
									.returnType	= method.retType,
									.argTypes	= {builder.append(method.argTypes)},
									.entry		= method.entrypoint,
									.size		= method.size
								}
							);
							auto& mm = moduleMethods.back();
							mm.id 		= method.id;
							mm.hash		= method.hash;
							mm.flags	= method.flags;
							if (!strip)
								mm.name	= {builder.append(method.name)};
							if (method.meta.isObject())
								mm.meta	= {builder.append(method.meta.toFLOWString())};
						}
					}
				builder.file.module = {builder.put(
					Module {
						.types		= {builder.include(moduleTypes)},
						.methods	= {builder.include(moduleMethods)}
					}
				)};
				List<Include> includes;
				for (auto& [id, include]: external)
					includes.pushBack(
						Include {
							.module		= id,
							.types		= {builder.include(include.types)},
							.methods	= {builder.include(include.methods)}
						}
					);
				builder.file.external = {builder.put(
					External {
						.modules = {builder.include(includes)}
					}
				)};
			}
		)
		.end()
	;
	return output;
}
