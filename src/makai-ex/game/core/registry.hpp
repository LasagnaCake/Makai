#ifndef MAKAILIB_EX_GAME_CORE_REGISTRY_H
#define MAKAILIB_EX_GAME_CORE_REGISTRY_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {

template<class T, usize ID = 0>
struct Registry {
	struct Member {
		constexpr void destroy() {
			if (destroying) return;
			destroying = true;
			Instance<Member>(this).destroy();
		}

		constexpr Member& queueDestroy() {
			Registry::queue(this);
			return *this;
		}

		template<Type::Subclass<Member> TSub, class... Args>
		[[nodiscard]] static constexpr Instance<TSub> create(Args... args) {
			return new TSub(args...);
		}

		constexpr virtual ~Member() {Registry::remove(this);}

	protected:
		constexpr Member() {
			Registry::add(this);
		}
	
	private:
		friend class Registry;

		bool destroying = true;
	};

	using FindPredicate = bool(Member const&);

	using QueryResult = List<Instance<Member>>;

	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult find(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (predicate(*m))
				res.pushBack(m);
		return res;
	}

	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult findNot(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (!predicate(*m))
				res.pushBack(m);
		return res;
	}

	constexpr static QueryResult all() {
		QueryResult res;
		for (auto const& m: members)
			res.pushBack(m);
		return res;
	}

	constexpr static void destroyQueued() {
		for (auto& q: queued)
			if (q) q->destroy();
		queued.clear();
	}

	template<Type::Subclass<Member> TSub, class... Args>
	[[nodiscard]] static constexpr Instance<TSub> create(Args... args) {
		return Member::template create<TSub>(args...);
	}
	
private:
	friend class Member;

	constexpr static void queue(owner<Member> const& member)	{queued.pushBack(member);	}
	constexpr static void unqueue(owner<Member> const& member)	{queued.eraseLike(member);	}

	constexpr static void add(owner<Member> const& member)		{members.pushBack(member);	}
	constexpr static void remove(owner<Member> const& member)	{members.eraseLike(member);	}

	inline static List<owner<Member>> members;
	inline static List<owner<Member>> queued;
};

}
#endif