#include "core.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Runtime;
using namespace Makai::Anima::V2::Toolchain::Compiler;
using namespace Makai::Anima::V2::Toolchain::Assembler;

Project Project::deserializeV2(Data::Value const& value) {
	Project proj;
	auto const type = value["type"].get<String>();
	if (type == "executable" || type == "exe")		proj.type = Type::AV2_TC_PT_EXECUTABLE;
	else if (type == "program" || type == "prg")	proj.type = Type::AV2_TC_PT_PROGRAM;
	else if (type == "module" || type == "mod")		proj.type = Type::AV2_TC_PT_MODULE;
	proj.main.path = value["main"].get<String>();
	proj.sources.pushBack("");
	for (auto path: value["sources"].get<Data::Value::ArrayType>())
		proj.sources.pushBack(path.get<String>());
	for (auto path: value["modules"].get<Data::Value::ArrayType>())
		if (path.isString())
			proj.modules.pushBack({path.get<String>(), "latest"});
		else if (path.isObject()) {
			// TODO: This
		}
	return proj;
}

void downloadModules(AAssembler::Context& context, Project const& project) {
	
}

Program Makai::Anima::V2::Toolchain::Compiler::buildProject(Project const& proj) {
	AAssembler::Context context;
	context.fileName = proj.main.path;
	context.sourcePaths = proj.sources;
	downloadModules(context, proj);
	context.stream.open(proj.main.source);
	Unique<AAssembler> assembler;
	switch (proj.main.type) {
		case Project::File::Type::AV2_TC_PFT_BREVE:		assembler.bind(new Breve(context));		break;
		case Project::File::Type::AV2_TC_PFT_MINIMA:	assembler.bind(new Minima(context));	break;
	}
	assembler->assemble();
	if (proj.main.type == Project::File::Type::AV2_TC_PFT_BREVE) {
		AAssembler::Context asmContext;
		asmContext.stream.open(context.compose());
		assembler.bind(new Minima(asmContext));
		assembler->assemble();
		context.program = asmContext.program;
	}
	return context.program;
}