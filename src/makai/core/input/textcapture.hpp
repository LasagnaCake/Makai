#ifndef MAKAI_CORE_INPUT_TEXTCAPTURE_H
#define MAKAI_CORE_INPUT_TEXTCAPTURE_H

#include "../../compat/ctl.hpp"
#include "../extern.hpp"
#include "../display.hpp"

/// @brief User input facilities.
namespace Makai::Input {
	struct TextCapture;

	using ATextCaptureObject = CTL::Ex::APeriodic<TextCapture, String const&>;

	struct TextCapture: IConstValue<String>, ATextCaptureObject {
		void clear();
		static void begin();
		static void end();
		static bool capturing();
		String value() const override final;

	private:
		void onUpdate(String const& text) override final;

		String buffer;
	};
}

#endif
