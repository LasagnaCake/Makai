#ifndef MAKAILIB_EX_GAME_DANMAKU_SERVER_H
#define MAKAILIB_EX_GAME_DANMAKU_SERVER_H

#include "core.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct Server {
		using DataType			= GameObject;
		using HandleType		= Handle<DataType>;
		using ObjectRefListType	= List<DataType*>;
		using ObjectQueryType	= List<HandleType>;

		constexpr Server() {}

		constexpr Server(Server&& other)		= default;
		constexpr Server(Server const& other)	= delete;

		constexpr virtual HandleType acquire() {
			if (free.size()) {
				HandleType object = free.popBack();
				used.pushBack(free.popBack());
				return object;
			}
			return nullptr;
		}

		template<Type::Derived<GameObject> T>
		constexpr Handle<T> acquire() {
			return acquire().polymorph<T>();
		}

		virtual void discardAll()	= 0;
		virtual void freeAll()		= 0;
		virtual void despawnAll()	= 0;
		
		virtual usize freeCount()	{return free.size();}
		virtual usize activeCount()	{return used.size();}
		virtual usize capacity()	= 0;

		virtual ObjectQueryType getInArea(C2D::IBound2D const& bound)		= 0;
		virtual ObjectQueryType getNotInArea(C2D::IBound2D const& bound)	= 0;
		
		virtual ObjectQueryType getActive() {
			ObjectQueryType query;
			query.resize(used.size());
			for (auto& o: used)
				query.pushBack(o);
			return query;
		}

	protected:
		template<class TTo, class TFrom>
		constexpr TTo& access(TFrom* const from) {
			return *(dynamic_cast<TTo*>(from));
		}

		constexpr virtual Server& release(HandleType const& object) {
			if (!contains(object)) return;
			used.removeLike(object);
			free.pushBack(object);
			return *this;
		}

	protected:
		constexpr virtual bool contains(HandleType const& object) = 0;

		friend class ServerObject;

		ObjectRefListType free, used;
	};

	struct ServerObject: GameObject {
		using GameObject::GameObject;

		virtual ~ServerObject() {}

		enum class State {
			AOS_FREE,
			AOS_SPAWNING,
			AOS_ACTIVE,
			AOS_DESPAWNING
		};
		
		enum class Action {
			AOA_SPAWN_BEGIN,
			AOA_SPAWN_END,
			AOA_DESPAWN_BEGIN,
			AOA_DESPAWN_END,
			AOA_BOUNCE,
			AOA_LOOP,
			AOA_UNPAUSE
		};

		Property<Vector2>	scale;

		Property<Vector4> color = {Graph::Color::WHITE};

		bool discardable	= true;

		virtual ServerObject& clear() {
			trans		= Transform2D();
			color		= {Graph::Color::WHITE};
			discardable	= true;
			task		= doNothing();
			pause		= {};
			if (auto collider = collision()) {
				collider->shape			= nullptr;
				collider->canCollide	= true;
			}
			onAction.clear();
			onObjectUpdate.clear();
			resetCollisionState();
		}

		virtual ServerObject& reset() {
			color.factor	= 0;
		}

		virtual ServerObject& discard(bool const force = false)	= 0;

		Functor<void(ServerObject&, Action const)>	onAction;
		Functor<void(ServerObject&, float)>			onObjectUpdate;

		virtual bool isFree() const = 0;

		ServerObject& free()	{setFree(true);		}
		ServerObject& enable()	{setFree(false);	}

		ServerObject& setCollisionState(bool const canCollide = true) {
			if (auto collider = collision())
				collider->canCollide = canCollide;
		}

		ServerObject& setCollisionMask(CollisionMask const& mask, bool const forAffectedBy = false) {
			if (!collision()) return;
			if (forAffectedBy)	collision()->affectedBy	= mask;
			else				collision()->affects	= mask;
		}
		CollisionMask getCollisionMask(bool const affectedBy = false) {
			if (!collision()) return {};
			if (affectedBy)	return collision()->affectedBy;
			else			return collision()->affects;
		}

		ServerObject& setCollisionTags(CollisionMask const& tags) {
			if (!collision()) return;
			collision()->tags = tags;
		}

		CollisionMask getCollisionTags() {
			if (!collision()) return {};
			return collision()->tags;
		}

		State state() {return objectState;};

		void onUpdate(float delta) override {
			if (objectState == State::AOS_FREE) return;
			GameObject::onUpdate(delta);
			onObjectUpdate(*this, delta);
		}

	protected:
		static void release(Server::HandleType const& object, Server& server) {
			server.release(object);
		}

		void onUnpause() override {
			onAction(*this, Action::AOA_UNPAUSE);
		}

		State objectState;

		virtual ServerObject& setFree(bool const state) = 0;
	};

	struct ServerConfig {
		usize const size;
	};

	struct ServerObjectConfig {
		using CollisionMask = GameObject::CollisionMask;
		Server& server;
	};

	struct ServerMeshConfig {
		Graph::ReferenceHolder&	mainMesh;
		Graph::ReferenceHolder&	glowMesh;
	};
}

#endif