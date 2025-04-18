#ifndef MAKAILIB_EX_GAME_DANMAKU_SERVER_H
#define MAKAILIB_EX_GAME_DANMAKU_SERVER_H

#include "core.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct AServer {
		using DataType			= AGameObject;
		using HandleType		= Reference<DataType>;
		using ObjectRefListType	= List<DataType*>;
		using ObjectQueryType	= List<HandleType>;

		constexpr AServer() {}

		constexpr AServer(AServer&& other)		= default;
		constexpr AServer(AServer const& other)	= delete;

		constexpr virtual HandleType acquire() {
			if (free.size()) {
				auto object = free.popBack();
				used.pushBack(object);
				return object;
			}
			return nullptr;
		}

		template<Type::Derived<AGameObject> T>
		constexpr Reference<T> acquire() {
			if (auto obj = acquire())
				return obj.as<T>();
			return nullptr;
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

		constexpr virtual AServer& release(HandleType const& object) {
			if (!contains(object)) return *this;
			used.eraseLike(object);
			free.pushBack(object);
			return *this;
		}

	protected:
		constexpr virtual bool contains(HandleType const& object) = 0;

		friend class AServerObject;

		ObjectRefListType free, used;
	};

	struct AServerObject: AGameObject {
		using AGameObject::AGameObject;

		virtual ~AServerObject() {}

		enum class State {
			SOS_FREE,
			SOS_SPAWNING,
			SOS_ACTIVE,
			SOS_DESPAWNING
		};
		
		enum class Action {
			SOA_SPAWN_BEGIN,
			SOA_SPAWN_END,
			SOA_DESPAWN_BEGIN,
			SOA_DESPAWN_END,
			SOA_BOUNCE,
			SOA_LOOP,
			SOA_UNPAUSE
		};

		Property<Vector2>	scale;

		Property<Vector4>	color = {Graph::Color::WHITE};

		bool discardable	= true;

		virtual AServerObject& clear() {
			trans		= Transform2D::identity();
			color		= {Graph::Color::WHITE};
			scale		= {1};
			discardable	= true;
			pause		= {};
			spawnTime	= 5;
			despawnTime	= 10;
			cycle		= 0;
			if (auto collider = collision())
				collider->canCollide = true;
			onAction.clear();
			onObjectUpdate.clear();
//			task.clear();
			resetCollisionTags();
			return *this;
		}

		virtual AServerObject& reset() {
			color.factor	= 0;
			scale.factor	= 0;
			return *this;
		}

		virtual AServerObject& discard(bool const immediately = false, bool const force = false)	= 0;

		Functor<void(AServerObject&, Action const)>	onAction;
		Functor<void(AServerObject&, float, usize)>	onObjectUpdate;

		bool isFree() const {
			return objectState == State::SOS_FREE;
		}

		AServerObject& free()	{return setFree(true);	}
		AServerObject& enable()	{return setFree(false);	}

		AServerObject& setCollisionState(bool const canCollide = true) {
			if (auto collider = collision())
				collider->canCollide = canCollide;
			return *this;
		}

		AServerObject& setCollisionTags(CollisionMask const& tags) {
			if (auto colli = collision())
				colli->tags = tags;
			return *this;
		}

		CollisionMask getCollisionTags() {
			if (auto colli = collision())
				return colli->tags;
			return CollisionMask(0);
		}

		State state() {return objectState;};

		void onUpdate(float delta) override {
			if (objectState == State::SOS_FREE) return;
			AGameObject::onUpdate(delta);
			if (!paused()) return;
			onObjectUpdate(*this, delta, cycle++);
		}

	protected:
		static void release(AServer::HandleType const& object, AServer& server) {
			server.release(object);
		}

		void onUnpause() override {
			onAction(*this, Action::SOA_UNPAUSE);
		}

		State objectState;

		virtual AServerObject& setFree(bool const state) = 0;

	private:
		usize cycle = 0;
	};

	struct ServerConfig {
		usize const size;
	};

	struct ServerObjectConfig {
		using CollisionMask = AGameObject::CollisionMask;
		AServer& server;
	};

	struct ServerMeshConfig {
		Graph::ReferenceHolder&	mainMesh;
	};

	struct ServerGlowMeshConfig {
		Graph::ReferenceHolder&	glowMesh;
	};
}

#endif