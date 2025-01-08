#ifndef MAKAILIB_EX_GAME_DANMAKU_SERVER_H
#define MAKAILIB_EX_GAME_DANMAKU_SERVER_H

#include "core.hpp"

namespace Makai::Ex::Game::Danmaku {
	template<class T>
	struct Server {
		using DataType			= T;
		using HandleType		= Handle<DataType>;
		using ObjectListType	= List<DataType>;
		using ObjectRefListType	= List<DataType*>;

		constexpr Server() {
		}

		constexpr Server(usize const count) {
			all.resize(count, DataType());
			free.resize(count);
			used.resize(count);
			for (auto& object: all)
				free.pushBack(&object);
		}

		constexpr Server(Server&& other)		= default;
		constexpr Server(Server const& other)	= delete;

		constexpr HandleType acquire() {
			if (free.size()) {
				HandleType object = free.popBack();
				used.pushBack(free.popBack());
				return object;
			}
			return nullptr;
		}

		constexpr Server& release(HandleType const& object) {
			if (!object || all.find(object) == -1) return;
			used.removeLike(object);
			free.pushBack(object);
			return *this;
		}

	private:
		ObjectListType		all;
		ObjectRefListType	free,	used;
	};
}

#endif