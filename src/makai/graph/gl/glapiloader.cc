#ifndef MAKAILIB_GRAPH_GLAPILOADER_CC
#define MAKAILIB_GRAPH_GLAPILOADER_CC

#define MAKAILIB_USE_GL3W 1
#define MAKAILIB_USE_GLAD 2

#ifndef MAKAILIB_GL_LOADER
#define MAKAILIB_GL_LOADER MAKAILIB_USE_GLAD
#endif // MAKAILIB_GL_LOADER

#if (MAKAILIB_GL_LOADER == MAKAILIB_USE_GL3W)
//	#include <glapi.hpp>
	#include <GL/gl3w.h>
#elif (MAKAILIB_GL_LOADER == MAKAILIB_USE_GLAD)
	#include <glad/glad.h>
	#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
		#include <windows.h>
		#define SDL_MAIN_HANDLED
	#endif
	#include <SDL2/SDL.h>
#endif
#include <GL/gl.h>

#endif // MAKAILIB_GRAPH_GLAPILOADER_CC
