#ifndef MAKAILIB_EX_GAME_CORE_APP_H
#define MAKAILIB_EX_GAME_CORE_APP_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {
	/// @brief Dual (2D & 3D) camera.
	struct DualCamera {
		/// @brief 3D camera.
		Graph::Camera3D cam2D;
		/// @brief 2D camera.
		Graph::Camera3D cam3D;

		/// @brief Enables one of the cameras.
		/// @param set2D Whether to enable the 2D camera. By default, it is `false`.
		void use(bool const set2D = false) {
			Graph::Global::camera = set2D ? cam2D : cam3D;
		}
	};

	/// @brief Base game application.
	struct App: Makai::App {
		using Makai::App::App;

		using LayerMap = Map<usize, Graph::Material::BufferMaterial>;

		/// @brief Destructor.
		virtual ~App() {}

		/// @brief Framebuffer material.
		Graph::Material::BufferMaterial& frame = getFrameBuffer().material;
		/// @brief Layerbuffer material.
		Graph::Material::BufferMaterial& layer = getLayerBuffer().material;

		/// @brief Materials for each layer.
		LayerMap layers;

		/// @brief Global camera.
		DualCamera camera;

		/// @brief Gets called when the application begins rendering a layer, before the the layer buffer is cleared.
		/// @param layerID Layer currently being processed. 
		void onLayerDrawBegin(usize const layerID) override {
			layer = layers[layerID];
		}
	};
}

#endif