#include "../glapiloader.cc"

#include "drawable.hpp"

using namespace Makai; using namespace Makai::Graph;

void ADrawable::render() {if (active) draw();}

ADrawable::ADrawable(bool const manual, usize const layer) {
	if(!manual) setAuto(layer);
	manualMode = manual;
}


ADrawable::ADrawable(bool const manual): ADrawable(manual, 0) {
}

ADrawable::~ADrawable() {
	DEBUGLN("Removing from rendering layers...");
	if(!manualMode)
		RenderServer::layers.removeFromAll(&doRender);
	DEBUGLN("Finalizing...\n");
}

ADrawable& ADrawable::setManual() {
	if(!manualMode)
		RenderServer::layers.removeFromAll(&doRender);
	manualMode = true;
	return *this;
}

ADrawable& ADrawable::setAuto(usize const renderLayer) {
	if(manualMode)
		RenderServer::layers.add(&doRender, renderLayer);
	manualMode = false;
	return *this;
}

ADrawable& ADrawable::setRenderLayer(usize const renderLayer) {
	RenderServer::layers.removeFromAll(&doRender);
	RenderServer::layers.add(&doRender, renderLayer);
	manualMode = false;
	return *this;
}

ADrawable& ADrawable::addToRenderLayer(usize const renderLayer) {
	RenderServer::layers.add(&doRender, renderLayer);
	manualMode = false;
	return *this;
}

ADrawable& ADrawable::removeFromRenderLayer(usize const renderLayer) {
	RenderServer::layers.remove(&doRender, renderLayer);
	if (RenderServer::layers.withObject(&doRender).empty())
		manualMode = true;
	return *this;
}

AGraphic::AGraphic(usize const layer, bool const manual): ADrawable(layer, manual), Blendable() {
	DEBUGLN("Creating drawable object...");
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	DEBUGLN("Drawable object created!");
}

AGraphic::~AGraphic() {
	DEBUGLN("Deleting buffers...");
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void AGraphic::display(
	Vertex* const		vertices,
	usize const		count,
	CullMode const&		culling,
	FillMode const&		fill,
	DisplayMode const&	mode,
	usize const		instances
) {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("DrawableObject::display");
	#endif // MAKAILIB_DEBUG
	// Set blend mode
	useBlendMode();
	// Set point size, if applicable
	if (pointSize)	glPointSize(*pointSize);
	else			glEnable(GL_PROGRAM_POINT_SIZE);
	// Set line width
	glLineWidth(lineWidth);
	// Set VBO as active
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Copy IVB to VBO
	glBufferData(
		GL_ARRAY_BUFFER,
		count * sizeof(Vertex),
		vertices,
		GL_STATIC_DRAW
	);
	// Set VAO as active
	glBindVertexArray(vao);
	// Define vertex data in VBO
	Vertex::setAttributes();
	// Set VAO as active
	glBindVertexArray(vao);
	// Enable attribute pointers
	Vertex::enableAttributes();
	// Set face culling
	setCullMode(culling);
	// Set fill mode (if none, exit)
	if (setFillMode(fill)) {
		// Draw object to screen
		if (instances)	glDrawArraysInstanced(getGLDisplayMode(mode), 0, count, instances);
		else			glDrawArrays(getGLDisplayMode(mode), 0, count);
	}
	// Disable culling
	setCullMode(CullMode::OCM_NONE);
	// Disable attributes
	Vertex::disableAttributes();
	// Unbind vertex array
	glBindVertexArray(0);
	// Disable program point size
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void AGraphic::prepare() {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("DrawableObject::prepare");
	#endif // MAKAILIB_DEBUG
	// Render with object's shader
	shader();
	// Get transformation matrices
	Matrix4x4 cameraWorldActor(Global::camera.matrix() * Global::space * trans);
	// Set transformation matrices
	shader["vertMatrix"](Global::camera.projection() * cameraWorldActor);
	shader["normalsMatrix"](cameraWorldActor.transposed().inverted());
	// Set world shader
	Global::world.use(shader);
}
