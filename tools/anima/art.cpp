#include <makai/makai.hpp>
#include <makai/main.hpp>

constexpr auto const VER = Makai::Data::Version{1};

DEFINE_ERROR_TYPE_EX(EngineError, FailedAction);

struct ARTE: Makai::Anima::V2::Runtime::Engine {
	struct BuiltinAPI {
		bool console	= false;
		bool time		= false;
		bool shell		= false;
	};

	BuiltinAPI bapi;

	AV2Call
	static void write_string(Makai::String const& str) {
		printf("%s", str.cstr());
	}

	AV2Call
	static void writeLine_string(Makai::String const& str) {
		printf("%s\n", str.cstr());
	}

	AV2Call
	static Makai::String toString(Makai::Data::Value const& val) {
		if (val.isUndefined()) return "";
		if (val.isString()) return val.getString();
		return val.toFLOWString();
	}

	AV2Call
	static void write_any(Makai::Data::Value const& what) {
		write_string(toString(what));
	}

	AV2Call
	static void writeLine_any(Makai::Data::Value const& what) {
		writeLine_string(toString(what));
	}

	AV2Call
	static int64 procNow(int64 const precision) {
		namespace Time = Makai::OS::Time;
		switch (precision) {
			case -2:	return Time::Clock::sinceStart<Time::Hours>();
			case -1:	return Time::Clock::sinceStart<Time::Minutes>();
			case 0:		return Time::Clock::sinceStart<Time::Seconds>();
			case +1:	return Time::Clock::sinceStart<Time::Millis>();
			case +2:	return Time::Clock::sinceStart<Time::Micros>();
			case +3:	return Time::Clock::sinceStart<Time::Nanos>();
			default:	return Makai::Limit::MAX<int64>;
		}
	}

	AV2Call
	static int64 localNow(int64 const precision) {
		namespace Time = Makai::OS::Time;
		switch (precision) {
			case -2:	return Time::Clock::sinceEpoch<Time::Hours>();
			case -1:	return Time::Clock::sinceEpoch<Time::Minutes>();
			case 0:		return Time::Clock::sinceEpoch<Time::Seconds>();
			case +1:	return Time::Clock::sinceEpoch<Time::Millis>();
			case +2:	return Time::Clock::sinceEpoch<Time::Micros>();
			case +3:	return Time::Clock::sinceEpoch<Time::Nanos>();
			default:	return Makai::Limit::MAX<int64>;
		}
	}

	AV2Call
	static int64 utcNow(int64 const precision) {
		using Zone = Makai::Zone;
		auto const local = localNow(precision);
		if (local == Makai::Limit::MAX<int64>) return local;
		auto secs = int64(local * Makai::Math::pow<double>(10, -precision));
		secs -= Zone::convert(secs, Zone::current(), Zone::utc());
		return local + int64(secs * Makai::Math::pow<double>(10, precision));
	}

	inline static Makai::Mutex threadEditLock;
	inline static Makai::List<Makai::AtomicCell<Makai::Thread>> processes;

	static bool AV2Call cd(Makai::String const& str) {
		#ifdef CTL_ON_WINDOWS
		//return _wchdir(str.cstr()) != -1;
		return false;
		#else
		return chdir(str.cstr()) != -1;
		#endif
	}

	using ProcessResult = Makai::Anima::V2::Core::Promise<int64>;

	static nulltype handleExec(Makai::AtomicCell<Makai::Thread> self, ProcessResult out, Makai::String command, Makai::StringList args) {
		out.set(Makai::OS::launch(command, "", args));
		threadEditLock.lock();
		processes.eraseLike(self);
		threadEditLock.unlock();
		return null;
	}

	static ProcessResult AV2Call exec(Makai::Anima::V2::Core::Context& context, Makai::String const& command, Makai::StringList const& args) {
		auto output = context.promise<int64>();
		auto const thread = Makai::AtomicCell<Makai::Thread>::create();
		threadEditLock.lock();
		processes.pushBack(thread);
		threadEditLock.unlock();
		thread->invoke<nulltype>(handleExec, thread, output, command, args);
		return output;
	}

	ARTE(
		bool const allowDynlibs	= false,
		BuiltinAPI const bapi	= BuiltinAPI{false, false, false}
	): Engine(Config{allowDynlibs}), bapi(bapi) {
	}

	void onBreakpoint() override {
		printf("<break>\n");
		printf("  <instruction id='%Zu' />\n", context.pointers.instruction);
		printf("  <stack size='%Zu'>\n", context.globalValueStack.size());
		for (auto& val: context.globalValueStack) {
			auto const e = val->toDynamicValue().toString();
			printf("    <value>%s</value>\n", e.cstr());
		}
		printf("  </stack>\n");
		printf("  <scope-stack size='%Zu'/>\n", context.scopeStack.size());
		printf("</break>\n");
		Engine::onBreakpoint();
	}

	void onLoad() override {
		if (bapi.console) {
			context.art.addNativeCall("av2/console/write_string", 		write_string		);
			context.art.addNativeCall("av2/console/write_any",			write_any			);
			context.art.addNativeCall("av2/console/writeLine_string",	writeLine_string	);
			context.art.addNativeCall("av2/console/writeLine_any",		writeLine_any		);
		}
		if (bapi.time) {
			context.art.addNativeCall("av2/time/localNow",	localNow	);
			context.art.addNativeCall("av2/time/utcNow",	utcNow		);
			context.art.addNativeCall("av2/time/procNow",	procNow		);
		}
		if (bapi.shell) {
			context.art.addNativeCall("av2/shell/exec",	exec	);
			context.art.addNativeCall("av2/shell/cd",	cd		);
		}
	}
};

struct ARTEMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]				= false;
		cfg["allow-dynlibs"]	= false;
		cfg["binary-first"]		= false;
		cfg["script"]			= false;
		cfg["add-sources"]		= cfg.array();
		cfg["bapi:console"]		= false;
		cfg["bapi:time"]		= false;
		cfg["bapi:shell"]		= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
		tl["DL"]	= "allow-dynlibs";
		tl["B"]		= "binary-first";
		tl["S"]		= "script";
		tl["i"]		= "add-sources";
		tl["BA:C"]	= "bapi:console";
		tl["BA:T"]	= "bapi:time";
		tl["BA:S"]	= "bapi:shell";
	}

	ARTEMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	void write(Makai::String const& what) const override {printf("%s", what.cstr());}

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
			writeLine("art <program> [-BA:C] [-BA:T] [-BA:S] [-DL] [-B] [-S]");
		} else {
			ARTE engine{
				args["allow-dynlibs"].getBoolean(),
				{
					args["bapi:console"].getBoolean(),
					args["bapi:time"].getBoolean(),
					args["bapi:shell"].getBoolean()
				}
			};
			Makai::Anima::V2::Core::Module file;
			if (!args.fetch("script", false)) {
				static auto const ext = Makai::StringList::from(".anp", ".anpb");
				Makai::String fpath = args["__args"][0].getString();
				if (Makai::OS::FS::exists(fpath + ext[args.fetch("binary-first", false)]))
					file = Makai::File::getFLOW(fpath + ext[args.fetch("binary-first", false)]);
				else Makai::Anima::V2::Core::BinaryFormat::fromBytes(Makai::File::getBinary(fpath + ext[!args.fetch("binary-first", false)]))
					.then([&] (auto const e) {file = e;})
					.onError([&] (auto const e) {throw Makai::Error::FailedAction(e.message, CTL_CPP_PRETTY_SOURCE);})
				;
			} else {
				using namespace Makai::Anima::V2::Toolchain;
				Compiler::Breve::Transformer::Import::importer
					= [args] (auto const path) ->
						Compiler::Breve::File {
						auto dirs = Makai::StringList::from("/");
						dirs.appendBack(args["add-sources"].getArray().toList<Makai::String>());
						static Makai::Dictionary<Compiler::Breve::File> cache;
						if (path.empty()) throw Makai::Error::FailedAction("Module name is empty!");
						if (cache.contains(path)) return cache[path];
						if (Makai::OS::FS::exists(path + ".bv"))
							return cache[path] = Compiler::Breve::parseFile(path + ".bv", Makai::File::getText(path + ".bv"));
						auto const brevecDir = Makai::OS::FS::sourceLocation() + "/anima/breve/lib";
						if (Makai::OS::FS::exists(brevecDir + "/" + path + ".bv"))
							return cache[path] = Compiler::Breve::parseFile(brevecDir + "/" + path + ".bv", Makai::File::getText(brevecDir + "/" + path + ".bv"));
						for (auto& dir: dirs)
							if (Makai::OS::FS::exists(dir + "/" + path + ".bv"))
								return cache[path] = Compiler::Breve::parseFile(dir + "/" + path + ".bv", Makai::File::getText(dir + "/" + path + ".bv"));
						throw Makai::Error::FailedAction("Failed to find module '" + path + "'");
					}
				;
				Makai::String fpath;
				Makai::String fdata;
				if (args.contains("pipe")) {
					fpath="<<stdin>>.bv";
					fdata = args["pipe"].getString();
				} else {
					fpath = args["__args"][0].getString();
					fdata = Makai::File::getText(fpath);
				}
				if (Makai::Regex::contains(fpath, R"re(\.bv$)re"))			file = Compiler::Breve::compile(fpath, fdata);
				else if (Makai::Regex::contains(fpath, R"re(\.min$)re"))	file = Assembler::Minima::assemble(fpath, fdata);
				else [[unlikely]] {
					auto const ext = Makai::Regex::findFirst(fpath, R"(\.(\w+)$)");
					if (!ext)
						throw Makai::Error::InvalidValue(
							"Invalid file extension!",
							CTL_CPP_PRETTY_SOURCE
						);
					Makai::String const moduleName = "lang." + Makai::Regex::replace(ext.value().match, R"re(\.)re", "") + ".builder";
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
												err.line,
												err.file
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
