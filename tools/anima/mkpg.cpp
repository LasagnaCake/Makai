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
		if (name == "embed")
			return Makai::File::getText(vars);
		else if (name == "image")
			return
				"data:image/"
			+	Makai::OS::FS::fileExtension(vars)
			+	";base64,"
			+	Makai::Data::encode(
					Makai::File::getBinary(vars),
					Makai::Data::EncodingType::ET_BASE64
				)
			;
		else if (name == "url")
			return env["project"]["url"].getString() + "/" + vars;
		else if (name == "page")
			return env["project"]["url"].getString() + "/" + vars + ".html";
		else throw Makai::Error::NonexistentValue("Invalid action '" + name + "'!");
	}

	Makai::String processExternalVariable(Makai::UTF8String const& var, Makai::Data::Value const& env) {
		auto const proc = var.rfind('$');
		auto const name = Makai::Regex::replace(var, R"(^\$\$?)", "");
		if (proc == 0) return env["page_meta"][name].getString();
		return env["project"][name].getString();
	}

	void doPage(Makai::UTF8String const& page, Makai::Data::Value env) {
		auto const pdat = Makai::FLOW::parse(page);
		auto const html = pdat["html"];
		Makai::Data::Value components;
		if (pdat.contains("import"))
			for (auto& imp: pdat["import"].getArray()) {
				if (!imp.isString()) continue;
				auto const path = imp.getString();
				if (!cache.contains(path))
					cache[path] = Makai::File::getFLOW(path + ".mp");
				auto const compName = cache["path"].fetch("name", Makai::OS::FS::fileName(path, true));
				if (components.contains(compName))
					throw Makai::Error::NonexistentValue("Component with name '" + compName + "' was already defined!");
				else env["page_meta"][compName] = compName;
			}
		if (pdat.contains("default"))
			for (auto const& [k, v]: pdat["default"].items())
				if (!env["local"].contains(k))
					env["local"][k] = v;
		auto subs = Makai::Regex::find(html, R"(\{\{.*?\}\})");
		auto expr = Makai::Regex::replace(html, R"(\{\{.*?\}\})", "\a").split({'\a'}).reverse();
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
			auto const bldat = block.splitAtFirst(' ');
			auto const type = block.find('/');
			Makai::String blname;
			Makai::Data::Value blparams;
			blname = bldat.front();
			if (bldat.size() > 2)
				blparams = Makai::FLOW::parse("{" + bldat.back() + "}");
			auto newEnv = env;
			newEnv["local"] = blparams;
			if (components.contains(blname)) switch (type) {
				case (-1):	doPage(components[blname].fetch<Makai::String>("html_begin", ""), newEnv);	break;
				case (0):	doPage(components[blname].fetch<Makai::String>("html_end", ""), newEnv);	break;
				default:	doPage(components[blname].fetch<Makai::String>("html", ""), newEnv);		break;
			} else throw Makai::Error::NonexistentValue("Component '" + blname + "' does not exist!");
			output += expr.popBack();
		}
	}

	UTF8String							output;
	UTF8Dictionary<Makai::Data::Value>	cache;
};

struct MakePageMain: AMain {
	PageProcessor proc;

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

	void write(Makai::String const& what) const override {DEBUGLN(what);}

	void buildFolder(OS::FS::FileTree::Entry const& contents, Makai::Data::Value const& env) {
		auto const outDir = env["output"].getString();
		for (auto& content: contents) {
			proc.output.clear();
			if (content.isFolder()) buildFolder(content, env);
			else proc.doPage(content.path() + ".mp", env);
			Makai::File::saveText(Makai::OS::FS::concatenate(outDir, content.path()), proc.output);
		}
	}

	static bool		isString(Makai::Data::Value const& e)	{return e.isString();	}
	static String	getString(Makai::Data::Value const& e)	{return e.getString();	}

	void doBuild() {
		auto const proj = Makai::File::getFLOW("project.flow");
		StringList const dirs =
			proj
				.fetch("build", Makai::Data::Value::ArrayType())
				.filter(isString)
				.toList<String>(getString)
		;
		Makai::Data::Value env;
		env["project"] = proj;
		for (auto const& dir: dirs)
			buildFolder(
				OS::FS::FileTree::getStructure(dir),
				env
			);
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
