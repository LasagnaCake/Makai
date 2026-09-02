#ifndef MAKAILIB_VIDEO_MVSX_H
#define MAKAILIB_VIDEO_MVSX_H

#include "../compat/ctl.hpp"
#include "../image/image.hpp"
#include "../audio/audio.hpp"
#include "../graph/graph.hpp"
#include "core.hpp"

namespace Makai::Video::V2D::MVSX {
	using namespace CTL::Ex::BinaryFormat;

	using Color8 = Graph::Color::Color8;
	using ImageFormat = Image::I2D::Format;

	enum class AudioFormat: uint64 {
		MV2P_QOA,
		AV2P_MP3,
		AV2P_OGG,
		AV2P_WAV
	};

	struct [[CTL_PACKED_STRUCT]] Region {
		uint64 x, y;
		uint64 width, height;
	};

	struct [[CTL_PACKED_STRUCT]] Frame {
		enum class Type: uint64 {
			/// @brief Normal frame.
			MV2_FT_NONE,
			/// @brief Derive from previous frame. Mask is stored in the alpha channel. DOES NOT SUPPORT TRANSPARENCY.
			MV2_FT_DELTA_ALPHA,
			/// @brief Derive from previous frame. Mask is stored afrer the delta frame. SUPPORTS TRANSPARENCY.
			MV2_FT_DELTA_MASKED,
		};

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
			/// @brief Take the average from region denoted in mask with image contents. (mix(dst, (dst + src) * 0.5, mask))
			MV2_FM_AVERAGE,
			/// @brief Subtract from image denoted in mask with destination contents. ((src * mask) - dst)
			MV2_FM_REVERSE_SUBTRACT,
			/// @brief Divide from image denoted in mask with destination contents. (mix(src / dst, dst, mask))
			MV2_FM_REVERSE_DIVIDE,
		};

		struct [[CTL_FLAG_STRUCT(uint64)]] Flags {
			uint64: 0;
		};

		struct [[CTL_PACKED_STRUCT]] Block {
			struct [[CTL_FLAG_STRUCT(uint64)]] Flags {
				uint64 solidColor: 1;
			};

			ImageFormat	format;
			Region		region;
			Flags		flags;
			Color8		background;
			Data		data;
		};

		Type				type;
		Mode				mode;
		Flags				flags;
		Color8				background;
		HeaderTable<Block>	blocks;
	};

	struct [[CTL_PACKED_STRUCT]] Track {
		AudioFormat	format;
		uint64		timeOffset;
		Text		language;
		Data		data;
	};

	struct [[CTL_PACKED_STRUCT]] Subtitles {
		struct [[CTL_PACKED_STRUCT]] Entry {
			uint64 frameStart, duration;
			Color8 foreground, background;
			uint64 fontID;
			Region region;
			int8 hjust: 2;
			int8 vjust: 2;
		};

		Text				language;
		HeaderTable<Entry>	entries;
	};

	struct [[CTL_PACKED_STRUCT]] Attributes {
		uint64		width;
		uint64		height;
		uint64		framerate;
	};

	struct [[CTL_PACKED_STRUCT]] Header: Attributes {
		template<class T> struct [[CTL_PACKED_STRUCT]] Section {
			T format;
		};

		scstring<10> const magic = "Makai::VSX";

		HeaderTable<Frame>		video;
		HeaderTable<Track>		audio;
		HeaderTable<Subtitles>	subtitles;
	};

	struct Decoder: ADecoder, private Graph::Blendable {
		struct Track {
			AudioFormat	format;
			uint64		timeOffset;
			String		language;
			StringList	entries;
		};

		struct Subtitle {
			struct Entry {
				uint64	frameStart, duration;
				Color8	foreground, background;
				uint64	fontID;
				Region	region;
				int8	hjust;
				int8	vjust;
				String	text;
			};
			String		language;
			List<Entry>	entries;
		};

		Decoder(BinaryFormat::IReadable& in);
		virtual ~Decoder();

		Attributes attributes() const;

		void reset()					override;
		bool finished() const			override;
		bool nextFrame()				override;

		usize frameCount() const;
		void readFrameInto(Graph::Image2D& image) const		override;
		void readFrameInto(Graph::Texture& texture) const	override;

		usize trackCount() const;
		Instance<Sound> track(usize const index) const;

		usize subtitleCount() const;
		Subtitle subtitle(usize const index) const;

	private:
		void decode(Frame const& frame);

		Atomic<usize>					current = 0;
		Atomic<bool>					inEvenFrame = false;
		Box<Header>						header;
		carr<Box<Graph::Image2D>, 2>	buffers;
		Box<Graph::Image2D>				block;
		Box<Graph::Image2D>				mask;

		Audio::Engine engine;

		void construct(Box<Graph::Image2D>& image, Frame const& frame);

		Shader shader;
	};
}
