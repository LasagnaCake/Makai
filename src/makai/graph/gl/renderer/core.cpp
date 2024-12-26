#include "core.hpp"

using namespace Makai; using namespace Makai::Graph;

void RenderServer::renderLayer(usize const layer) {
	return renderLayer(layers[layer]);
}

void RenderServer::renderLayer(typename RenderServer::Layers::GroupType const& layer) {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("RenderServer::renderLayer");
	#endif // MAKAILIB_DEBUG
	for (auto draw: layer)
		(*draw)();
}
