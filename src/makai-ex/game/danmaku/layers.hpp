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

	namespace Collision {
		namespace GJK = CTL::Ex::Collision::GJK;
		namespace C2D = CTL::Ex::Collision::C2D;
	}
	
	namespace Collision::Layer {
		constexpr uint16 PLAYER				= 0x0;
		constexpr uint16 PLAYER_BULLET		= 0x1;
		constexpr uint16 PLAYER_LASER		= 0x2;
		constexpr uint16 PLAYER_SPELL		= 0x3;
		constexpr uint16 PLAYER_GRAZEBOX	= 0x4;
		constexpr uint16 PLAYER_ITEMBOX		= 0x5;
		constexpr uint16 ENEMY				= 0x6;
		constexpr uint16 ENEMY_BULLET		= 0x7;
		constexpr uint16 ENEMY_LASER		= 0x8;
		constexpr uint16 ITEM				= 0x9;
		constexpr uint16 BULLET_ERASER		= 0xF;

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

	namespace Collision::Mask {
		using MaskType = Makai::Collision::C2D::LayerMask;
		
		constexpr MaskType	PLAYER			= 1u << (0 + 0*8);
		constexpr MaskType	PLAYER_BULLET	= 1u << (1 + 0*8);
		constexpr MaskType	PLAYER_LASER	= 1u << (2 + 0*8);
		constexpr MaskType	PLAYER_SPELL	= 1u << (3 + 0*8);
		constexpr MaskType	PLAYER_GRAZEBOX	= 1u << (4 + 0*8);
		constexpr MaskType	PLAYER_ITEMBOX	= 1u << (5 + 0*8);
		constexpr MaskType	ENEMY			= 1u << (0 + 1*8);
		constexpr MaskType	ENEMY_BULLET	= 1u << (1 + 1*8);
		constexpr MaskType	ENEMY_LASER		= 1u << (2 + 1*8);
		constexpr MaskType	ITEM			= 1u << (0 + 2*8);
		constexpr MaskType	BULLET_ERASER	= 1u << (0 + 3*8);

		constexpr MaskType PLAYER_ATTACK	=	PLAYER_BULLET	| PLAYER_LASER	| PLAYER_SPELL	;
		constexpr MaskType ENEMY_ATTACK		=	ENEMY_BULLET	| ENEMY_LASER					;

		constexpr MaskType PLAYER_MASK		=	PLAYER	| PLAYER_ATTACK	;
		constexpr MaskType ENEMY_MASK		=	ENEMY	| ENEMY_ATTACK	;

		constexpr MaskType PLAYER_COLLISION			= ENEMY_MASK							;
		constexpr MaskType ENEMY_COLLISION			= PLAYER_MASK							;
		constexpr MaskType ITEM_COLLISION			= PLAYER_GRAZEBOX	| PLAYER_ITEMBOX	;
		
		constexpr MaskType ENEMY_ATTACK_COLLISION	= PLAYER	| PLAYER_GRAZEBOX	| PLAYER_ITEMBOX	;
		constexpr MaskType PLAYER_ATTACK_COLLISION	= ENEMY												;
	}

	namespace Collision::Tag {
		using MaskType = Makai::Collision::C2D::LayerMask;

		constexpr MaskType	BULLET_ERASER	= 1u << (0 + 0*8);
		constexpr MaskType	FOR_PLAYER_1	= 1u << (0 + 1*8);
		constexpr MaskType	FOR_PLAYER_2	= 1u << (1 + 1*8);
	}
}

#endif