#ifndef MAKAI_GAME_SYSTEM_DATA
#define MAKAI_GAME_SYSTEM_DATA

#include "anchors.hpp"
#include "graphical.hpp"
#include "program.hpp"

#define DERIVED_CONSTRUCTOR(NAME, BASE, CODE)\
	NAME(string name = #NAME, bool uniqueEntity = true) : BASE(name, uniqueEntity) CODE\
	NAME(Entity* parent, string name = #NAME , bool uniqueEntity = true) : BASE(parent, name, uniqueEntity) CODE

#define DERIVED_CLASS(NAME, BASE)\
	inline	virtual string getClass() {return #NAME;}\
	inline	virtual string getBaseClass() {return #BASE;}\
	inline	static string getCoreClass() {return BASE::getCoreClass();}
	// NOTE: This is the WORST way this could have been done, but it works I guess

#include "gamedata/layout.hpp"

#ifndef GAME_PARALLEL_THREAD_COUNT
#define GAME_PARALLEL_THREAD_COUNT PARALLEL_THREAD_COUNT
#endif // GAME_PARALLEL_THREAD_COUNT

#ifndef GAME_PARALLEL_FOR
#define GAME_PARALLEL_FOR PRAGMA_PARALLEL_FOR(GAME_PARALLEL_THREAD_COUNT)
#endif // GAME_PARALLEL_FOR

#define GAME_CLASS(NAME, BASE)\
	inline	virtual String getClass() {return #NAME;}\
	inline	virtual String getBaseClass() {return #BASE;}\
	inline	static String getCoreClass() {return BASE::getCoreClass();}\
	NAME(String name = #NAME, bool uniqueEntity = true) : BASE(name, uniqueEntity) {onCreate();}\
	NAME(Entities::Entity* parent, String name = #NAME , bool uniqueEntity = true) : BASE(parent, name, uniqueEntity) {onCreate();}

namespace GameData {
	namespace {
		using
		Entities::AreaCircle2D,
		Entities::Entity2D,
		Entities::Entity,
		RenderData::Renderable,
		RenderData::Reference3D::Plane,
		RenderData::Reference3D::AnimatedPlane,
		Drawer::Texture2D,
		VecMath::Transform2D,
		VecMath::Transform3D,
		RenderData::Material::WorldMaterial,
		RenderData::Material::BufferMaterial,
		RenderData::Material::ObjectMaterial,
		RenderData::Material::setMaterial,
		Collision::AreaCollisionData,
		Collision::CollisionType,
		Event::Signal,
		Makai::InputManager,
		Makai::Program,
		std::string;

		using namespace RenderData::Bar;
		using namespace RenderData::Text;

		namespace Reference3D {using namespace RenderData::Reference3D;}

		namespace Material {using namespace RenderData::Material;}
	}

	typedef HashMap<String, String> ButtonNameMap;

	template <typename T>
	using TypedSignal = TypedEvent::Signal<T>;

	void addToGame(Entity* const& e, String const& gameType) {
		if (!Entities::_ROOT)
			throw Error::NonexistentValue(
				"Root wasn't created!",
				__FILE__,
				toString(__LINE__),
				"GameData::addToGame()"
			);
		Entity* gameRoot = Entities::_ROOT->getChild("Game");
		if (gameRoot == nullptr) {
			gameRoot = new Entities::Entity("Game");
			Entities::_ROOT->addChild(gameRoot);
		}
		Entity* game = gameRoot->getChild(gameType);
		if (game == nullptr) {
			game = new Entities::Entity(gameType);
			gameRoot->addChild(game);
		}
		game->addChild(e);
	}

	Entity* getGame(String const& gameType) {
		return Entities::_ROOT->getChild("Game/"+gameType);
	}

	template <typename T> using Callback = Function<void(T&)>;

	#include "gamedata/structures.hpp"
	#include "gamedata/replayer.hpp"
	#include "gamedata/program.hpp"
	#include "gamedata/dialogue.hpp"
	#include "gamedata/saving.hpp"
	#include "gamedata/animation.hpp"
	#include "gamedata/menu.hpp"

	#ifdef MAKAILIB_DANMAKU_GAME

	namespace Danmaku {
		namespace {
			using
				GameData::Material::PolarWarpEffect
			;
		}
		class BossEntity2D;
		class EnemyEntity2D;
		class PlayerEntity2D;
		class Stage;
		class DanmakuApp;
		AreaCircle2D* mainPlayer = nullptr;
		#include "gamedata/danmaku/predefs.hpp"
		#include "gamedata/danmaku/structs.hpp"
		#include "gamedata/danmaku/bullet.hpp"
		#include "gamedata/danmaku/laser.hpp"
		#include "gamedata/danmaku/player.hpp"
		#include "gamedata/danmaku/item.hpp"
		#include "gamedata/danmaku/spellcard.hpp"
		#include "gamedata/danmaku/enemy.hpp"
		#include "gamedata/danmaku/boss.hpp"
		#include "gamedata/danmaku/stage.hpp"
		#include "gamedata/danmaku/program.hpp"
		#include "gamedata/danmaku/ui.hpp"
		#include "gamedata/danmaku/dialogue.hpp"
	}

	#endif
}

#undef DERIVED_CONSTRUCTOR
#undef DERIVED_CLASS

#endif // MAKAI_GAME_SYSTEM_DATA