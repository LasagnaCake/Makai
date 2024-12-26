#ifndef MAKAILIB_GRAPH_GLOBAL_H
#define MAKAILIB_GRAPH_GLOBAL_H

#include "../../compat/ctl.hpp"
#include "camera.hpp"
#include "material/material.hpp"

/// @brief Graphical facilites.
namespace Makai::Graph {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	/// @brief Global graphical parameters.
	struct Global {
		/// @brief Camera.
		static Camera3D					camera;
		/// @brief World matrix.
		static Matrix4x4				space;
		/// @brief World material.
		static Material::WorldMaterial	world;
	};
	#pragma GCC diagnostic pop

	/// @brief Scope global state capturer.
	struct GlobalState {
		/// @brief Captures the global state.
		GlobalState(): camera(Global::camera), space(Global::space), world(Global::world) {}

		/// @brief Captures and sets the global state.
		/// @param newCamera New camera.
		/// @param newSpace New world matrix.
		/// @param newWorld New world material.
		GlobalState(
			Camera3D const&					newCamera,
			Matrix4x4 const&				newSpace,
			Material::WorldMaterial const&	newWorld
		): GlobalState() {
			set(newCamera, newSpace, newWorld);
		}

		/// @brief Sets the global state.
		/// @param newCamera New camera.
		/// @param newSpace New world matrix.
		/// @param newWorld New world material.
		void set(
			Camera3D const&					newCamera,
			Matrix4x4 const&				newSpace,
			Material::WorldMaterial const&	newWorld
		) {
			Global::camera	= newCamera;
			Global::space	= newSpace;
			Global::world	= newWorld;
		}

		/// @brief Resets the global state to its previous state.
		void reset() {
			Global::camera	= camera;
			Global::space	= space;
			Global::world	= world;
		}

		/// @brief Resets the global state to its previous state.
		~GlobalState() {reset();}

		/// @brief Previous camera.
		Camera3D const					camera;
		/// @brief Previous world matrix.
		Matrix4x4 const					space;
		/// @brief Previous world material.
		Material::WorldMaterial const	world;
	};
}

#endif // MAKAILIB_GRAPH_GLOBAL_H
