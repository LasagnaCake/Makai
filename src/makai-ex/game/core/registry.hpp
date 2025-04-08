#ifndef MAKAILIB_EX_GAME_CORE_REGISTRY_H
#define MAKAILIB_EX_GAME_CORE_REGISTRY_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {

template<class T, usize ID = 0>
struct Registry {
	struct Member;

	struct Entry {
		template <Type::Subclass<Member> TSub>
		constexpr Reference<TSub> as() const			{return member.template as<TSub>();				}
		template <Type::Subclass<Member> TSub>
		constexpr Reference<TSub> morph() const			{return member.template morph<TSub>();			}
		template <Type::Subclass<Member> TSub>
		constexpr Reference<TSub> mutate() const		{return member.template mutate<TSub>();			}
		template <Type::Subclass<Member> TSub>
		constexpr Reference<TSub> reinterpret() const	{return member.template reinterpret<TSub>();	}

		constexpr Reference<Member> content() const		{return member.reference();	}

		constexpr bool exists() const	{return member.exists();	}
		constexpr operator bool() const	{return exists();			}

	private:
		friend class Member;

		Unique<Member> member;
	};

	struct Member {
		constexpr void destroy() {
			if (destroying) return;
			destroying = true;
			self->member.unbind();
		}

		constexpr Member& queueDestroy() {
			Registry::queue(this);
			queued = true;
			return *this;
		}

		template<Type::Subclass<Member> TSub, class... Args>
		[[nodiscard]] static constexpr Instance<Entry> create(Args&&... args) {
			return (new TSub(forward<Args>(args)...))->self;
		}

		constexpr virtual ~Member() {
			Registry::remove(this);
			if (queued) Registry::unqueue(this);
		}

	protected:
		constexpr Member() {
			self->member.bind(this);
			Registry::add(this);
		}
	
	private:
		Entry* self = new Entry();

		constexpr void destroy(bool const) {
			queued = false;
			destroy();
		}

		friend class Registry;

		bool destroying = false;
		bool queued		= false;
	};

	using FindPredicate = bool(Member const&);

	using QueryResult = List<Reference<Member>>;

	using Object = Instance<Entry>;

	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult find(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (predicate(*m->content()))
				res.pushBack(m->content());
		return res;
	}

	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult findNot(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (!predicate(*m->content()))
				res.pushBack(m->content());
		return res;
	}

	constexpr static QueryResult all() {
		QueryResult res;
		for (auto const& m: members)
			res.pushBack(m->content());
		return res;
	}

	constexpr static void destroyQueued() {
		auto qc = queued;
		for (auto& q: qc)
			if (q) q->destroy(true);
		queued.clear();
	}

	template<Type::Subclass<Member> TSub, class... Args>
	[[nodiscard]] static constexpr Object create(Args&&... args) {
		return Member::template create<TSub>(forward<Args>(args)...);
	}
	
private:
	friend class Member;

	constexpr static void queue(owner<Member> const& member)	{queued.pushBack(member);	}
	constexpr static void unqueue(owner<Member> const& member)	{queued.eraseLike(member);	}

	constexpr static void add(owner<Member> const& member)		{members.pushBack(member->self);	}
	constexpr static void remove(owner<Member> const& member)	{members.eraseLike(member->self);	}

	inline static List<owner<Entry>>	members;
	inline static List<owner<Member>>	queued;
};

}
#endif