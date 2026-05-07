#ifndef CTL_IO_FORMAT_H
#define CTL_IO_FORMAT_H

#include "../namespace.hpp"
#include "../container/strings/strings.hpp"
#include "../algorithm/strconv.hpp"
#include "../typetraits/enum.hpp"

CTL_NAMESPACE_BEGIN

namespace ANSI {
	struct [[gnu::packed]] Code {
		enum class Format: uint16 {
			ACF_NONE		= 0,
			ACF_BOLD		= 1 << 0,
			ACF_DIM			= 1 << 1,
			ACF_ITALIC		= 1 << 2,
			ACF_UNDERLINE	= 1 << 3,
			ACF_BLINK		= 1 << 4,
			ACF_FLASH		= 1 << 5,
			ACF_INVERT		= 1 << 6,
			ACF_HIDE		= 1 << 7,
			ACF_STRIKE		= 1 << 8
		};

		enum class Font: uint8 {
			ACF_DEFAULT_FONT,
			ACF_ALT_FONT_0,
			ACF_ALT_FONT_1,
			ACF_ALT_FONT_2,
			ACF_ALT_FONT_3,
			ACF_ALT_FONT_4,
			ACF_ALT_FONT_5,
			ACF_ALT_FONT_6,
			ACF_ALT_FONT_7,
			ACF_ALT_FONT_8,
			ACF_ALT_GOTHIC_FONT,
		};

		enum class Color: uint8 {
			ACC_BLACK,
			ACC_RED,
			ACC_GREEN,
			ACC_YELLOW,
			ACC_BLUE,
			ACC_MAGENTA,
			ACC_CYAN,
			ACC_WHITE,
			ACC_LIGHT_BLACK,
			ACC_LIGHT_RED,
			ACC_LIGHT_GREEN,
			ACC_LIGHT_YELLOW,
			ACC_LIGHT_BLUE,
			ACC_LIGHT_MAGENTA,
			ACC_LIGHT_CYAN,
			ACC_LIGHT_WHITE,
		};

		Format	format:	9	= Format::ACF_NONE;
		Font	font:	4	= Font::ACF_DEFAULT_FONT;
		Color	text:	4	= Color::ACC_WHITE;
		Color	bg:		4	= Color::ACC_BLACK;

		constexpr String toString() const {
			String out = "\033[0;";
			auto fmt = enumcast(format);
			usize code = 1;
			if (fmt) do {
				if (fmt & 1) out += ::CTL::toString(code) + ";";
				++code;
			} while (fmt >> 1);
			out += "38;";
			auto color = enumcast(text) + 30;
			if (color > 37) color += 53;
			out += ::CTL::toString(color, ";");
			out += "38;";
			color = enumcast(bg) + 40;
			if (color > 47) color += 53;
			out += ::CTL::toString(color, ";");
			return out + "m";
		}
	};

	constexpr Code::Format operator|(Code::Format const a, Code::Format const b) {
		return Code::Format{enumcast(a) | enumcast(b)};
	}

	constexpr Code::Format operator&(Code::Format const a, Code::Format const b) {
		return Code::Format{enumcast(a) & enumcast(b)};
	}

	constexpr Code::Format operator~(Code::Format const a) {
		return Code::Format{~enumcast(a)};
	}

	constexpr Code const NONE = Code{};

	constexpr String format(String const& str, Code const& code) {
		return code.toString() + str + NONE.toString();
	}

	template <Code C>
	constexpr String format(String const& str) {
		return format(str, C);
	}
}

CTL_NAMESPACE_END

#endif
