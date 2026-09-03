#ifndef MAKAILIB_VIDEO_CORE_H
#define MAKAILIB_VIDEO_CORE_H

#include "../compat/ctl.hpp"
#include "../image/core.hpp"
#include "../graph/gl/image.hpp"

namespace Makai::Video::V2D {
	enum class Container {
		/// @brief Quite OK Video.
		MV2C_QOV,
		/// @brief Makai Video Storage & eXcgange format.
		MV2C_MVSX,
	};

	struct ADecoder {
		struct Info {
			Container container;
			uint64 width = 0, height = 0;
			uint64 frameCount		= 0;
			uint64 frameRate		= 60;
		};

		ADecoder(BinaryFormat::IReadable& in): in(in) {}

		virtual void reset()					= 0;
		virtual bool finished()					= 0;
		virtual bool nextFrame()				= 0;
		virtual Info videoInfo()				= 0;
		virtual Span<byte const> currentFrame()	= 0;

		virtual ~ADecoder() {}

	protected:
		BinaryFormat::IReadable& in;
	};
}

#endif
