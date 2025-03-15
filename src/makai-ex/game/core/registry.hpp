#ifndef MAKAILIB_EX_GAME_CORE_REGISTRY_H
#define MAKAILIB_EX_GAME_CORE_REGISTRY_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {

template<Type::Virtual T>
struct Registry {
	template <class TSub = T>
	struct Member;

	template <Type::Derived<T> TSub>
	struct Member<TSub> {
		constexpr void destroy() {
			if (destroying) return;
			destroying = true;
			registry.remove(this);
		}

		constexpr Member& queueDestroy() {
			registry.queue(this);
			return *this;
		}

		constexpr TSub* operator->() const	{return static_cast<TSub*>(content->operator->());	}
		constexpr TSub& operator*() const	{return static_cast<TSub&>(*content);				}

		constexpr Reference<TSub>	reference() const	{return content.template as<TSub>();			}
		template <class TNew = TSub>
		constexpr Reference<TNew>	as() const			{return content.template as<TNew>();			}
		template <class TNew = TSub>
		constexpr Reference<TNew>	morph() const		{return content.template morph<TNew>();			}
		template <class TNew = TSub>
		constexpr Reference<TNew>	reinterpret() const	{return content.template reinterpret<TNew>();	}
		template <class TNew = TSub>
		constexpr Reference<TNew>	mutate() const		{return content.template mutate<TNew>();		}

		constexpr operator<=>(Member const& other) const	{return content <=> other.content;	}
		constexpr operator==(Member const& other) const		{return content == other.content;	}

		constexpr ~Member() {destroy();}

	private:
		friend class Registry;

		bool destroying = true;

		constexpr void destroy(bool const) {Instance<Member<T>>(this).destroy();}

		template <Type::Derived<T> TClass>
		constexpr Member(Registry& registry, Unique<TClass>&& content): registry(registry), content(move(content)) {
			registry.add(this);
		}

		Registry&	registry;
		Unique<T>	content;
	};

	using FindPredicate = bool(Member<T> const&);

	using QueryResult = List<Handle<Member<T>>>;

	template<Type::Derived<T> TClass, class... Args>
	constexpr Instance<Member<TClass>> create(Args... args) {
		return new Member<TClass>(*this, new TClass(args...));
	}

	template <Type::Functional<FindPredicate> TPred>
	constexpr QueryResult find(TPred const& predicate) const {
		QueryResult res;
		for (auto const& m: members)
			if (predicate(*m))
				res.pushBack(m.asWeak());
		return res;
	}

	template <Type::Functional<FindPredicate> TPred>
	constexpr QueryResult findNot(TPred const& predicate) const {
		QueryResult res;
		for (auto const& m: members)
			if (!predicate(*m))
				res.pushBack(m.asWeak());
		return res;
	}

	constexpr QueryResult all() const {
		QueryResult res;
		for (auto const& m: members)
			res.pushBack(m.asWeak());
		return res;
	}

	constexpr Registry& destroyQueued() {
		for (auto& q: queued)
			if (q) q->destroy();
		queued.clear();
		return *this;
	}

	constexpr ~Registry() {
		for (auto& m: members)
			if (m) m->destroy(true);
		members.clear();
	}
	
private:
	template <class> friend class Member;

	constexpr void queue(Instance<Member<T>> const& member)		{queued.pushBack(member);						}
	constexpr void unqueue(Instance<Member<T>> const& member)	{queued.eraseLike(member);						}

	constexpr void add(Instance<Member<T>> const& member)		{members.pushBack(member);						}
	constexpr void remove(Instance<Member<T>> const& member)	{members.eraseLike(member); member.destroy();	}

	List<Instance<Member<T>>> members;
	List<Instance<Member<T>>> queued;
};

}
#endif