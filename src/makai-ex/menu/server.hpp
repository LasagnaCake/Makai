#ifndef MAKAILIB_EX_MENU_SERVER_H
#define MAKAILIB_EX_MENU_SERVER_H

#include <makai/makai.hpp>

/// @brief Menu system extensions.
namespace Makai::Ex::Menu {
	using EventHandler = Makai::Anima::V2::Runtime::Engine;

	constexpr usize const DEFAULT_SERVER_ID = hash("Makai::Ex::Menu::Server");

	template <usize I = DEFAULT_SERVER_ID>
	struct Server: AUpdateable, ANotifiable<I>, EventHandler {
		using CollisionHandler = Collision::C2D::CollisionServer<I>;
		using NotificationHandler = ANotifiable<I>;

		CollisionHandler	collisions;
		Input::Manager		input;

		using typename NotificationHandler::MessageHandleType;

		using
			NotificationHandler::broadcast,
			NotificationHandler::subscribeTo
		;

		struct EventMessage: NotificationHandler::IMessage {
			Server&	server;
			Data::Value value;
		};

		struct Component: ID::Identifiable<Component const, uint64>, AUpdateable, NotificationHandler, NotificationHandler::IMessage {
			using Server = Server<I>;

			using typename NotificationHandler::MessageHandleType;

			using ID::Identifiable<Component const, uint64>::id;

			using
				NotificationHandler::broadcast,
				NotificationHandler::subscribeTo
			;

			virtual ~Component() {}

			String const type;
			String const name;

			Transform3D trans;

			Component(String const& type, String const& name):
				type(type),
				name(name) {
				mouseClickBounds->onCollision = [&] (auto, auto) {
					if (updating) NotificationHandler::broadcast("mk/ex/menu/events/mouse/v1", this);
				};
				mouseHoverBounds->onCollision = [&] (auto, auto) {
					if (updating) NotificationHandler::broadcast("mk/ex/menu/events/mouse/hover/v1", this);
				};
				auto const base = "mk/ex/menu/events/component/" + toString(id()) + "/v1";
				subscribeTo(
					eventPath("/transform/position"),
					eventPath("/transform/rotation"),
					eventPath("/transform/scale")
				);
				broadcast("mk/ex/menu/events/initComponent/v1");
			}

			void onUpdate(float delta, App& app) override {}

			Collision::C2D::Area& hoverArea() const {return *mouseHoverBounds;}
			Collision::C2D::Area& clickArea() const {return *mouseClickBounds;}

			void enable()	{updating = false; mouseClickBounds->canCollide = false; mouseHoverBounds->canCollide = false;}
			void disable()	{updating = false; mouseClickBounds->canCollide = false; mouseHoverBounds->canCollide = false;}

			void onMessage(String const& signal, MessageHandleType const message) override {
				EventMessage& m = *message.template morph<As<EventMessage const>>();
				if (signal == eventPath("/transform/position"))			trans.position	= m.value.getVector();
				else if (signal == eventPath("/transform/rotation"))	trans.rotation	= m.value.getVector();
				else if (signal == eventPath("/transform/scale"))		trans.scale		= m.value.getVector();
				else if (signal == eventPath("/enable"))	enable();
				else if (signal == eventPath("/disable"))	disable();
			}

			String eventPath(String const& name) {
				return "mk/ex/menu/events/component/" + toString(id()) + name + "/v1";
			}

		private:
			Unique<typename CollisionHandler::Collider> const mouseClickBounds = CollisionHandler::createCollider(1);
			Unique<typename CollisionHandler::Collider> const mouseHoverBounds = CollisionHandler::createCollider(2);
		};

		struct Callbacks {
			using Callback = EventHandler::ExposedCall<void(uint64)>;
			Callback const onMouseDown;
			Callback const onMouseUp;
			Callback const onFocus;
			Callback const onUnfocus;
			Callback const onInit;

			Callbacks(EventHandler& events, Component& component):
				onMouseDown(events.template expose<void(uint64)>(component.type + ":" + component.name + ":onMouseDown")),
				onMouseUp(events.template expose<void(uint64)>(component.type + ":" + component.name + ":onMouseUp")),
				onFocus(events.template expose<void(uint64)>(component.type + ":" + component.name + ":onFocus")),
				onUnfocus(events.template expose<void(uint64)>(component.type + ":" + component.name + ":onUnfocus")),
				onInit(events.template expose<void(uint64)>(component.type + ":" + component.name + ":onInit"))
		 	{}
		};

		void onMessage(String const& signal, MessageHandleType const message) override {
			if (!message) return;
			auto const& component = *message.template morph<Component const>();
			Callbacks callbacks{*this, *component};
			if (signal == "mk/ex/menu/events/mouse/v1") {
				if (input.isButtonDown(Input::MouseCode::MC_LEFT))
					callbacks.onMouseDown(component.id());
				else callbacks.onMouseUp(component.id());
			} else if (signal == "mk/ex/menu/events/focus/v1")
				callbacks.onFocus(component.id());
			else if (signal == "mk/ex/menu/events/unfocus/v1")
				callbacks.onUnfocus(component.id());
			else if (signal == "mk/ex/menu/events/initComponent/v1")
				callbacks.onInit(component.id());
		}

		void onUpdate(float delta, App& app) override {
			mouse->position = input.getWindowMousePosition();
			if (
				input.isButtonJustPressed(Input::MouseCode::MC_LEFT)
			or	input.isButtonJustReleased(Input::MouseCode::MC_LEFT)
			) collisions.process();
			while (EventHandler::process()) {}
		}

		~Server() {}

		Server(Server const&)	= delete;
		Server(Server&&)		= delete;

		Server& operator=(Server const&)	= delete;
		Server& operator=(Server&&)			= delete;

		static Server& instance() {
			static Server server;
			return server;
		}

	private:
		Instance<Collision::C2D::Point> const mouse						= Instance<Collision::C2D::Point>::create();

		Unique<typename CollisionHandler::Collider> const mouseCollider	= CollisionHandler::createCollider(0, Collision::C2D::Area{mouse});

		void broadcastToComponent(u64 const id, String const& event, Data::Value value) {
			typename Component::EventMessage message{*this};
			message.value = value;
			broadcast("mk/ex/menu/component/" + toString(id) + "/" + event + "/v1", &value);
		}

		void broadcastGlobally(String const& event, Data::Value value) {
			static typename Component::EventMessage message;
			message.value = value;
			broadcast(event, &value);
		}

		Server() {
			CollisionHandler::layers[0].affects		= 0b110;
			CollisionHandler::layers[1].affectedBy	= 0b001;
			CollisionHandler::layers[2].affectedBy	= 0b001;
			subscribeTo(
				StringList::from(
					"mk/ex/menu/events/mouse/v1",
					"mk/ex/menu/events/focus/v1",
					"mk/ex/menu/events/unfocus/v1",
					"mk/ex/menu/events/initComponent/v1"
				)
			);
		}

		void onLoad() override {
			context().addNativeCall("mk/ex/menu/broadcastToComponent/v1", memberCall(*this, &AsNormal<decltype(*this)>::broadcastToComponent));
			context().addNativeCall("mk/ex/menu/broadcastGlobally/v1", memberCall(*this, &AsNormal<decltype(*this)>::broadcastGlobally));
		}
	};
}

#endif
