#include "mvsx.hpp"

using namespace Makai::Video::V2D::MVSX;

void Decoder::reset() {
	in.go(0);
	if (auto const head = header.fetch(in)) {
		header = head.value();
		for (auto& b: buffer)
			b.create(
				header.width,
				header.height,
				Graph::Image2D::ComponentType::CT_UBYTE,
				Graph::Image2D::ImageFormat::IF_RGBA,
				Graph::Image2D::FilterMode::FM_SMOOTH,
				Graph::Image2D::FilterMode::FM_SMOOTH,
			);
		mask.create(
			header.width,
			header.height,
			Graph::Image2D::ComponentType::CT_UBYTE,
			Graph::Image2D::ImageFormat::IF_RGBA,
			Graph::Image2D::FilterMode::FM_SMOOTH,
			Graph::Image2D::FilterMode::FM_SMOOTH,
		);
		even = false;
		current = 0;
	}
}

// TODO: Break this pyramid of doom.

bool Decoder::nextFrame() {
	if (current >= video.size) return false;
	if (auto const fh = header.video.getEntry(in, current)) {
		auto const frame = fh.value();
		construct(buffer[even], frame);
		if (frame.type == Frame::Type::MV2_FT_DELTA_MASKED) {
			++current;
			if (auto const fh = header.video.getEntry(in, current))
				construct(mask, fh.value());
		}
	}
	decode();
	++even;
	++current;
}

Makai::Graph::Image2D& Decoder::currentFrame() {
	return buffer[even];
}

static inline uint32 multiPurposeFrameBuffer() {
	MAKAILIB_DEBUGLN_FULL("Creating copy buffer...");
	uint id = 0;
	glGenFramebuffers(1, &id);
	return id;
}

void Decoder::decode() {
	static uint32 const decoder = multiPurposeFrameBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, decoder);
	glFramebufferTexture2D(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		buffer[!even].getID(),
		0
	);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	buffer[even].use(0);
	buffer[!even].use(1);
	// TODO: the rest of the owl
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Decoder::construct(Makai::Graph::Image2D& image, Frame const& frame) {
	image.fill({frame.r / 255.0, frame.g / 255.0, frame.b / 255.0, frame.a / 255.0});
	for (usize i = 0; i < frame.block.size; ++i) {
		if (auto const bh = frame.block.getEntry(in, i)) {
			auto const block = bh.value();
			if (auto const d = block.data.fromBytes(in)) {
				if (block.flags.solidColor) {
					block.make(
						block.region.width,
						block.region.height
					).fill({block.r / 255.0, block.g / 255.0, block.b / 255.0, block.a / 255.0});
				} else {
					auto const data = Makai::Image::I2D::decode(d.value(), block.format);
					block.make(
						block.region.width,
						block.region.height,
						Graph::Image2D::ComponentType::CT_UBYTE,
						Graph::Image2D::ImageFormat::IF_RGBA,
						Graph::Image2D::FilterMode::FM_SMOOTH,
						Graph::Image2D::FilterMode::FM_SMOOTH,
						data.data()
					);
					Makai::Graph::Image2D::blit(
						{
							block,
							{0, 0, block.region.width, block.region.height}
						},
						{
							image,
							{block.region.x, block.region.y, block.region.width, block.region.height}
						}
					);
				}
			}
		}
	}
}
