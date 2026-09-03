#include "mvsx.hpp"
#include "../graph/gl/glapiloader.cc"
#include <CL/cl.h>
#include "../graph/graph.hpp"

using namespace Makai;
using namespace Makai::Video::V2D::MVSX;

//extern char mkEmbed_MVSXShaderVert[];
extern dcstring mkEmbed_MVSXShaderFrag;

//extern int mkEmbed_MVSXShaderVert_Size;
extern int mkEmbed_MVSXShaderFrag_Size;

//String const MVSX_VERT	= String(mkEmbed_MVSXShaderVert, mkEmbed_MVSXShaderVert_Size);
String const MVSX_FRAG		= String(mkEmbed_MVSXShaderFrag, mkEmbed_MVSXShaderFrag_Size);

struct Decoder::Impl {
	Graph::Shader					shader;
	GLuint							vao, vbo, fbo;
	carr<Box<Graph::Image2D>, 2>	buffers;
	Box<Graph::Image2D>				block;
	Box<Graph::Image2D>				mask

	bool construct(Box<Graph::Image2D>& image, Frame const& frame);
};

Decoder::Decoder(BinaryFormat::IReadable& in): ADecoder(in) {
	impl = new Impl;
	glGenBuffers(1, &impl->vbo);
	glGenVertexArrays(1, &impl->vao);
	glGenFramebuffers(1, &impl->fbo);
	impl->shader = Graph::Shader(MVSX_FRAG, Graph::ShaderType::ST_FRAGMENT);
}

Decoder::~Decoder() {
	glDeleteBuffers(1, &impl->vbo);
	glDeleteVertexArrays(1, &impl->vao);
	glDeleteFramebuffers(1, &impl->fbo);
	delete impl;
}

void Decoder::reset() {
	cache.clear();
	in.go(0);
	if (auto const head = BinaryFormat::Header<Header>::fetch(in)) {
		auto [_1, hh] = header.open();
		hh = head.value();
		for (auto& buffer: impl->buffers) {
			auto [_2, bb] = buffer.open();
			bb.create(
				hh.width,
				hh.height,
				Graph::Image2D::ComponentType::CT_UBYTE,
				Graph::Image2D::ImageFormat::IF_RGBA,
				Graph::Image2D::FilterMode::FM_SMOOTH,
				Graph::Image2D::FilterMode::FM_SMOOTH
			);
		}
		auto [_3, mm] = impl->mask.open();
		mm.create(
			hh.width,
			hh.height,
			Graph::Image2D::ComponentType::CT_UBYTE,
			Graph::Image2D::ImageFormat::IF_RGBA,
			Graph::Image2D::FilterMode::FM_SMOOTH,
			Graph::Image2D::FilterMode::FM_SMOOTH
		);
		inEvenFrame = false;
		current = 0;
	}
}

bool Decoder::nextFrame() {
	cache.clear();
	auto [_, hh] = header.open();
	if (current >= hh.video.size) return false;
	if (auto const fh = hh.video.getEntry(in, current)) {
		auto const frame = fh.value();
		if (!impl->construct(buffers[inEvenFrame], frame))
		 return false;
		if (frame.type == Frame::Type::MV2_FT_DELTA_MASKED) {
			++current;
			if (auto const fh = hh.video.getEntry(in, current))
				if (!impl->construct(mask, fh.value()))
					return false;
		} else return false;
		if (frame.type != Frame::Type::MV2_FT_NONE)
			decode(frame);
		else inEvenFrame = !inEvenFrame;
	} else return false;
	inEvenFrame = !inEvenFrame;
	++current;
	return true;
}

void Decoder::go(usize const frame) {
	usize ffwd = 0;
	auto [_, hh] = header.open();
	for (current = frame; current.value() > 0; --current)
		if (auto const fh = hh.video.getEntry(in, current)) {
			auto const frame = fh.value();
			if(frame.type != Frame::Type::MV2_FT_NONE) {
				--current;
				if (!frame.flags.isPartOfFrame)
					++ffwd;
			} else {
				break;
			}
		} else break;
	for (;ffwd > 0; --ffwd)
		nextFrame();
}

bool Decoder::finished() {return false;}

usize Decoder::frameCount() {
	auto const& [_, hh] = header.open();
	return hh.viewableFrames;
}

Span<byte const> Decoder::currentFrame() {
	if (cache.size()) return {cache.cbegin(), cache.cend()};
	auto const& [_, bb] = impl->buffers[!inEvenFrame].open();
	cache = bb.getData().data;
	return {cache.cbegin(), cache.cend()};
}

Attributes Decoder::attributes() {
	return {header.value()};
}

Decoder::Info Decoder::videoInfo() {
	auto [_, hh] = header.open();
	return {
		Container::MV2C_MVSX,
		hh.width,
		hh.height,
		hh.viewableFrames,
		hh.framerate
	};
}

void Decoder::decode(Frame const& frame) {
	static carr<float[2], 4> const uvx = {
		{0, 0},
		{1, 0},
		{0, 1},
		{1, 1}
	};
	auto [_0, src] = impl->buffers[inEvenFrame].open();
	auto [_1, dst] = impl->buffers[!inEvenFrame].open();
	glBindFramebuffer(GL_FRAMEBUFFER, impl->fbo);
	glFramebufferTexture2D(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		dst.getID(),
		0
	);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	src.use(0);
	dst.use(1);
	Graph::API::setClearColor(frame.background.normalize());
	Graph::API::clear(Graph::API::Buffer::GAB_COLOR);
	if (frame.type == Frame::Type::MV2_FT_DELTA_MASKED) {
		auto [_2, mm] = impl->mask.open();
		mm.use(2);
	}
	impl->shader.enable();
	impl->shader["previous"](0, 1, (frame.type == Frame::Type::MV2_FT_DELTA_MASKED) + 1);
	impl->shader["packing"](uint32(frame.type), uint32(frame.mode));
	useBlendMode();
	glBindBuffer(GL_ARRAY_BUFFER, impl->vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(uvx) * sizeof(float),
		uvx,
		GL_STATIC_DRAW
	);
	glBindVertexArray(impl->vao);
	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 2,
		0
	);
	glEnableVertexAttribArray(0);
	Graph::setFillMode(Graph::FillMode::OFM_FILL);
	Graph::setCullMode(Graph::CullMode::OCM_BACK);
	glDrawArrays(Graph::getGLDisplayMode(Graph::DisplayMode::ODM_TRIS), 0, 4);
	Graph::setCullMode(Graph::CullMode::OCM_NONE);
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// TODO: Break this pyramid of doom

bool Decoder::Impl::construct(Box<Makai::Graph::Image2D>& image, Frame const& frame) {
	auto [_, ii] = image.open();
	auto [_, bb] = block.open();
	ii.fill(frame.background.normalize());
	for (usize i = 0; i < frame.blocks.size; ++i) {
		if (auto const bh = frame.blocks.getEntry(in, i)) {
			auto const fblock = bh.value();
			if (fblock.flags.solidColor) {
					bb.make(
						fblock.region.width,
						fblock.region.height
					).fill(fblock.background.normalize());
			} else {
				auto sin = InputSubstream<Bytes<>>(in, fblock.data.start, fblock.data.size);
				if (auto const img = Makai::Image::I2D::decodeStream(sin, fblock.format)) {
					auto const image = img.value();
					bb.make(
						fblock.region.width,
						fblock.region.height,
						Graph::Image2D::ComponentType::CT_UBYTE,
						Graph::Image2D::ImageFormat::IF_RGBA,
						Graph::Image2D::FilterMode::FM_SMOOTH,
						Graph::Image2D::FilterMode::FM_SMOOTH,
						image.data.data()
					);
				} else return false;
			}
			Makai::Graph::Image2D::blit(
				{
					bb,
					{0, 0, fblock.region.width, fblock.region.height}
				},
				{
					ii,
					{fblock.region.x, fblock.region.y, fblock.region.width, fblock.region.height}
				}
			);
		} else return false;
	}
	return true;
}
