#ifndef MAKAILIB_VIDEO_CORE_H
#define MAKAILIB_VIDEO_CORE_H

#include "../compat/ctl.hpp"
#include "../image/core.hpp"
#include "../graph/gl/image.hpp"
#include "get.hpp"

namespace Makai::Video::V2D {
	enum class Container {
		MV2F_INVALID = -1,
		MV2F_UNKNOWN,
		/// @brief Quite OK Video.
		MV2F_QOV,
		/// @brief Makai Video Storage & eXcgange format.
		MV2F_MVSX,
	};

	struct Attributes {
		Container container;
	};

	struct ADecoder {
		IVideoStream(BinaryFormat::IReadable& in): in(in) {}

		void reset()					= 0;
		bool finished()					= 0;
		bool nextFrame()				= 0;
		Graph::Image2D& currentFrame()	= 0;

	protected:
		BinaryFormat::IReadable& in;
	};
}
