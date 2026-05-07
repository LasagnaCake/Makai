#ifndef CTL_IO_FORMAT_H
#define CTL_IO_FORMAT_H

#include "../namespace.hpp"
#include "../container/strings/strings.hpp"
#include "../algorithm/strconv.hpp"

CTL_NAMESPACE_BEGIN

namespace ANSI {
	namespace TextCode {
		struct Code: Streamable<char>, SelfIdentified<Code> {
			using Streamable		= ::CTL::Streamable<char>;
			using SelfIdentified	= ::CTL::SelfIdentified<Code>;

			using
				typename SelfIdentified::SelfType
			;

			using
				typename Streamable::InputStreamType,
				typename Streamable::OutputStreamType
			;

			/// @brief Stream insertion operator.
			constexpr OutputStreamType& operator<<(OutputStreamType& o) const							{o << "\033[" << code << "m"; return o;			}
			/// @brief Stream insertion operator.
			friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType const& self)	{o << "\033[" << self.code << "m"; return o;	}

			String toString() const {return ::CTL::toString("\033[", code, "m");}

			uint8 code;
		};

		constexpr static Code const NONE			= {0};
		constexpr static Code const BOLD			= {1};
		constexpr static Code const DIM				= {2};
		constexpr static Code const ITALIC			= {3};
		constexpr static Code const UNDERLINE		= {4};
		constexpr static Code const BLINK			= {5};
		constexpr static Code const FLASH			= {6};
		constexpr static Code const INVERT			= {7};
		constexpr static Code const HIDE			= {8};
		constexpr static Code const STRIKE			= {9};
		constexpr static Code const DEFAULT_FONT	= {10};
		constexpr static Code const ALT_FONT_0		= {11};
		constexpr static Code const ALT_FONT_1		= {12};
		constexpr static Code const ALT_FONT_2		= {13};
		constexpr static Code const ALT_FONT_3		= {14};
		constexpr static Code const ALT_FONT_4		= {15};
		constexpr static Code const ALT_FONT_5		= {16};
		constexpr static Code const ALT_FONT_6		= {17};
		constexpr static Code const ALT_FONT_7		= {18};
		constexpr static Code const ALT_FONT_8		= {19};
		constexpr static Code const GOTHIC_FONT		= {20};

		constexpr static Code const BLACK_TEXT		= {30};
		constexpr static Code const BLACK_BG		= {40};

		constexpr static Code const RED_TEXT		= {31};
		constexpr static Code const RED_BG			= {41};

		constexpr static Code const GREEN_TEXT		= {32};
		constexpr static Code const GREEN_BG		= {42};

		constexpr static Code const YELLOW_TEXT		= {33};
		constexpr static Code const YELLOW_BG		= {43};

		constexpr static Code const BLUE_TEXT		= {34};
		constexpr static Code const BLUE_BG			= {44};

		constexpr static Code const MAGENTA_TEXT	= {35};
		constexpr static Code const MAGENTA_BG		= {45};

		constexpr static Code const CYAN_TEXT		= {36};
		constexpr static Code const CYAN_BG			= {46};

		constexpr static Code const WHITE_TEXT		= {37};
		constexpr static Code const WHITE_BG		= {47};

		constexpr static Code const BRIGHT_BLACK_TEXT	= {90};
		constexpr static Code const BRIGHT_BLACK_BG		= {100};

		constexpr static Code const BRIGHT_RED_TEXT		= {91};
		constexpr static Code const BRIGHT_RED_BG		= {101};

		constexpr static Code const BRIGHT_GREEN_TEXT	= {92};
		constexpr static Code const BRIGHT_GREEN_BG		= {102};

		constexpr static Code const BRIGHT_YELLOW_TEXT	= {93};
		constexpr static Code const BRIGHT_YELLOW_BG	= {103};

		constexpr static Code const BRIGHT_BLUE_TEXT	= {94};
		constexpr static Code const BRIGHT_BLUE_BG		= {104};

		constexpr static Code const BRIGHT_MAGENTA_TEXT	= {95};
		constexpr static Code const BRIGHT_MAGENTA_BG	= {105};

		constexpr static Code const BRIGHT_CYAN_TEXT	= {96};
		constexpr static Code const BRIGHT_CYAN_BG		= {106};

		constexpr static Code const BRIGHT_WHITE_TEXT	= {97};
		constexpr static Code const BRIGHT_WHITE_BG		= {107};
	}
}

CTL_NAMESPACE_END
