#ifndef MAKAILIB_GRAPH_CORE_H
#define MAKAILIB_GRAPH_CORE_H

#include "../../compat/ctl.hpp"

#define SUGGEST_NVIDIA_GPU extern "C" __declspec(dllexport) uint32 NvOptimusEnablement = 1
#define SUGGEST_AMD_GPU extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1

#define SUGGEST_DEDICATED_GPU SUGGEST_NVIDIA_GPU; SUGGEST_AMD_GPU

/// @brief Underlying graphical API facilities.
namespace Makai::Graph::API {
	/// @brief Graphical API debugging facilities.
	namespace Debug {
		/// @brief Debug context.
		struct Context {
			Context(String const& name);
			~Context();

			void append(String const& message);

			String getMessage();

			String report();
		private:
			String message;
			String const name;
			String const indent;
		};

		void addEntry(String const& type);
		void addEntry(String const& type, String const& value);
	}

	/// @brief Initializes the graphical API.
	/// @throw Error::FailedAction when API fails to initialize.
	void open();

	/// @brief Sets the viewport size.
	/// @param width Viewport width.
	/// @param height Viewport height.
	/// @param x Viewport start X position.
	/// @param y Viewport start Y position.
	void setViewport(uint32 const width, uint32 const height, uint32 const x = 0, uint32 const y = 0);

	/// @brief Returns whether the device has the required graphical API version.
	/// @return Whether device has required API version.
	bool hasRequiredVersion();

	/// @brief Returns the name of the graphical API.
	/// @return API name.
	String name();

	/// @brief Graphical API facility.
	enum class Facility {
		GAF_DEPTH_TEST,
		GAF_BLEND,
		GAF_DEBUG

	};

	/// @brief Toggles a graphical API facility.
	/// @param fac Facility to toggle.
	/// @param state State to toggle facility to.
	void toggle(Facility const& fac, bool const state);

	/// @brief Toggles a series of graphical API facilites.
	/// @tparam ...Args Argument types.
	/// @param state State to toggle facilities to.
	/// @param ...facs Facilities to toggle.
	template<typename... Args>
	inline void toggle(bool const state, Args const&... facs)
	requires (... && Makai::Type::Equal<Args, Facility>) {
		(..., toggle(facs, state));
	}

	/// @brief Graphical API buffer.
	enum class Buffer {
		GAB_COLOR,
		GAB_DEPTH,
		GAB_STENCIL
	};

	/// @brief Clears a buffer.
	/// @param buffer Buffer to clear.
	void clear(Buffer const& buffer);

	/// @brief Clears a series of buffers.
	/// @tparam ...Args Argument types.
	/// @param ...buffers Buffers to clear.
	template<typename... Args>
	inline void clear(Args const&... buffers)
	requires (... && Makai::Type::Equal<Args, Buffer>) {
		(..., clear(buffers));
	}

	/// @brief Declares the start of all render operations.
	void beginRender();
	/// @brief Declares the end of all render operations.
	void endRender();

	/// @brief Sets the screen's clear color.
	/// @param color Clear color.
	void setClearColor(Vector4 const& color);

	/// @brief Sets the front face detection mode.
	/// @param clockwise Whether a front face has vertices in clockwise winding order. By default, it is true.
	void setFrontFace(bool const clockwise = true);

	/// @brief Sets the main buffer as the framebuffer to use.
	void useMainBuffer();
}

#endif // MAKAILIB_GRAPH_CORE_H
