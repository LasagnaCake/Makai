#ifndef MAKAILIB_ANIMA_V2_RUNTIME_LOCATION_H
#define MAKAILIB_ANIMA_V2_RUNTIME_LOCATION_H

#include "basictypes.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Location {
		struct Content {
			Unique<IObject>	val;
			usize			refs	= 1;

			constexpr Content() = default;
		};

		constexpr Location(): content(new Content()) {}

		Location(Location const& other)				{bind(other);				}

		Location& operator=(Location const& other)	{return bind(other);		}

		Location& bind(Location const& other);

		Location& unbind();

	private:
		owner<Content> content = nullptr;
	};
}

#endif
