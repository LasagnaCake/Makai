#ifndef MAKAI_CORE_INPUT_TEXTCAPTURE_H
#define MAKAI_CORE_INPUT_TEXTCAPTURE_H

#include "../../compat/ctl.hpp"
#include "../extern.hpp"
#include "../display.hpp"

/// @brief User input facilities.
namespace Makai::Input {
	/// @brief Text capture.
	struct TextCapture {
		/// @brief Clears the capture buffer.
		static void clear();
		/// @brief Starts capturing text input.
		static void begin();
		/// @brief Stops capturing text input.
		static void end();
		/// @brief Updates text stored in the buffer.
		/// @param text Text to add to the buffer.
		static void update(String const& text);
		/// @brief Returns whether text capturing is active.
		/// @return Whether text is being capture.
		static bool capturing();
		/// @brief Returns the current text in the capture buffer.
		/// @return Capture buffer text.
		static String value();

	private:
		/// @brief Capture buffer.
		static String buffer;
	};
}

#endif
