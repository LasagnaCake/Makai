#ifndef MAKAILIB_EX_GAME_CORE_REGISTRY_H
#define MAKAILIB_EX_GAME_CORE_REGISTRY_H

#include <makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {

/// @brief Object registry ("Garbage Collector"-ish structure).
/// @tparam T Object type.
/// @tparam ID Unique identifier. By default, it is `0`.
template<class T, usize ID = 0>
struct Registry {
	/// @brief Registry member.
	struct Member;

	/// @brief Registry entry.
	using Entry = Unique<Member>;

	/// @brief Registry member.
	struct Member {
		constexpr void destroy() {
			if (destroying) return;
			destroying = true;
			self->unbind();
		}

		constexpr Member& queueDestroy() {
			Registry::queue(this);
			queued = true;
			return *this;
		}

		template<Type::Subclass<Member> TSub, class... Args>
		[[nodiscard]] static constexpr Instance<Entry> create(Args&&... args) {
			return (new TSub(forward<Args>(args)...))->self.raw();
		}

		constexpr virtual ~Member() {
			Registry::remove(this);
			if (queued) Registry::unqueue(this);
		}

	protected:
		constexpr Member() {
			self->bind(this);
			Registry::add(this);
		}
	
	private:
		Handle<Entry> self = new Entry();

		constexpr void destroy(bool const) {
			queued = false;
			destroy();
		}

		friend class Registry;

		bool destroying = false;
		bool queued		= false;
	};

	using FindPredicate = bool(Member const&);

	using Object		= Instance<Entry>;
	using ObjectHandle	= Handle<Entry>;

	using QueryResult = List<ObjectHandle>;

	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult find(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (*m && predicate(*m->reference()))
				res.pushBack(m);
		return res;
	}

	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult findNot(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (*m && !predicate(*m->reference()))
				res.pushBack(m);
		return res;
	}

	constexpr static QueryResult all() {
		QueryResult res;
		for (auto const& m: members)
			if (*m) res.pushBack(m);
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