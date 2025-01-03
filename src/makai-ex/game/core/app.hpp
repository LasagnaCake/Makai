#ifndef MAKAILIB_EX_GAME_CORE_H
#define MAKAILIB_EX_GAME_CORE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {
	struct App: Makai::App {
		using App::App;

		using LayerMap = Map<usize, Graph::Material::BufferMaterial>;

		virtual ~App() {};

		Graph::Material::BufferMaterial& frame = getFrameBuffer().material;
		Graph::Material::BufferMaterial& layer = getLayerBuffer().material;

		LayerMap layers;
	};
}

#endif