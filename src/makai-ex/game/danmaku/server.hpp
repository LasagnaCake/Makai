#ifndef MAKAILIB_EX_GAME_DANMAKU_SERVER_H
#define MAKAILIB_EX_GAME_DANMAKU_SERVER_H

#include "core.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct Server {
		using DataType			= GameObject;
		using HandleType		= Handle<DataType>;
		using ObjectRefListType	= List<DataType*>;

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

		constexpr Server& release(HandleType const& object) {
			if (!contains(object)) return;
			used.removeLike(object);
			free.pushBack(object);
			return *this;
		}

	protected:
		constexpr virtual bool contains(HandleType const& object) = 0;

		ObjectRefListType free, used;
	};
}

#endif