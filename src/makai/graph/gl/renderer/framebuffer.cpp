#include "../glapiloader.cc"

#include "framebuffer.hpp"
#include "../global.hpp"

namespace ImageSlot {
	constexpr uint8 POSITION	= 28;
	constexpr uint8 NORMAL		= 29;
	constexpr uint8 SCREEN		= 30;
	constexpr uint8 DEPTH		= 31;
}

using namespace Makai; using namespace Makai::Graph;

Graph::Base::ABuffer::ABuffer(uint32 const width, uint32 const height) {
	create(width, height);
}

Graph::Base::ABuffer::~ABuffer() {
	destroy();
}

Graph::Base::ABuffer& Graph::Base::ABuffer::destroy() {
	if (!created) return *this;
	else created = false;
	glDeleteFramebuffers(1, &id);
	return *this;
}

Graph::Base::ABuffer& Graph::Base::ABuffer::create(uint32 const width, uint32 const height) {
	if (created) return *this;
	else created = true;
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	this->width = width;
	this->height = height;
	disable();
	return *this;
}

Graph::Base::ABuffer& Graph::Base::ABuffer::enable() {
	if (!created) return *this;
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	return *this;
}

Graph::Base::ABuffer& Graph::Base::ABuffer::operator()() {
	return enable();
}

Graph::Base::ABuffer& Graph::Base::ABuffer::disable() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return *this;
}

bool Graph::Base::ABuffer::exists() const		{return created;	}

uint32 Graph::Base::ABuffer::getWidth() const	{return width;		}
uint32 Graph::Base::ABuffer::getHeight() const	{return height;		}
uint32 Graph::Base::ABuffer::getID() const		{return id;			}

Graph::Base::ABuffer& Graph::Base::ABuffer::render(Base::ABuffer const& target) {
	if (!exists()) return *this;
	if (!target.exists()) return *this;
	return renderTo(target.data());
}

Graph::DrawBuffer::DrawBuffer(
	unsigned int const width,
	unsigned int const height
): Base::ABuffer() {
	create(width, height);
}

Graph::DrawBuffer::~DrawBuffer() {
	if (exists()) destroy();
}

Graph::DrawBuffer& Graph::DrawBuffer::destroy() {
	if (!exists()) return *this;
	DEBUGLN("Destroying framebuffer...");
	buffer.screen.destroy();
	buffer.depth.destroy();
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	Base::ABuffer::destroy();
	DEBUGLN("Framebuffer destroyed!");
	return *this;
}

inline void makeColorTexture(Graph::Texture2D& tex, uint32 const width, uint32 const height, GLuint const attachment) {
	tex.create(
		width,
		height,
		Image2D::ComponentType::CT_FLOAT,
		Image2D::ImageFormat::IF_RGBA,
		Image2D::FilterMode::FM_SMOOTH,
		Image2D::FilterMode::FM_SMOOTH,
		NULL,
		Image2D::ComponentLayout::CL_RGBA_16F
	);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		attachment,
		GL_TEXTURE_2D,
		tex.getID(),
		0
	);
}

Graph::DrawBuffer& Graph::DrawBuffer::create(uint32 const width, uint32 const height) {
	if (exists()) return *this;
	Base::ABuffer::create(width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, getID());
	makeColorTexture(buffer.screen, width, height, GL_COLOR_ATTACHMENT0);
	makeColorTexture(buffer.normal, width, height, GL_COLOR_ATTACHMENT1);
	makeColorTexture(buffer.position, width, height, GL_COLOR_ATTACHMENT2);
	buffer.depth.create(
		width,
		height,
		Image2D::ComponentType::CT_UINT_24_8,
		Image2D::ImageFormat::IF_DS,
		Image2D::FilterMode::FM_SMOOTH,
		Image2D::FilterMode::FM_SMOOTH,
		NULL,
		Image2D::ComponentLayout::CL_D24_S8
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_TEXTURE_2D,
		buffer.depth.getID(),
		0
	);
	// Setup buffers to use
	unsigned int attachments[3] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(3, attachments);
	// Setup display rectangle
	rect[0] = Vertex(-1, +1, 0, 0, 1);
	rect[1] = Vertex(+1, +1, 0, 1, 1);
	rect[2] = Vertex(-1, -1, 0, 0, 0);
	rect[3] = Vertex(+1, -1, 0, 1, 0);
	// Create buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	disable();
	return *this;
}

Graph::DrawBuffer& Graph::DrawBuffer::enable() {
	if (!exists()) return *this;
	Base::ABuffer::enable();
	this->clearDepthBuffer();
	return *this;
}

Graph::Base::BufferObject Graph::Base::ABuffer::data() const {
	if (!exists())
		return Base::BufferObject{};
	return *this;
}

Graph::DrawBuffer& Graph::DrawBuffer::clearBuffers() {
	this->clearColorBuffer();
	this->clearDepthBuffer();
	return *this;
}

Graph::DrawBuffer& Graph::DrawBuffer::clearColorBuffer() {
	// Why does this line cause a problem?
	API::setClearColor(clearColor);
	API::clear(API::Buffer::GAB_COLOR);
	return *this;
}

Graph::DrawBuffer& Graph::DrawBuffer::clearDepthBuffer() {
	API::clear(API::Buffer::GAB_DEPTH);
	return *this;
}

Graph::DrawBuffer& Graph::DrawBuffer::renderTo(Graph::Base::BufferObject const& target) {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("FrameBuffer::render");
	#endif // MAKAILIB_DEBUG
	if (!exists()) return *this;
	// Set target buffer
	glBindFramebuffer(GL_FRAMEBUFFER, target.id);
	// set blend func & equation data
	useBlendMode();
	// Set VBO as active
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Copy vertices to VBO
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Vertex) * 4,
		rect,
		GL_STATIC_DRAW
	);
	// Set VAO as active
	glBindVertexArray(vao);
	// Define vertex data in VBO
	Vertex::setAttributes();
	// Get shader
	// Activate shader
	shader();
	// Set shader textures
	buffer.depth(ImageSlot::DEPTH);
	buffer.screen(ImageSlot::SCREEN);
	buffer.position(ImageSlot::POSITION);
	buffer.normal(ImageSlot::NORMAL);
	// Set camera's near and far plane
	shader["near"](Global::camera.zNear);
	shader["far"](Global::camera.zFar);
	// Set texture locations
	shader["depth"](ImageSlot::DEPTH);
	shader["screen"](ImageSlot::SCREEN);
	shader["position"](ImageSlot::POSITION);
	shader["normal"](ImageSlot::NORMAL);
	// Set transformation matrix
	shader["posMatrix"](Matrix4x4(trans));
	shader["uvMatrix"](Matrix4x4(uv));
	Vector2 const resolution = Vector2(getWidth(), getHeight());
	// Set attributes
	shader["resolution"](resolution);
	shader["screenVUSpace"](screenVUSpace);
	shader["pixelVU"](resolution / screenVUSpace);
	shader["cameraPosition"](Global::camera.eye);
	// Enable attribute pointers
	Vertex::enableAttributes();
	// Set VAO as active
	glBindVertexArray(vao);
	// Set polygon rendering mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	// Draw object to screen
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	// Unbind vertex array
	glBindVertexArray(0);
	// Disable attributes
	Vertex::disableAttributes();
	disable();
	return *this;
}

Graph::DrawBuffer& Graph::DrawBuffer::disable() {
	Base::ABuffer::disable();
	return *this;
}
