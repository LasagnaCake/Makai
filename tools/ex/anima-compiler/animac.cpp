#include <makai/makai.hpp>
#include <makai-ex/anima/compiler.hpp>

#define CONCAT(A,B) #A #B

#define ANSI(CODE) "\x1b["#CODE"m"

// FG_RESET	39
// BG_RESET	39
// FG_RED	31
// BOLD		1

constexpr auto CONSOLE_INVERT	= ANSI(7);

constexpr auto CONSOLE_BLACK	= ANSI(30);

constexpr auto CONSOLE_RED		= ANSI(31);
constexpr auto CONSOLE_GREEN	= ANSI(32);
constexpr auto CONSOLE_YELLOW	= ANSI(33);
constexpr auto CONSOLE_BLUE		= ANSI(34);
constexpr auto CONSOLE_CYAN		= ANSI(35);
constexpr auto CONSOLE_MAGENTA	= ANSI(36);

constexpr auto CONSOLE_L_BLACK		= ANSI(90;2);

constexpr auto CONSOLE_L_RED		= ANSI(91);
constexpr auto CONSOLE_L_GREEN		= ANSI(92);
constexpr auto CONSOLE_L_YELLOW		= ANSI(93);
constexpr auto CONSOLE_L_BLUE		= ANSI(94);
constexpr auto CONSOLE_L_CYAN		= ANSI(95);
constexpr auto CONSOLE_L_MAGENTA	= ANSI(96);
constexpr auto CONSOLE_L_WHITE		= ANSI(97);

constexpr auto CONSOLE_BG_BLACK		= ANSI(40);
constexpr auto CONSOLE_BG_RED		= ANSI(41);
constexpr auto CONSOLE_BG_GREEN		= ANSI(42);
constexpr auto CONSOLE_BG_YELLOW	= ANSI(43);
constexpr auto CONSOLE_BG_BLUE		= ANSI(44);
constexpr auto CONSOLE_BG_CYAN		= ANSI(45);
constexpr auto CONSOLE_BG_MAGENTA	= ANSI(46);

constexpr auto CONSOLE_BG_L_RED		= ANSI(101);
constexpr auto CONSOLE_BG_L_GREEN	= ANSI(102);
constexpr auto CONSOLE_BG_L_YELLOW	= ANSI(103);
constexpr auto CONSOLE_BG_L_BLUE	= ANSI(104);
constexpr auto CONSOLE_BG_L_CYAN	= ANSI(105);
constexpr auto CONSOLE_BG_L_MAGENTA	= ANSI(106);
constexpr auto CONSOLE_BG_L_WHITE	= ANSI(107);

constexpr auto CONSOLE_BOLD			= ANSI(1);
constexpr auto CONSOLE_RESET		= ANSI(0;97);
constexpr auto CONSOLE_TRUE_RESET	= ANSI(0);

int compile(Makai::String const src, Makai::String const& out) {
	try {
		Makai::String const file = Makai::File::getText(src);
		try {
			Makai::Ex::AVM::Compiler::compileSourceToFile(file, out, Makai::OS::FS::fileName(src));
		} catch (Makai::Error::Generic const& e) {
			usize eline = 0;
			DEBUGLN(CONSOLE_RED, CONSOLE_BOLD, "\n<error>\n", CONSOLE_RESET);
			// This is jank, but it works, so not that big of a deal for me
			if (e.line != "unspecified") {
				auto const ff = file.replaced('\t', ' ');
				eline = CTL::toUInt64(e.line.stripped(), 10);
				//DEBUGLN("'", eline, "'");
				ssize const lhs = ff.sliced(0, eline).rfind('\n'), rhs = ff.sliced(eline).find('\n');
				usize linei = ff.sliced(0, eline).split('\n').size();
				constexpr usize DISPLAY_SIZE = 80 - 8;
				usize c = eline - lhs;
				Makai::String dl = rhs < 0 ? ff.sliced(lhs > 1 ? lhs : 0) : ff.substring(lhs > 1 ? lhs : 0, rhs + 2);
				bool prepend = c > DISPLAY_SIZE;
				while (c > DISPLAY_SIZE) {
					c -= DISPLAY_SIZE;
					dl = dl.substring(DISPLAY_SIZE);
				}
				bool postpend = dl.size() > DISPLAY_SIZE;
				dl = dl.substring(0, DISPLAY_SIZE);
				if (prepend) DEBUG("... ");
				DEBUG(dl);
				if (postpend) DEBUG(" ...");
				DEBUGLN("");
				if (c) for (usize i = 0; i < c-1; ++i)
					DEBUG(" ");
				DEBUG(CONSOLE_L_RED);
				DEBUGLN("^");
				if (c) for (usize i = 0; i < c-1; ++i)
					DEBUG("~");
				DEBUG("Here", CONSOLE_RESET);
				DEBUGLN("\n");
				DEBUG(CONSOLE_L_YELLOW, CONSOLE_BOLD);
				DEBUGLN("FILE:", CONSOLE_RESET, " ", e.caller);
				DEBUG(CONSOLE_L_YELLOW, CONSOLE_BOLD);
				DEBUGLN("LINE:", CONSOLE_RESET, " ", linei);
				DEBUG(CONSOLE_L_YELLOW, CONSOLE_BOLD);
				DEBUGLN("COLUMN:", CONSOLE_RESET, " ", eline - lhs, "\n");
			}
			else DEBUGLN("\n");
			DEBUG(CONSOLE_L_RED, CONSOLE_BOLD);
			DEBUGLN(e.type, ":", CONSOLE_RESET, " ", CONSOLE_L_WHITE, e.message, "\n", CONSOLE_RESET);
			if (e.info != "none") DEBUGLN(e.info, "\n");
			DEBUGLN(CONSOLE_RED, CONSOLE_BOLD, "</error>\n", CONSOLE_TRUE_RESET);
			return 2;
		}
	} catch (Makai::Error::Generic const& e) {
		DEBUGLN(CONSOLE_RED, "\n<error>\n", CONSOLE_RESET);
		DEBUG(CONSOLE_L_RED, CONSOLE_BOLD);
		DEBUGLN(e.type, ":", CONSOLE_RESET, " ", CONSOLE_L_WHITE, e.message, "\n", CONSOLE_RESET);
		DEBUGLN(CONSOLE_RED, CONSOLE_BOLD, "</error>\n", CONSOLE_TRUE_RESET);
		return 1;
	}
	return 0;
}

int compile(Makai::String const& src) {
	Makai::String const out = Makai::OS::FS::concatenate(
		Makai::OS::FS::directoryFromPath(src),
		Makai::OS::FS::fileName(src, true) + ".anb"
	);
	return compile(src, out);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		DEBUGLN("Anima Compiler Binary Exeutable V0.1");
		DEBUGLN("Usage:");
		DEBUGLN("\n    animac.exe \"path/to/source\"");
		DEBUGLN("    animac.exe \"path/to/source\" \"path/to/output\"");
		DEBUGLN("\nIf output path is not specified, will be placed source directory, with a name of \"<file-name>.anb\".");
	}
	else if (argc < 3)	return compile(argv[1]);
	else				return compile(argv[1], argv[2]);
	return 0;
}