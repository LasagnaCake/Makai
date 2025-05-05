#ifndef MAKAILIB_EX_GAME_DANMAKU_SERVER_H
#define MAKAILIB_EX_GAME_DANMAKU_SERVER_H

#include "core.hpp"

namespace Makai::Ex::Game::Danmaku {
	/// @brief Object server abstract base.
	struct AServer {
		/// @brief Object type.
		using DataType			= AGameObject;
		/// @brief Handle-to-object type.
		using HandleType		= Reference<DataType>;
		/// @brief Object reference list type.
		using ObjectRefListType	= List<DataType*>;
		/// @brief Object query result type.
		using ObjectQueryType	= List<HandleType>;

		/// @brief Default constructor.
		constexpr AServer() {}

		/// @brief Move constructor (defaulted).
		constexpr AServer(AServer&& other)		= default;
		/// @brief Move constructor (deleted).
		constexpr AServer(AServer const& other)	= delete;

		/// @brief Tries to acquire an object.
		/// @return Reference to object, or `nullptr`.
		constexpr virtual HandleType acquire() {
			if (free.size()) {
				auto object = free.popBack();
				used.pushBack(object);
				return object;
			}
			return nullptr;
		}

		/// @brief Tries to acquire an object as a given type.
		/// @tparam T Type to get as.
		/// @return Reference to object, or `nullptr`.
		template<Type::Derived<AGameObject> T>
		constexpr Reference<T> acquire() {
			if (auto obj = acquire())
				return obj.as<T>();
			return nullptr;
		}

		/// @brief Discards all active objects, if applicable. Must be implemented.
		virtual void discardAll()	= 0;
		/// @brief Frees all active objects, if applicable. Must be implemented.
		virtual void freeAll()		= 0;
		/// @brief Despawns all active objects, if applicable. Must be implemented.
		virtual void despawnAll()	= 0;
		
		/// @brief Returns the amount of remaining free objects.
		/// @return Free object count.
		virtual usize freeCount()	{return free.size();}
		/// @brief Returns the amount of remaining active objects.
		/// @return Active object count.
		virtual usize activeCount()	{return used.size();}
		/// @brief Returns the server's object capacity. Must be implemented.
		virtual usize capacity()	= 0;

		/// @brief Returns all active objects in a given area. Must be implemented.
		virtual ObjectQueryType getInArea(C2D::IBound2D const& bound)		= 0;
		/// @brief Returns all active objects not in a given area. Must be implemented.
		virtual ObjectQueryType getNotInArea(C2D::IBound2D const& bound)	= 0;
		
		/// @brief Returns all active objects.
		/// @return All active objects.
		virtual ObjectQueryType getActive() {
			ObjectQueryType query;
			query.resize(used.size());
			for (auto& o: used)
				query.pushBack(o);
			return query;
		}

	protected:
		/// @brief Access an object as a different object type.
		/// @tparam TTo Type to convert to.
		/// @tparam TFrom Type to convert from.
		/// @param from Pointer to object to access as another object.
		/// @return Reference to object, as new type.
		template<class TTo, class TFrom>
		constexpr TTo& access(TFrom* const from) {
			return *(dynamic_cast<TTo*>(from));
		}

		/// @brief Frees up an object from use.
		/// @param object Object to free.
		/// @return Reference to self.
		constexpr virtual AServer& release(HandleType const& object) {
			if (!contains(object)) return *this;
			used.eraseLike(object);
			free.pushBack(object);
			return *this;
		}

	protected:
		/// @brief Returns whether an object is in the active objects list. Must be implemented.
		constexpr virtual bool contains(HandleType const& object) = 0;

		friend class AServerObject;

		/// @brief All free objects in the server.
		ObjectRefListType free;
		/// @brief All active objects in the server.
		ObjectRefListType used;
	};

	/// @brief Object server object abstract base.
	struct AServerObject: AGameObject {
		using AGameObject::AGameObject;

		/// @brief Destructor.
		virtual ~AServerObject() {}

		/// @brief Server object state.
		enum class State {
			SOS_FREE,
			SOS_SPAWNING,
			SOS_ACTIVE,
			SOS_DESPAWNING
		};
		
		/// @brief Server object action.
		enum class Action {
			SOA_SPAWN_BEGIN,
			SOA_SPAWN_END,
			SOA_DESPAWN_BEGIN,
			SOA_DESPAWN_END,
			SOA_BOUNCE,
			SOA_LOOP,
			SOA_UNPAUSE
		};

		/// @brief Scale property.
		Property<Vector2>	scale;

		/// @brief Color property.
		Property<Vector4>	color = {Graph::Color::WHITE};

		/// @brief Whether the object can be discarded.
		bool discardable	= true;

		/// @brief The object's lifetime. If `-1`, it never dies.
		ssize lifetime		= -1;

		/// @brief Resets all of the object's properties to their default values.
		/// @return Reference to self.
		virtual AServerObject& clear() {
			trans		= Transform2D::identity();
			color		= {Graph::Color::WHITE};
			scale		= {1};
			discardable	= true;
			pause		= {};
			spawnTime	= 5;
			despawnTime	= 10;
			cycle		= 0;
			lifetime	= -1;
			if (auto collider = collision())
				collider->canCollide = true;
			onAction.clear();
			onObjectUpdate.clear();
//			task.clear();
			resetCollisionTags();
			return *this;
		}

		/// @brief Restarts the object's transformable properties to the beginning.
		/// @return Reference to self.
		virtual AServerObject& reset() {
			color.factor	= 0;
			scale.factor	= 0;
			return *this;
		}

		/// @brief Discards the object, if applicable. Must be implemented.
		virtual AServerObject& discard(bool const immediately = false, bool const force = false)	= 0;

		/// @brief Called when a server object action is executed.
		Functor<void(AServerObject&, Action const)>	onAction;
		/// @brief Called every execution cycle the object is active and unpaused.
		Functor<void(AServerObject&, float, usize)>	onObjectUpdate;

		/// @brief Returns whether the object is currently free.
		/// @return Whether object is free.
		bool isFree() const {return objectState == State::SOS_FREE;}

		/// @brief Frees the object.
		/// @return Reference to self.
		AServerObject& free()	{return setFree(true);	}
		/// @brief Enables the object.
		/// @return Reference to self.
		AServerObject& enable()	{return setFree(false);	}

		/// @brief Sets whether the object's collider can collide.
		/// @param canCollide Whether object can collide. By default, it is `true`.
		/// @return Reference to self.
		AServerObject& setCollisionState(bool const canCollide = true) {
			if (auto collider = collision())
				collider->canCollide = canCollide;
			return *this;
		}

		/// @brief Sets the collider's tags.
		/// @param tags Tags to set.
		/// @return Reference to self.
		AServerObject& setCollisionTags(CollisionMask const& tags) {
			if (auto colli = collision())
				colli->tags = tags;
			return *this;
		}

		/// @brief Returns the collider's tags.
		/// @return Collider tags.
		CollisionMask getCollisionTags() {
			if (auto colli = collision())
				return colli->tags;
			return CollisionMask(0);
		}

		/// @brief Returns the object's current state.
		/// @return Current object state.
		State state() {return objectState;};
		
		/// @brief Executes every update cycle.
		void onUpdate(float delta) override {
			if (objectState == State::SOS_FREE) return;
			AGameObject::onUpdate(delta);
			if (!paused()) return;
			onObjectUpdate(*this, delta, cycle++);
			if (lifetime > 0 && static_cast<ssize>(cycle) >= lifetime) despawn();
		}

	protected:
		/// @brief Releases an object from a server.
		/// @param object Object to release.
		/// @param server Server to release from.
		static void release(AServer::HandleType const& object, AServer& server) {
			server.release(object);
		}

		/// @brief Gets called when the object's timed pause is finished. Does not get called when pause is stopped early.
		void onUnpause() override {
			onAction(*this, Action::SOA_UNPAUSE);
		}

		/// @brief Current object state.
		State objectState;

		/// @brief Sets the object's "free state". Must be implemented.
		virtual AServerObject& setFree(bool const state) = 0;

	private:
		/// @brief Current cycle.
		usize cycle = 0;
	};

	/// @brief Server configuration.
	struct ServerConfig {
		/// @brief Server capacity.
		usize const capacity;
	};

	/// @brief Server object configuration.
	struct ServerObjectConfig {
		/// @brief Collision mask type.
		using CollisionMask = AGameObject::CollisionMask;
		/// @brief Server associated with the object.
		AServer& server;
	};

	/// @brief Server mesh configuration.
	using ServerMeshConfig		= ReferencesSpriteMesh;
	/// @brief Server glow mesh configuration.
	using ServerGlowMeshConfig	= ReferencesGlowSpriteMesh;
}

#endif