#include "graph.hpp"

#define MAKAILIB_USE_OPENGL     (1)
#define MAKAILIB_USE_VULKAN     (2)
#define MAKAILIB_USE_DIRECT3D   (3)

#ifndef MAKAILIB_GRAPHICAL_API
#define MAKAILIB_GRAPHICAL_API MAKAILIB_USE_OPENGL
#endif

Makai::Graph::GraphicalAPI Makai::Graph::getUsedAPI() {
    #if (MAKAILIB_GRAPHICAL_API == MAKAILIB_USE_OPENGL)
    return Makai::Graph::GraphicalAPI::GA_OPENGL;
    #endif
}