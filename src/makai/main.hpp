#ifndef MAKAILIB_MAIN_H
#define MAKAILIB_MAIN_H

#include "makai.hpp"

namespace Makai {
	/// @brief Supplied main() structure.
	struct AMain {
		/// @brief Whether to show a dialog box on error.
		bool showDialogOnError	= true;
		/// @brief Base arguments to add to main function.
		Data::Value baseArgs	= Data::Value::object();
		/// @brief CLI parser.
		CLI::Parser& cli;

		/// @brief Initializes main.
		/// @param parser Command-line parser.
		AMain(CLI::Parser& parser): cli(parser) {}

		/// @brief Destructor.
		virtual ~AMain() {}

		/// @brief Called when program runs.
		/// @param args Arguments passed to the program, as parsed by the CLI parser.
		virtual void run(Data::Value const& args) = 0;

		/// @brief Wrapper.
		inline int main() try {
			if (Makai::CPP::Debug::hasDebugger())
				Makai::CPP::Debug::Traceable::trap = true;
			run(cli.parse(baseArgs));
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

		/// @brief Actual main implementation.
		template <Type::Derived<AMain> TApp>
		static inline int main(int argc, char** argv) {
			Makai::CLI::Parser parser{static_cast<usize>(argc), argv};
			TApp app{parser};
			return app.main();
		}
	};
}

/// @brief Implements main.
#define Makai_bindMain(MAIN_APP_TYPE) int main(int argc, char** argv) {return Makai::Main::main<MAIN_APP_TYPE>(argc, argv);}

#endif
