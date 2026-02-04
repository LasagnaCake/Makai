#include "glapiloader.cc"

#include "core.hpp"

using namespace Makai; using namespace Makai::Graph::API;

constexpr GLenum API_DEBUG_MODE = GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB;

constexpr GLenum convert(Facility const& facility) {
	switch (facility) {
		case Facility::GAF_DEPTH_TEST:	return GL_DEPTH_TEST;
		case Facility::GAF_BLEND:		return GL_BLEND;
		case Facility::GAF_DEBUG:		return API_DEBUG_MODE;
	}
	return 0;
}

bool opened		= false;

#ifdef MAKAILIB_DEBUG
bool apiDebug	= false;

inline String glDebugSource(GLenum const& source) {
	switch (source) {
		case GL_DEBUG_SOURCE_API:				return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER:	return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY:		return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION:		return "APPLICATION";
		default: return "UNKNOWN";
	}
}

inline String glDebugSeverity(GLenum const& severity) {
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:				return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM:			return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH:			return "HIGH";
		case GL_DEBUG_SEVERITY_NOTIFICATION:	return "NOTIFICATION";
		default: return "UNKNOWN";
	}
}

inline String glDebugType(GLenum const& type) {
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:				return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	return "DEPRECATED BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	return "UNDEFINED BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY:			return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE:			return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER:				return "MARKER";
		case GL_DEBUG_TYPE_PUSH_GROUP:			return "PUSH GROUP";
		case GL_DEBUG_TYPE_POP_GROUP:			return "POP GROUP";
		case GL_DEBUG_TYPE_OTHER:				return "OTHER";
		default: return "UNKNOWN";
	}
}

usize glMessageID = 1;
inline void getContextErrors() {
	if (!apiDebug) return;
	GLenum error = GL_NO_ERROR;
	while ((error = glGetError()) != GL_NO_ERROR) {
		switch(error) {
			case GL_INVALID_ENUM:					Debug::addEntry("error", "INVALID ENUM"			);
			case GL_INVALID_VALUE:					Debug::addEntry("error", "INVALID VALUE"		);
			case GL_INVALID_OPERATION:				Debug::addEntry("error", "INVALID OPERATION"	);
			case GL_INVALID_FRAMEBUFFER_OPERATION:	Debug::addEntry("error", "INVALID FB OPERATION"	);
			case GL_OUT_OF_MEMORY:					Debug::addEntry("error", "OUT OF MEMORY"		);
			case GL_STACK_UNDERFLOW:				Debug::addEntry("error", "STACK UNDERFLOW"		);
			case GL_STACK_OVERFLOW:					Debug::addEntry("error", "STACK OVERFLOW"		);
		}
	}
}

inline void printErrors() {
	if (!apiDebug) return;
	GLenum error = GL_NO_ERROR;
	while ((error = glGetError()) != GL_NO_ERROR) {
		switch(error) {
			case GL_INVALID_ENUM:					DEBUGLN("error: NVALID ENUM"			);
			case GL_INVALID_VALUE:					DEBUGLN("error: INVALID VALUE"			);
			case GL_INVALID_OPERATION:				DEBUGLN("error: INVALID OPERATION"		);
			case GL_INVALID_FRAMEBUFFER_OPERATION:	DEBUGLN("error: INVALID FB OPERATION"	);
			case GL_OUT_OF_MEMORY:					DEBUGLN("error: OUT OF MEMORY"			);
			case GL_STACK_UNDERFLOW:				DEBUGLN("error: STACK UNDERFLOW"		);
			case GL_STACK_OVERFLOW:					DEBUGLN("error: STACK OVERFLOW"			);
		}
	}
}

struct DebugStack {
	inline static List<Debug::Context*> contexts;
	inline static String indent = "";

	static void append(String const& message) {
		if (!contexts.empty())
			contexts.back()->append(message);
	}

	static void appendWithIndent(String const& message) {
		if (!contexts.empty())
			contexts.back()->append(indent + message);
	}

	static void push(Debug::Context* const ctx) {
		contexts.pushBack(ctx);
		indent += "  ";
	}

	static Debug::Context* pop() {
		if (contexts.empty()) return nullptr;
		indent = indent.substring(2);
		return contexts.popBack();
	}
};
#endif // MAKAILIB_DEBUG

void Debug::Context::append(String const& msg) {
	#ifdef MAKAILIB_DEBUG
	if (!apiDebug) return;
	message += msg;
	#endif // MAKAILIB_DEBUG
}

Debug::Context::Context(String const& name):
	message(""),
	name(name)
	#ifdef MAKAILIB_DEBUG
	,indent(DebugStack::indent)
	#else
	,indent("")
	#endif // MAKAILIB_DEBUG
{
	#ifdef MAKAILIB_DEBUG
	if (!apiDebug) return;
	getContextErrors();
	message += indent + ("<api:context name=\"" + name + "\">\n");
	DebugStack::push(this);
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, glMessageID++, -1, name.cstr());
	#endif // MAKAILIB_DEBUG_MODE
}

Debug::Context::~Context() {
	#ifdef MAKAILIB_DEBUG
	if (!apiDebug) return;
	DebugStack::pop();
	DebugStack::append(report());
	glPopDebugGroup();
	--glMessageID;
	#endif // MAKAILIB_DEBUG_MODE
}

String Debug::Context::report() {
	#ifdef MAKAILIB_DEBUG
	if (!apiDebug) return "";
	getContextErrors();
	if (message == (indent + "<api:context name=\"" + name + "\">\n"))
		return indent + "<api:context name=\"" + name + "\" />\n";
	return message + indent + ("</api:context>\n");
	#else
	return "";
	#endif // MAKAILIB_DEBUG
}

String Debug::Context::getMessage() {
	#ifdef MAKAILIB_DEBUG
	return message;
	#else
	return "";
	#endif
}

void Debug::addEntry(String const& type) {
	#ifdef MAKAILIB_DEBUG
	if (!apiDebug)
		DebugStack::appendWithIndent("<api:entry type=\"" + type + "\" />\n");
	#endif // MAKAILIB_DEBUG
}

void Debug::addEntry(String const& type, String const& value) {
	#ifdef MAKAILIB_DEBUG
	if (apiDebug)
		DebugStack::appendWithIndent("<api:entry type=\"" + type + "\" value=\"" + value + "\" />\n");
	#endif // MAKAILIB_DEBUG
}

#ifdef MAKAILIB_DEBUG
void GLAPIENTRY glAPIMessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
) {
	if (type == GL_DEBUG_TYPE_PUSH_GROUP) {
		DEBUGLN("<api:context name=\"", message, "\">");
		return;
	}
	if (type == GL_DEBUG_TYPE_POP_GROUP) {
		DEBUGLN("</api:context>");
		return;
	}
	DEBUGLN(
		"<api:message>\n"
		, "Source: "	, glDebugSource(source)		, "\n"
		, "Type: "		, glDebugType(type)			, "\n"
		, "Severity: "	, glDebugSeverity(severity)	, "\n"
		, "Message: "	, String(message)			, "\n"
		, "</api:message>"
	);
}
#endif // MAKAILIB_DEBUG

void Makai::Graph::API::open() {
	if (opened) return;
	#if (MAKAILIB_GL_LOADER == MAKAILIB_USE_GL3W)
	if (gl3wInit() != GL3W_OK)
		throw Error::FailedAction("Failed to initialize GL3W!", CTL_CPP_PRETTY_SOURCE);
	#elif (MAKAILIB_GL_LOADER == MAKAILIB_USE_GLAD)
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        throw Error::FailedAction("Failed to initialize GLAD!", CTL_CPP_PRETTY_SOURCE);
	#endif
	opened = true;
	#ifdef MAKAILIB_DEBUG
	glDebugMessageCallback(glAPIMessageCallback, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP, GL_DONT_CARE, 0, NULL, GL_TRUE);
	printErrors();
	#endif // MAKAILIB_DEBUG
}

void Makai::Graph::API::setViewport(uint32 const width, uint32 const height, uint32 const x, uint32 const y) {
	glViewport(x, y, width, height);
}

bool Makai::Graph::API::hasRequiredVersion() {
	if (!opened) return false;
	#if (MAKAILIB_GL_LOADER == MAKAILIB_USE_GL3W)
	return gl3wIsSupported(4, 2);
	#elif (MAKAILIB_GL_LOADER == MAKAILIB_USE_GLAD)
	return GLAD_GL_VERSION_4_2;
	#endif
}

String Makai::Graph::API::name() {
	return "OpenGL 4.2";
}

inline String facilityName(Facility const& fac) {
	switch (fac) {
		case Facility::GAF_DEPTH_TEST:	return "[DEPTH TEST]";
		case Facility::GAF_BLEND:		return "[BLENDING]";
		case Facility::GAF_DEBUG:		return "[DEBUG]";
	}
	return "";
}

void Makai::Graph::API::toggle(Facility const& fac, bool const state) {
	#ifdef MAKAILIB_DEBUG
	if (fac == Facility::GAF_DEBUG) {
		apiDebug = state;
		if (state)	glEnable(API_DEBUG_MODE);
		else		glDisable(API_DEBUG_MODE);
		return;
	}
	Debug::addEntry("API::toggle", facilityName(fac) + "=" + (state ? "true" : "false"));
	#endif // MAKAILIB_DEBUG
	if (state)	glEnable(convert(fac));
	else		glDisable(convert(fac));

}

inline String bufferName(Buffer const& buffer) {
	switch (buffer) {
		case Buffer::GAB_COLOR:		return "COLOR";
		case Buffer::GAB_DEPTH:		return "DEPTH";
		case Buffer::GAB_STENCIL:	return "STENCIL";
	}
	return "";
}

void Makai::Graph::API::clear(Buffer const& buffer) {
	#ifdef MAKAILIB_DEBUG
	Debug::addEntry("API::clear", bufferName(buffer));
	#endif // MAKAILIB_DEBUG
	switch (buffer) {
		case Buffer::GAB_COLOR:		glClear(GL_COLOR_BUFFER_BIT		);
		case Buffer::GAB_DEPTH:		glClear(GL_DEPTH_BUFFER_BIT		);
		case Buffer::GAB_STENCIL:	glClear(GL_STENCIL_BUFFER_BIT	);
	}
}

void Makai::Graph::API::setClearColor(Vector4 const& color) {
	#ifdef MAKAILIB_DEBUG
	Debug::addEntry("API::setClearColor", toString("(", color.r, ", ", color.g, ", ", color.b, ", ", color.a, ")"));
	#endif // MAKAILIB_DEBUG
	glClearColor(color.r, color.g, color.b, color.a);
}

void Makai::Graph::API::setFrontFace(bool const clockwise) {
	#ifdef MAKAILIB_DEBUG
	Debug::addEntry("API::setFrontFace", (clockwise ? "true" : "false"));
	#endif // MAKAILIB_DEBUG
	glFrontFace(clockwise ? GL_CW : GL_CCW);
}

#ifdef MAKAILIB_DEBUG
Unique<Debug::Context> renderContext;
#endif // MAKAILIB_DEBUG

void Makai::Graph::API::beginRender() {
	#ifdef MAKAILIB_DEBUG
	if (apiDebug)
		renderContext.bind(new Debug::Context("render"));
	#endif // MAKAILIB_DEBUG
}
void Makai::Graph::API::endRender() {
	#ifdef MAKAILIB_DEBUG
	if (apiDebug) {
		DEBUGLN(renderContext->report());
		renderContext.unbind();
	}
	#endif // MAKAILIB_DEBUG
	useMainBuffer();
}

void Makai::Graph::API::useMainBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
