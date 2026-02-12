#ifndef MAKAILIB_MAIN_H
#define MAKAILIB_MAIN_H

#include "makai.hpp"

/// @brief Simplified main function.
struct Main {
	/// @brief Whether to show a dialog box on error.
	inline static bool showDialogOnError		= true;
	/// @brief Base arguments to add to main function.
	inline static Makai::Data::Value baseArgs	= Makai::Data::Value::object();

	/// @brief Called when program runs.
	/// @param args Arguments passed to program.
	static void run(Makai::Data::Value const& args);

	/// @brief Actual main implementation.
	inline static int run(int argc, char** argv) try {
		if (Makai::CPP::Debug::hasDebugger())
			Makai::CPP::Debug::Traceable::trap = true;
		Makai::CLI::Parser parser{argc, argv};
		run(parser.parse(baseArgs));
		return 0;
	} catch (Makai::Error::Generic const& e) {
		if (showDialogOnError)
			Makai::Popup::showError(e.report());
		else DEBUGLN(e.what());
		return -1;
	} catch (Makai::Exception const& e) {
		if (showDialogOnError)
			Makai::Popup::showError(e.what());
		else DEBUGLN(e.what());
		return -1;
	}
};

/// @brief Implements the function.
#define main(ARGS_NAME) int main(int argc, char** argv) {return Main::run(argc, argv);} void Main::run(Makai::Data::Value const& ARGS_NAME)

#endif
