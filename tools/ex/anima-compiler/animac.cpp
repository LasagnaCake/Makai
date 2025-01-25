#include <makai/makai.hpp>
#include <anima/compiler.hpp>

void compile(Makai::String const src, Makai::String const& out) {
	try {
		Makai::String const file = Makai::File::getText(src);
		try {
			Makai::Ex::AVM::Compiler::compileSourceToFile(file, out, Makai::OS::FS::fileName(src));
		} catch (Makai::Error::Generic const& e) {
			auto const lines = file.split('\n');
			usize ccount = 0, linei = 0;
			usize eline = 0;
			if (e.line != "unspecified") {
				DEBUGLN("'",e.line,"'");
				eline = CTL::toUInt64(e.line.stripped(), 10);
				for (auto const& line: lines) {
					if ((ccount + line.size()) >= eline) break;
					ccount += line.size() + 1;
					++linei;
				}
			}
			DEBUGLN("\n<error>\n");
			if (e.line != "unspecified") {	
				constexpr usize DISPLAY_SIZE = 80 - 8;
				usize c = eline - ccount;
				Makai::String dl = lines[linei];
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
				DEBUGLN("^");
				if (c) for (usize i = 0; i < c-1; ++i)
					DEBUG(" ");
				DEBUG("Here");
				DEBUGLN("\n");
				DEBUGLN("FILE: ", e.caller);
				DEBUGLN("LINE: ", linei);
				DEBUGLN("COLUMN: ", eline - ccount, "\n");
			}
			else DEBUGLN("\n");
			DEBUGLN(e.type, ": ", e.message, "\n");
			DEBUGLN(e.info, "\n");
			DEBUGLN("</error>\n");
		}
	} catch (Makai::Error::Generic const& e) {
		DEBUGLN("\n<error>");
		DEBUGLN(e.type, ": ", e.message);
		DEBUGLN("</error>\n");
	}
}

void compile(Makai::String const& src) {
	Makai::String const out = Makai::OS::FS::concatenate(
		Makai::OS::FS::directoryFromPath(src),
		Makai::OS::FS::fileName(src, true) + ".anb"
	);
	compile(src, out);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		DEBUGLN("Anima Compiler Binary Exeutable V0.1");
		DEBUGLN("Usage:");
		DEBUGLN("\n    animac.exe \"path/to/source\"");
		DEBUGLN("    animac.exe \"path/to/source\" \"path/to/output\"");
		DEBUGLN("\nIf output path is not specified, will be placed source directory, with a name of \"<file-name>.anb\".");
	}
	else if (argc < 3)	compile(argv[1]);
	else				compile(argv[1], argv[2]);
	return 0;
}