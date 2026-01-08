#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H

#include "../../runtime/runtime.hpp"
#include "../assembler/assembler.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Project {
		using Version = Data::Version;

		constexpr static Version const CONCERTO_VER	= {1};
		constexpr static Version const LANG_VER		= Runtime::Program::LANG_VER;

		enum class Type {
			AV2_TC_PT_EXECUTABLE,
			AV2_TC_PT_PROGRAM,
			AV2_TC_PT_MODULE,
		};

		struct File {
			enum class Type {
				AV2_TC_PFT_MINIMA,
				AV2_TC_PFT_BREVE,
			};
			Type	type	= Type::AV2_TC_PFT_BREVE;
			String	path;
			String	source;
		};

		struct Module {
			String version;
			String source;
		};


		String			name;
		Type			type		= Type::AV2_TC_PT_EXECUTABLE;
		File			main;
		StringList		sources;
		List<Module>	modules;

		Version			package, language = LANG_VER, concerto = CONCERTO_VER;
		bool			local		= false;

		static Project deserialize(Data::Value const& value) {
			Project project;
			if (!value.contains("concerto_version")) return deserializeV1(project, value);
			project.concerto = Version::deserialize(value["concerto_version"]);
			switch (project.concerto.major) {
				case 1: return deserializeV1(project, value);
				default: throw Error::InvalidValue(toString("Unsupported concerto version!\nGiven version is ", project.concerto.serialize().toFLOWString()));
			}
		}

		constexpr Data::Value serialize() const {
			Data::Value result;
			result["concerto_version"]	= concerto;
			result["language_version"]	= language;
			result["package_version"]	= package;
			result["name"]	= name;
			switch (type) {
				case Type::AV2_TC_PT_EXECUTABLE:	result["type"] = "executable";	break;
				case Type::AV2_TC_PT_PROGRAM:		result["type"] = "program";		break;
				case Type::AV2_TC_PT_MODULE:		result["type"] = "module";		break;
			}
			result["main"]		= main.path;
			result["sources"]	= Data::Value::array();
			auto& pkgSources = result["sources"];
			for (auto const& src: sources)
				pkgSources[pkgSources.size()] = src;
			result["modules"]	= Data::Value::object();
			auto& pkgModules = result["modules"];
			for (auto const& mod: modules)
				pkgModules[mod.source] = mod.version;
			return result;
		}

		static Project deserializeV1(Project& project, Data::Value const& value);
	};

	using SourceResolver = Functor<void(Project& project, String const& name, String const& ver)>;

	void setModuleSourceResolver(SourceResolver const& resolver);
	
	template<Type::Derived<Assembler::AAssembler> TAsm = Assembler::Breve>
	inline void build(Assembler::Context& context, Makai::String const& source) {
		context.stream.open(source);
		TAsm assembler(context);
		assembler.assemble();
		context.stream.close();
	}
	
	void fetchModule(
		Assembler::Context& context,
		Project const& project,
		Project::Module const& module,
		String const& root,
		Data::Value& cache
	);

	void downloadProjectModules(Assembler::Context& context, Project const& proj);

	void buildProject(Assembler::Context& context, Project const& proj, bool const onlyUpToIntermediate = false);
}

#endif