#include <makai/makai.hpp>
#include <makai/main.hpp>
#include <iostream>

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

constexpr auto const VER = Makai::Data::Version{1};

DEFINE_ERROR_TYPE_EX(EngineError, FailedAction);

struct ARTE: Makai::Anima::V2::Runtime::Engine {
	bool cliEnabled = false;

	static void AV2Call write_string(Makai::String const& str) {
		doWrite(str);
	}

	static void AV2Call writeLine_string(Makai::String const& str) {
		doWriteLine(str);
	}

	static Makai::String AV2Call toString(Makai::Data::Value const& val) {
		if (val.isUndefined()) return "";
		if (val.isString()) return val.getString();
		return val.toFLOWString();
	}

	static void AV2Call write_any(Makai::Data::Value const& what) {
		write_string(toString(what));
	}

	static void AV2Call writeLine_any(Makai::Data::Value const& what) {
		writeLine_string(toString(what));
	}

	ARTE(
		bool const allowDynlibs	= false,
		bool const cliEnabled	= false
	): Engine(Config{allowDynlibs}), cliEnabled(cliEnabled) {
	}

	void onLoad() override {
		if (cliEnabled) {
			context.art.addExternalMethod("av2/console/write_string", 		write_string		);
			context.art.addExternalMethod("av2/console/write_any",			write_any			);
			context.art.addExternalMethod("av2/console/writeLine_string",	writeLine_string	);
			context.art.addExternalMethod("av2/console/writeLine_any",		writeLine_any		);
		}
	}
};

struct ARTEMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]				= false;
		cfg["cli"]				= false;
		cfg["allow-dynlibs"]	= false;
		cfg["binary-first"]		= false;
		cfg["script"]			= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
		tl["C"]		= "cli";
		tl["DL"]	= "allow-dynlibs";
		tl["B"]		= "binary-first";
		tl["S"]		= "script";
	}

	ARTEMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	void write(Makai::String const& what) const override {doWrite(what);}

	static ARTE::Error handleError(ARTE::Error const& e) {
		throw EngineError(Makai::toString(
			"!!! ERROR !!!",
			"\n", "At bytecode offset ", e.location,
			"\n", "At instruction ", Makai::Anima::V2::Core::Instruction::asString(e.instruction.name),
			"\n", "Message: [", e.message, "]"
		));
	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			writeLine("Anima RunTime - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("art <program> [-C] [-DL] [-B] [-S]");
		} else {
			ARTE engine{
				args["allow-dynlibs"].getBoolean(),
				args["cli"].getBoolean()
			};
			Makai::Anima::V2::Core::Module file;
			if (!args.fetch("script", false)) {
				static Makai::StringList const ext = Makai::StringList::from(".anp", ".anpb");
				Makai::String fpath = args["__args"][0].getString() + ext[args.fetch("binary-first", false)];
				if (Makai::OS::FS::exists(fpath))
					file = Makai::File::getFLOW(fpath);
				else Makai::Anima::V2::Core::BinaryFormat::fromBytes(Makai::File::getBinary(fpath + ext[!args.fetch("binary-first", false)]))
					.then([&] (auto const e) {file = e;})
					.onError([&] (auto const e) {throw Makai::Error::FailedAction(e.message, CTL_CPP_PRETTY_SOURCE);})
				;
			} else {
				Makai::String fpath = args["__args"][0].getString();
				auto const fdata = Makai::File::getText(fpath);
				if (Makai::Regex::contains(fpath, R"re(\.bv$)re"))			file = Makai::Anima::V2::Toolchain::Compiler::Breve::compile(fpath, fdata);
				else if (Makai::Regex::contains(fpath, R"re(\.min$)re"))	file = Makai::Anima::V2::Toolchain::Assembler::Minima::assemble(fpath, fdata);
				else [[unlikely]] {
					auto const ext = Makai::Regex::findFirst(fpath, R"(\.(\w+)$)");
					if (ext.match.empty())
						throw Makai::Error::InvalidValue(
							"Invalid file extension!",
							CTL_CPP_PRETTY_SOURCE
						);
					Makai::String const moduleName = "lang." + Makai::Regex::replace(ext.match, R"re(\.)re", "") + ".builder";
					static auto const moduleSources = Makai::StringList::from(
						Makai::OS::FS::currentDirectory() + "/",
						Makai::OS::FS::sourceLocation() + "/module/art/",
						Makai::OS::FS::sourceLocation() + "/module/3p/"
					);
					bool hit = false;
					for (auto& src: moduleSources) {
						if (Makai::OS::FS::exists(src + moduleName)) {
							hit = true;
							Makai::CPP::Library modlib;
							modlib.open(src + moduleName);
							auto const getBuilder = modlib.function<ref<Makai::Anima::V2::Runtime::ARTModule::IBuilder>()>(
								Makai::Anima::V2::Runtime::ARTModule::BUILDER_FN_NAME
							);
							if (!getBuilder)
								throw Makai::Error::FailedAction(
									"Improper module (missing builder function)!",
									CTL_CPP_PRETTY_SOURCE
								);
							else if (auto const builder = getBuilder()) {
								builder.value()->build(fdata)
									.then([&] (auto const& bin) {
										Makai::Anima::V2::Core::BinaryFormat::fromBytes(bin)
											.then([&] (auto const e) {file = e;})
											.onError([&] (auto const e) {throw Makai::Error::FailedAction(e.message, CTL_CPP_PRETTY_SOURCE);})
										;
									}).onError([&] (auto const& err) {
										throw Makai::Error::FailedAction(
											"Failed to build file!",
											err.message,
											Makai::CPP::SourceFile(
												"COLUMN " + Makai::toString(err.column),
												err.file,
												err.line
											)
										);
									})
								;
							} else throw Makai::Error::FailedAction(
								"Improper module (missing builder function)!",
								CTL_CPP_PRETTY_SOURCE
							);
						}
					}
					if (!hit)
						throw Makai::Error::InvalidValue(
							"Invalid file extension!",
							CTL_CPP_PRETTY_SOURCE
						);
				}
			}
			engine.load(file);
			engine.execute();
			DEBUGLN("<art:output>");
			while (engine.process()) {
				DEBUGLN("Frame!");
			}
			DEBUGLN("</art:output>");
			engine.error().then(handleError);
		}
	}
};

Makai_bindMain(ARTEMain);
