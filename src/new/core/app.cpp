#ifndef AUDIO_SAMPLE_FRAMES
#define AUDIO_SAMPLE_FRAMES 2048
#endif // AUDIO_SAMPLE_FRAMES

#define GLEW_STATIC
#include <GLEW/include/GL/glew.h>
#include <GLEW/include/GL/wglew.h>
#include <GL/gl.h>

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <winuser.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include <SDL2/SDL_mixer.h>

#define sdlWindow ((SDL_Window*)window)

#include "app.hpp"

using namespace Makai;

void GLAPIENTRY glAPIMessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
) {
	DEBUGLN(
		"[GL CALLBACK"
		, String(type == GL_DEBUG_TYPE_ERROR ? " (GL ERROR)" : "")	, "] "
		, "Type: "		, type				, ", "
		, "Severity: "	, severity			, ", "
		, "Message: "	, String(message)	, ", "
	);
}

inline void setGLAttribute(SDL_GLattr const& a, int const& v) {
	if (SDL_GL_SetAttribute(a, v))
		throw Error::FailedAction(
			"Failed to set attribute!",
			__FILE__,
			"unspecified",
			"unspecified",
			SDL_GetError()
		);
}

App* mainApp = nullptr;

App::App (
	unsigned int width,
	unsigned int height,
	string windowTitle,
	bool fullscreen = false,
	bool useMIDI = false,
	string bufferShaderPath = "shaders/framebuffer/compose.slf",
	string mainShaderPath = "shaders/base/base.slf"
) {
	// If there is another app open, throw error
	if (mainApp)
		throw Error::DuplicateValue(
			"Cannot have two apps open at the same time!",
			__FILE__,
			toString(__LINE__),
			"App::constructor",
			"Having two apps open is forbidden!"
		);
	else mainApp = this;
	DEBUGLN(Entities::_ROOT == nullptr);
	// Save window resolution
	this->width = width;
	this->height = height;
	// Initialize SDL
	DEBUGLN("Starting SDL...");
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		ERR_LOG(String("Unable to start SDL! (") + SDL_GetError() + ")");
		throw Error::FailedAction(String("SDL (") + SDL_GetError() + ")");
	}
	DEBUGLN("Started!");
	// Initialize YSE
	DEBUGLN("Starting Audio System...");
	if (!Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | (useMIDI ? MIX_INIT_MID : 0))) {
		ERR_LOG(String("Unable to start Mixer! (") + Mix_GetError() + ")");
		throw Error::FailedAction(String("Mixer (") + Mix_GetError() + ")");
	}
	Audio::openSystem();
	DEBUGLN("Started!");
	// Create window and make active
	DEBUGLN("Creating window...");
	window = (Extern::Resource)SDL_CreateWindow(
		windowTitle.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL
	);
	if (!window) {
		throw Error::FailedAction(
			"Failed to create window!",
			__FILE__,
			toString(__LINE__),
			"Program constructor",
			SDL_GetError()
		);
	}
	SDL_SetWindowFullscreen(sdlWindow, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	// Set OpenGL flags
	setGLAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	setGLAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	setGLAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	setGLAttribute(SDL_GL_ALPHA_SIZE, 16);
	setGLAttribute(SDL_GL_BUFFER_SIZE, 16);
	setGLAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	setGLAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	// Create OpenGL context
	SDL_GL_CreateContext(sdlWindow);
	if (!input.window)
		input.window = window;
	DEBUGLN("Created!");
	DEBUGLN("Starting GLEW...");
	// Try and initialize GLEW
	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		ERR_LOG("Error: glewInit: ");
		ERR_LOG(glewGetErrorString(glewStatus));
		throw Error::FailedAction(string("glewInit: ") + (const char*)glewGetErrorString(glewStatus));
	}
	if (!GLEW_VERSION_4_2) {
		ERR_LOG("Your computer does not support OpenGL 4.2+!");
		throw Error::InvalidValue(string("No OpenGL 4.2+"));
	}
	DEBUGLN("Started!");
	// Create default shader
	/*DEBUGLN("Creating default shader...");
	Shader::defaultShader.create();
	DEBUGLN("Created!");*/
	//glViewport(0, 0, width, height);
	glDebugMessageCallback(glAPIMessageCallback, 0);
	// This keeps the alpha from shitting itself
	setGLFlag(GL_BLEND);
	setGLFlag(GL_ALPHA_TEST);
	setGLFlag(GL_DEPTH_TEST);
	// Setup camera
	DEBUGLN("Setting starting camera...");
	Scene::camera.aspect = Vector2(width, height);
	Scene::camera.fov = Math::radians(45.0f);
	DEBUGLN("creating default framebuffer...");
	// Create framebuffer
	framebuffer.create(width, height);
	// Create layer buffer
	layerbuffer.create(width, height);
	enableMainBuffer();
	// Create buffer shaders
	DEBUGLN("Creating shaders...");
	SLF::SLFData data = SLF::parseFile(bufferShaderPath);
	framebuffer.shader.create(data);
	layerbuffer.shader = framebuffer.shader;
	// Create main shader
	Makai::Graph::defaultShader.destroy();
	Makai::Graph::defaultShader.create(SLF::parseFile(mainShaderPath));
	DEBUGLN(Entities::_ROOT ? "Root Exists!" : "Root does not exist!");
	if (!Entities::_ROOT) {
		DEBUGLN("Initializing root tree...");
		Entities::init();
	}
	DEBUGLN("All core systems initialized!");
}

void App::setGLDebug(bool const& state = false) {
	setGLFlag(GL_DEBUG_OUTPUT, state);
}

void App::setWindowTitle(String const& title) {
	SDL_SetWindowTitle(sdlWindow, windowTitle.c_str());
}

void App::setFullscreen(bool const& state = false) {
	SDL_SetWindowFullscreen(sdlWindow, state);
}

void App::run() {
	// The timer process
	auto timerFunc	= [&](float delta)-> void {
		Tweening::Tweener::process(1.0);
		Event::Timeable::process(1.0);
	};
	// The logical process
	auto logicFunc	= [&](float delta)-> void {
		onLogicFrame(cycleDelta);
		taskers.yield(cycleDelta);
		if (Entities::_ROOT)
			Entities::_ROOT->yield(delta);
	};
	// Clear screen
	Graph::clearColorBuffer(color);
	glClear(GL_DEPTH_BUFFER_BIT);
	// Render reserved layer
	renderReservedLayer();
	// Render simple frame
	SDL_GL_SwapWindow(window);
	// Call on open function
	onOpen();
	// SDL's events
	SDL_Event event;
	// Delta time
	float frameDelta = 1.0/maxFrameRate;
	float cycleDelta = 1.0/maxCycleRate;
	// Last time of execution
	size_t frameTicks = SDL_GetTicks() + frameDelta * 1000.0;
	size_t cycleTicks = SDL_GetTicks() + cycleDelta * 1000.0;
	// Refresh mouse capture stuff
	input.refreshCapture();
	// While program is running...
	while(shouldRun) {
		// Poll events and check if should close
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					DEBUGLN("SDL Event: EXIT");
					shouldRun = false;
				} break;
				case SDL_WINDOWEVENT: {
					DEBUGLN("SDL Event: WINDOW EVENT");
					switch (event.window.event) {
						case SDL_WINDOWEVENT_FOCUS_GAINED: input.refreshCapture(); break;
						default: break;
					}
				} break;
				default: {
				} break;
			}
		}
		// Get deltas
		frameDelta	= 1.0/maxFrameRate;
		cycleDelta	= 1.0/maxCycleRate;
		// Get rates
		cycleRate = SDL_GetTicks() - cycleTicks;
		frameRate = SDL_GetTicks() - frameTicks;
		// If should process, then do so
		#ifndef MAKAILIB_PROCESS_RENDER_BEFORE_LOGIC
		if (cycleRate > (cycleDelta * 1000.0 - 1)) {
			// Update audio system
			Audio::updateAudioSystem();
			// Update input manager
			input.update();
			// Get current time
			cycleTicks = SDL_GetTicks();
			// Increment cycle counter
			cycle++;
			// Do timer-related stuff
			timerFunc(cycleDelta * speed);
			#ifndef MAKAILIB_FRAME_DEPENDENT_PROCESS
			// Do normal logic-related stuff
			logicFunc(cycleDelta * speed);
			// Destroy queued entities
			Entities::destroyQueued();
			#endif // FRAME_DEPENDENT_PROCESS
		}
		#endif
		if (frameRate > (frameDelta * 1000 - 1)) {
			// Get current time
			frameTicks = SDL_GetTicks();
			// increment frame counter
			frame++;
			// Update audio system
			// TODO: Audio System
			#ifdef MAKAILIB_FRAME_DEPENDENT_PROCESS
			// Do normal logic-related stuff
			logicFunc(frameDelta);
			// Destroy queued entities
			Entities::destroyQueued();
			#endif // FRAME_DEPENDENT_PROCESS
			// Render screen
			render();
		}
		#ifdef MAKAILIB_PROCESS_RENDER_BEFORE_LOGIC
		if (cycleRate > (cycleDelta * 1000.0 - 1)) {
			// Update audio system
			Audio::updateAudioSystem();
			// Update input manager
			input.update();
			// Get current time
			cycleTicks = SDL_GetTicks();
			// Increment cycle counter
			cycle++;
			// Do timer-related stuff
			timerFunc(cycleDelta * speed);
			#ifndef MAKAILIB_FRAME_DEPENDENT_PROCESS
			// Do normal logic-related stuff
			logicFunc(cycleDelta * speed);
			// Destroy queued entities
			Entities::destroyQueued();
			#endif // FRAME_DEPENDENT_PROCESS
		}
		#endif
	}
	// Terminate program
	DEBUGLN("\nClosing incoherent program...");
	terminate();
}

void App::close() {
	shouldRun = false;
}

inline bool	App::running() {
	return (shouldRun);
}

inline void App::setWindowSize(Vector2 const& size) {}

usize App::getCurrentFrame() {
	return frame;
}

App* getOpenApp() {
	return mainApp;
}

size_t App::getCurrentFrame() {
	return frame;
}

size_t App::getCurrentCycle() {
	return cycle;
}

size_t App::getCycleRate() {
	return cycleRate;
}

size_t App::getFrameRate() {
	return frameRate;
}

inline void App::renderReservedLayer() {
	Graph::clearColorBuffer(color);
	glClear(GL_DEPTH_BUFFER_BIT);
	//framebuffer().clearBuffers();
	Graph::renderLayer(NumberLimit<usize>::HIGHEST);
	//framebuffer.render(toFrameBufferData());
	SDL_GL_SwapWindow(window);
	// Clear target depth buffer
	framebuffer();
	// Enable layer buffer
	layerbuffer();
	// Reset layerbuffer's positions
	layerbuffer.trans	= VecMath::Transform3D();
	layerbuffer.uv		= VecMath::Transform3D();
	// Call onLayerDrawBegin function
	onReservedLayerDrawBegin();
	// Clear buffers
	layerbuffer.clearBuffers();
	// Call onLayerDrawBegin function
	onPostReservedLayerClear();
	// Render layer
	Graph::renderLayer(NumberLimit<usize>::HIGHEST);
	// Call onPreLayerDraw function
	onPreReservedLayerDraw();
	// Render layer buffer
	layerbuffer.render(framebuffer);
	// Call onLayerDrawEnd function
	onReservedLayerDrawEnd();
}

inline static void App::setGLFlag(usize const& flag, bool const& state = true) {
	if (state) glEnable(flag);
	else glDisable(flag);
}

inline static void App::setGLValue(usize const& flag, int const& value, bool const& state = true) {
	if (state) glEnablei(flag, value);
	else glDisablei(flag, value);
}

constexpr inline Graph::FrameBuffer& App::getFrameBuffer() {
	return framebuffer;
}

constexpr inline Graph::FrameBuffer& App::getLayerBuffer() {
	return layerbuffer;
}

inline Vector2 App::getWindowSize() {
	return Vector2(width, height);
}

inline Vector2 App::getWindowScale() {
	Vector2 ws = getWindowSize();
	return Vector2(ws.x / ws.y, 1);
}

void App::queueScreenCopy(Graph::Texture2D target) {
	screenQueue.push_back(target);
}

void App::unqueueScreenCopy(Graph::Texture2D target) {
	ERASE_IF(screenQueue, elem == target);
}

void App::skipDrawingThisLayer() {skipLayer = true;}

void App::pushLayerToFrame() {pushToFrame = true;}

void App::terminate() {
	// Call final function
	onClose();
	// Remove window from input manager
	if (input.window == window)
		input.window = nullptr;
	// Close YSE
	DEBUGLN("Closing sound system...");
	Audio::closeSystem();
	Mix_Quit();
	Entities::_ROOT->destroyChildren();
	DEBUGLN("Sound system closed!");
	// Destroy buffers
	DEBUGLN("Destroying frame buffers...");
	framebuffer.destroy();
	layerbuffer.destroy();
	DEBUGLN("Frame buffers destroyed!");
	// Quit SDL
	DEBUGLN("Ending SDL...");
	SDL_Quit();
	DEBUGLN("SDL ended!");
	//exit(0);
}

void App::render() {
	// Clear screen
	Drawer::clearColorBuffer(color);
	glClear(GL_DEPTH_BUFFER_BIT);
	// Enable depth testing
	setFlag(GL_DEPTH_TEST, true);
	// Enable frame buffer
	framebuffer();
	// Call rendering start function
	onDrawBegin();
	// Clear frame buffer
	framebuffer.clearBuffers();
	// Set frontface order
	Drawer::setFrontFace(true);
	// Call post frame clearing function
	onPostFrameClear();
	// Enable layer buffer
	layerbuffer();
	layerbuffer.clearBuffers();
	// Draw objects
	vector<size_t> rLayers = Drawer::layers.getAllGroups();;
	for (auto layer : rLayers) {
		if (layer != Math::Max::SIZET_V && !Drawer::layers[layer].empty()) {
			// Clear layer skip flag
			skipLayer = false;
			// If previous layer was pushed to framebuffer...
			if (pushToFrame) {
				// Clear target depth buffer
				framebuffer();
				// Enable layer buffer
				layerbuffer();
			}
			// Clear depth buffer
			//layerbuffer.clearDepthBuffer();
			// Reset layerbuffer's positions
			layerbuffer.trans	= VecMath::Transform3D();
			layerbuffer.uv		= VecMath::Transform3D();
			// Call onLayerDrawBegin function
			onLayerDrawBegin(layer);
			// Skip layer if applicable
			if (!skipLayer) {
				// Clear buffers
				if (pushToFrame)	layerbuffer.clearBuffers();
				else				layerbuffer.clearDepthBuffer();
				// Call onLayerDrawBegin function
				onPostLayerClear(layer);
				// Render layer
				Drawer::renderLayer(layer);
				// Clear layer push flag
				pushToFrame = false;
				// Call onPreLayerDraw function
				onPreLayerDraw(layer);
				// Render layer buffer
				if (pushToFrame) layerbuffer.render(framebuffer);
			}
			// Call onLayerDrawEnd function
			onLayerDrawEnd(layer);
		}
	}
	// If last layer wasn't rendered, do so
	if (!pushToFrame) layerbuffer.render(framebuffer);
	// Call pre frame drawing function
	onPreFrameDraw();
	// Render frame buffer
	framebuffer.render(toFrameBufferData());
	// Copy screen to queued textures
	copyScreenToQueued();
	// Call rendering end function
	onDrawEnd();
	// Disable depth testing
	setFlag(GL_DEPTH_TEST, false);
	// Display window
	SDL_GL_SwapWindow(window);
}

void App::copyScreenToQueued() {
	if (!screenQueue.empty()) {
		auto& screen = *framebuffer.toFrameBufferData().screen;
		for (Drawer::Texture2D target: screenQueue)
			target->make(screen);
		screenQueue.clear();
	}
}

void App::shouldClose() {
	SDL_Event ev;
	SDL_PollEvent(&ev);
	return ev.type == SDL_QUIT;
}