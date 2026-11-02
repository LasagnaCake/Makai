#ifndef MAKAILIB_EX_MENU_COMPONENT_H
#define MAKAILIB_EX_MENU_COMPONENT_H

#include <makai/makai.hpp>

#include "server.hpp"

/// @brief Menu system extensions.
namespace Makai::Ex::Menu {
	template <usize I = DEFAULT_SERVER_ID>
	struct Component: Server<I>::Component {
		using Base = typename Server<I>::Component;

		Component(String const& type, String const& name): Base(type, name) {}
	};

	template <usize I = DEFAULT_SERVER_ID>
	struct VisualComponent: Component<I> {
		using Base = Component<I>;

		Graph::UTF8Label	text;
		Graph::Renderable	body;

		Transform3D trans;

		VisualComponent(String const& type, String const& name): Base("Visual" + type, name) {}

		void onUpdate(float delta, App& app) override {
			Base::onUpdate(delta, app);
			text.trans = trans;
			body.trans = trans;
		}
	};
}

#endif
