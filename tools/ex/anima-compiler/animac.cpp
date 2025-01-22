#include <makai/makai.hpp>
#include <anima/compiler.hpp>

int main(int argc, char** argv) {
	if (argc < 2) {
		DEBUGLN("Anima Compiler Binary Exeutable V0.1");
		DEBUGLN("Usage:");
		DEBUGLN("\n    animac.exe \"path/to/source\"");
		DEBUGLN("    animac.exe \"path/to/source\" \"path/to/output\"");
		DEBUGLN("\nIf output path is not specified, will be placed source directory, with a name of \"<file-name>.anb\".");
	}
	else if (argc < 3) {
		try {
			Makai::String const src = argv[1];
			Makai::String out = Makai::OS::FS::fileName(src, true);
			out = Makai::OS::FS::concatenate(
				Makai::OS::FS::directoryFromPath(out),
				out
			);
			Makai::Ex::AVM::Compiler::compileFileToFile(src, out);
		} catch (Makai::Error::Generic const& e) {
			DEBUGLN(e.report());
		}
	}
	else {
		try {
			Makai::String const src = argv[1];
			Makai::String const out = argv[2];
			Makai::Ex::AVM::Compiler::compileFileToFile(src, out);
		} catch (Makai::Error::Generic const& e) {
			DEBUGLN("\n", e.report());
		}
	}
	return 0;
}