#include "mvsx.hpp"

using namespace Makai;
using namespace Makai::Video::V2D::MVSX;

//extern char mkEmbed_MVSXShaderVert[];
extern dcstring mkEmbed_MVSXShaderFrag;

//extern int mkEmbed_MVSXShaderVert_Size;
extern int mkEmbed_MVSXShaderFrag_Size;

//String const MVSX_VERT	= String(mkEmbed_MVSXShaderVert, mkEmbed_MVSXShaderVert_Size);
String const MVSX_FRAG		= String(mkEmbed_MVSXShaderFrag, mkEmbed_MVSXShaderFrag_Size);

Decoder::Decoder(BinaryFormat::IReadable& in): ADecoder(in), shader(MVSX_FRAG, Graph::Core::ShaderType::ST_FRAGMENT) {
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glGenFramebuffers(1, &fbx);
}

Decoder::~Decoder() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteFramebuffers(1, &fbx);
}

void Decoder::reset() {
	in.go(0);
	if (auto const head = Header::fetch<Header>(in)) {
		auto [_, hh] = header.open();
		hh = head.value();
		for (auto& buffer: buffers) {
			auto [_, bb] = buffer.open();
			buffer.create(
				hh.width,
				hh.height,
				Graph::Image2D::ComponentType::CT_UBYTE,
				Graph::Image2D::ImageFormat::IF_RGBA,
				Graph::Image2D::FilterMode::FM_SMOOTH,
				Graph::Image2D::FilterMode::FM_SMOOTH
			);
		}
		auto [_, mm] = mask.open();
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
	if (current >= video.size) return false;
	auto [_, hh] = header.open();
	if (auto const fh = hh.video.getEntry(in, current)) {
		auto const frame = fh.value();
		if (!construct(buffers[inEvenFrame], frame))
		 return false;
		if (frame.type == Frame::Type::MV2_FT_DELTA_MASKED) {
			++current;
			if (auto const fh = hh.video.getEntry(in, current))
				if (!construct(mask, fh.value()))
					return false;
		} else return false;
		decode(frame);
	} else return false;
	++inEvenFrame;
	++current;
	return true;
}

void Decoder::readFrameInto(Graph::Image2D& image) {
	auto [_, ff] = buffer[inEvenFrame].open();
	auto [_, hh] = header.open();
	Makai::Graph::Image2D::blit(
		{
			ff,
			{0, 0, hh.width, hh.height}
		},
		{
			image,
			{0, 0, image.width(), image.height()}
		}
	);
}

Attributes Decoder::attributes() const {
	return {header.value()};
}

void Decoder::readFrameInto(Graph::Texture2D& texture) {
	readInto(texture.getImage());
}

void Decoder::decode(Frame const& frame) {
	static uint32 const carr<float[2], 4> uvx = {
		{0, 0},
		{1, 0},
		{0, 1},
		{1, 1}
	};
	auto [_0, src] = buffer[inEvenFrame].open();
	auto [_1, dst] = buffer[!inEvenFrame].open();
	auto [_1, mm] = mask.open();
	glBindFramebuffer(GL_FRAMEBUFFER, fbx);
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
	Graph::API::setClearColor(frame.background);
	Graph::API::clear(API::Buffer::GAB_COLOR);
	if (frame.type == Frame::Type::MV2_FT_DELTA_MASKED)
		mm.use(2);
	shader["previous"](0, 1, (frame.type == Frame::Type::MV2_FT_DELTA_MASKED) + 1);
	shader["packing"](enumcast(frame.type), enumcast(frame.mode));
	useBlendMode();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(uvx) * sizeof(float),
		uvx,
		GL_STATIC_DRAW
	);
	glBindVertexArray(vao);
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

bool Decoder::construct(Box<Makai::Graph::Image2D>& image, Frame const& frame) {
	auto [_, ii] = image.open();
	auto [_, bb] = block.open();
	ii.fill(frame.background.normalize());
	for (usize i = 0; i < frame.block.size; ++i) {
		if (auto const bh = frame.block.getEntry(in, i)) {
			auto const block = bh.value();
			if (block.flags.solidColor) {
					block.make(
						block.region.width,
						block.region.height
					).fill(block.background.normalize());
			} else {
				auto sin = InputSubstream<Bytes<>>(in, block.data.start, block.data.size);
				if (auto const img = Makai::Image::I2D::decodeStream(sin, block.format)) {
					auto const image = img.value();
					block.make(
						block.region.width,
						block.region.height,
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
					{0, 0, block.region.width, block.region.height}
				},
				{
					ii,
					{block.region.x, block.region.y, block.region.width, block.region.height}
				}
			);
		} else return false;
	}
	return true;
}
