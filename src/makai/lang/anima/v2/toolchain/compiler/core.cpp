#include "core.hpp"
#include "../../../../../net/net.hpp"
#include "../../../../../tool/tool.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Runtime;
using namespace Makai::Anima::V2::Toolchain::Compiler;
using namespace Makai::Anima::V2::Toolchain::Assembler;

SourceResolver resolver;

void Makai::Anima::V2::Toolchain::Compiler::setModuleSourceResolver(SourceResolver const& r) {
	resolver = r;
}

Project Project::deserializeV1(Project& proj, Data::Value const& value) {
	auto const type = value["type"].get<String>();
	if (value.contains("name"))
		proj.name = value["name"].get<Makai::String>();
	else proj.name = "project";
	if (type == "executable" || type == "exe")		proj.type = Type::AV2_TC_PT_EXECUTABLE;
	else if (type == "program" || type == "prg")	proj.type = Type::AV2_TC_PT_PROGRAM;
	else if (type == "module" || type == "mod")		proj.type = Type::AV2_TC_PT_MODULE;
	auto const mode = value["mode"].get<String>("cli");
	if (mode == "console" || mode == "cli")			proj.mode = Mode::AV2_TC_PM_CONSOLE;
	else if (mode == "window" || mode == "win")		proj.mode = Mode::AV2_TC_PM_WINDOW;
	else if (mode == "worker" || mode == "bg")		proj.mode = Mode::AV2_TC_PM_WORKER;
	proj.main.path = value["main"].get<String>();
	proj.sources.pushBack(String(""));
	for (auto path: value["sources"].get<Data::Value::ArrayType>())
		if (!value.isString()) continue;
		else proj.sources.pushBack(path.get<String>());
	if (value["modules"].isObject()) {
		for (auto [name, ver]: value["modules"].get<Data::Value::ObjectType>()) {
			if (ver.isFalsy()) continue;
			if (Regex::contains(name, "^https?:\\/\\/"))
				proj.modules.pushBack({name, ver});
			else resolver(proj, name, ver);
		}
	}
	return proj;
}

static void downloadModules(AAssembler::Context& context, Project const& project, String const& root);

void buildModule(AAssembler::Context& context, Project const& proj, String const& root) {
	context.sourcePaths.appendBack(
		proj.sources.transformed(
			[&] (String const& source) {
				return root + "/module/" + proj.name + "/" + source;
			}
		)
	);
	downloadModules(context, proj, root + "/module/" + proj.name);
}

void Makai::Anima::V2::Toolchain::Compiler::fetchModule(
	AAssembler::Context& context,
	Project const& project,
	Project::Module const& module,
	String const& root,
	Data::Value& cache
) {
	auto const info = Net::HTTP::fetch(
		module.source, {
			.type = Net::HTTP::Request::Type::MN_HRT_GET,
			.data = module.version
		}
	);
	if (info.status != Net::HTTP::Response::Status::MN_HRS_OK)
		throw Error::FailedAction("Failed to fetch module from source '"+module.source+"'!");
	else {
		auto const data = FLOW::parse(info.content);
		auto package		= data["package"].get<FLOW::Value::ByteListType>();
		auto const name		= data["name"].get<String>();
		MemoryBuffer membuf{Cast::rewrite<ref<char>>(package.data()), package.size()};
		Tool::Arch::FileArchive arch{membuf};
		auto modpath = cache["modules"][cache["modules"].size()];
		modpath = root + "/module/" + name;
		arch.unpackTo(modpath);
		context.sourcePaths.pushBack(modpath);
		auto modproj = Project::deserialize(Makai::File::getFLOW(OS::FS::absolute(modpath.get<String>() + "/project.flow")));
		if (modproj.language.major > project.language.major)
			throw Error::InvalidValue("Module language major version is greater than main project language major version!");
		modproj.type = decltype(modproj.type)::AV2_TC_PT_MODULE;
		modproj.name = name;
		buildModule(context, modproj, root + "/module/" + name);
	}
}

static void downloadModules(AAssembler::Context& context, Project const& project, String const& root) {
	if (OS::FS::exists("cache.flow")) {
		auto const cache = File::getFLOW(OS::FS::absolute(root + "/cache.flow"));
		for (auto module: cache["modules"].get<FLOW::Value::ArrayType>())
			context.sourcePaths.pushBack(module.get<String>());
	} else {
		OS::FS::makeDirectory(OS::FS::absolute(root + "/module"));
		if (project.modules.empty()) return;
		auto cache = FLOW::Value::object();
		cache["modules"] = FLOW::Value::array();
		for (auto& module: project.modules)
			fetchModule(context, project, module, root, cache);
		File::saveText(OS::FS::absolute(root + "/cache.flow"), cache.toFLOWString("\t"));
	}
}

void Makai::Anima::V2::Toolchain::Compiler::downloadProjectModules(AAssembler::Context& context, Project const& proj) {
	downloadModules(context, proj, ".");
}

void Makai::Anima::V2::Toolchain::Compiler::buildProject(AAssembler::Context& context, Project const& proj, bool const onlyUpToIntermediate) {
	context.sourcePaths = proj.sources;
	if (!proj.local) {
		context.sourcePaths.pushBack(Makai::OS::FS::sourceLocation() + "/anima/breve/lib");
		downloadProjectModules(context, proj);
	}
	if (proj.type == Project::Type::AV2_TC_PT_MODULE)
		return;
	else context.fileName = proj.main.path;
	context.program.showCommandLine = proj.mode == Project::Mode::AV2_TC_PM_CONSOLE;
	auto const src = proj.main.source.empty() ? Makai::File::getText(OS::FS::absolute(proj.main.path)) : proj.main.source;
	if (proj.main.type == Project::File::Type::AV2_TC_PFT_MINIMA)
		return build<Minima>(context, src);
	build<Breve>(context, src);
	if (proj.main.type == Project::File::Type::AV2_TC_PFT_BREVE && !onlyUpToIntermediate)
		build<Minima>(context, context.intermediate());
}
