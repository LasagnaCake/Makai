#ifndef MAKAILIB_EX_GAME_CORE_APP_H
#define MAKAILIB_EX_GAME_CORE_APP_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {
	struct DualCamera {
		Graph::Camera3D cam2D;
		Graph::Camera3D cam3D;

		void use(bool const set2D = false) {
			Graph::Global::camera = set2D ? cam2D : cam3D;
		}
	};

	struct App: Makai::App {
		using Makai::App::App;

		using LayerMap = Map<usize, Graph::Material::BufferMaterial>;

		virtual ~App() {};

		Graph::Material::BufferMaterial& frame = getFrameBuffer().material;
		Graph::Material::BufferMaterial& layer = getLayerBuffer().material;

		LayerMap layers;

		DualCamera camera;

		void onLayerDrawBegin(usize const layerID) override {
			layer = layers[layerID];
		}
	};
}

#endif