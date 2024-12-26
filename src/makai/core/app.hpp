#ifndef MAKAILIB_CORE_APP_H
#define MAKAILIB_CORE_APP_H

#include "../compat/ctl.hpp"

#include "extern.hpp"
#include "input/manager.hpp"
#include "../graph/graph.hpp"
#include "../audio/audio.hpp"

/// @brief Makai core API.
namespace Makai {
	/// @brief Resolution.
	struct Resolution {
		/// @brief Width.
		uint width;
		/// @brief Height.
		uint height;
	};

	namespace Config {
		/// @brief Window configuration.
		struct Window {
			/// @brief Window size.
			Resolution	size;
			/// @brief Window title.
			String		title;
			/// @brief Whether to open in fullscreen. By default, it is `false`.
			bool		fullscreen	= false;
		};

		/// @brief Audio configuration.
		struct Audio {
			/// @brief Audio formats to use.
			Makai::Audio::Formats const& formats = {
				Makai::Audio::Format::AF_OGG,
				Makai::Audio::Format::AF_MP3
			};
			/// @brief Number of output channels to use. 1 → mono, 2 → stereo, and so on.
			uint channels	= 2;
			/// @brief Number of audio (non-music) tracks to open.
			uint tracks		= 16;
		};

		/// @brief Renderer configuration.
		struct Renderer {
			/// @brief
			///		Internal rendering size.
			///		If null, will use the window size.
			///		By default, it is null.
			Nullable<Resolution> size = nullptr;
		};

		/// @brief App configuration.
		struct App {
			/// @brief Window configuration.
			Window		window;
			/// @brief Audio configuration.
			Audio		audio		= {};
			/// @brief Renderer configuration.
			Renderer	renderer	= {};
		};
	}

	struct App;

	/// @brief App-specific periodic event.
	struct IUpdateable: CTL::Ex::IPeriodic<App, usize, App&> {
		/// @brief Called every application logic cycle. Must be implemented.
		virtual void onUpdate(usize delta, App& app) = 0;
	};

	/// @brief Main application class.
	struct App {
	public:
		/// @brief Initializes the application.
		/// @param config Configuration.
		App (Config::App const& config);

		/// @brief Copy constructor (deleted).
		App (App const& other)	= delete;
		/// @brief Move constructor (deleted).
		App (App&& other)		= delete;

		/// @brief Application state.
		enum class AppState {
			AS_INVALID = -1,
			AS_CLOSED,
			AS_OPENING,
			AS_RUNNING,
			AS_CLOSING,
		};

		/// @brief Destructor.
		virtual ~App();

		/// @brief Returns whether the user pressed the close button.
		static bool closeButtonPressed();

		/// @brief Sets the window's title.
		/// @param title Title to set to.
		void setWindowTitle(String const& title);

		/// @brief Sets the application's fullscreen state.
		/// @param state `true` = fullscreen, `false` = windowed.
		void setFullscreen(bool const state = false);

		/// @brief Loads the application's shaders from SLF data.
		/// @param main Main object shader.
		/// @param buffer Framebuffer shader.
		void loadShaders(SLF::SLFData const& main, SLF::SLFData const& buffer);

		/// @brief Loads the application's shaders from the default built-in shaders.
		void loadDefaultShaders();

		/// @brief Returns the current open app. Returns `nullptr` if no app is open.
		/// @return Current open app, or `nullptr`.
		static App* current();

		/// @brief Runs the application.
		void run();

		/// @brief Closes the application.
		void close();

		/// @brief Returns whether the application is currently running.
		/// @return Whether application is running.
		bool running();

		/// @brief Returns the application's current state.
		/// @return Current app state.
		AppState state();

		/// @brief Sets the application's window size.
		/// @param size Size to set to.
		void setWindowSize(Vector2 const& size);

		/// @brief Set the application window's opacity.
		/// @param opacity Opacity to set to.
		void setWindowOpacity(float const opacity = 1);

		/// @brief Enables transparent window background in the application.
		/// @warning Not sure if it actually works. SDL is kind of a pain in this.
		void enableTransparentWindowBackground();

		/// @brief Returns the current frame.
		/// @return Current frame.
		usize getCurrentFrame();
		/// @brief Returns the current logic cycle.
		/// @return Current logic cycle.
		usize getCurrentCycle();

		/// @brief Returns the current logic cycle delta, in milliseconds.
		/// @return Cycle delta in milliseconds.
		usize getCycleDelta();
		/// @brief Returns the current frame delta, in milliseconds.
		/// @return Fraame delta in milliseconds.
		usize getFrameDelta();

		/// @brief Returns the application's main framebuffer.
		/// @return Main framebuffer.
		Graph::FrameBuffer& getFrameBuffer();
		/// @brief Returns the application's layer framebuffer.
		/// @return layer framebuffer.
		Graph::FrameBuffer& getLayerBuffer();

		/// @brief Returns the application window's size.
		/// @return Window size.
		Vector2 getWindowSize();

		/// @brief Returns the application window's aspect ratio.
		/// @return Window aspect ratio.
		Vector2 getWindowScale();

		/// @brief Gets called when the application is opened.
		virtual void onOpen() {}

		// Gets called whenever the application is rendering to the screen.

		/// @brief Happens before the screen is rendered, before the frame buffer is cleared.
		virtual void onDrawBegin()		{}
		/// @brief Happens before the screen is rendered, after the frame buffer is cleared.
		virtual void onPostFrameClear()	{}
		/// @brief Gets called when the application begins rendering a layer, before the the layer buffer is cleared.
		/// @param layerID Layer currently being processed.
		virtual void onLayerDrawBegin(usize const layerID)	{}
		/// @brief Gets called when the application begins rendering a layer, after the the layer buffer is cleared.
		/// @param layerID Layer currently being processed.
		virtual void onPostLayerClear(usize const layerID)	{}
		/// @brief Gets called when the application ends rendering a layer, before the layer buffer is drawn to the screen.
		/// @param layerID Layer currently being processed.
		virtual void onPreLayerDraw(usize const layerID)	{pushLayerToFrame();}
		/// @brief Gets called when the application ends rendering a layer, after the layer buffer is drawn to the screen.
		/// @param layerID Layer currently being processed.
		virtual void onLayerDrawEnd(usize const layerID)	{}
		/// @brief Happens after the screen is rendered, before the frame buffer is drawn to the screen.
		virtual void onPreFrameDraw()	{}
		/// @brief Happens after the screen is rendered, after the frame buffer is drawn to the screen.
		virtual void onDrawEnd()		{}

		/// @brief Gets called every logic cycle, along with all `IUpdateable`s.
		/// @param delta Seconds between each logic cycle.
		virtual void onUpdate(float delta)	{}

		/// @brief Gets called at the end of the processing cycle, when the application was requested to close.
		/// @return Whether the application should close.
		virtual bool onAppClosureRequest() {return true;}

		/// @brief Gets called when the application is closing. Happens before window is terminated.
		virtual void onClose()	{}

		/// @brief Queues a texture to recieve a copy of the screen.
		/// @param target Texture to queue.
		void queueScreenCopy(Graph::Texture2D target);

		/// @brief Removes a texture from the screen copy queue.
		/// @param target Texture to unqueue.
		void unqueueScreenCopy(Graph::Texture2D target);

		/// @brief
		///		Skips the drawing process of the current layer being drawn.
		///		Can only be used during onLayerDrawBegin().
		void skipDrawingThisLayer();

		/// @brief
		///		Queues the current data in the layerbuffer to be pushed to the framebuffer.
		///		Can only be used during onPreLayerDraw().
		void pushLayerToFrame();

		/// @brief Application window background color.
		Vector4 background = Graph::Color::BLACK;

		/// @brief Input manager.
		Input::Manager input;

		/// @brief Maximum frame rate.
		float maxFrameRate = 30.0;

		/// @brief Maximum logic cycle rate.
		float maxCycleRate = 60.0;

		/// @brief Speed scale.
		float speed = 1.0f;

		/// @brief Notification handler.
		Notifier notifier;

	protected:
		Graph::Base::BufferObject toBufferObject();

	private:
		/// @brief Delta between processes in milliseconds.
		usize frameDelta = 0, cycleDelta = 0;

		/// @brief Whether the current render layer should not be rendered.
		bool skipLayer		= false;
		/// @brief Whether the current render layer should be drawn to the framebuffer.
		bool pushToFrame	= false;

		/// @brief Screen queue.
		List<Graph::Texture2D> screenQueue;

		/// @brief Main framebuffer.
		Graph::FrameBuffer framebuffer;
		/// @brief Layer framebuffer.
		Graph::FrameBuffer layerbuffer;

		/// @brief Window resolution.
		unsigned int width, height;

		void finalize();

		void render();

		void copyScreenToQueued();

		/// @brief Frame counter.
		usize frame = 0;

		/// @brief Logic cycle counter.
		usize cycle = 0;

		/// @brief Current execution state.
		AppState appState = AppState::AS_CLOSED;

		/// @brief App window.
		Extern::Resource window;
	};
}

#endif // MAKAILIB_CORE_APP_H
