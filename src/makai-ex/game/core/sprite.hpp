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
	using SpriteInstance	= Instance<Sprite>;
	/// @brief Sprite handle type.
	using SpriteHandle		= Handle<Sprite>;
	/// @brief Sprite unique pointer type.
	using SpriteHolder		= Unique<Sprite>;

	/// @brief Tile instance type.
	using TileInstance		= Instance<Tile>;
	/// @brief Tile handle type.
	using TileHandle		= Handle<Tile>;
	/// @brief Tile unique pointer type.
	using TileHolder		= Unique<Tile>;

	/// @brief Three-patch shape instance type.
	using ThreePatchInstance	= Instance<Graph::Ref::ThreePatch1D>;
	/// @brief Three-patch shape handle type.
	using ThreePatchHandle		= Handle<Graph::Ref::ThreePatch1D>;
	/// @brief Three-patch shape unique pointer type.
	using ThreePatchHolder		= Unique<Graph::Ref::ThreePatch1D>;
	
	/// @brief Nine-patch shape instance type.
	using NinePatchInstance		= Instance<Graph::Ref::NinePatch2D>;
	/// @brief Nine-patch shape handle type.
	using NinePatchHandle		= Handle<Graph::Ref::NinePatch2D>;
	/// @brief Nine-patch shape unique pointer type.
	using NinePatchHolder		= Unique<Graph::Ref::NinePatch2D>;
}

#endif