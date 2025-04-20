#ifndef MAKAILIB_EX_GAME_DANMAKU_LAYERS_H
#define MAKAILIB_EX_GAME_DANMAKU_LAYERS_H

#include <makai/makai.hpp>

#ifndef MK_EX_DANMAKU_SUBLAYER_COUNT
#define MK_EX_DANMAKU_SUBLAYER_COUNT 8
#endif

#ifndef MK_EX_DANMAKU_UI_LAYER_OFFSET
#define MK_EX_DANMAKU_UI_LAYER_OFFSET 32
#endif

#define MK_EX_DANMAKU_TRUE_SUBLAYER_COUNT (MK_EX_DANMAKU_SUBLAYER_COUNT * 2 + 1)
#define MK_EX_DANMAKU_TRUE_UI_LAYER_OFFSET (MK_EX_DANMAKU_TRUE_SUBLAYER_COUNT * MK_EX_DANMAKU_UI_LAYER_OFFSET)

namespace Makai::Ex::Game::Danmaku {
	/// @brief Render layer defaults.
	namespace Render::Layer {
		#define MK_EX_DANMAKU_LAYER(NAME)\
			NAME##_BOTTOM_LAYER,\
			NAME##_LAYER		= (NAME##_BOTTOM_LAYER + (MK_EX_DANMAKU_SUBLAYER_COUNT)),\
			NAME##_TOP_LAYER	= (NAME##_LAYER + (MK_EX_DANMAKU_SUBLAYER_COUNT))
		
		#define MK_EX_DANMAKU_LAYER_CUSTOM(NAME, INDEX)\
			NAME##_BOTTOM_LAYER = (INDEX),\
			NAME##_LAYER		= (NAME##_BOTTOM_LAYER + (MK_EX_DANMAKU_SUBLAYER_COUNT)),\
			NAME##_TOP_LAYER	= (NAME##_LAYER + (MK_EX_DANMAKU_SUBLAYER_COUNT))
		
		#define MK_EX_DANMAKU_LAYER_WITH_OVERLAY(NAME)\
			MK_EX_DANMAKU_LAYER(NAME),\
			MK_EX_DANMAKU_LAYER(NAME##_OVERLAY)

		#define MK_EX_DANMAKU_LAYER_PLAYER_THINGS(NAME)\
			MK_EX_DANMAKU_LAYER_WITH_OVERLAY(NAME##_SPELL_BG),\
			MK_EX_DANMAKU_LAYER(NAME##_SPELL),\
			MK_EX_DANMAKU_LAYER(NAME##_LASER),\
			MK_EX_DANMAKU_LAYER(NAME##_BULLET),\
			MK_EX_DANMAKU_LAYER(NAME##_ITEM),\
			MK_EX_DANMAKU_LAYER(NAME##_OPTION),\
			MK_EX_DANMAKU_LAYER(NAME)

		#define MK_EX_DANMAKU_LAYER_ENEMY_THINGS(NAME)\
			MK_EX_DANMAKU_LAYER(NAME##_BULLET),\
			MK_EX_DANMAKU_LAYER(NAME##_LASER),\
			MK_EX_DANMAKU_LAYER(NAME)
		
		#define MK_EX_DANMAKU_DUAL_LAYER(NAME, TYPE)\
			TYPE(NAME##1),\
			TYPE(NAME##2)

		#define MK_EX_DANMAKU_LAYER_SPELL_BG(NAME)\
			MK_EX_DANMAKU_LAYER_WITH_OVERLAY(NAME##_SPELL_BG)

		enum Layer: usize {
			MK_EX_DANMAKU_LAYER_WITH_OVERLAY(WORLD),
			MK_EX_DANMAKU_DUAL_LAYER(BOSS, MK_EX_DANMAKU_LAYER_SPELL_BG),
			MK_EX_DANMAKU_DUAL_LAYER(PLAYER, MK_EX_DANMAKU_LAYER_PLAYER_THINGS),
			MK_EX_DANMAKU_DUAL_LAYER(ITEM, MK_EX_DANMAKU_LAYER),
			MK_EX_DANMAKU_DUAL_LAYER(ENEMY, MK_EX_DANMAKU_LAYER_ENEMY_THINGS),
			MK_EX_DANMAKU_LAYER(HITBOX),
			MK_EX_DANMAKU_LAYER(STAGE_CARD),
			MK_EX_DANMAKU_LAYER_CUSTOM(INGAME_OVERLAY, MK_EX_DANMAKU_TRUE_UI_LAYER_OFFSET),
			MK_EX_DANMAKU_LAYER(INGAME_UI),
			MK_EX_DANMAKU_LAYER(DIALOG),
			MK_EX_DANMAKU_LAYER(MENU),
		};

		#undef MK_EX_DANMAKU_LAYER
		#undef MK_EX_DANMAKU_LAYER_CUSTOM
		#undef MK_EX_DANMAKU_LAYER_WITH_OVERLAY
		#undef MK_EX_DANMAKU_LAYER_PLAYER_THINGS
		#undef MK_EX_DANMAKU_LAYER_ENEMY_THINGS
		#undef MK_EX_DANMAKU_DUAL_LAYER
		#undef MK_EX_DANMAKU_LAYER_SPELL_BG
	}

	/// @brief Collision-related facilities.
	namespace Collision {
		namespace GJK = CTL::Ex::Collision::GJK;
		namespace C2D = CTL::Ex::Collision::C2D;
	}
	
	/// @brief Collision layer defaults.
	namespace Collision::Layer {
		/// @brief Player layer.
		constexpr uint16 PLAYER				= 0x0;
		/// @brief Player bullet layer.
		constexpr uint16 PLAYER_BULLET		= 0x1;
		/// @brief Player laser layer.
		constexpr uint16 PLAYER_LASER		= 0x2;
		/// @brief Player spell layer.
		constexpr uint16 PLAYER_SPELL		= 0x3;
		/// @brief Player grazebox layer.
		constexpr uint16 PLAYER_GRAZEBOX	= 0x4;
		/// @brief Player itembox layer.
		constexpr uint16 PLAYER_ITEMBOX		= 0x5;
		/// @brief Enemy layer.
		constexpr uint16 ENEMY				= 0x6;
		/// @brief Enemy bullet layer.
		constexpr uint16 ENEMY_BULLET		= 0x7;
		/// @brief Enemy laser layer.
		constexpr uint16 ENEMY_LASER		= 0x8;
		/// @brief Item layer.
		constexpr uint16 ITEM				= 0x9;
		/// @brief Bullet eraser layer.
		constexpr uint16 BULLET_ERASER		= 0xF;

		/// @brief Returns the collision layer as its name.
		/// @param layer Layer to get name for.
		/// @return Layer name, or `UNKNOWN`.
		constexpr String asName(uint16 const layer) {
			#define MKEX_DMK_LAYER_NAME_CASE(LAYER) case LAYER: return #LAYER
			switch (layer) {
				MKEX_DMK_LAYER_NAME_CASE(PLAYER);
				MKEX_DMK_LAYER_NAME_CASE(PLAYER_BULLET);
				MKEX_DMK_LAYER_NAME_CASE(PLAYER_LASER);
				MKEX_DMK_LAYER_NAME_CASE(PLAYER_SPELL);
				MKEX_DMK_LAYER_NAME_CASE(PLAYER_GRAZEBOX);
				MKEX_DMK_LAYER_NAME_CASE(PLAYER_ITEMBOX);
				MKEX_DMK_LAYER_NAME_CASE(ENEMY);
				MKEX_DMK_LAYER_NAME_CASE(ENEMY_BULLET);
				MKEX_DMK_LAYER_NAME_CASE(ENEMY_LASER);
				MKEX_DMK_LAYER_NAME_CASE(ITEM);
				MKEX_DMK_LAYER_NAME_CASE(BULLET_ERASER);
			}
			return "UNKNOWN";
			#undef MKEX_DMK_LAYER_NAME_CASE
		}
	}

	/// @brief Collision mask defaults.
	namespace Collision::Mask {
		/// @brief Collision mask type.
		using MaskType = Makai::Collision::C2D::LayerMask;
		
		/// @brief Player entity mask.
		constexpr MaskType	PLAYER			= 1u << (0 + 0*8);
		/// @brief Player bullet mask.
		constexpr MaskType	PLAYER_BULLET	= 1u << (1 + 0*8);
		/// @brief Player laser mask.
		constexpr MaskType	PLAYER_LASER	= 1u << (2 + 0*8);
		/// @brief Player spell mask.
		constexpr MaskType	PLAYER_SPELL	= 1u << (3 + 0*8);
		/// @brief Player grazebox mask.
		constexpr MaskType	PLAYER_GRAZEBOX	= 1u << (4 + 0*8);
		/// @brief Player itembox mask.
		constexpr MaskType	PLAYER_ITEMBOX	= 1u << (5 + 0*8);
		/// @brief Enemy entity mask.
		constexpr MaskType	ENEMY			= 1u << (0 + 1*8);
		/// @brief Enemy bullet mask.
		constexpr MaskType	ENEMY_BULLET	= 1u << (1 + 1*8);
		/// @brief Enemy laser mask.
		constexpr MaskType	ENEMY_LASER		= 1u << (2 + 1*8);
		/// @brief Item mask.
		constexpr MaskType	ITEM			= 1u << (0 + 2*8);
		/// @brief Bullet eraser mask.
		constexpr MaskType	BULLET_ERASER	= 1u << (0 + 3*8);

		/// @brief Player attack mask.
		constexpr MaskType PLAYER_ATTACK	=	PLAYER_BULLET	| PLAYER_LASER	| PLAYER_SPELL	;
		/// @brief Enemy attack mask.
		constexpr MaskType ENEMY_ATTACK		=	ENEMY_BULLET	| ENEMY_LASER					;

		/// @brief Player mask.
		constexpr MaskType PLAYER_MASK		=	PLAYER	| PLAYER_ATTACK	;
		/// @brief Enemy mask.
		constexpr MaskType ENEMY_MASK		=	ENEMY	| ENEMY_ATTACK	;

		/// @brief Objects that can interact with the player.
		constexpr MaskType PLAYER_COLLISION			= ENEMY_MASK							;
		/// @brief Objects that can interact with the enemy.
		constexpr MaskType ENEMY_COLLISION			= PLAYER_MASK							;
		/// @brief Objects that can interact with items.
		constexpr MaskType ITEM_COLLISION			= PLAYER_GRAZEBOX	| PLAYER_ITEMBOX	;
		
		/// @brief Objects that enemy attacks can interact with.
		constexpr MaskType ENEMY_ATTACK_COLLISION	= PLAYER	| PLAYER_GRAZEBOX	| PLAYER_ITEMBOX	;
		/// @brief Objects player attacks can interact with.
		constexpr MaskType PLAYER_ATTACK_COLLISION	= ENEMY												;
	}

	/// @brief Collision tag defaults.
	namespace Collision::Tag {
		/// @brief Collision mask type.
		using MaskType = Makai::Collision::C2D::LayerMask;

		/// @brief Bullet eraser tag.
		constexpr MaskType	BULLET_ERASER	= 1u << (0 + 0*8);
		/// @brief For player 1 tag.
		constexpr MaskType	FOR_PLAYER_1	= 1u << (0 + 1*8);
		/// @brief For player 2 tag.
		constexpr MaskType	FOR_PLAYER_2	= 1u << (1 + 1*8);
	}
}

#endif