#ifndef MAKAILIB_EX_GAME_DANMAKU_PLAYER_H
#define MAKAILIB_EX_GAME_DANMAKU_PLAYER_H

#include "core.hpp"

#include "../core/controlable.hpp"
#include "../core/sprite.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct PlayerConfig: GameObjectConfig {};

	struct Player: Controllable, AGameObject {
		Player(PlayerConfig const& cfg):
			AGameObject(cfg) {
				bindmap = Dictionary<String>({
					{"left", "player/left"},
					{"right", "player/right"},
					{"up", "player/up"},
					{"down", "player/down"},
					{"shot", "player/shot"},
					{"bomb", "player/bomb"},
					{"focus", "player/focus"}
				});
			}
	};
}

#endif