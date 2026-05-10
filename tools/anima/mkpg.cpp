#include <makai/makai.hpp>
#include <makai/main.hpp>
#include "base.cc"

using namespace Makai;

constexpr auto const VER = Makai::Data::Version{1};

constexpr auto const PROJBASE = R"###(
{
	type "doc"
	build ["page"]
	pack ["res"]
	url "**{{url}}"
	output "output/www"
}
)###";

struct PageProcessor {
	Makai::String processAction(Makai::UTF8String const& act, Makai::Data::Value const& env) {
		auto const name = Makai::Regex::findFirst(act, "@.*?:").match;
		auto const vars = Makai::Regex::replace(act, "@.*?:", "");
		if (name == "@embed:")
			return Makai::File::getText(vars);
		else if (name == "@image:")
			return
				"data:image/"
			+	Makai::OS::FS::fileExtension(vars)
			+	";base64,"
			+	Makai::Data::encode(
					Makai::File::getBinary(vars),
					Makai::Data::EncodingType::ET_BASE64
				)
			;
		else if (name == "@file:")
			return env["project"]["url"].getString() + "/" + vars;
		else if (name == "@page:")
			return env["project"]["url"].getString() + "/" + vars + ".html";
		else throw Makai::Error::NonexistentValue("Invalid action '" + name + "'!");
	}

	Makai::String processExternalVariable(Makai::UTF8String const& var, Makai::Data::Value const& env) {
		auto const proc = var.rfind('$');
		auto const name = Makai::Regex::replace(var, R"(^\$\$?)", "");
		if (proc == 0) {
			if (env["page_meta"].contains(name))
				return env["page_meta"][name].getString();
			else throw Makai::Error::NonexistentValue("Page attribute '" + name + "' does not exist!");
		} else if (env["project"]["vars"].contains(name))
			return env["project"]["vars"][name].getString();
		else throw Makai::Error::NonexistentValue("Project variable '" + name + "' does not exist!");
	}

	void doPage(Makai::UTF8String const& page, Makai::Data::Value env) {
		DEBUGLN("Page {");
		DEBUGLN(page);
		DEBUGLN("}");
		auto const pdat = env["page_meta"];
		Makai::Data::Value components;
		if (pdat.contains("import"))
			for (auto& imp: pdat["import"].getArray()) {
				if (!imp.isString()) continue;
				auto const path = imp.getString();
				DEBUGLN("Importing '", path, "'...");
				if (!cache.contains(path))
					cache[path] = Makai::FLOW::parse("{" + Makai::File::getText(path + ".mp") + "}");
				auto const compName = cache[path].fetch("name", Makai::OS::FS::fileName(path, true));
				if (components.contains(compName))
					throw Makai::Error::NonexistentValue("Component with name '" + compName + "' was already defined!");
				else components[compName] = cache[path];
			}
		if (pdat.contains("default"))
			for (auto const& [k, v]: pdat["default"].items())
				if (!env["local"].contains(k))
					env["local"][k] = v;
		auto subs = Makai::Regex::find(page, R"(\{\{.*?\}\})");
		auto expr = Makai::Regex::replace(page, R"(\{\{.*?\}\})", "\a").split({'\a'}).reverse();
		Makai::UTF8String buf = expr.popBack();
		for (auto& sub: subs) {
			auto const var = Makai::Regex::replace(sub.match, R"(\{\{|\}\})", "");
			if (var.front() == '@')
				buf += processAction(var, env);
			else if (var.front() == '$')
				buf += processExternalVariable(var, env);
			else if (env["local"].contains(buf))
				buf += env["local"][var].getString();
			else throw Makai::Error::NonexistentValue("Missing value for variable '" + var + "'!");
			buf += expr.popBack();
		}
		subs = Makai::Regex::find(buf, R"(<<.*?>>)");
		expr = Makai::Regex::replace(buf, R"(<<.*?>>)", "\a").split({'\a'}).reverse();
		output += expr.popBack();
		for (auto& sub: subs) {
			auto const block = Makai::Regex::replace(sub.match, R"(<<|>>)", "");
			DEBUGLN("Component: ", block);
			auto const bldat = block.splitAtFirst(' ');
			auto type = block.rfind('/');
			if (type > 0 && type < (block.size() - 1))
				type = block.find('/');
			Makai::String blname;
			Makai::Data::Value blparams;
			blname = bldat.erase(type);
			if (bldat.size() > 2)
				blparams = Makai::FLOW::parse("{" + bldat.back() + "}");
			auto newEnv = env;
			if (!components.contains(blname))
				throw Makai::Error::NonexistentValue("Component '" + blname + "' does not exist!");
			auto const blinfo = components[blname];
			newEnv["local"] = blparams;
			newEnv["page_meta"] = blinfo;
			DEBUGLN("Vars: ", blparams.toFLOWString());
			if (type == (block.size() - 1))	doPage(blinfo.fetch<Makai::String>("html_end"), newEnv);
			else if (type == 0)				doPage(blinfo.fetch<Makai::String>("html_begin"), newEnv);
			else							doPage(blinfo.fetch<Makai::String>("html"), newEnv);
			output += expr.popBack();
		}
	}

	UTF8String							output;
	UTF8Dictionary<Makai::Data::Value>	cache;
};

struct MakePageMain: AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]	= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]	= "help";
	}

	MakePageMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	void write(Makai::String const& what) const override {DEBUG(what);}

	void buildFolder(Makai::StringList const& folders, Makai::Data::Value const& env) {
		auto const outDir = env["project"]["output"].getString();
		for (auto& folder: folders) {
			{
				auto const files = Makai::OS::FS::filesIn(folder);
				for (auto const& file: files)
					if (Makai::OS::FS::exists(file) && Makai::OS::FS::fileExtension(file) == "mp") {
						writeLine("Building: ", file);
						Makai::Data::Value pageEnv = env;
						auto const page = Makai::FLOW::parse("{" + Makai::File::getText(file) + "}");
						pageEnv["page_meta"] = page;
						PageProcessor proc;
						proc.doPage(page["html"].getString(), pageEnv);
						auto basePath =
							Regex::replace(
								Regex::replace(file, "[\\\\/]", "/"),
								"\\.mp",
								".html"
							)
						;
						auto const dir = basePath.find('/');
						writeLine("AAAA: ", basePath);
						writeLine("At: ", dir);
						if (dir != -1) basePath = basePath.sliced(dir);
						auto const outPath = Makai::OS::FS::concatenate(outDir, basePath);
						writeLine("Saving to: ", outPath);
						Makai::File::saveText(outPath, proc.output);
					}
			}
			buildFolder(Makai::OS::FS::foldersIn(folder), env);
		}
	}

	static bool		isString(Makai::Data::Value const& e)	{return e.isString();	}
	static String	getString(Makai::Data::Value const& e)	{return e.getString();	}

	void doBuild() {
		auto const proj = Makai::File::getFLOW("project.flow");
		StringList const dirs =
			proj["build"]
				.getArray()
				.filter(isString)
				.toList<String>(getString)
		;
		Makai::Data::Value env;
		env["project"] = proj;
		buildFolder(dirs, env);
	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			writeLine("MakePage - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("mkpg <action>");
		} else {
			if (args["__args"].size() < 1)
				throw Error::FailedAction("Expected action to follow 'mkpg'!");
			auto const act = args["__args"][0].getString();
			if (act == "build") doBuild();
		}
	}
};

Makai_bindMain(MakePageMain)

// TODO: This (again)
