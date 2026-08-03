#include "project.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler;

Makai::Data::Value Project::serialize() const {
	Makai::Data::Value out;
	out["name"]		= name;
	out["main"]		= main;
	out["src"]		= sources.toList<Makai::Data::Value>();
	out["ver"]		= version;
	out["concerto"]	= concerto;
	out["art"]		= art;
	switch (type) {
		case Type::AV2_TCPT_LIBRARY:		out["type"]	= "lib"; break;
		case Type::AV2_TCPT_WEB_PROGRAM:	out["type"]	= "anp"; break;
		case Type::AV2_TCPT_BIN_PROGRAM:	out["type"]	= "anpb"; break;
		case Type::AV2_TCPT_EXECUTABLE:		out["type"]	= "exe"; break;
	}
	switch (language) {
		case Language::AV2_TCPL_MINIMA:	out["lang"]	= "breve"; break;
		case Language::AV2_TCPL_BREVE:	out["lang"]	= "minima"; break;
	}
	if (!libraries.empty()) {
		out["lib"] = out.object();
		for (auto& [name, lib]: libraries) {
			out["lib"][name] = out.object();
			out["lib"][name]["src"] = lib.source;
			out["lib"][name]["ver"] = lib.version;
		}
	}
	return out;
}

Project Project::deserialize(Makai::Data::Value const& v) {
	Project out;
	out.name = v["name"].toString();
	if (v.contains("main"))
		out.main = v["main"].getString();
	else throw Makai::Error::NonexistentValue("Missing main file path!");
	if (v.contains("ver"))		out.version = v["ver"];
	if (v.contains("art"))		out.art = v["art"];
	else						out.art = {1};
	if (v.contains("concerto"))	out.concerto = v["concerto"];
	else						out.concerto = {1};
	auto const type = v["type"].getString();
	if (type == "lib")			out.type = Type::AV2_TCPT_LIBRARY;
	else if (type == "anp")		out.type = Type::AV2_TCPT_WEB_PROGRAM;
	else if (type == "anpb")	out.type = Type::AV2_TCPT_BIN_PROGRAM;
	else if (type == "exe")		out.type = Type::AV2_TCPT_EXECUTABLE;
	else throw Makai::Error::NonexistentValue("Invali/unsupported project type!");
	auto const lang = v["lang"].getString();
	if (lang == "breve")		out.language = Language::AV2_TCPL_BREVE;
	else if (lang == "minima")	out.language = Language::AV2_TCPL_MINIMA;
	else throw Makai::Error::NonexistentValue("Invali/unsupported project language!");
	if (v.contains("lib")) {
		auto const libraries = v["lib"].keys();
		for (auto& lib: libraries) {
			out.libraries[lib] = {
				v["lib"][lib]["src"].getString(),
				v["lib"][lib]["ver"]
			};
		}
	}
	return out;
}
