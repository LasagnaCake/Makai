#ifndef MAKAILIB_GRAPH_RENDERER_CORE_H
#define MAKAILIB_GRAPH_RENDERER_CORE_H

#include "../../../compat/ctl.hpp"
#include "../core.hpp"

/// @brief Makai core API.
namespace Makai {
	struct App;
}

/// @brief Graphical facilities.
namespace Makai::Graph {
	class ADrawable;

	/// @brief Graphical renderer coordinator.
	struct RenderServer {
		/// @brief Rendering executor.
		struct IEntity {
			virtual ~IEntity() {}

		protected:
			virtual void doRender() = 0;

			friend class RenderServer;
		};

		/// @brief Layer structure type.
		using Layers	= Groups<ref<IEntity>>;

		/// @brief Renders a specific layer.
		/// @param layer Layer to render.
		static void renderLayer(usize const layer);

		/// @brief Returns all layers in the server.
		/// @return Layers in server.
		static typename Layers::IdentifierListType getLayers() {
			return layers.all();
		}

		/// @brief Returns whether a given layer has any drawable objects in it.
		/// @param layer Layer to check.
		/// @return Whether it is empty.
		static bool isLayerEmpty(usize const layer) {
			return layers.get(layer).empty();
		}

	private:
		/// @brief Underlying layer structure.
		inline static Layers layers;

		static void renderLayer(typename Layers::GroupType const& layer);

		friend class ADrawable;
		friend class ::Makai::App;
	};

	/// @brief Callback called when rendering.
	using IServerEntity = typename RenderServer::IEntity;
}

#endif // MAKAILIB_GRAPH_RENDERER_CORE_H
