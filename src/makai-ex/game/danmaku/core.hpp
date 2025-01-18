#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

#include "layers.hpp"
#include "../core/sprite.hpp"

namespace Makai::Ex::Game::Danmaku {
	namespace C2D = Collision::C2D;

	using CollisionServer = C2D::Server;

	template<Type::Ex::Tween::Tweenable T>
	struct Property {
		T					value		= 0;
		bool				interpolate	= false;
		T					start		= 0;
		T					stop		= 0;
		float				speed		= 0;
		Math::Ease::Mode	ease		= Math::Ease::linear;
		float				factor		= 0;

		constexpr T next() {
			if (!interpolate || speed == 0)
				return value;
			factor = Math::clamp<float>(factor, 0, 1);
			if (factor == 0)		value = start;
			else if (factor < 1)	value = Math::lerp<T>(start, stop, ease(factor));
			else					value = stop;
			factor += speed;
			return value;
		}

		constexpr Property& reverse() {
			CTL::swap(start, stop);
			factor = 1 - factor;
		}
	};

	struct PauseState {
		llong	time	= -1;
		bool	enabled	= false;
	};

	struct GameArea {
		Vector2 center;
		Vector2 size;

		constexpr Vector2 topLeft() const		{return center + size * Vector2(-1, +1);	}
		constexpr Vector2 topRight() const		{return center + size * Vector2(+1, +1);	}
		constexpr Vector2 bottomLeft() const	{return center + size * Vector2(-1, -1);	}
		constexpr Vector2 bottomRight() const	{return center + size * Vector2(+1, -1);	}

		constexpr C2D::Box asArea() const		{return C2D::Box(center, size);				}
	};

	struct BoundedObjectConfig {
		GameArea&			board;
		GameArea&			playfield;
	};

	struct ColliderConfig {
		using CollisionMask = CollisionLayer::CollisionMask;
		CollisionMask const	affects		= {};
		CollisionMask const	affectedBy	= {};
		CollisionMask const	tags		= {};
	};

	struct GameObjectConfig: BoundedObjectConfig, ColliderConfig {};

	struct AGameObject {
		using PromiseType			= Makai::Co::Promise<usize, true>;
		using Collider				= CollisionServer::Collider;
		using CollisionArea			= C2D::Area;
		using CollisionDirection	= C2D::Direction;
		using CollisionMask			= GameObjectConfig::CollisionMask;

		AGameObject(GameObjectConfig const& cfg):
			board(cfg.board),
			playfield(cfg.playfield),
			colli(cfg) {
				bindCollisionHandler(*this);
				collider->data = this;
			}

		virtual ~AGameObject() {}

		PromiseType task;

		PauseState pause;

		Math::Transform2D trans;

		usize spawnTime		= 5;
		usize despawnTime	= 5;
		
		virtual AGameObject& spawn()	= 0;
		virtual AGameObject& despawn()	= 0;

		virtual void onUpdate(float) {
			if (!active) return;
			if (pause.enabled && pause.time > 0) {
				--pause.time;
				return;
			} else if (pause.enabled) {
				pause.time		= -1;
				pause.enabled	= false;
				onUnpause();
			}
			if (delay > 0) {
				--delay;
				return;
			}
			while (!delay && task)
				delay = task.next();
		}
		
		bool paused() const {
			if (pause.enabled)
				return pause.time > 0;
			return true;
		}

		virtual void onCollision(Collider const& collider, CollisionDirection const direction) = 0;

	protected:
		GameArea&	board;
		GameArea&	playfield;

		virtual void onUnpause() {}

		void resetCollisionState() {
			collider->affects		= colli.affects;
			collider->affectedBy	= colli.affectedBy;
			collider->tags			= colli.tags;
		}

		static PromiseType doNothing() {co_return 1;}

		bool active = false;

		Reference<CollisionArea> collision() const {
			return collider.as<CollisionArea>();
		}

	private:
		static void bindCollisionHandler(AGameObject& self) {
			self.collider->onCollision = [&self = self] (Collider const& collider, CollisionDirection const direction) {
				self.onCollision(collider, direction);
			};
		}

		Unique<Collider> collider = CollisionServer::createCollider();

		ColliderConfig const colli;

		usize delay = 0;
	};

	struct ISpriteContainer {
		struct SpriteSetting {
			Vector2 frame;
			Vector2 sheetSize;
		} sprite;

		virtual ISpriteContainer& setSpriteRotation(float const angle)	= 0;
		virtual float getSpriteRotation() const							= 0;
		virtual ~ISpriteContainer() {}
	};

	struct ThreePatchContainer {
		struct PatchFrame {
			Vector2 head = Vector2(0, 0);
			Vector2 body = Vector2(1, 0);
			Vector2 tail = Vector2(2, 0);
		};

		struct PatchSetting {
			PatchFrame	frame		= {};
			Vector2		size		= 1;
			bool		vertical	= false;
		} patch;
	};

	struct AttackObject {
		Property<float>	velocity;
		Property<float>	rotation;
		Property<float>	damage;
	};

	struct Circular {
		Property<Vector2> radius;
	};

	struct Long {
		Property<float>	length;
	};

	struct Glowing {
		bool glowing = false;
	};

	struct Magnetizable {
		struct MagnetSetting {
			bool			enabled		= false;
			Handle<Vector2>	target		= nullptr;
			Property<float> strength	= {};
		} magnet;
	};

	struct IToggleable {
		enum State {
			TS_UNTOGGLED,
			TS_TOGGLING,
			TS_TOGGLED,
			TS_UNTOGGLING,
		};
		usize toggleTime	= 5;
		usize untoggleTime	= 5;
		virtual IToggleable& toggle(bool const state, bool const immediately = false) = 0;
		bool isToggled() {return toggleState == State::TS_TOGGLED;}
		virtual ~IToggleable() {}
	protected:
		State toggleState = State::TS_UNTOGGLED;
	};

	struct Weighted {
		Property<Vector2> gravity;
		Property<Vector2> terminalVelocity;
	};

	struct Dope {
		bool dope = false;
	};

	struct RotatesSprite {
		bool rotateSprite = true;
	};
}

#endif