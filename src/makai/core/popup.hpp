#ifndef MAKAI_CORE_POPUP_H
#define MAKAI_CORE_POPUP_H

#include "../compat/ctl.hpp"

/// @brief Popup dialog box facilities.
namespace Makai::Popup {
	/// @brief Default popup option buttons.
	namespace Option {
		/// @brief "Ok" button.
		const StringList OK				= {"Ok"};
		/// @brief "Yes" button.
		const StringList YES			= {"Yes"};
		/// @brief "Yes" and "No" button.
		const StringList YES_NO			= {"Yes", "No"};
		/// @brief "Yes", "No" and "Cancel" button.
		const StringList YES_NO_CANCEL	= {"Yes", "No", "Cancel"};
	}

	/// @brief Popup type.
	enum class PopupType {
		PT_INFO,
		PT_WARN,
		PT_ERROR
	};

	/// @brief Invokes a dialog box with a given set of buttons.
	/// @param title Message box title.
	/// @param text Message box contents.
	/// @param options Option buttons. By default, it is only an "Ok" button.
	/// @param type Popup type. By default, it is `PT_INFO`.
	/// @return Selected button, or -1 if none were selected.
	int showDialog(
		String const& title,
		String const& text,
		StringList const& options = Option::OK,
		PopupType const& type = PopupType::PT_INFO
	);

	/// @brief Shows an error popup.
	/// @param what Error message to display.
	void showError(String const& what);
}

#endif // MAKAI_CORE_POPUP_H
