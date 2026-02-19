#ifndef AUDIO_SAMPLE_FRAMES
#define AUDIO_SAMPLE_FRAMES 2048
#endif // AUDIO_SAMPLE_FRAMES

//#include "../graph/gl/glapiloader.cc"

#include "../compat/ctl.hpp"

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <windows.h>
#include <dwmapi.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#else
#include <SDL2/SDL.h>
#endif

#include <SDL2/SDL_mixer.h>

#define sdlWindow ((SDL_Window*)window)

#include "../file/slf.hpp"
#include "app.hpp"

using namespace Makai;

void setGLAttribute(SDL_GLattr const a, int const v) {
	if (SDL_GL_SetAttribute(a, v))
		throw Error::FailedAction(
			"Failed to set attribute!",
			SDL_GetError(),
			CTL_CPP_UNKNOWN_SOURCE
		);
}

App* mainApp = nullptr;

App::App (Config::App const& config):
appState(App::AppState::AS_CLOSED) {
	// If there is another app open, throw error
	if (mainApp)
		throw Error::DuplicateValue(
			"Cannot have two apps open at the same time!",
			"Having two apps open is forbidden!",
			CTL_CPP_PRETTY_SOURCE
		);
	else mainApp = this;
	DEBUGLN("Starting app...");
	// Save window resolution
	width = config.window.size.width;
	height = config.window.size.height;
	// Initialize SDL
	DEBUGLN("Starting SDL...");
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		throw Error::FailedAction(
			"Unable to start SDL!",
			SDL_GetError(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	DEBUGLN("Started!");
	// Initialize sound system
	DEBUGLN("Starting Audio System...");
	audio.open();
	DEBUGLN("Started!");
	// Create window and make active
	DEBUGLN("Creating window...");
	window = (Extern::Resource)SDL_CreateWindow(
		config.window.title.cstr(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL
	);
	if (!window) {
		throw Error::FailedAction(
			"Failed to create window!",
			SDL_GetError(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	SDL_SetWindowFullscreen(sdlWindow, config.window.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
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
	Makai::Input::Manager::setTargetWindow(window);
	DEBUGLN("Created!");
	DEBUGLN("Starting graphical API...");
	// Try and initialize graphical API
	Makai::Graph::API::open();
	if (!Makai::Graph::API::hasRequiredVersion()) {
		throw Error::FailedAction(
			"Your computer does not support the required graphical API version!",
			"Required version: " + Makai::Graph::API::name(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	DEBUGLN("Started!");
	// Toggle depth test & blend
	{
		using enum Makai::Graph::API::Facility;
		Makai::Graph::API::toggle(true, GAF_DEPTH_TEST, GAF_BLEND);
	}
	// Get internal render resolution
	Resolution res = config.window.size;
	if (config.renderer.size) {
		res = *config.renderer.size;
		Graph::API::setViewport(res.width, res.height);
	}
	// Setup camera
	DEBUGLN("Setting starting camera...");
	Graph::Global::camera.aspect	= Vector2(res.width, res.height);
	Graph::Global::camera.fov		= Math::radians<float>(45);
	DEBUGLN("creating default framebuffer...");
	// Create framebuffer
	framebuffer.create(res.width, res.height);
	// Create layer buffer
	layerbuffer.create(res.width, res.height);
	framebuffer();
	DEBUGLN("All core systems initialized!");
}

void App::loadShaders(SLF::SLFData const& main, SLF::SLFData const& buffer) {
	// Create buffer shaders
	DEBUGLN("Creating shaders...");
	DEBUGLN("> Making framebuffer & layerbuffer shader");
	framebuffer.shader.create(buffer);
	layerbuffer.shader = framebuffer.shader;
	// Create main shader
	DEBUGLN("> Making default shader");
	Makai::Graph::Shader::DEFAULT.create(main);
	DEBUGLN("Done!");
}

void App::loadDefaultShaders() {
	framebuffer.shader.create(
		Makai::Graph::Shader::Program::DEFAULT_BUFFER_VERT,
		Makai::Graph::Shader::Program::DEFAULT_BUFFER_FRAG
	);
	layerbuffer.shader = framebuffer.shader;
	Makai::Graph::Shader::DEFAULT.create(
		Makai::Graph::Shader::Program::DEFAULT_MAIN_VERT,
		Makai::Graph::Shader::Program::DEFAULT_MAIN_FRAG
	);
}

App::~App() {
	finalize();
	if (mainApp == this)
		mainApp = nullptr;
}

void App::setWindowTitle(String const& title) {
	SDL_SetWindowTitle(sdlWindow, title.cstr());
}

void App::setFullscreen(bool const state) {
	SDL_SetWindowFullscreen(sdlWindow, state);
}

inline bool isAppClosing(App::AppState const state) {
	return
		state == App::AppState::AS_CLOSING
	||	state == App::AppState::AS_INVALID
	;
}

void App::run() {
	if (isAppClosing(appState)) return finalize();
	appState = App::AppState::AS_OPENING;
	// The timer process
	auto timerFunc	= [&](float delta)-> void {
		ATweenPeriodic::process(1);
		ATimerPeriodic::process(1);
		CTL::Ex::Collision::C2D::Server::process();
	};
	// The logical process
	auto logicFunc	= [&](float delta)-> void {
		onUpdate(delta);
		AUpdateable::process(delta, *this);
	};
	// Clear screen
	Makai::Graph::API::setClearColor(background);
	Makai::Graph::API::clear(Makai::Graph::API::Buffer::GAB_COLOR);
	// Render simple frame
	SDL_GL_SwapWindow(sdlWindow);
	// Call on open function
	onOpen();
	// SDL's events
	SDL_Event event;
	// Delta time
	float frameRate = 1.0/maxFrameRate;
	float cycleRate = 1.0/maxCycleRate;
	// Last time of execution
	usize frameTicks = SDL_GetTicks() + frameRate * 1000.0;
	usize cycleTicks = SDL_GetTicks() + cycleRate * 1000.0;
	// Refresh mouse capture stuff
	input.refreshCapture();
	// If app is should close, do so
	if (isAppClosing(appState)) return finalize();
	// Change app state
	appState = App::AppState::AS_RUNNING;
	// While program is running...
	while(appState == App::AppState::AS_RUNNING) {
		// Poll events and check if should close
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					DEBUGLN("SDL Event: EXIT");
					appState = App::AppState::AS_CLOSING;
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
		// Get rates
		cycleRate	= 1.0/maxFrameRate;
		cycleRate	= 1.0/maxCycleRate;
		// Get deltas
		cycleDelta = SDL_GetTicks() - cycleTicks;
		frameDelta = SDL_GetTicks() - frameTicks;
		// If should process, then do so
		#ifndef MAKAILIB_PROCESS_RENDER_BEFORE_LOGIC
		if (cycleDelta > (cycleRate * 1000.0 - 1)) {
			// Update audio system
			// Audio::updateAll();
			// Update input manager
			input.update();
			// Get current time
			cycleTicks = SDL_GetTicks();
			// Increment cycle counter
			cycle++;
			// Do timer-related stuff
			timerFunc(cycleRate * speed);
			#ifndef MAKAILIB_FRAME_DEPENDENT_PROCESS
			// Do normal logic-related stuff
			logicFunc(cycleRate * speed);
			#endif // FRAME_DEPENDENT_PROCESS
		}
		#endif
		if (frameDelta > (frameRate * 1000 - 1)) {
			// Get current time
			#ifdef MAKAILIB_FRAME_DEPENDENT_PROCESS
			cycleTicks =
			#endif
			frameTicks = SDL_GetTicks();
			// increment frame counter
			frame++;
			// Update audio system
			Audio::APeriodicAudioEvent::process();
			#ifdef MAKAILIB_FRAME_DEPENDENT_PROCESS
			// Increment cycle counter
			cycle++;
			// Do normal logic-related stuff
			logicFunc(frameRate);
			#endif // FRAME_DEPENDENT_PROCESS
			// Render screen
			render();
		}
		#ifdef MAKAILIB_PROCESS_RENDER_BEFORE_LOGIC
		if (cycleDelta > (cycleRate * 1000.0 - 1)) {
			// Update audio system
			// Audio::updateAll();
			// Update input manager
			input.update();
			// Get current time
			cycleTicks = SDL_GetTicks();
			// Increment cycle counter
			cycle++;
			// Do timer-related stuff
			timerFunc(cycleRate * speed);
			#ifndef MAKAILIB_FRAME_DEPENDENT_PROCESS
			// Do normal logic-related stuff
			logicFunc(cycleRate * speed);
			#endif // FRAME_DEPENDENT_PROCESS
		}
		#endif
		if (!onAppClosureRequest())
			appState = App::AppState::AS_RUNNING;
	}
	// Terminate program
	finalize();
}

void App::close() {
	appState = App::AppState::AS_CLOSING;
}

bool App::running() {
	return (appState != App::AppState::AS_CLOSED);
}

App::AppState App::state() {
	return (appState);
}

void App::setWindowSize(Vector2 const& size) {}

Reference<App> App::current() {
	return mainApp;
}

usize App::getCurrentFrame() {
	return frame;
}

usize App::getCurrentCycle() {
	return cycle;
}

usize App::getCycleDelta() {
	return cycleDelta;
}

usize App::getFrameDelta() {
	return frameDelta;
}

Graph::FrameBuffer& App::getFrameBuffer() {
	return framebuffer;
}

Graph::FrameBuffer& App::getLayerBuffer() {
	return layerbuffer;
}

Vector2 App::getWindowSize() {
	return Vector2(width, height);
}

Vector2 App::getWindowScale() {
	Vector2 ws = getWindowSize();
	return Vector2(ws.x / ws.y, 1);
}

Graph::Base::BufferObject App::toBufferObject() {
	Graph::Base::BufferObject self;
	self.id		= 0;
	self.width	= width;
	self.height	= height;
	return self;
}

void App::queueScreenCopy(Graph::Texture2D target) {
	screenQueue.pushBack(target);
}

void App::unqueueScreenCopy(Graph::Texture2D target) {
	screenQueue.eraseIf([&] (auto e) {return e == target;});
}

void App::setWindowOpacity(float const opacity) {
	SDL_SetWindowOpacity(sdlWindow, opacity);
}

void App::setBorderless(bool const borderless) {
	SDL_SetWindowBordered(sdlWindow, borderless ? SDL_FALSE : SDL_TRUE);
}

void App::enableClearWindow() {
	#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
	SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdlWindow, &wmInfo);
    HWND hWnd = wmInfo.info.win.window;
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	//SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
	HRGN region = CreateRectRgn(-1, -1, 0, 0);
	// Backported from SDL3 source: https://github.com/libsdl-org/SDL/blob/646f1f243f8be853531703c1f656325580435f9e/src/video/windows/SDL_windowswindow.c#L792
	DWM_BLURBEHIND blur;
	blur.dwFlags = (DWM_BB_ENABLE | DWM_BB_BLURREGION);
	blur.fEnable = TRUE;
	blur.hRgnBlur = region;
	blur.fTransitionOnMaximized = FALSE;
	DwmEnableBlurBehindWindow(hWnd, &blur);
	DeleteObject(region);
	#endif
}

void App::skipDrawingThisLayer() {skipLayer = true;}

void App::pushLayerToFrame() {pushToFrame = true;}

void App::finalize() {
	if (appState != App::AppState::AS_CLOSING)
		return;
	DEBUGLN("\nClosing incoherent program...");
	// Call final function
	onClose();
	// Remove window from input manager
	DEBUGLN("Detaching target window...");
	Makai::Input::Manager::clearTargetWindow(window);
	// Close sound system
	DEBUGLN("Closing sound system...");
	audio.close();
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
	appState = App::AppState::AS_CLOSED;
}

inline void setDepthTest(bool const state) {
	Makai::Graph::API::toggle(Makai::Graph::API::Facility::GAF_DEPTH_TEST, state);
}

void App::render() {
	// Initialize render
	Makai::Graph::API::beginRender();
	// Clear screen
	Makai::Graph::API::setClearColor(background);
	Makai::Graph::API::clear(
		Makai::Graph::API::Buffer::GAB_COLOR,
		Makai::Graph::API::Buffer::GAB_DEPTH
	);
	// Enable depth testing
	setDepthTest(true);
	// Enable frame buffer
	#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
	framebuffer();
	#endif // MAKAILIB_DO_NOT_USE_BUFFERS
	// Call rendering start function
	onDrawBegin();
	// Clear frame buffer
	#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
	framebuffer.clearBuffers();
	#endif // MAKAILIB_DO_NOT_USE_BUFFERS
	// Set frontface order
	Makai::Graph::API::setFrontFace(true);
	// Call post frame clearing function
	onPostFrameClear();
	// Enable layer buffer
	#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
	layerbuffer();
	layerbuffer.clearBuffers();
	#endif // MAKAILIB_DO_NOT_USE_BUFFERS
	// Draw objects
	for (auto layer: Graph::RenderServer::layers) {
		if (!layer.value.empty()) {
			// Clear layer skip flag
			skipLayer = false;
			// If previous layer was pushed to framebuffer...
			#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
			if (pushToFrame) {
				// Clear target depth buffer
				framebuffer();
				// Enable layer buffer
				layerbuffer();
			}
			#endif // MAKAILIB_DO_NOT_USE_BUFFERS
			// Call onLayerDrawBegin function
			onLayerDrawBegin(layer.key);
			// Skip layer if applicable
			if (!skipLayer) {
				// Clear buffers
				#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
				if (pushToFrame)	layerbuffer.clearBuffers();
				else				layerbuffer.clearDepthBuffer();
				#else
				Makai::Graph::API::clear(Makai::Graph::API::Buffer::GAB_DEPTH);
				#endif
				// Call onLayerDrawBegin function
				onPostLayerClear(layer.key);
				// Render layer to buffer
				Makai::Graph::RenderServer::renderLayer(layer.value);
				// Clear layer push flag
				pushToFrame = false;
				// Call onPreLayerDraw function
				onPreLayerDraw(layer.key);
				// Push layer to framebuffer
				#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
				if (pushToFrame) layerbuffer.render(framebuffer);
				#endif // MAKAILIB_DO_NOT_USE_BUFFERS
			}
			// Prune layer for removed objects
			layer.value.removeLike(nullptr);
			// Call onLayerDrawEnd function
			onLayerDrawEnd(layer.key);
		}
	}
	// If last layer wasn't rendered, do so
	#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
	if (!pushToFrame) layerbuffer.render(framebuffer);
	#endif // MAKAILIB_DO_NOT_USE_BUFFERS
	// Set clear for next frame
	pushToFrame = true;
	// Call pre frame drawing function
	onPreFrameDraw();
	// Render frame buffer
	#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
	framebuffer.renderTo(toBufferObject());
	#endif // MAKAILIB_DO_NOT_USE_BUFFERS
	// Copy screen to queued textures
	copyScreenToQueued();
	// Call rendering end function
	onDrawEnd();
	// Disable depth testing
	setDepthTest(false);
	// Finalize render
	Makai::Graph::API::endRender();
	// Display window
	SDL_GL_SwapWindow(sdlWindow);
}

void App::copyScreenToQueued() {
	if (!screenQueue.empty()) {
		#ifndef MAKAILIB_DO_NOT_USE_BUFFERS
		auto screen = framebuffer.storage().screen;
		for (Graph::Texture2D target: screenQueue)
			target.make(screen);
		#endif // MAKAILIB_DO_NOT_USE_BUFFERS
		screenQueue.clear();
	}
}

bool App::closeButtonPressed() {
	SDL_Event ev;
	SDL_PollEvent(&ev);
	return ev.type == SDL_QUIT;
}
