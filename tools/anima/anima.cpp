#include <makai/makai.hpp>
#include <makai/main.hpp>

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["net"]		= true;
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"]		= "help";
	tl["N"]		= "net";
	tl["Net"]	= "net";
}

MakaiInit(cli) {
	translationBase(cli.tl);
	baseArgs = configBase();
}

MakaiMain(args) {
	auto const f = Makai::File::getFLOW(args["__args"][0].getString() + ".anp");
	auto const prog = Makai::Anima::V2::Runtime::Program::deserialize(f);
	CTL::OS::launch(
		CTL::OS::FS::sourceLocation()
	+	"/" + (prog.showCommandLine ? "carte" : "warte")
	#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
	+	".exe"
	#endif
		,
		CTL::OS::FS::currentDirectory(),
		Makai::StringList::from(
			"--net", Makai::toString(args["net"].get<bool>()),
			"--help", Makai::toString(args["net"].get<bool>())
		)
	);
}
