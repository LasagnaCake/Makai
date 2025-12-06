#ifndef MAKAILIB_EX_GAME_SPRITE_H
#define MAKAILIB_EX_GAME_SPRITE_H

#include <makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {
	/// @brief Animated sprite type.
	using Sprite		= Graph::Ref::AnimationPlane;
	/// @brief Tile sprite type.
	using Tile			= Graph::Ref::TilePlane;

	/// @brief Sprite instance type.
	using SpriteInstance	= Makai::Instance<Sprite>;
	/// @brief Sprite handle type.
	using SpriteHandle		= Makai::Handle<Sprite>;

	/// @brief Tile instance type.
	using TileInstance		= Makai::Instance<Tile>;
	/// @brief Tile handle type.
	using TileHandle		= Makai::Handle<Tile>;

	/// @brief Three-patch shape instance type.
	using ThreePatchInstance	= Instance<Graph::Ref::ThreePatch1D>;
	/// @brief Three-patch shape handle type.
	using ThreePatchHandle		= Handle<Graph::Ref::ThreePatch1D>;
	
	/// @brief Nine-patch shape instance type.
	using NinePatchInstance		= Instance<Graph::Ref::NinePatch2D>;
	/// @brief Nine-patch shape handle type.
	using NinePatchHandle		= Handle<Graph::Ref::NinePatch2D>;
}

#endif