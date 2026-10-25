#ifndef MAKAILIB_EX_GAME_DIALOG_CORE_H
#define MAKAILIB_EX_GAME_DIALOG_CORE_H

#include <makai/makai.hpp>

/// @brief Game dialog extensions.
namespace Makai::Ex::Game::Dialog {
	struct TextContent {
		UTF8String title;
		UTF8String text;
	};

	struct IAnimatable {
		virtual ~IAnimatable() {}

		virtual usize play(usize const animation) {}
	};

	struct IPresentable {
		virtual ~IPresentable() {}

		virtual usize enter()		= 0;
		virtual usize exit()		= 0;
		virtual usize present()		= 0;
		virtual usize withdraw()	= 0;
	};

	struct ITextDisplay {
		virtual ~ITextDisplay() {}

		virtual usize title(UTF8String const& content)		= 0;
		virtual usize text(UTF8String const& content)		= 0;
		virtual usize display(TextContent const& content)	= 0;
	};

	struct ITalkative {
		virtual ~ITalkative() {}

		virtual usize say(UTF8String const& content) = 0;
		virtual usize add(UTF8String const& content) = 0;
	};

	struct IPerformable {
		virtual ~IPerformable() {}

		virtual usize perform(String const& action) = 0;
	};

	struct ITransformable {
		virtual ~ITransformable() {}

		virtual void setTransform(Transform3D const& trans) = 0;
		virtual Transform3D getTransform() const = 0;
	};

	struct IEmotive {
		virtual ~IEmotive() {}

		virtual usize emote(String const& emotion) = 0;
	};

	struct ICostumed {
		virtual ~ICostumed() {}

		virtual usize change(String const& costume) = 0;
	};

	struct IBox:	IVisible, ITransformable, IAnimatable, IPresentable, ITextDisplay, ITalkative	{};
	struct IActor:	IBox, IPerformable, IEmotive, ICostumed											{};
}

#endif
