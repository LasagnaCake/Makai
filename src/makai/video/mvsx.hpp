#ifndef MAKAILIB_VIDEO_MVSX_H
#define MAKAILIB_VIDEO_MVSX_H

#include "../compat/ctl.hpp"
#include "../image/core.hpp"
#include "../graph/color/color.hpp"
#include "get.hpp"

namespace Makai::Video::V2D::MVSX {
	using namespace CTL::Ex::BinaryFormat;

	using Color8 = Graph::Color::Color8;
	using ImageFormat = Image::I2D::Format;

	enum class AudioFormat: uint64 {
		MV2P_INVALID = Limit::MAX<uint64>,
		MV2P_QOA,
		AV2P_MP3,
		AV2P_OGG,
		AV2P_WAV
	};

	enum class Packing: uint64 {
		MV2P_INVALID = Limit::MAX<uint64>,
		/// @brief No packing (frames are stored as-is).
		MV2P_NONE,
		/// @brief Only first frame + difference between frames are stored. Mask is stored in the alpha channel. DOES NOT SUPPORT TRANSPARENCY.
		MV2P_DELTA_ALPHA,
		/// @brief Only first frame + difference between frames are stored. Mask is stored before the actual frame. SUPPORTS TRANSPARENCY.
		MV2P_DELTA_MASKED,
		/// @brief Block-based packing.
		MV2P_BLOCK,
		/// @brief Delta-Alpha storage with block packing.
		MV2P_BLOCK_DELTA_ALPHA,
		/// @brief Delta-Masked storage with block packing.
		MV2P_BLOCK_DELTA_MASKED,
	};

	struct [[CTL_PACKED_STRUCT]] Frame {
		enum class Mode: uint16 {
			/// @brief Mix in region denoted in mask with image contents. (src * mask + dst * (1 - mask))
			MV2_FM_MIX,
			/// @brief Add to region denoted in mask with image contents. (dst + (src * mask))
			MV2_FM_ADD,
			/// @brief Subtract from region denoted in mask with image contents. (dst - (src * mask))
			MV2_FM_SUBTRACT,
			/// @brief Multiply to region denoted in mask with image contents. (mix(dst, src * dst, mask))
			MV2_FM_MULTIPLY,
			/// @brief Divide from region denoted in mask with image contents. (mix(dst, dst / src, mask))
			MV2_FM_DIVIDE,
			/// @brief Take the average from region denoted in mask with image contents. (mix(dst, (dst + src) * 2, mask))
			MV2_FM_AVERAGE,
		};

		struct [[CTL_FLAG_STRUCT(uint64)]] Flags {
			uint64: 0;
		};

		struct [[CTL_PACKED_STRUCT]] Block {
			struct [[CTL_FLAG_STRUCT(uint64)]] Flags {
				uint64 hasSubBlocks:				1;
				uint64 solidColor:					1;
				uint64 backgroundColorIsBaseColor:	1;
			};
			Flags	flags;
			Color8	background;
			Data	data;
		};

		Mode				mode;
		Flags				flags;
		HeaderTable<Block>	blocks;
	};

	struct [[CTL_PACKED_STRUCT]] Audio {
		Text	language;
		Data	data;
	};

	struct [[CTL_PACKED_STRUCT]] Subtitles {
		struct [[CTL_PACKED_STRUCT]] Entry {
			uint64 frameStart, duration;
			Color8 foreground, background;
			uint64 fontID;
			uint64 x, y;
			uint64 width, height;
			int8 hjust: 3;
			int8 vjust: 3;
		};

		Text				language;
		HeaderTable<Entry>	entries;
	};

	struct [[CTL_PACKED_STRUCT]] Header {
		struct [[CTL_PACKED_STRUCT]] Block {
			uint64	log2Length: 8;
		};

		template<class T> struct [[CTL_PACKED_STRUCT]] Section {
			T format;
		};

		scstring<10> const magic = "Makai::VSX";

		uint64		width;
		uint64		height;
		uint64		framerate;

		Packing		packing;

		ImageFormat imageFormat;
		AudioFormat audioFormat;

		HeaderTable<Frame>		video;
		HeaderTable<Audio>		audio;
		HeaderTable<Subtitles>	subtitles;
	};
}
